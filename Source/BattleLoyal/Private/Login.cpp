// Fill out your copyright notice in the Description page of Project Settings.


#include "Login.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
// Sets default values
ALogin::ALogin()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ALogin::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALogin::OnClickLogin(FText Email, FText Password) const
{
	UE_LOG(LogTemp, Warning, TEXT("Clicked"));
}

