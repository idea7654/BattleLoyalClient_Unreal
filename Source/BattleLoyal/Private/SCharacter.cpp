// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "SWeapon.h"
#include "InGameWidget.h"
#include "SHealthComponent.h"
#include "Math/Quat.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ProgressBar.h"
#include "BPlayerController.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	RootComponent = GetCapsuleComponent();

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);
	
	ZoomedFOV = 65.0f;
	ZoomInterpSpeed = 20.0f;

	WeaponAttachSocketName = "WeaponSocket";

	isInteract = false;
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	//FVector Destination = FVector(-816.959778f, -27386.173828f, 288.368652f);
	//TeleportTo(Destination, FRotator::ZeroRotator);

	DefaultFOV = CameraComp->FieldOfView;
	//Spawn a default weapon
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);

	GameUI = CreateWidget<UInGameWidget>(GetWorld(), GameUISub);

	Socket = ClientSocket::GetSingleton();

	HealthAmount = 100.0f;
}

void ASCharacter::BeginCrouch()
{
	if (!this->GetMovementComponent()->IsFalling() && hasGun)
	{
		Crouch();
		CurrentCrouch = true;
	}
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
	CurrentCrouch = false;
}

void ASCharacter::Interact()
{
	if (isInteract)
	{
		DetectedWeapon->SetActorEnableCollision(false);
		CurrentWeapon = DetectedWeapon;
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
		hasGun = true;
		FString gunNum = DetectedWeapon->GetName();
		int32 intGun = FCString::Atoi(*gunNum);
		int32 size = 0;
		auto packet = Socket->WRITE_PU_C2S_PICKUP_GUN(size, intGun);
		Socket->WriteTo(packet, size);
	}
}

void ASCharacter::BeginZoom()
{
	bWantsToZoom = true;
}

void ASCharacter::EndZoom()
{
	bWantsToZoom = false;
}

void ASCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void ASCharacter::CheckMove()
{
	if (CurrentMFV != MFV || CurrentMRV != MRV || TurnSpeed != TurnSpeedLast || isJump == true || isCrouch != CurrentCrouch)
	{
		if (Socket && FString(Socket->Nickname.c_str()) == this->GetName())
		{
			FVector userPos = this->GetActorTransform().GetLocation();
			FRotator userDir = this->GetActorTransform().GetRotation().Rotator();
			int32 size = 0;
			uint8_t *packet = Socket->WRITE_PU_C2S_MOVE(size, userPos, userDir, CurrentMFV, CurrentMRV, TurnSpeed, isJump, CurrentCrouch);
			Socket->WriteTo(packet, size);
		}
		isJump = false;
		MFV = CurrentMFV;
		MRV = CurrentMRV;
		TurnSpeedLast = TurnSpeed;
		isCrouch = CurrentCrouch;
	}
}

void ASCharacter::JumpFunc()
{
	isJump = true;
	Jump();
}

void ASCharacter::OnHealthChanged(USHealthComponent* InHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	//packet here
	//if (Health <= 0.0f && !bDied)
	//{
	//	//Die!
	//	bDied = true;
	//	GetMovementComponent()->StopMovementImmediately();
	//	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//	DetachFromControllerPendingDestroy();

	//	SetLifeSpan(10.0f);

	//	ABPlayerController *BController = Cast<ABPlayerController>(Controller);
	//	if (BController)
	//	{
	//		BController->Characters.Remove(this);
	//	}
	//}
}

float ASCharacter::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	HealthAmount -= DamageAmount;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Debug %f"), DamageAmount));
	if (HealthAmount <= 0.0f && !bDied)
	{
		//Die!
		bDied = true;
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();

		SetLifeSpan(10.0f);

		ABPlayerController *BController = Cast<ABPlayerController>(Controller);
		if (BController)
		{
			BController->Characters.Remove(this);
		}
	}

	return DamageAmount;
}

void ASCharacter::SearchObjects()
{
	FVector EyeLocation;
	FRotator EyeRotation;
	this->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector ShotDirection = EyeRotation.Vector();
	FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	//QueryParams.bReturnPhysicalMaterial = true;

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
	{
		AActor *HitActor = Hit.GetActor();
		DetectedWeapon = Cast<ASWeapon>(HitActor);
		if (DetectedWeapon)
		{
			GameUI->ShowInteractText();
			isInteract = true;
		}
		else {
			isInteract = false;
		}
	}
}

void ASCharacter::Move(float Delta)
{
	if (MoveDirection.IsZero()) {
		return;
	}

	MoveDirection.Normalize();
	AddMovementInput(MoveDirection, 1.0f);
	MoveDirection.Set(0.0f, 0.0f, 0.0f);
}

void ASCharacter::SetUIMine()
{
	GameUI->AddToViewport(800);
}

void ASCharacter::SetHPUI()
{
	GameUI->ProgressBar_HP->SetPercent(HealthAmount / 100.0f);
}

void ASCharacter::SetDie()
{
	if (!bDied)
	{
		//Die!
		bDied = true;
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();

		SetLifeSpan(10.0f);
	}
}

void ASCharacter::SetGameOver()
{
	GameUI->ShowGameOver();
}

void ASCharacter::SetVictory()
{
	GameUI->ShowVictory();
}

void ASCharacter::SetGameInfoUI(FString action, int32 count)
{
	if (action == "kill")
		GameUI->SetKillCount(count);
	else
		GameUI->SetPersonCount(count);
}

void ASCharacter::Attack()
{
	FHitResult HitResult;
	FVector StartTrace = GetCapsuleComponent()->GetComponentLocation() + GetCapsuleComponent()->GetForwardVector() * 70.0f;
	FVector EndTrace = StartTrace + (GetCapsuleComponent()->GetForwardVector() * 130.0f);

	FCollisionObjectQueryParams ObjQueryParam;
	ObjQueryParam.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
	
	if (GetWorld()->LineTraceSingleByObjectType(HitResult, StartTrace, EndTrace, ObjQueryParam))
	{
		if (HitResult.GetActor())
		{
			UE_LOG(LogTemp, Warning, TEXT("%d"), CurrentCombo);
			//맞았을 때 패킷!
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Nothing Attacked"));
	}
}

void ASCharacter::AttackEnd()
{
}


// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;

	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

	CameraComp->SetFieldOfView(NewFOV);

	SearchObjects();

	CheckMove();
	
	OtherPlayerMove(DeltaTime);
	Move(DeltaTime);

	if (this->GetMovementComponent()->IsFalling())
		isJump = false;
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	//PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	//PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::RotatePitch);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::RotateYaw);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::JumpFunc);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASCharacter::Interact);
}

void ASCharacter::MoveForward(float Value)
{
	CurrentMFV = Value;
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	Direction.Z = 0.0f;
	Direction.Normalize();
	MoveDirection += Direction * FMath::Clamp(Value, -1.0f, 1.0f);
	//this->AddMovementInput(Direction, Value * GetWorld()->GetDeltaSeconds() * 45.0f);
}

void ASCharacter::MoveRight(float Value)
{
	CurrentMRV = Value;
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	Direction.Z = 0.0f;
	Direction.Normalize();
	MoveDirection += Direction * FMath::Clamp(Value, -1.0f, 1.0f);
	//this->AddMovementInput(Direction, Value * GetWorld()->GetDeltaSeconds() * 45.0f);
}

void ASCharacter::RotateYaw(float Value)
{
	TurnSpeed = 0.0f;
	if (Value < .0f)
	{
		TurnSpeed = -1.0f;
	}
	else if (Value > .0f)
	{
		TurnSpeed = 1.0f;
	}
	else {
		TurnSpeed = 0.0f;
	}
	//수정필요..좀 더 생각해보자
	AddControllerYawInput(Value * GetWorld()->GetDeltaSeconds() * 45.0f);
}

void ASCharacter::RotatePitch(float Value)
{
	AddControllerPitchInput(Value);
}

FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

void ASCharacter::OtherPlayerMove(float Delta)
{
	if (Socket->Nickname != "" && this->GetName() != FString(Socket->Nickname.c_str()))
	{
		for (auto &i : Socket->players)
		{
			if (FString(i->Nickname.c_str()) == this->GetName()) //이 캐릭터 클래스 이름과..소켓에서 넘어온 이름
			{
				SetActorRelativeRotation(FRotator(i->Roll, i->Yaw, i->Pitch));

				FVector DirectionX = FRotationMatrix(GetActorRotation()).GetScaledAxis(EAxis::X);
				DirectionX.Z = 0.0f;
				DirectionX.Normalize();
				MoveDirection += DirectionX * FMath::Clamp(i->VFront, -1.0f, 1.0f);
				FVector DirectionY = FRotationMatrix(GetActorRotation()).GetScaledAxis(EAxis::Y);
				DirectionY.Z = 0.0f;
				DirectionY.Normalize();
				MoveDirection += DirectionY * FMath::Clamp(i->VRight, -1.0f, 1.0f);

				if (i->isJump)
				{
					this->Jump();
					i->isJump = false;
				}

				if (i->isCrouch == true)
					this->Crouch();
				else
					this->UnCrouch();
				break;
			}
		}
	}
}