// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameWidget.h"
#include "SCharacter.h"
#include "Kismet/GameplayStatics.h"

void UInGameWidget::NativeConstruct()
{
	
}

void UInGameWidget::NativeTick(const FGeometry & MyGeometry, float InDeltaTime)
{
	
}

void UInGameWidget::ShowInteractText()
{
	UE_LOG(LogTemp, Warning, TEXT("Process!"));
}