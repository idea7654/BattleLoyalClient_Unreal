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
	class UTextBlock *Interact;

	UFUNCTION(BlueprintCallable)
	void ShowInteractText();

	void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
