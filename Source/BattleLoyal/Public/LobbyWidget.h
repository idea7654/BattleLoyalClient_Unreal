// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClientSocket.h"
#include "LobbyWidget.generated.h"
/**
 * 
 */
UCLASS()
class BATTLELOYAL_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock *Nickname;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock *MatchingText;

	UPROPERTY(meta = (BindWidget))
	class UButton *MatchingButton;

	ClientSocket *Socket;

protected:
	UFUNCTION()
	void OnClickedMatching();
};