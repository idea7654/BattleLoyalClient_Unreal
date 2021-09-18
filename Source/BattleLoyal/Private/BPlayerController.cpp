// Fill out your copyright notice in the Description page of Project Settings.


#include "BPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SCharacter.h"
#include "Engine/LevelStreaming.h"
#include "GameFramework/Actor.h"
#include "UdpProtocol.h"

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
		Socket->Begin();
		Socket->Bind();
		Socket->isStart = true;
		Socket->StartListen();
	}
}

void ABPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (GetWorld()->GetName() == "GameLevel")
	{
		EndOfPlay();
	}
}

void ABPlayerController::SetPlayers()
{
	FVector SampleVector = FVector(4527.06543f, -26278.300781f, 299.38736f);
	for (auto &playerVar : Socket->players)
	{
		//if(*FString(playerVar->Nickname.c_str()) == )
		//else
		FVector SpawnLocationPos;
		/*SpawnLocationPos.X = playerVar->X;
		SpawnLocationPos.Y = playerVar->Y;
		SpawnLocationPos.Z = playerVar->Z;*/
		SpawnLocationPos = FVector(-816.959778f, -27386.173828f, 288.368652f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		//SpawnParams.Name = FName(*FString(playerVar->Nickname.c_str()));

		//ASCharacter* SpawnCharacter = GetWorld()->SpawnActor<ASCharacter>(WhoToSpawn, SpawnLocationPos, FRotator::ZeroRotator, SpawnParams);
		//SpawnCharacter->SpawnDefaultController();
		if (playerVar->Nickname.c_str() == Socket->Nickname)
		{
			ASCharacter* SpawnCharacter = GetWorld()->SpawnActor<ASCharacter>(WhoToSpawn, SpawnLocationPos, FRotator::ZeroRotator, SpawnParams);
			SpawnCharacter->SpawnDefaultController();
			SpawnedCharacter = SpawnCharacter;
			OnPossess(SpawnCharacter);
		}
		else
		{
			ASCharacter* SpawnCharacter = GetWorld()->SpawnActor<ASCharacter>(WhoToSpawn, SampleVector, FRotator::ZeroRotator, SpawnParams);
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
		GetWorldTimerManager().SetTimer(Timer, this, &ABPlayerController::ResetSessionTime, 1.0f, true);
	}
}

void ABPlayerController::ResetSessionTime()
{
	Socket->ResetTimeSession();
}
