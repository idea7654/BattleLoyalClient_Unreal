// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ClientSocket.h"
#include "BAIController.generated.h"

/**
 * 
 */
UCLASS()
class BATTLELOYAL_API ABAIController : public AAIController
{
	GENERATED_BODY()
	
protected:
	virtual void Tick(float Delta);
	virtual void PostInitializeComponents();

	void OtherPlayerMove();

public:
	ClientSocket *Socket;
};
