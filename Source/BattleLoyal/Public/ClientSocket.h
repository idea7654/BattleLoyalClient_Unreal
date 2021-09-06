// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <windows.h>
#include <string>
#include "Runtime/Core/Public/HAL/Runnable.h"

#define	MAX_BUFFER_LENGTH	1024
#define SERVER_PORT			8000
#define SERVER_IP			"203.250.133.43"

class cCharacter {
public:
	cCharacter() {};
	~cCharacter() {};

	// 세션 아이디
	std::string Nickname;
	// 위치
	float X;
	float Y;
	float Z;
	// 회전값
	float Yaw;
	float Pitch;
	float Roll;
	// 속도
	float VX;
	float VY;
	float VZ;
	// 속성
	bool	IsAlive;
	float	HP;
};

class BATTLELOYAL_API ClientSocket : public FRunnable
{
public:
	ClientSocket();
	~ClientSocket();

	// FRunnable Thread members	
	FRunnableThread* Thread;
	FThreadSafeCounter StopTaskCounter;

	// FRunnable override 함수
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	virtual void Exit();

	// 스레드 시작 및 종료
	bool StartListen();
	void StopListen();

	// 싱글턴 객체 가져오기
	static ClientSocket* GetSingleton()
	{
		static ClientSocket ins;
		return &ins;
	}

public:
	bool Begin();
	bool Bind();
	bool RecvFrom();
	bool WriteTo(BYTE* data, DWORD dataLength);
	bool WriteTo();

private:
	char			mReadBuffer[MAX_BUFFER_LENGTH];
	char			mWriteBuffer[MAX_BUFFER_LENGTH];
	SOCKET			mSocket;
	SOCKADDR_IN		mUdpRemoteInfo;
	SOCKADDR_IN		mServerInfo;
	WSADATA			mWsaData;
};
