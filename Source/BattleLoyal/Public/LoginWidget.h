// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClientSocket.h"
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
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox *Email;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox *Password;

	UPROPERTY(meta = (BindWidget))
	class UButton *LoginButton;

	UPROPERTY(meta = (BindWidget))
	class UButton *RegisterButton;

	UFUNCTION()
	void OnClickedLogin();

	void NativeConstruct() override;
};
