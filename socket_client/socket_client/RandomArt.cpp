#include "StdAfx.h"
#include "RandomArt.h"


RandomArt::RandomArt(int _wd, int _ht)
{
	wd=_wd;
	ht=_ht;
	mandel=cvCreateImage(cvSize(wd,ht),IPL_DEPTH_8U,3); //Resultant image initialized
}
RandomArt::RandomArt()
{
	wd=600;
	ht=600;
	mandel=cvCreateImage(cvSize(wd,ht),IPL_DEPTH_8U,3); //Resultant image initialized
}
IplImage* RandomArt::getArt(Mood mood, int max_it)
{
	float p,q,x,y,i,j;
	int colour;
	int it,iz=0;
	float xtemp;
	
    double cRe, cIm;                   //real and imaginary part of the constant c, determinate shape of the Julia Set
    double newRe, newIm, oldRe, oldIm;   //real and imaginary parts of new and old
    double zoom = 1, moveX = 0, moveY = 0; //you can change these to zoom and change position
    int maxIterations = 300; //after how much iterations the function should stop
    //pick some values for the constant c, this determines the shape of the Julia St
    //cRe = -0.7;
    //cIm = 0.27015;
    cRe = -0.9+0.4*((double)rand()/((double)RAND_MAX));
    cIm = 0.2+0.2*((double)rand()/(double)RAND_MAX);
    //loop through every pixel
    for(i=0;i<wd;i++)
    for(j=0;j<ht;j++)
    {
        //calculate the initial real and imaginary part of z, based on the pixel location and zoom and position values
        newRe = 1.5 * (i-wd/2) / (0.5*zoom*wd) + moveX;
        newIm = (j-ht/2) / (0.5*zoom*ht) + moveY;
        //i will represent the number of iterations
        //start the iteration process
        for(it = 0; it< max_it; it++)
        {
            //remember value of previous iteration
            oldRe = newRe;
            oldIm = newIm;
            //the actual iteration, the real and imaginary part are calculated
            newRe = oldRe * oldRe - oldIm * oldIm + cRe;
            newIm = 2 * oldRe * oldIm + cIm;
            //if the point is outside the circle with radius 2: stop
            if((newRe * newRe + newIm * newIm) > 4) break;
        }
        //use color model conversion to get rainbow palette, make brightness black if maxIterations reached
        //*IMGDATA(mandel,(int)i,(int)j,0)=255-255*it/((float)max_it);
        //*IMGDATA(mandel,(int)i,(int)j,1)=255-255*it/((float)max_it);
        //*IMGDATA(mandel,(int)i,(int)j,2)=255*it/((float)max_it);
            if(mood==HAPPY)
            {
            *IMGDATA(mandel,(int)i,(int)j,0)=255-255*it/((float)max_it);
            *IMGDATA(mandel,(int)i,(int)j,1)=255+0*it/((float)max_it);
            *IMGDATA(mandel,(int)i,(int)j,2)=255+0*it/((float)max_it);
            }
            if(mood==SAD)
            {
            *IMGDATA(mandel,(int)i,(int)j,0)=139-139*it/((float)max_it);
            *IMGDATA(mandel,(int)i,(int)j,1)=61-61*it/((float)max_it);
            *IMGDATA(mandel,(int)i,(int)j,2)=72-72*it/((float)max_it);
            }
            if(mood==NEUTRAL)
            {
            *IMGDATA(mandel,(int)i,(int)j,0)=113+(142-113)*it/((float)max_it);
            *IMGDATA(mandel,(int)i,(int)j,1)=198+(142-198)*it/((float)max_it);
            *IMGDATA(mandel,(int)i,(int)j,2)=113+(142-113)*it/((float)max_it);
            }
            if(mood==SURPRISED)
            {
            *IMGDATA(mandel,(int)i,(int)j,0)=144+(255-144)*it/((float)max_it);
            *IMGDATA(mandel,(int)i,(int)j,1)=128+(245-128)*it/((float)max_it);
            *IMGDATA(mandel,(int)i,(int)j,2)=250-250*it/((float)max_it);
            }
            if(mood==ASTONISHED)
            {
            *IMGDATA(mandel,(int)i,(int)j,0)=0+0*it/((float)max_it);
            *IMGDATA(mandel,(int)i,(int)j,1)=238+(205-238)*it/((float)max_it);
            *IMGDATA(mandel,(int)i,(int)j,2)=238+(205-238)*it/((float)max_it);
            }
        //color = HSVtoRGB(ColorHSV(it % 256, 255, 255 * (it<max_it)));
        //draw the pixel
        //pset(i,j, color);
    }
    return mandel;
}
RandomArt::~RandomArt(void)
{
	cvReleaseImage(&mandel);
}
