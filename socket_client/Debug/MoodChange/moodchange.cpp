// moodchange.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "moodchange.h"
#include "windows.h"
#include "string.h"
#include <iostream>
#include <winsock2.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#pragma comment(lib,"ws2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

WSADATA WsaDat;

// The one and only application object

CWinApp theApp;

using namespace std;

int main(void)
{
	int nRetCode = 0;	
		
	HMODULE hModule = ::GetModuleHandle(NULL);
	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
				srand ( time(NULL) );
				for(;;)
				{	
				char buffer[10];
				char wallpaper= '0' + rand()%5 + 1;
				cout<<"Mood : ";
				cin>>buffer;
				if(strcmp(buffer,"happy")==0)
					{
						WCHAR wppath[15] = {'h','a','p','p','y','w','p',wallpaper,'.','j','p','g','\0'};
						wprintf(L"%s\n",wppath);
						SystemParametersInfo(SPI_SETDESKWALLPAPER,0,wppath,SPIF_SENDCHANGE);
						system("start wmplayer.exe \"C:\\Users\\Abhishek\\Documents\\Visual Studio 2010\\Projects\\moodchange\\moodchange\\Happy.wpl\"");
						
					}
				else if(strcmp(buffer,"sad")==0)
					{
						WCHAR wppath[15] = {'s','a','d','w','p',wallpaper,'.','j','p','g','\0'};
						SystemParametersInfo(SPI_SETDESKWALLPAPER,0,wppath,SPIF_SENDCHANGE);
						system("start wmplayer.exe \"C:\\Users\\Abhishek\\Documents\\Visual Studio 2010\\Projects\\moodchange\\moodchange\\Sad.wpl\"");
					}	
				}
		}
	}
		
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}
	
	WSACleanup();
	return 0;
}