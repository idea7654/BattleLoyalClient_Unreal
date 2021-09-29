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

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed;
	/* Default FOV set during begin play*/
	float DefaultFOV;

	void BeginZoom();
	void EndZoom();

	ASWeapon *CurrentWeapon;
	ASWeapon *DetectedWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> StarterWeaponClass;

	void StartFire();
	void StopFire();
	void CheckMove();
	void JumpFunc();

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* InHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
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

	bool isJump = false;
	bool isCrouch = false;
	bool CurrentCrouch = false;

	UFUNCTION()
	void OtherPlayerMove();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool hasGun;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;
private:
	ClientSocket	*Socket;
};
