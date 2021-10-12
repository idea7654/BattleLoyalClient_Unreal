// Fill out your copyright notice in the Description page of Project Settings.


#include "RegisterWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "ClientSocket.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"

void URegisterWidget::ToLogin()
{
}

void URegisterWidget::RegisterSuccess()
{
	NotifyUI->SetText(FText::FromString(TEXT("REGISTER SUCCESS!")));
	NotifyUI->SetRenderOpacity(1.0f);
	PlayAnimation(Notification_Anim);
}

void URegisterWidget::NativeConstruct()
{
	Super::NativeConstruct();
	LoginButton->OnClicked.AddDynamic(this, &URegisterWidget::OnClickedRegister);
	Password->SetIsPassword(true);

	Socket = ClientSocket::GetSingleton();

	NotifyUI->SetRenderOpacity(0.0f);
}

void URegisterWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (Socket->isLoginError)
	{
		RegisterError();
		Socket->isLoginError = false;
	}
	if (Socket->isRegisterSuccess)
	{
		RegisterSuccess();
		Socket->isRegisterSuccess = false;
	}
}

void URegisterWidget::OnClickedRegister()
{
	if (Socket->GetSocket())
	{
		int32 packetSize = 0;
		//std::string userEmail = //static_cast<std::string>TCHAR_TO_UTF8(*Email->GetText().ToString());
		std::string userEmail = TCHAR_TO_UTF8(*Email->GetText().ToString());
		std::string userPassword = TCHAR_TO_UTF8(*Password->GetText().ToString());
		std::string userNick = TCHAR_TO_UTF8(*Nickname->GetText().ToString());
		uint8_t* packet = Socket->WRITE_PU_C2S_REQUEST_REGISTER(userEmail, userNick, userPassword, packetSize);
		Socket->WriteTo(packet, packetSize);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%d"), Socket->GetSocket());
	}
}

void URegisterWidget::RegisterError()
{
	NotifyUI->SetText(FText::FromString(TEXT("REGISTER ERROR!")));
	NotifyUI->SetRenderOpacity(1.0f);
	PlayAnimation(Notification_Anim);
}
