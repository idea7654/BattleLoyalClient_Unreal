// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <windows.h>
#include <string>
#include "UdpProtocol.h"
#include <queue>
#include <mutex>
#include "Runtime/Core/Public/HAL/Runnable.h"

#define	MAX_BUFFER_LENGTH	1024
#define SERVER_PORT			9999
#define SERVER_IP			"203.250.133.43"

class sCharacter {
public:
	sCharacter() {};
	~sCharacter() {};

	// 세션 아이디
	std::string Nickname;
	// 위치
	float X;
	float Y;
	float Z;
	// 회전값
	float Yaw;
	float Roll;
	float Pitch;

	float VFront;
	float VRight;

	bool isJump = false;
	bool isCrouch = false;
	// 속성
	bool	IsAlive = true;
	float	HP = 100;
};

struct Gun
{
public:
	int32 id;
	float X;
	float Y;
	float Z;
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
	bool			Begin();
	bool			Bind();
	bool			RecvFrom();
	bool			WriteTo(BYTE* data, DWORD dataLength);
	void			CloseSocket();
	SOCKET			GetSocket();
	void			ResetTimeSession();

public: //For Game Thread
	bool								isLoginError = false;
	bool								isMatching = false;
	bool								isStart = false;
	std::string							Nickname;			

	std::queue<const Message*>			MessageQueue;
	std::mutex							QueueMutex;
	void								SendReliable();

	UPROPERTY(VisibleAnywhere, Category = "Spawning")
	TArray<TSharedPtr<sCharacter>>		players;

	UPROPERTY(VisibleAnywhere, Category = "Spawning")
	TArray<TSharedPtr<Gun>>		Guns;
private:
	char			mReadBuffer[MAX_BUFFER_LENGTH];
	char			mWriteBuffer[MAX_BUFFER_LENGTH];
	SOCKET			mSocket;
	SOCKADDR_IN		mServerInfo;
	WSADATA			mWsaData;
	int32			mPacketNumber = 2;

public:
	uint8_t*		WRITE_PU_C2S_REQUEST_LOGIN(std::string email, std::string password, int32 &refLength);
	uint8_t*		WRITE_PU_C2S_START_MATCHING(int32 &refLength);
	uint8_t*		WRITE_PU_C2S_CANCEL_MATCHING(int32 &refLength);
	uint8_t*		WRITE_PU_C2S_EXTEND_SESSION(int32 &refLength);
	uint8_t*		WRITE_PU_C2S_MOVE(int32 &refLength, FVector Pos, FRotator Dir, float vfront, float vright, float vyaw, bool isJump, bool isCrouch);
	uint8_t*		WRITE_PU_C2S_PICKUP_GUN(int32 &refLength, int32 gunNum);
	uint8_t*		WRITE_PU_C2S_SHOOT(int32 &refLength, std::string target, float Damage);
};

