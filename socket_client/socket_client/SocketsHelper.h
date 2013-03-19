#pragma once
#ifndef SOCKETSHELPER_H
#define SOCKETSHELPER_H
#include "stdafx.h"
#include <iostream>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#include <conio.h>
#include "Mood.h"
class IFTResultDataPacket
{
public:
	FLOAT m_pAU[6];
	UINT m_numAU;
	FLOAT m_scale;
	FLOAT m_rotationXYZ[3];
	FLOAT m_translationXYZ[3];
	bool isFailed;
	IFTResultDataPacket(): m_numAU(0), isFailed(false) {}
};
typedef IFTResultDataPacket SocketDataType;
class SocketsHelper
{
public:
	SocketsHelper(void);
	~SocketsHelper(void);
	bool getNewPacket();
	SOCKET m_Socket;
	bool isOpen;
	SocketDataType *m_pPacket;
	char *m_buffer;
};

#endif