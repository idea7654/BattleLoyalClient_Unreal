// Fill out your copyright notice in the Description page of Project Settings.


#include "BSafeZone.h"
#include "Kismet/GameplayStatics.h"
#include "BPlayerController.h"
// Sets default values
ABSafeZone::ABSafeZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABSafeZone::BeginPlay()
{
	Super::BeginPlay();

	ABPlayerController *MyController = Cast<ABPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	MyController->Func_DeleSingle.BindUFunction(this, FName("CallDeleFunc_SetRound"));
}

// Called every frame
void ABSafeZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

