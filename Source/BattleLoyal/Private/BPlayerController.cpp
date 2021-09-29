// Fill out your copyright notice in the Description page of Project Settings.


#include "BPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SCharacter.h"
#include "Engine/LevelStreaming.h"
#include "GameFramework/Actor.h"
#include "UdpProtocol.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SWeapon.h"

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
	Socket->isMatching = false;
	Socket->Nickname = "";
	Socket->Guns.Empty();
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
			SpawnCharacter->SpawnDefaultController();
			SpawnedCharacter = SpawnCharacter;
			Characters.Add(SpawnCharacter);
		}
		else
		{
			ASCharacter* SpawnCharacter = GetWorld()->SpawnActor<ASCharacter>(WhoToSpawn, SpawnLocationPos, FRotator::ZeroRotator, SpawnParams);
			SpawnCharacter->SpawnDefaultController();
			Characters.Add(SpawnCharacter);
		}
	}
	OnPossess(SpawnedCharacter);

	for (auto &gun : Socket->Guns)
	{
		FVector SpawnLocationPos;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.Name = FName(*FString::FromInt(gun->id));
		SpawnLocationPos = FVector(gun->X, gun->Y, gun->Z);

		ASWeapon *SpawnGun = GetWorld()->SpawnActor<ASWeapon>(GunSpawn, SpawnLocationPos, FRotator::ZeroRotator, SpawnParams);
		Guns.Add(SpawnGun);
	}
}

void ABPlayerController::ResetSessionTime()
{
	if (Socket->Nickname != "")
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

			for (int32 i = 0; i < Characters.Num(); i++)
			{
				if (Characters[i])
				{
					if (FString(userNick.c_str()) == Characters[i]->GetName())
					{
						Characters[i]->SetActorLocation(FVector(RecvData->pos()->x(), RecvData->pos()->y(), RecvData->pos()->z()));
						break;
					}
				}
			}

			for (int32 i = 0; i < Socket->players.Num(); i++)
			{
				if (Socket->players[i]->Nickname == userNick)
				{
					Socket->players[i]->Yaw = RecvData->dir()->z();

					Socket->players[i]->VFront = RecvData->vfront();
					Socket->players[i]->VRight = RecvData->vright();

					Socket->players[i]->isJump = RecvData->jump();
					Socket->players[i]->isCrouch = RecvData->crouch();
					break;
				}
			}
			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_PICKUP_GUN:
		{
			auto RecvData = static_cast<const S2C_PICKUP_GUN*>(message->packet());
			std::string userNick = RecvData->nickname()->c_str();
			int32 gunNum = RecvData->gunnum();
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, (TEXT("Debug %s"), FString(userNick.c_str())));
			if (userNick != Socket->Nickname)
			{
				for (auto &gun : Guns)
				{
					if (gun->GetName() == FString::FromInt(gunNum))
					{
						TargetGun = gun;
						break;
					}
				}
				//ASCharacter *Character;
				for (auto &chara : Characters)
				{
					if (chara->GetName() == FString(userNick.c_str()))
					{
						SpawnedCharacter = chara;
					}
				}
				TargetGun->SetActorEnableCollision(false);
				TargetGun->SetOwner(SpawnedCharacter);
				TargetGun->AttachToComponent(SpawnedCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SpawnedCharacter->WeaponAttachSocketName);
				SpawnedCharacter->hasGun = true;
				
			}
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

	for (int32 i = 0; i < (int32)gunLength; i++)
	{
		TSharedPtr<Gun> gun(new Gun());
		gun->X = RecvData->gundata()->Get(i)->pos()->x();
		gun->Y = RecvData->gundata()->Get(i)->pos()->y();
		gun->Z = RecvData->gundata()->Get(i)->pos()->z();

		Socket->Guns.Add(gun);
	}

	UGameplayStatics::OpenLevel(GetWorld(), TEXT("GameLevel"));
}

