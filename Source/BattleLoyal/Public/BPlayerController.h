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

DECLARE_DELEGATE(FDele_Single);
DECLARE_DELEGATE(FZone_Time);

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
	TSubclassOf<class ASWeapon> GunSpawn;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	FName LevelName = FName(TEXT("GameLevel"));

	UPROPERTY(BlueprintReadWrite, Category = "Spawning")
	ASCharacter* SpawnedCharacter;

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

	UPROPERTY()
	TArray<ASWeapon*>			Guns;

	UPROPERTY()
	class ASWeapon* TargetGun;

	UPROPERTY()
	ASCharacter *removeCharacter;

	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UUserWidget> loadingWidget;

	UFUNCTION()
	void SpawnMap();

	UFUNCTION()
	void SpawnGame();

	UFUNCTION(BlueprintCallable)
	void CancelLoading();

	class ULoadingWidget *loading;

	UPROPERTY()
	int32 PlayerCount;

	UPROPERTY()
	int32 PlayerKill;

	UPROPERTY()
	TArray<FVector> RoundVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
	FVector NowRound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
	int32 RoundNum;

	FDele_Single Func_DeleSingle;
	FZone_Time Func_ZoneTime;
private:
	ClientSocket	*Socket;
};
