#pragma once
#ifndef WAM_H
#define WAM_H
#include "Mood.h"
class WallpaperAndMusic
{
public:
	WallpaperAndMusic(void);
	void changeWallpaper(Mood md);
	void changeMusic(Mood md);
	~WallpaperAndMusic(void);
};
#endif

