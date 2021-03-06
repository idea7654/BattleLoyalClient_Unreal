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
#include "LoadingWidget.h"
#include "SWeapon.h"
#include "SRecover.h"

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
	Socket->Recovers.Empty();
}

void ABPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GetPacket();
}

void ABPlayerController::BeginPlay()
{
	Super::BeginPlay();
	Socket->Begin();
	Socket->Bind();
	Socket->StartListen();
	Socket->SetPlayerController(this);
	GetWorld()->GetTimerManager().SetTimer(Timer, this, &ABPlayerController::ResetSessionTime, 1.0f, true);
	PlayerCount = 0;
	PlayerKill = 0;
}

void ABPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	EndOfPlay();
	GetWorld()->GetTimerManager().ClearTimer(Timer);
	Func_DeleSingle.Unbind();
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
	SpawnedCharacter->SetUIMine();
	SpawnedCharacter->SetGameInfoUI("person", PlayerCount);

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

	for (auto &recover : Socket->Recovers)
	{
		FVector SpawnLocationPos;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.Name = FName(*(FString::FromInt(recover->id) + "Recover"));
		SpawnLocationPos = FVector(recover->X, recover->Y, recover->Z);

		ASRecover *SpawnRecover = GetWorld()->SpawnActor<ASRecover>(RecoverSpawn, SpawnLocationPos, FRotator::ZeroRotator, SpawnParams);
		Recovers.Add(SpawnRecover);
	}
}

void ABPlayerController::ResetSessionTime()
{
	if (Socket->Nickname != "")
		Socket->ResetTimeSession();
}

void ABPlayerController::GetPacket()
{
	//while (Socket->MessageQueue.size() > 0)
	if(Socket->MessageQueue.size() > 0)
	{
		Socket->QueueMutex.lock();
		const Message *message = Socket->MessageQueue.front();
		Socket->MessageQueue.pop();
		Socket->QueueMutex.unlock();
		//if (message == nullptr)
		//	continue;

		auto protocol = message->packet_type();

		switch (protocol)
		{
		case MESSAGE_ID::MESSAGE_ID_S2C_COMPLETE_LOGIN:
		{
			auto RecvData = static_cast<const S2C_COMPLETE_LOGIN*>(message->packet());
			Socket->Nickname = RecvData->nickname()->c_str();
			FLatentActionInfo info;
			info.CallbackTarget = this;
			info.ExecutionFunction = FName("SpawnMap");
			info.UUID = 0;
			info.Linkage = 0;

			UGameplayStatics::UnloadStreamLevel(GetWorld(), "Login", info, false);

			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_LOGIN_ERROR:
		{
			Socket->isLoginError = true;
			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_REGISTER_ERROR:
		{
			Socket->isLoginError = true;
			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_COMPLETE_REGISTER:
		{
			Socket->isRegisterSuccess = true;
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
					Socket->players[i]->Roll = RecvData->dir()->x();
					Socket->players[i]->Pitch = RecvData->dir()->y();

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
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Pickup Gun!")));

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

				for (auto &chara : Characters)
				{
					if (chara->GetName() == FString(userNick.c_str()))
					{
						SpawnedCharacter = chara;
						break;
					}
				}
				TargetGun->SetActorEnableCollision(false);
				TargetGun->SetOwner(SpawnedCharacter);
				TargetGun->AttachToComponent(SpawnedCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SpawnedCharacter->WeaponAttachSocketName);
				SpawnedCharacter->hasGun = true;
				SpawnedCharacter->EquipGun = true;
				SpawnedCharacter->CurrentWeapon = TargetGun;
			}
			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_SHOOT:
		{
			auto RecvData = static_cast<const S2C_SHOOT*>(message->packet());
			std::string userNick = RecvData->nickname()->c_str();
			std::string targetNick = RecvData->target()->c_str();
			float damage = RecvData->damage();
			for (auto &chara : Characters)
			{
				if (chara->GetName() == FString(userNick.c_str()))
				{
					chara->StartFire();
				}

				if (chara->GetName() == FString(targetNick.c_str()))
				{
					chara->HealthAmount -= damage;
					chara->SetHPUI();
				}
			}
			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_PLAYER_DIE:
		{
			auto RecvData = static_cast<const S2C_PLAYER_DIE*>(message->packet());
			std::string userNick = RecvData->nickname()->c_str();
			std::string targetNick = RecvData->target()->c_str();
			std::string type = RecvData->type()->c_str();
			int32 combo = RecvData->combo();

			PlayerCount--;

			for (auto &chara : Characters)
			{
				if (chara->GetName() == FString(userNick.c_str()))
				{
					//if(type == "SHOOT")
					//chara->StartFire();

					PlayerKill++;
					chara->SetGameInfoUI("kill", PlayerKill);
				}

				if (chara->GetName() == FString(Socket->Nickname.c_str()) && userNick != "Zone")
				{
					chara->SetGameInfoUI("person", PlayerCount);
					chara->AddSlot(FString(userNick.c_str()), FString(targetNick.c_str()));
				}

				if (chara->GetName() == FString(targetNick.c_str()))
				{
					chara->HealthAmount = 0.0f;
					chara->SetHPUI();
					chara->SetDie();
					removeCharacter = chara;
					chara->SetGameOver();
					
					if (targetNick == Socket->Nickname)
					{
						FTimerHandle WaitHandle;
						float WaitTime = 3.0f; //?????? ????????
						GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
						{
							chara->RemoveGameUI();

							FLatentActionInfo info;
							info.CallbackTarget = this;
							info.ExecutionFunction = FName("ToLobby");
							info.UUID = 0;
							info.Linkage = 0;

							UGameplayStatics::UnloadStreamLevel(GetWorld(), "GameLevel", info, true);
						}), WaitTime, false);
					}
				}

				if (userNick == "Zone")
				{
					if (chara->GetName() == FString(targetNick.c_str()))
					{
						chara->AddSlot("Zone", FString(targetNick.c_str()));
						chara->HealthAmount = 0.0f;
						chara->SetHPUI();
						chara->SetDie();
						removeCharacter = chara;
						chara->SetGameOver();
					}
				}
			}

			if (removeCharacter)
			{
				Characters.Remove(removeCharacter);
				removeCharacter = nullptr;
			}

			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_USER_VICTORY:
		{
			auto RecvData = static_cast<const S2C_USER_VICTORY*>(message->packet());
			std::string userNick = RecvData->nickname()->c_str();

			for (auto &chara : Characters)
			{
				if (chara->GetName() == FString(userNick.c_str()))
				{
					if (!chara->bVictory)
					{
						chara->SetVictory();
						FTimerHandle WaitHandle;
						float WaitTime = 5.0f; //?????? ????????
						GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
							{
								FLatentActionInfo info;
								info.CallbackTarget = this;
								info.ExecutionFunction = FName("ToLobby");
								info.UUID = 0;
								info.Linkage = 0;

								chara->RemoveGameUI();

								UGameplayStatics::UnloadStreamLevel(GetWorld(), "GameLevel", info, true);
							}), WaitTime, false);
					}
				}
			}

			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_MELEE_ATTACK:
		{
			auto RecvData = static_cast<const S2C_MELEE_ATTACK*>(message->packet());
			std::string userNick = RecvData->nickname()->c_str();
			std::string target = RecvData->target()->c_str();
			float damage = RecvData->damage();
			int32 combo = RecvData->combo();

			ASCharacter *Attacker = nullptr;

			for (auto &chara : Characters)
			{
				if (userNick != Socket->Nickname)
				{
					if (chara->GetName() == FString(userNick.c_str()))
					{
						if (combo == 0)
							chara->CurrentCombo = 2;
						else
							chara->CurrentCombo = combo - 1;

						Attacker = chara;
						//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Combo: %d"), chara->CurrentCombo));
						chara->PlayAttack(false);
					}

					if (chara->GetName() == FString(target.c_str()))
					{
						chara->HealthAmount -= damage;
						chara->SetHPUI();
					}
				}
			}

			if (userNick != Socket->Nickname)
			{
				for (auto &chara : Characters)
				{
					if (Socket->Nickname == target && chara->GetName() == FString(target.c_str()))
					{
						chara->PlayNiagaraEffect(chara->GetActorLocation(), Attacker, false);
						break;
					}
				}
			}
			
			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_EQUIP_GUN:
		{
			auto RecvData = static_cast<const S2C_EQUIP_GUN*>(message->packet());
			std::string userNick = RecvData->nickname()->c_str();
			bool state = RecvData->state();

			if (userNick != Socket->Nickname)
			{
				for (auto &chara : Characters)
				{
					if (chara->GetName() == FString(userNick.c_str()))
					{
						chara->EquipGun = !state;
						chara->Equip();
						break;
					}
				}
			}
			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_CHANGE_GUN:
		{
			auto RecvData = static_cast<const S2C_CHANGE_GUN*>(message->packet());
			std::string userNick = RecvData->nickname()->c_str();
			int32 originGun = RecvData->originid();
			int32 nowGun = RecvData->nowid();

			if (userNick != Socket->Nickname)
			{
				for (auto &chara : Characters)
				{
					if (chara->GetName() == FString(userNick.c_str()))
					{
						chara->CurrentWeapon->SetActorEnableCollision(true);
						chara->CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
						chara->CurrentWeapon->SetActorLocation(Guns[originGun]->GetActorLocation());
						Guns[nowGun]->SetActorEnableCollision(false);
						chara->CurrentWeapon = Guns[nowGun];
						chara->CurrentWeapon->SetOwner(chara);
						chara->CurrentWeapon->AttachToComponent(chara->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, chara->WeaponAttachSocketName);
						chara->hasGun = true;
						chara->EquipGun = true;
						break;
					}
				}
			}
			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_START_ROUND:
		{
			auto RecvData = static_cast<const S2C_START_ROUND*>(message->packet());
			int32 round = RecvData->round();

			FVector RoundInfo = RoundVector[round - 1];
			NowRound = RoundInfo;
			RoundNum = round;
			if (Func_DeleSingle.IsBound()) Func_DeleSingle.Execute();
			if (Func_ZoneTime.IsBound()) Func_ZoneTime.Execute();
			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_ZONE_DAMAGE:
		{
			UE_LOG(LogTemp, Warning, TEXT("Take Damage"));
			auto RecvData = static_cast<const S2C_ZONE_DAMAGE*>(message->packet());
			std::string userNick = RecvData->nickname()->c_str();

			int32 damage = RecvData->damage();
			for (auto &i : Characters)
			{
				if (i->GetName() == FString(userNick.c_str()))
				{
					i->HealthAmount -= (float)damage;
					i->SetHPUI();
					break;
				}
			}
			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_SET_USER_POSITION:
		{
			auto RecvData = static_cast<const S2C_SET_USER_POSITION*>(message->packet());
			int32 Sector = RecvData->sector();
			OtherUserSetPos(Sector);
			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_START_SIGN:
		{
			auto RecvData = static_cast<const S2C_START_SIGN*>(message->packet());
			auto userLength = RecvData->userdata()->Length();
			for (int32 i = 0; i < (int32)userLength; i++)
			{
				std::string userNick = RecvData->userdata()->Get(i)->nickname()->str();
				for (int32 j = 0; j < Characters.Num(); j++)
				{
					if (FString(userNick.c_str()) == Characters[j]->GetName())
					{
						FVector newPos = FVector(RecvData->userdata()->Get(i)->pos()->x(), RecvData->userdata()->Get(i)->pos()->y(), RecvData->userdata()->Get(i)->pos()->z());
						Characters[j]->SetActorLocation(newPos);
					}
				}
			}
			Sleep(1000);
			StartSign();
			break;
		}
		case MESSAGE_ID::MESSAGE_ID_S2C_RECOVER_HP:
		{
			auto RecvData = static_cast<const S2C_RECOVER_HP*>(message->packet());
			int32 objNum = RecvData->obj();
			std::string userNick = RecvData->nickname()->c_str();

			for (auto &i : Recovers)
			{
				if (i->GetName() == FString::FromInt(objNum) + "Recover")
				{
					i->Destroy();
				}
			}

			for (auto &i : Characters)
			{
				if (i->GetName() == FString(userNick.c_str()))
				{
					i->HealthAmount += 20;
					if (i->HealthAmount > 100)
						i->HealthAmount = 100;
					i->SetHPUI();
					break;
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
	auto RoundLength = RecvData->rounddata()->Length();
	auto RecoverLength = RecvData->recoverdata()->Length();

	PlayerCount = (int32)userLength;

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
		gun->id = i;
		gun->X = RecvData->gundata()->Get(i)->pos()->x();
		gun->Y = RecvData->gundata()->Get(i)->pos()->y();
		gun->Z = RecvData->gundata()->Get(i)->pos()->z();

		Socket->Guns.Add(gun);
	}

	for (int32 i = 0; i < (int32)RoundLength; i++)
	{
		FVector newVector = FVector(RecvData->rounddata()->Get(i)->pos()->x(), RecvData->rounddata()->Get(i)->pos()->y(), /*RecvData->rounddata()->Get(i)->pos()->z()*/ 280.0f);
		RoundVector.Add(newVector);
	}

	for (int32 i = 0; i < (int32)RecoverLength; i++)
	{
		TSharedPtr<Recover> recover(new Recover());
		recover->id = RecvData->recoverdata()->Get(i)->id();
		recover->X = RecvData->recoverdata()->Get(i)->pos()->x();
		recover->Y = RecvData->recoverdata()->Get(i)->pos()->y();
		recover->Z = RecvData->recoverdata()->Get(i)->pos()->z();

		Socket->Recovers.Add(recover);
	}

	FLatentActionInfo info;
	info.CallbackTarget = this;
	info.ExecutionFunction = FName("SpawnGame");
	info.UUID = 0;
	info.Linkage = 0;

	UGameplayStatics::UnloadStreamLevel(GetWorld(), "Lobby_Sample", info, true);
}

void ABPlayerController::SpawnMap()
{
	FLatentActionInfo info;
	UGameplayStatics::LoadStreamLevel(GetWorld(), "Lobby_Sample", true, false, info);
	ULevelStreaming* level = UGameplayStatics::GetStreamingLevel(GetWorld(), "Lobby_Sample");
	level->SetShouldBeVisible(true);
}

void ABPlayerController::SpawnGame()
{
	loading = CreateWidget<ULoadingWidget>(GetWorld(), loadingWidget);
	loading->AddToViewport(9000);

	FLatentActionInfo info;
	UGameplayStatics::LoadStreamLevel(GetWorld(), "GameLevel", true, false, info);
	ULevelStreaming* level = UGameplayStatics::GetStreamingLevel(GetWorld(), "GameLevel");
	//level->SetShouldBeVisible(true);
}

void ABPlayerController::CancelLoading()
{
	loading->RemoveFromViewport();
}

void ABPlayerController::SetMyPos(int32 Section)
{
	//????????
	int32 size = 0;
	auto packet = Socket->WRITE_PU_C2S_SET_USER_POSITION(size, Section);
	Socket->WriteTo(packet, size);
}

void ABPlayerController::ToLobby()
{
	FLatentActionInfo info;
	UGameplayStatics::LoadStreamLevel(GetWorld(), "Lobby_Sample", true, false, info);
	ULevelStreaming* level = UGameplayStatics::GetStreamingLevel(GetWorld(), "Lobby_Sample");
	Socket->players.Empty();
	Socket->Guns.Empty();
	Socket->Recovers.Empty();
	for (auto &i : Characters)
	{
		i->Destroy();
	}
	Characters.Empty();
	for (auto &i : Guns)
	{
		i->Destroy();
	}
	Guns.Empty();
	for (auto &i : Recovers)
	{
		i->Destroy();
	}
	Recovers.Empty();
	Socket->isMatching = false;
}
