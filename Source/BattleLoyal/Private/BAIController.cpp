// Fill out your copyright notice in the Description page of Project Settings.


#include "BAIController.h"
//#include "..\Public\BAIController.h"
//#include "ClientSocket.h"
//#include "Actions/PawnActionsComponent.h"
//#include "SCharacter.h"

void ABAIController::Tick(float Delta)
{
	Super::Tick(Delta);
}

void ABAIController::PostInitializeComponents()
{
	UE_LOG(LogTemp, Warning, TEXT("Hello"));
	Socket = ClientSocket::GetSingleton();
}

void ABAIController::OtherPlayerMove()
{
	/*if (Socket->Nickname != "" && this->GetName() != FString(Socket->Nickname.c_str()))
	{
		for (auto &i : Socket->players)
		{
			if (FString(i->Nickname.c_str()) == this->GetName())
			{
				SetActorRelativeRotation(FRotator(0.0f, i->Yaw, 0.0f));
				this->AddMovementInput(this->GetActorForwardVector() * i->VFront * GetWorld()->GetDeltaSeconds() * 45.0f);
				this->AddMovementInput(this->GetActorRightVector() * i->VRight * GetWorld()->GetDeltaSeconds() * 45.0f);

				break;
			}
			ASCharacter *controlledPawn = this->GetControlledPawn();
		}
	}*/
}