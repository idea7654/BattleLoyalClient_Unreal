// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ClientSocket.h"
#include "BattleLoyalGameModeBase.generated.h"


/**
 * 
 */
UCLASS()
class BATTLELOYAL_API ABattleLoyalGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

public:
	//ClientSocket * Socket;
	
};
