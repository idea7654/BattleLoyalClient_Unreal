// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientSocket.h"
#include "UdpProtocol.h"
#include "Kismet/GameplayStatics.h"
#include "LoginWidget.h"

#pragma comment(lib, "Ws2_32.lib")

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
	
	while (StopTaskCounter.GetValue() == 0 && PlayerController)
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
	CloseSocket();
	mSocket = NULL;
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
		UE_LOG(LogTemp, Warning, TEXT("WSAStartUp Error"));
		WSACleanup(); // WS2_32.DLL의 사용 끝냄
		return false;
	}

	mSocket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mSocket == INVALID_SOCKET) // 에러 발생시
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket Init Error"));
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
	QueueMutex.lock();
	MessageQueue.push(message);
	QueueMutex.unlock();

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
	//UE_LOG(LogTemp, Warning, TEXT("%d"), dataLength);
	memcpy(SendBuffer + sizeof(int32) * 2, data, dataLength);

	if (mServerInfo.sin_port != 0)
	{
		int32 returnVal = sendto(mSocket, SendBuffer, PacketLength, 0, (SOCKADDR*)&mServerInfo, sizeof(mServerInfo));
		if (returnVal < 0)
			UE_LOG(LogTemp, Warning, TEXT("%d"), WSAGetLastError());
			return false;
	}

	return true;
}

void ClientSocket::CloseSocket()
{
	closesocket(mSocket);
	WSACleanup();
}

SOCKET ClientSocket::GetSocket()
{
	return mSocket;
}

void ClientSocket::ResetTimeSession()
{
	int32 size = 0;
	uint8_t* packet = WRITE_PU_C2S_EXTEND_SESSION(size);
	WriteTo(packet, size);
}

static flatbuffers::FlatBufferBuilder builder(1024);

void ClientSocket::SendReliable()
{
	char AckByte[4];
	int32 Ack = 8888;
	memset(AckByte, 0, sizeof(AckByte));
	memcpy(AckByte, &Ack, sizeof(int32));
	int32 returnVal = sendto(mSocket, AckByte, sizeof(AckByte), 0, (SOCKADDR*)&mServerInfo, sizeof(mServerInfo));
}

void ClientSocket::SetPlayerController(ABPlayerController * playerController)
{
	if (playerController)
	{
		PlayerController = playerController;
	}
}

uint8_t* ClientSocket::WRITE_PU_C2S_REQUEST_LOGIN(std::string email, std::string password, int32 &refLength)
{
	//flatbuffers::FlatBufferBuilder builder;
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

uint8_t * ClientSocket::WRITE_PU_C2S_REQUEST_REGISTER(std::string email, std::string nickname, std::string password, int32 & refLength)
{
	auto userEmail = builder.CreateString(email);
	auto userPassword = builder.CreateString(password);
	auto userNick = builder.CreateString(nickname);
	auto makePacket = CreateC2S_REQUEST_REGISTER(builder, userEmail, userNick, userPassword);
	auto newPacket = CreateMessage(builder, MESSAGE_ID::MESSAGE_ID_C2S_REQUEST_REGISTER, makePacket.Union());

	builder.Finish(newPacket);
	refLength = builder.GetSize();

	const auto data = builder.GetBufferPointer();
	builder.Clear();

	return data;
}

uint8_t* ClientSocket::WRITE_PU_C2S_START_MATCHING(int32 &refLength)
{
	if (Nickname == "")	return NULL;
	auto userNick = builder.CreateString(Nickname);
	auto makePacket = CreateC2S_START_MATCHING(builder, userNick);
	auto newPacket = CreateMessage(builder, MESSAGE_ID::MESSAGE_ID_C2S_START_MATCHING, makePacket.Union());

	builder.Finish(newPacket);
	refLength = builder.GetSize();

	const auto data = builder.GetBufferPointer();
	builder.Clear();

	return data;
}

uint8_t* ClientSocket::WRITE_PU_C2S_CANCEL_MATCHING(int32 &refLength)
{
	if (Nickname == "")	return NULL;
	auto userNick = builder.CreateString(Nickname);
	auto makePacket = CreateC2S_CANCEL_MATCHING(builder, userNick);
	auto newPacket = CreateMessage(builder, MESSAGE_ID::MESSAGE_ID_C2S_CANCEL_MATCHING, makePacket.Union());

	builder.Finish(newPacket);
	refLength = builder.GetSize();

	const auto data = builder.GetBufferPointer();
	builder.Clear();

	return data;
}

uint8_t *ClientSocket::WRITE_PU_C2S_EXTEND_SESSION(int32 &refLength)
{
	if (Nickname == "")	return NULL;
	auto userNick = builder.CreateString(Nickname);
	auto makePacket = CreateC2S_EXTEND_SESSION(builder, userNick);
	auto newPacket = CreateMessage(builder, MESSAGE_ID::MESSAGE_ID_C2S_EXTEND_SESSION, makePacket.Union());

	builder.Finish(newPacket);
	refLength = builder.GetSize();

	const auto data = builder.GetBufferPointer();
	builder.Clear();

	return data;
}

uint8_t* ClientSocket::WRITE_PU_C2S_MOVE(int32 &refLength, FVector Pos, FRotator Dir, float vfront, float vright, float vyaw, bool isJump, bool isCrouch)
{
	if (Nickname == "") return NULL;
	auto userNick = builder.CreateString(Nickname);
	auto userPos = Vec3(Pos.X, Pos.Y, Pos.Z);
	auto userDir = Vec3(Dir.Roll, Dir.Pitch, Dir.Yaw);

	auto makePacket = CreateC2S_MOVE(builder, userNick, &userPos, &userDir, vfront, vright, vyaw, isJump, isCrouch);
	auto newPacket = CreateMessage(builder, MESSAGE_ID::MESSAGE_ID_C2S_MOVE, makePacket.Union());

	builder.Finish(newPacket);
	refLength = builder.GetSize();

	const auto data = builder.GetBufferPointer();
	builder.Clear();

	return data;
}

uint8_t * ClientSocket::WRITE_PU_C2S_PICKUP_GUN(int32 & refLength, int32 gunNum)
{
	if (Nickname == "") return NULL;

	auto userNick = builder.CreateString(Nickname);

	auto makePacket = CreateC2S_PICKUP_GUN(builder, userNick, gunNum);
	auto newPacket = CreateMessage(builder, MESSAGE_ID::MESSAGE_ID_C2S_PICKUP_GUN, makePacket.Union());

	builder.Finish(newPacket);
	refLength = builder.GetSize();

	const auto data = builder.GetBufferPointer();
	builder.Clear();

	return data;
}

uint8_t * ClientSocket::WRITE_PU_C2S_SHOOT(int32 & refLength, std::string target, float Damage)
{
	if (Nickname == "") return NULL;
	
	auto userNick = builder.CreateString(Nickname);
	auto targetActor = builder.CreateString(target);
	auto makePacket = CreateC2S_SHOOT(builder, userNick, targetActor, Damage);
	auto newPacket = CreateMessage(builder, MESSAGE_ID::MESSAGE_ID_C2S_SHOOT, makePacket.Union());

	builder.Finish(newPacket);
	refLength = builder.GetSize();

	const auto data = builder.GetBufferPointer();
	builder.Clear();

	return data;
}

uint8_t * ClientSocket::WRITE_PU_C2S_MELEE_ATTACK(int32 & refLength, std::string target, int32 combo)
{
	if (Nickname == "") return NULL;

	auto userNick = builder.CreateString(Nickname);
	auto targetActor = builder.CreateString(target);
	auto makePacket = CreateC2S_MELEE_ATTACK(builder, userNick, targetActor, combo);
	auto newPacket = CreateMessage(builder, MESSAGE_ID::MESSAGE_ID_C2S_MELEE_ATTACK, makePacket.Union());

	builder.Finish(newPacket);
	refLength = builder.GetSize();

	const auto data = builder.GetBufferPointer();
	builder.Clear();

	return data;
}
