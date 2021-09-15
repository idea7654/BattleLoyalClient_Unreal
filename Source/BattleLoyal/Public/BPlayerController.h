// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ClientSocket.h"
#include "SCharacter.h"
#include "BPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BATTLELOYAL_API ABPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ABPlayerController();

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class ACharacter> Spawns;

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void SetPlayers();
private:
	ClientSocket	*Socket;
	
};
