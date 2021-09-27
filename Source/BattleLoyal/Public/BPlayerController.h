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
	~ABPlayerController();

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class ACharacter> WhoToSpawn;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	FName LevelName = FName(TEXT("GameLevel"));

	UPROPERTY(BlueprintReadWrite, Category = "Spawning")
	ASCharacter* SpawnedCharacter;

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void MyCharacterSetup();

	UFUNCTION()
	void EndOfPlay();

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void SetPlayers();

	FTimerHandle		Timer;

	UFUNCTION()
	void				ResetSessionTime();
	void				GetPacket();
	void				GameStart(const Message *packetMessage);

	UPROPERTY()
	TArray<ASCharacter*>		Characters;

private:
	ClientSocket	*Socket;
	
};
