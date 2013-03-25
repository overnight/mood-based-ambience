#include "StdAfx.h"
#include "SocketsHelper.h"


SocketsHelper::SocketsHelper(void)
{
	isOpen = false;
	WSADATA WsaDat;
	if(WSAStartup(MAKEWORD(2,2),&WsaDat)!=0)
	{
		std::cout<<"Winsock error - Winsock initialization failed\r\n";
		WSACleanup();
		system("PAUSE");
		return;
	}

	// Create our socket

	m_Socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(m_Socket==INVALID_SOCKET)
	{
		std::cout<<"Winsock error - Socket creation Failed!\r\n";
		WSACleanup();
		system("PAUSE");
		return;
	}

	// Resolve IP address for hostname
	struct hostent *host;
	if((host=gethostbyname("localhost"))==NULL)
	{
		std::cout<<"Failed to resolve hostname.\r\n";
		WSACleanup();
		system("PAUSE");
		return;
	}

	// Setup our socket address structure
	SOCKADDR_IN SockAddr;
	SockAddr.sin_port=htons(8888);
	SockAddr.sin_family=AF_INET;
	SockAddr.sin_addr.s_addr=*((unsigned long*)host->h_addr);

	// Attempt to connect to server
	if(connect(m_Socket,(SOCKADDR*)(&SockAddr),sizeof(SockAddr))!=0)
	{
		std::cout<<"Failed to establish connection with server\r\n";
		WSACleanup();
		system("PAUSE");
		return;
	}

	// If iMode!=0, non-blocking mode is enabled.
	u_long iMode=1;
	ioctlsocket(m_Socket,FIONBIO,&iMode);
	m_pPacket = new SocketDataType;
	m_buffer = new char[sizeof(SocketDataType)];
	isOpen = true;
}

bool SocketsHelper::getNewPacket()
{
	memset(m_buffer,0,sizeof(SocketDataType));
	int inDataLength=recv(m_Socket,m_buffer,sizeof(SocketDataType),0);
	if(inDataLength < 0)
		return false;
	memcpy(m_pPacket, m_buffer, sizeof(SocketDataType));
	int nError=WSAGetLastError();
	if(nError!=WSAEWOULDBLOCK&&nError!=0)
	{
		std::cout<<"Winsock error code: "<<nError<<"\r\n";
		std::cout<<"Server disconnected!\r\n";
		// Shutdown our socket
		shutdown(m_Socket,SD_SEND);

		// Close our socket entirely
		closesocket(m_Socket);		
	}
	return true;
}

SocketsHelper::~SocketsHelper(void)
{
	closesocket(m_Socket);
	int nError = WSAGetLastError();
	if(nError)
		std::cout<< "Error in close: "<< nError << "\n";
	WSACleanup();
}
