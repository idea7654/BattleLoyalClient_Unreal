// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ClientSocket.h"
#include "SCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;
class ASWeapon;

UCLASS()
class BATTLELOYAL_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void RotateYaw(float Value);
	void RotatePitch(float Value);

	void BeginCrouch();
	void EndCrouch();

	void Interact();

	UPROPERTY(VIsibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent *CameraComp;

	UPROPERTY(VIsibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent *SpringArmComp;

	class USHealthComponent* HealthComp;

	bool bWantsToZoom;
	float PressedTime;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed;
	/* Default FOV set during begin play*/
	float DefaultFOV;

	void BeginZoom();
	void EndZoom();

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> StarterWeaponClass;

	void CheckMove();
	void JumpFunc();

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* InHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bDied;

	UFUNCTION()
	void SearchObjects();

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameUISub;

	UPROPERTY()
	class UInGameWidget *GameUI;

	bool isInteract;

	float CurrentMFV;
	float CurrentMRV;
	float MFV; //MoveFrontValue
	float MRV; //MoveRightValue

	float TurnSpeed;
	float TurnSpeedLast;

	UPROPERTY(EditAnywhere)
	bool isJump = false;
	bool isCrouch = false;
	bool CurrentCrouch = false;

	UFUNCTION()
	void OtherPlayerMove(float Delta);

	FVector MoveDirection;

	//UFUNCTION()
	void Move(float Delta);

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	UAnimMontage *EquipMontage;

	ASCharacter *TargetCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	UUserWidget *OptionWidget;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	bool isOption = false;

	UFUNCTION(BlueprintImplementableEvent)
	void Option();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool hasGun;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool EquipGun;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	void StartFire();
	void StopFire();

	ASWeapon *CurrentWeapon;
	ASWeapon *DetectedWeapon;

	UFUNCTION()
	void SetUIMine();

	UPROPERTY()
	float HealthAmount;

	UFUNCTION()
	void SetHPUI();

	UFUNCTION()
	void SetDie();

	UFUNCTION()
	void SetGameOver();

	UFUNCTION()
	void SetVictory();

	UFUNCTION()
	void SetGameInfoUI(FString action, int32 count);

	UFUNCTION()
	void Attack();

	UFUNCTION()
	void AttackEnd();

	UPROPERTY(EditAnywhere)
	class UBoxComponent *AttackCheck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentCombo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxCombo;

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAttack(bool ByControl);

	DECLARE_DELEGATE_OneParam(FPlayAttackDelegate, bool);

	UFUNCTION(BlueprintImplementableEvent)
	void AddSlot(const FString &Killer, const FString &Target);

	UFUNCTION(BlueprintCallable)
	void SetChild(UUserWidget *MyWidget);

	void Equip();

	UPROPERTY()
	int32 Bullet;

	UFUNCTION()
	void SetBullet();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player")
	UAnimMontage *HitReactMontage;

private:
	ClientSocket	*Socket;
};
