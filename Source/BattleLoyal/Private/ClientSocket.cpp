// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientSocket.h"

ClientSocket::ClientSocket() :StopTaskCounter(0)
{

}

ClientSocket::~ClientSocket()
{
	delete Thread;
	Thread = nullptr;

	closesocket(mSocket);
	WSACleanup();
}

bool ClientSocket::Init()
{
	return false;
}

uint32 ClientSocket::Run()
{
	FPlatformProcess::Sleep(0.03);
	while (StopTaskCounter.GetValue() == 0)
	{
		int nRecvLen = RecvFrom();
		if (nRecvLen > 0)
		{
			// 패킷 처리
		}
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
	memset(&mUdpRemoteInfo, 0, sizeof(mUdpRemoteInfo));

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
	return true;
}

bool ClientSocket::Bind()
{
	if (!mSocket)
		return false;

	mUdpRemoteInfo.sin_family = AF_INET;
	mUdpRemoteInfo.sin_port = htons(SERVER_PORT);
	mUdpRemoteInfo.sin_addr.s_addr = inet_addr(SERVER_IP);

	if (::bind(mSocket, (SOCKADDR*)&mUdpRemoteInfo, sizeof(mUdpRemoteInfo)) == SOCKET_ERROR)
	{
		closesocket(mSocket);
		WSACleanup();
		return false;
	}
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
	char* remoteAddress = inet_ntoa(mServerInfo.sin_addr);
	int32 remotePort = htons(mServerInfo.sin_port);

	int32 PacketLength = 0;
	::memcpy(&PacketLength, mReadBuffer, sizeof(int32));
	//UE_LOG(LogTemp, Log, TEXT("Packet Come!"));

	return true;
}

bool ClientSocket::WriteTo(BYTE* data, DWORD dataLength)
{
	if (!mSocket)
		return false;

	char SendBuffer[MAX_BUFFER_LENGTH];
	memset(SendBuffer, 0, sizeof(SendBuffer));
	DWORD PacketLength = sizeof(DWORD) * 2 + dataLength;
	//int32 PacketNumber = 1;

	memcpy(SendBuffer, &PacketLength, sizeof(int32));
	memcpy(SendBuffer + sizeof(int32), 0, sizeof(int32));
	memcpy(SendBuffer + sizeof(int32) * 2, data, dataLength);

	int32 returnVal = sendto(mSocket, mWriteBuffer, dataLength, 0, (SOCKADDR*)&mServerInfo, sizeof(mServerInfo));

	if (returnVal < 0)
		return false;

	return true;
}

bool ClientSocket::WriteTo()
{
	if (!mSocket)
		return false;

	char SendBuffer[MAX_BUFFER_LENGTH];
	memset(SendBuffer, 0, sizeof(SendBuffer));
	DWORD PacketLength = 7654;
	//int32 PacketNumber = 1;

	memcpy(SendBuffer, &PacketLength, sizeof(int32));
	memcpy(SendBuffer + sizeof(int32), 0, sizeof(int32));

	int32 returnVal = sendto(mSocket, SendBuffer, sizeof(int32) * 2, 0, (SOCKADDR*)&mServerInfo, sizeof(mServerInfo));
	
	if (returnVal < 0)
		return false;
	UE_LOG(LogTemp, Warning, TEXT("Send Packet!"));
	return true;
}
