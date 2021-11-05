// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"
#include "ClientSocket.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "BPlayerController.h"

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
		Socket->isMatching = false;
		int32 size = 0;
		uint8_t *packet = Socket->WRITE_PU_C2S_CANCEL_MATCHING(size);
		Socket->WriteTo(packet, size);
		MatchingText->SetText(FText::FromString(TEXT("Start Matching")));
	}
	else
	{
		Socket->isMatching = true;
		int32 size = 0;
		uint8_t *packet = Socket->WRITE_PU_C2S_START_MATCHING(size);
		Socket->WriteTo(packet, size);
		MatchingText->SetText(FText::FromString(TEXT("Now Matching...")));
	}

}