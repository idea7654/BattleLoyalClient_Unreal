// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingWidget.generated.h"

/**
 * 
 */
UCLASS()
class BATTLELOYAL_API ULoadingWidget : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock *LoadingText;

	UPROPERTY(meta = (BindWidget))
	class UCircularThrobber *Circular;

	UPROPERTY(meta = (BindWidget))
	class UImage *BackgroundImage;

	UPROPERTY(meta = (BindWidget))
	class UThrobber *Throbber_0;
};
