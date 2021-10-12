// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClientSocket.h"
#include "RegisterWidget.h"
#include "LoginWidget.generated.h"

/**
 * 
 */
UCLASS()
class BATTLELOYAL_API ULoginWidget : public UUserWidget
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

	UFUNCTION()
	void OnClickedLogin();

	UFUNCTION()
	void LoginError();

	UFUNCTION()
	void ToRegister();

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> WhatWidget;

	UPROPERTY()
	URegisterWidget *RegisterWidget;

	void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
