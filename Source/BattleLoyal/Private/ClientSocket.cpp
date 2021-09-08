// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientSocket.h"
#include "UdpProtocol.h"
#include "LoginWidget.h"

ClientSocket::ClientSocket() :StopTaskCounter(0)
{

}

ClientSocket::~ClientSocket()
{
	//UE_LOG(LogTemp, Warning, TEXT("Destructor"));
	delete Thread;
	Thread = nullptr;

	closesocket(mSocket);
	WSACleanup();
}

bool ClientSocket::Init()
{
	return true;
}

uint32 ClientSocket::Run()
{
	//UE_LOG(LogTemp, Warning, TEXT("Start Thread"));
	FPlatformProcess::Sleep(0.03);
	
	while (StopTaskCounter.GetValue() == 0 && isStart)
	{
		RecvFrom();
	}
	return 0;
}

void ClientSocket::Stop()
{
	StopTaskCounter.Increment();
}

void ClientSocket::Exit()
{
}

bool ClientSocket::StartListen()
{
	if (Thread != nullptr) return false;
	Thread = FRunnableThread::Create(this, TEXT("ClientSocket"), 0, TPri_BelowNormal);
	return (Thread != nullptr);
}

void ClientSocket::StopListen()
{
	Stop();
	Thread->WaitForCompletion();
	Thread->Kill();
	delete Thread;
	Thread = nullptr;
	StopTaskCounter.Reset();
}

bool ClientSocket::Begin()
{
	if (mSocket)
		return false;

	memset(&mReadBuffer, 0, sizeof(mReadBuffer));
	memset(&mWriteBuffer, 0, sizeof(mWriteBuffer));
	memset(&mServerInfo, 0, sizeof(mServerInfo));

	if (WSAStartup(0x202, &mWsaData) == SOCKET_ERROR) // WSAStartup 설정에서 문제 발생하면
	{
		WSACleanup(); // WS2_32.DLL의 사용 끝냄
	}

	mSocket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mSocket == INVALID_SOCKET) // 에러 발생시
	{
		closesocket(mSocket);
		WSACleanup();
		return false;
	}
	//UE_LOG(LogTemp, Warning, TEXT("Init Finish"));
	return true;
}

bool ClientSocket::Bind()
{
	if (!mSocket)
		return false;

	mServerInfo.sin_family = AF_INET;
	mServerInfo.sin_port = htons(SERVER_PORT);
	mServerInfo.sin_addr.s_addr = inet_addr(SERVER_IP);

	return true;
}

bool ClientSocket::RecvFrom()
{
	if (!mSocket)
		return false;
	int32 clientSize = sizeof(mServerInfo);
	int32 recvLen = 0;
	if ((recvLen = recvfrom(mSocket, mReadBuffer, sizeof(mReadBuffer) - 1, 0, (SOCKADDR*)&mServerInfo, &clientSize)) == -1)
		return false;
	//UE_LOG(LogTemp, Warning, TEXT("%d"), recvLen);
	char* remoteAddress = inet_ntoa(mServerInfo.sin_addr);
	int32 remotePort = htons(mServerInfo.sin_port);

	int32 PacketLength = 0;
	::memcpy(&PacketLength, mReadBuffer, sizeof(int32));

	int32 PacketNumber = 0;
	::memcpy(&PacketNumber, mReadBuffer + sizeof(int32), sizeof(int32));

	char packet[MAX_BUFFER_LENGTH];
	::memcpy(&packet, mReadBuffer + sizeof(int32) * 2, PacketLength);

	auto message = GetMessage(packet);
	auto protocol = message->packet_type();

	switch (protocol)
	{
	case MESSAGE_ID::MESSAGE_ID_S2C_COMPLETE_LOGIN:
	{
		//로그인 성공
		UE_LOG(LogTemp, Warning, TEXT("Complete Login"));
		break;
	}

	default:
		break;
	}

	return true;
}

bool ClientSocket::WriteTo(BYTE* data, DWORD dataLength)
{
	if (!mSocket)
		return false;
	
	char SendBuffer[MAX_BUFFER_LENGTH];
	memset(SendBuffer, 0, sizeof(SendBuffer));
	int32 PacketLength = sizeof(int32) * 2 + dataLength;
	mPacketNumber++;

	memcpy(SendBuffer, &PacketLength, sizeof(int32));
	memcpy(SendBuffer + sizeof(int32), &mPacketNumber, sizeof(int32));
	memcpy(SendBuffer + sizeof(int32) * 2, data, dataLength);

	if (mServerInfo.sin_port != 0)
	{
		int32 returnVal = sendto(mSocket, SendBuffer, dataLength, 0, (SOCKADDR*)&mServerInfo, sizeof(mServerInfo));
		UE_LOG(LogTemp, Warning, TEXT("%d"), returnVal);
		if (returnVal < 0)
			return false;
	}

	return true;
}

//bool ClientSocket::WriteTo()
//{
//	UE_LOG(LogTemp, Warning, TEXT("Init Write!"));
//
//	if (!mSocket)
//		return false;
//
//	char SendBuffer[MAX_BUFFER_LENGTH];
//	memset(SendBuffer, 0, sizeof(SendBuffer));
//	int32 PacketLength = 7654;
//	//int32 PacketNumber = 1;
//
//	memcpy(SendBuffer, &PacketLength, sizeof(int32));
//
//	int32 asdf = 0;
//	memcpy(&asdf, SendBuffer, sizeof(int32));
//
//	if (mServerInfo.sin_port != 0)
//	{
//		int32 returnVal = sendto(mSocket, SendBuffer, sizeof(int32), 0, (SOCKADDR*)&mServerInfo, sizeof(mServerInfo));
//		UE_LOG(LogTemp, Warning, TEXT("%d"), returnVal);
//		if (returnVal < 0)
//			return false;
//	}
//	
//	return true;
//}

SOCKET ClientSocket::GetSocket()
{
	return mSocket;
}

//static flatbuffers::FlatBufferBuilder builder(1024);

inline uint8_t* ClientSocket::WRITE_PU_C2S_REQUEST_LOGIN(std::string email, std::string password, int32 &refLength)
{
	flatbuffers::FlatBufferBuilder builder;
	auto userEmail = builder.CreateString(email);
	auto userPassword = builder.CreateString(password);
	auto makePacket = CreateC2S_REQUEST_LOGIN(builder, userEmail, userPassword);
	auto newPacket = CreateMessage(builder, MESSAGE_ID::MESSAGE_ID_C2S_REQUEST_LOGIN, makePacket.Union());

	builder.Finish(newPacket);
	refLength = builder.GetSize();

	const auto data = builder.GetBufferPointer();
	builder.Clear();

	return data;
}