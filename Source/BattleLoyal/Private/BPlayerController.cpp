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

void ABPlayerController::MyCharacterSetup()
{
}

void ABPlayerController::EndOfPlay()
{
	Socket->CloseSocket();
	Socket->StopListen();
	Socket->players.Empty();
	Socket->isStart = false;
	Socket->isMatching = false;
}

void ABPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GetPacket();
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
			//SpawnCharacter->SpawnDefaultController();
			SpawnedCharacter = SpawnCharacter;
			Characters.Add(SpawnCharacter);
		}
		else
		{
			ASCharacter* SpawnCharacter = GetWorld()->SpawnActor<ASCharacter>(WhoToSpawn, SpawnLocationPos, FRotator::ZeroRotator, SpawnParams);
			//SpawnCharacter->SpawnDefaultController();
			Characters.Add(SpawnCharacter);
		}
	}
	OnPossess(SpawnedCharacter);
}

void ABPlayerController::ResetSessionTime()
{
	if (Socket->players.Num() != 0)
		Socket->ResetTimeSession();
}

void ABPlayerController::GetPacket()
{
	while (Socket->MessageQueue.size() != 0)
	{
		Socket->QueueMutex.lock();
		const Message *message = Socket->MessageQueue.front();
		Socket->MessageQueue.pop();
		Socket->QueueMutex.unlock();
		auto protocol = message->packet_type();

		switch (protocol)
		{
		case MESSAGE_ID::MESSAGE_ID_S2C_COMPLETE_LOGIN:
		{
			auto RecvData = static_cast<const S2C_COMPLETE_LOGIN*>(message->packet());
			Socket->Nickname = RecvData->nickname()->c_str();
			UGameplayStatics::OpenLevel(GetWorld(), TEXT("Lobby_Sample"));
			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_LOGIN_ERROR:
		{
			Socket->isLoginError = true;
			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_GAME_START:
		{
			Socket->SendReliable();
			GameStart(message);
			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_MOVE:
		{
			auto RecvData = static_cast<const S2C_MOVE*>(message->packet());
			std::string userNick = RecvData->nick_name()->c_str();
			/*for (auto &i : Socket->players)
			{
				if (i->Nickname == userNick)
				{
					i->X = RecvData->pos()->x();
					i->Y = RecvData->pos()->y();
					i->Z = RecvData->pos()->z();
					i->Roll = RecvData->dir()->x();
					i->Pitch = RecvData->dir()->y();
					i->Yaw = RecvData->dir()->z();
				}
			}*/
			
			for (int32 i = 0; i < Characters.Num(); i++)
			{
				if (Characters[i]->GetName() == FString(userNick.c_str()))
				{
					//요거 작동합니다!
					//Characters[i]->SetActorLocation(FVector(RecvData->pos()->x(), RecvData->pos()->y(), RecvData->pos()->z()));

				}
			}
			break;
		}

		default:
			break;
		}
	}
}

void ABPlayerController::GameStart(const Message *packetMessage)
{
	auto RecvData = static_cast<const S2C_GAME_START*>(packetMessage->packet());
	auto userLength = RecvData->userdata()->Length();
	auto gunLength = RecvData->gundata()->Length();

	for (int32 i = 0; i < (int32)userLength; i++)
	{
		TSharedPtr<sCharacter> chara(new sCharacter());
		chara->X = RecvData->userdata()->Get(i)->pos()->x();
		chara->Y = RecvData->userdata()->Get(i)->pos()->y();
		chara->Z = RecvData->userdata()->Get(i)->pos()->z();
		chara->Nickname = RecvData->userdata()->Get(i)->nickname()->str();

		Socket->players.Add(chara);
	}

	UGameplayStatics::OpenLevel(GetWorld(), TEXT("GameLevel"));
}