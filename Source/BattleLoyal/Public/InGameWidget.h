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

	UPROPERTY(meta = (BindWidget))
	class UImage *GameOver;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation *GameOverAnim;

	UPROPERTY(meta = (BindWidget))
	class UImage *Victory;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation *VictoryAnim;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock *PersonText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock *KillText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock *RingTime;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox *VerticalBox_0;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock *BulletText;

	UFUNCTION()
	void ShowInteractText();

	UFUNCTION()
	void ShowGameOver();

	UFUNCTION()
	void ShowVictory();

	UFUNCTION()
	void SetPersonCount(int32 count);

	UFUNCTION()
	void SetKillCount(int32 count);

	UFUNCTION()
	void SetBulletValue(int32 bullet);

	void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

};
