// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClientSocket.h"
#include "RegisterWidget.generated.h"

/**
 * 
 */
UCLASS()
class BATTLELOYAL_API URegisterWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	ClientSocket *Socket;

public:
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox *Email;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox *Password;

	UPROPERTY(meta = (BindWidget))
	class UButton *LoginButton;

	UPROPERTY(meta = (BindWidget))
	class UButton *RegisterButton;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox *NotifyUI;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation *Notification_Anim;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock *TextBlock_3;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox *Nickname;

	UFUNCTION()
	void OnClickedRegister();

	UFUNCTION()
	void RegisterError();

	UFUNCTION()
	void ToLogin();

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> WhatWidget;

	UPROPERTY()
	URegisterWidget *LoginWidget;

	UFUNCTION()
	void RegisterSuccess();

	void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
