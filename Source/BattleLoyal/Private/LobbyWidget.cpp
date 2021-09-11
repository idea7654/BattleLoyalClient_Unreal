// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"
#include "ClientSocket.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void ULobbyWidget::NativeConstruct()
{
	Socket = ClientSocket::GetSingleton();
	MatchingButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnClickedMatching);
	FText NickLabael = FText::FromString(*FString(Socket->Nickname.c_str()));
	Nickname->SetText(NickLabael);
}

void ULobbyWidget::OnClickedMatching()
{
	if (Socket->isMatching)
	{
		//¸ÅÄªÃë¼Ò
		int32 size = 0;
		uint8_t *packet = Socket->WRITE_PU_C2S_CANCEL_MATCHING(size);
		Socket->WriteTo(packet, size);
		Socket->isMatching = false;
		MatchingText->SetText(FText::FromString(TEXT("Start Matching")));
	}
	else
	{
		int32 size = 0;
		uint8_t *packet = Socket->WRITE_PU_C2S_START_MATCHING(size);
		Socket->WriteTo(packet, size);
		Socket->isMatching = true;
		MatchingText->SetText(FText::FromString(TEXT("Now Matching...")));
	}
}