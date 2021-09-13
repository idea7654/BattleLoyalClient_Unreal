// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "ClientSocket.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/GameplayStatics.h"

void ULoginWidget::NativeConstruct()
{
	Super::NativeConstruct();
	LoginButton->OnClicked.AddDynamic(this, &ULoginWidget::OnClickedLogin);
	Password->SetIsPassword(true);

	Socket = ClientSocket::GetSingleton();
	Socket->Begin();
	Socket->Bind();
	Socket->isStart = true;
	Socket->StartListen();

	Socket->Func_Login_Error.BindUFunction(this, FName("LoginError"));

	NotifyUI->SetRenderOpacity(0.0f);
}

void ULoginWidget::OnClickedLogin()
{
	if (Socket->GetSocket())
	{
		int32 packetSize = 0;
		//std::string userEmail = //static_cast<std::string>TCHAR_TO_UTF8(*Email->GetText().ToString());
		std::string userEmail = TCHAR_TO_UTF8(*Email->GetText().ToString());
		std::string userPassword = TCHAR_TO_UTF8(*Password->GetText().ToString());
		uint8_t* packet = Socket->WRITE_PU_C2S_REQUEST_LOGIN(userEmail, userPassword, packetSize);
		Socket->WriteTo(packet, packetSize);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%d"), Socket->GetSocket());
	}
}

void ULoginWidget::LoginError()
{
	NotifyUI->SetText(FText::FromString(TEXT("LOGIN ERROR!")));
	NotifyUI->SetRenderOpacity(1.0f);
	PlayAnimation(Notification_Anim);
	UE_LOG(LogTemp, Warning, TEXT("Login Failed"));
}

void ULoginWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (Socket->isLoginError)
	{
		LoginError();
		Socket->isLoginError = false;
		
	}
	if (Socket->isLoginSuccess)
	{
		Socket->isLoginSuccess = false;
		UGameplayStatics::OpenLevel(this, TEXT("Lobby_Sample"));
	}
}