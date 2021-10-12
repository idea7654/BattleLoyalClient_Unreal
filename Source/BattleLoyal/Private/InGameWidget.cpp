// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameWidget.h"
#include "SCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/EditableTextBox.h"
#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UInGameWidget::ShowGameOver()
{
	PlayAnimation(GameOverAnim);
}

void UInGameWidget::ShowVictory()
{
	PlayAnimation(VictoryAnim);
}

void UInGameWidget::SetPersonCount(int32 count)
{
	PersonText->SetText(FText::AsNumber(count));
}

void UInGameWidget::SetKillCount(int32 count)
{
	KillText->SetText(FText::AsNumber(count));
}

void UInGameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Interact->SetRenderOpacity(0.0f);
	GameOver->SetRenderOpacity(0.0f);
	Victory->SetRenderOpacity(0.0f);
	KillText->SetText(FText::AsNumber(0));
}

void UInGameWidget::NativeTick(const FGeometry & MyGeometry, float InDeltaTime)
{
	
}

void UInGameWidget::ShowInteractText()
{
	Interact->SetRenderOpacity(1.0f);
	PlayAnimation(Notify_Interact);
}