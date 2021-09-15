// Fill out your copyright notice in the Description page of Project Settings.


#include "BPlayerController.h"
#include "UdpProtocol.h"

ABPlayerController::ABPlayerController()
{
	Socket = ClientSocket::GetSingleton();

	PrimaryActorTick.bCanEverTick = true;
}

void ABPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetPlayers();
}

void ABPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ABPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ABPlayerController::SetPlayers()
{

}