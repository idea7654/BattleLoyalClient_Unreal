// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SCharacter.h"
#include "InGameWidget.generated.h"



/**
 * 
 */
UCLASS()
class BATTLELOYAL_API UInGameWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox *Interact;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation *Notify_Interact;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar *ProgressBar_HP;

	UFUNCTION()
	void ShowInteractText();

	void NativeConstruct() override;
};
