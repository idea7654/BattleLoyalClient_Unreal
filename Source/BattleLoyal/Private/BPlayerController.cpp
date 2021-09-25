// Fill out your copyright notice in the Description page of Project Settings.


#include "BPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SCharacter.h"
#include "Engine/LevelStreaming.h"
#include "GameFramework/Actor.h"
#include "UdpProtocol.h"
#include "TimerManager.h"
#include "Engine/World.h"

ABPlayerController::ABPlayerController()
{
	Socket = ClientSocket::GetSingleton();

	PrimaryActorTick.bCanEverTick = true;
}

ABPlayerController::~ABPlayerController()
{
	//Socket->players.Empty();
}

void ABPlayerController::EndOfPlay()
{
	Socket->CloseSocket();
	Socket->StopListen();
	Socket->players.Empty();
	Socket->isStart = false;
	
}

void ABPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetNextLevel();
}

void ABPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (GetWorld()->GetName() == "Login")
	{
		bool a = Socket->Begin();
		bool b = Socket->Bind();
		Socket->isStart = true;
		bool c = Socket->StartListen();
		
	}
	GetWorld()->GetTimerManager().SetTimer(Timer, this, &ABPlayerController::ResetSessionTime, 1.0f, true);
}

void ABPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (GetWorld()->GetName() == "GameLevel")
	{
		EndOfPlay();
	}
	GetWorld()->GetTimerManager().ClearTimer(Timer);
}

void ABPlayerController::SetPlayers()
{
	for (auto &playerVar : Socket->players)
	{
		FVector SpawnLocationPos;
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.Name = FName(*FString(playerVar->Nickname.c_str()));

		SpawnLocationPos = FVector(playerVar->X, playerVar->Y, playerVar->Z);
		if (playerVar->Nickname.c_str() == Socket->Nickname)
		{
			
			ASCharacter* SpawnCharacter = GetWorld()->SpawnActor<ASCharacter>(WhoToSpawn, SpawnLocationPos, FRotator::ZeroRotator, SpawnParams);
			SpawnCharacter->SpawnDefaultController();
			SpawnedCharacter = SpawnCharacter;
			OnPossess(SpawnCharacter);
		}
		else
		{
			ASCharacter* SpawnCharacter = GetWorld()->SpawnActor<ASCharacter>(WhoToSpawn, SpawnLocationPos, FRotator::ZeroRotator, SpawnParams);
			SpawnCharacter->SpawnDefaultController();
		}
	}
}

void ABPlayerController::SetNextLevel()
{
	if (Socket->isPlayers)
	{
		Socket->isPlayers = false;
		UGameplayStatics::OpenLevel(GetWorld(), TEXT("GameLevel"));
	}

	if (Socket->isLoginSuccess)
	{
		Socket->isLoginSuccess = false;
		
		UGameplayStatics::OpenLevel(GetWorld(), TEXT("Lobby_Sample"));
	}
}

void ABPlayerController::ResetSessionTime()
{
	if (Socket->players.Num() != 0)
		Socket->ResetTimeSession();
}
