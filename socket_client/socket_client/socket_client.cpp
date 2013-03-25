// socket_test1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#include <conio.h>
#include "SocketsHelper.h"
#include "FaceClassifier.h"
#include "WallpaperAndMusic.h"
#include "RandomArt.h"
#define CALIBRATION

int main(void)
{
	SocketsHelper sh;
	FaceClassifier fc;
	Mood md;
	WallpaperAndMusic wp;
	char *moodDisplay[Mood::MAX_MOOD] = {
		"NEUTRAL",
		"HAPPY",
		"SAD",
		"ANGRY",
		"ASTONISHED",
		"SURPRISED",
		"NOT_FOUND",
	};	
	if(!sh.isOpen)
	{
		printf("Socket not Open.\n");
		system("PAUSE");
		return 1;
	}
	RandomArt ra(600,600);
#ifndef CALIBRATION
	cvNamedWindow("Fractal Art", CV_WINDOW_AUTOSIZE);
#endif
	IplImage *fractal;
	// Main loop
	for(int i=0;;i++)
	{
		if(kbhit())
			return 0;
		cvWaitKey(10);
		if(!sh.getNewPacket())
			continue;
		fc.populate(sh.m_pPacket);
		md = fc.getMood();
		if(fc.getTransition() && md!=NOT_FOUND && md!=NEUTRAL)
		{
#ifndef CALIBRATION
			printf("transition! ");
			/*wp.changeWallpaper(md);
			wp.changeMusic(md);*/
			fractal = ra.getArt(md, 60);
			cvShowImage("Fractal Art",fractal);
#endif
		}
		for (int i=0; i<6; i++)
		{
			printf("%f ", fc.m_avg[i]);
		}
		printf("%s\n", moodDisplay[(int)md]);		
	}
	
	return 0;//
}