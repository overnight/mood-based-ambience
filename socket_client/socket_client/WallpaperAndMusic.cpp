#include "stdafx.h"
#include "WallpaperAndMusic.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#pragma comment(lib,"ws2_32.lib")
#include <atlbase.h>
#define NUM_WALLPAPERS_HAPPY 10
#define NUM_WALLPAPERS_SAD 10
#define NUM_WALLPAPERS_AST 4
#define NUM_WALLPAPERS_SUR 2
#define NUM_MUSIC_HAPPY 3
#define NUM_MUSIC_SAD 3

WallpaperAndMusic::WallpaperAndMusic(void)
{
	srand ( time(NULL) );
}
void WallpaperAndMusic::changeWallpaper(Mood md)
{
	char buf[200];
	int wallpaper=0;
	USES_CONVERSION;
	WCHAR *w;
	switch(md)
	{
	case HAPPY:
		wallpaper = rand()%NUM_WALLPAPERS_HAPPY +1;
		
		sprintf(buf, "C:\\Users\\arpit\\Documents\\Visual Studio 2010\\Projects\\socket_client\\Debug\\wallpaper\\happy\\happywp%d.jpg", wallpaper);	
		w = A2W(buf);
		wprintf(L"%s\n",w);		
		SystemParametersInfo(SPI_SETDESKWALLPAPER,0,w,SPIF_SENDCHANGE);
		break;
	case SAD:
		wallpaper =rand()%NUM_WALLPAPERS_SAD+1;
		
		sprintf(buf, "C:\\Users\\arpit\\Documents\\Visual Studio 2010\\Projects\\socket_client\\Debug\\wallpaper\\sad\\sadwp%d.jpg", wallpaper);
		w = A2W(buf);
		wprintf(L"%s\n",w);	
		SystemParametersInfo(SPI_SETDESKWALLPAPER,0,w,SPIF_SENDCHANGE);
		break;
	case ASTONISHED:
		wallpaper =rand()%NUM_WALLPAPERS_AST+1;
		
		sprintf(buf, "C:\\Users\\arpit\\Documents\\Visual Studio 2010\\Projects\\socket_client\\Debug\\wallpaper\\astonished\\astwp%d.jpg", wallpaper);
		w = A2W(buf);
		wprintf(L"%s\n",w);	
		SystemParametersInfo(SPI_SETDESKWALLPAPER,0,w,SPIF_SENDCHANGE);
		break;
	case SURPRISED:
		wallpaper =rand()%NUM_WALLPAPERS_SUR+1;
		
		sprintf(buf, "C:\\Users\\arpit\\Documents\\Visual Studio 2010\\Projects\\socket_client\\Debug\\wallpaper\\surprised\\surwp%d.jpg", wallpaper);
		w = A2W(buf);
		wprintf(L"%s\n",w);	
		SystemParametersInfo(SPI_SETDESKWALLPAPER,0,w,SPIF_SENDCHANGE);
		break;
	}
}
void WallpaperAndMusic::changeMusic(Mood md)
{
	int music = 1;
	char command[200];
	switch(md)
	{
	case HAPPY:
		music = rand()%NUM_MUSIC_HAPPY + 1; 
		sprintf(command,"start wmplayer.exe -h \"C:\\Users\\arpit\\Documents\\Visual Studio 2010\\Projects\\socket_client\\Debug\\MoodChange\\happymusic%d.mp3\"",music);
		system(command);
		break;
	case SAD:
		music = rand()%NUM_MUSIC_SAD + 1;
		sprintf(command,"start wmplayer.exe -h \"C:\\Users\\arpit\\Documents\\Visual Studio 2010\\Projects\\socket_client\\Debug\\MoodChange\\sadmusic%d.mp3\" ",music);
		system(command);
		break;
	}
}
WallpaperAndMusic::~WallpaperAndMusic(void)
{
}
