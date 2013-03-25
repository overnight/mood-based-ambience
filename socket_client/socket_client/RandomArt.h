#pragma once
#ifndef RANDOM_ART_H
#define RANDOM_ART_H
#include "stdafx.h"
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "Mood.h"
#define IMGDATA(image,i,j,k) ((char*)&image->imageData[(i)*(image->widthStep) + (j)*(image->nChannels) + (k)])	//extracts pixel information
class RandomArt
{
public:
	RandomArt(void);
	RandomArt::RandomArt(int _wd, int _ht);
	IplImage* getArt(Mood mood, int max_it=60);
	~RandomArt(void);
private:
	IplImage *mandel;
	int wd;
	int ht;
};
#endif

