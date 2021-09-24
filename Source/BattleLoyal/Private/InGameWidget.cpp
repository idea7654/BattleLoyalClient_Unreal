// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameWidget.h"
#include "SCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/EditableTextBox.h"
#include "Animation/WidgetAnimation.h"

void UInGameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Interact->SetRenderOpacity(0.0f);
}

void UInGameWidget::ShowInteractText()
{
	Interact->SetRenderOpacity(1.0f);
	PlayAnimation(Notify_Interact);
}