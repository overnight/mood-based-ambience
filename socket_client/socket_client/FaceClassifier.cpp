#include "StdAfx.h"
#include "FaceClassifier.h"
#include <fstream>
#include <string>
#define UPPERLIP 0
#define JAWLOWER 1
#define STRETCHEDLIPS 2
#define BROWLOWER 3
#define LIPCORNERDEP 4
#define OUTERBROW 5
using namespace cv;

FaceClassifier::FaceClassifier(void)
{
	std::string windowName(TB_WINDOW);
	cvNamedWindow(TB_WINDOW, 1);
	//cv::namedWindow(TB_WINDOW,);
	cursor = 0;
	m_isTransition = false;
	m_mood = NOT_FOUND;
	m_prevMood = NOT_FOUND;
	FILE *f = fopen("calibration.txt",  "r");
	if(f == NULL)
	{
		printf("Calibration file not opened!\n");
		m_JawLowerDownTB = 50;
		m_JawLowerUpTB = 50;
		m_StretchedLipDownTB = 50;
		m_StretchedLipUpTB = 50;
		m_StretchedLipDownSadTB = 50;
	}
	else
	{
		fscanf(f, "%d%d%d%d%d", &m_StretchedLipUpTB, &m_StretchedLipDownTB,  &m_JawLowerUpTB, &m_JawLowerDownTB, &m_StretchedLipDownSadTB);
		fclose(f);
	}
	/*std::string lu("Lip Upper:"), ld("Lip Down:"), lds("Lip D (Sad):"), ju("Jaw Upper:"), jd("Jaw Down:");
	cv::createTrackbar(lu, windowName, &m_StretchedLipUpTB, 100, NULL);
	cv::createTrackbar(ld, windowName, &m_StretchedLipDownTB, 100, NULL);
	cv::createTrackbar(lds, windowName, &m_StretchedLipDownSadTB, 100, NULL);
	cv::createTrackbar(ju, windowName, &m_JawLowerUpTB, 100, NULL);
	cv::createTrackbar(jd, windowName, &m_JawLowerDownTB, 100, NULL);*/
	cvResizeWindow(TB_WINDOW, 400, 400);
	cvCreateTrackbar("Lip Upper:", TB_WINDOW, &m_StretchedLipUpTB, 100, NULL);
	cvCreateTrackbar("Lip Down:", TB_WINDOW, &m_StretchedLipDownTB, 100, NULL);
	cvCreateTrackbar("Lip D (Sad):", TB_WINDOW, &m_StretchedLipDownSadTB, 100, NULL);
	cvCreateTrackbar("Jaw Upper:", TB_WINDOW, &m_JawLowerUpTB, 100, NULL);
	cvCreateTrackbar("Jaw Down:", TB_WINDOW, &m_JawLowerDownTB, 100, NULL);
	
}
void FaceClassifier::populate(SocketDataType *pPacket)
{
	if(pPacket->m_numAU != 6)
		return;
	/*if(pPacket->isFailed)
		return;*/
	cursor = (cursor+1)%5;
	memcpy(&m_data[cursor], pPacket, sizeof(SocketDataType));
}
void FaceClassifier::calcMood()
{
	Sleep(5);
	for (int i=0; i<6; i++) // WARNING: Assumed numAu = 6!
	{
		m_avg[i] = 0;
		for (int j=0; j<NUM_HISTORY; j++)
		{
			m_avg[i] += m_data[j].m_pAU[i];
		}
		m_avg[i] /= NUM_HISTORY;
	}
	//if(m_mood != NEUTRAL && m_mood != NOT_FOUND)
	m_prevMood =  m_mood;
	/*for (int i=0; i<6; i++)
	{
		m_avg[i] = m_data[cursor].m_pAU[i];
	}*/
	//NOT_FOUND
	if(m_data[cursor].isFailed)
	{
		m_mood = NOT_FOUND;
		m_isTransition = false;
		return;
	}
	float lipUp = (m_StretchedLipUpTB-50)/50.0;
	float lipDown = (m_StretchedLipDownTB-50)/50.0;
	float jawUp = (m_JawLowerUpTB-50)/50.0;
	float jawDown = (m_JawLowerDownTB-50)/50.0;
	float lipDownSad = (m_StretchedLipDownSadTB-50)/50.0;
	//HAPPY
	if(m_avg[STRETCHEDLIPS] > lipUp && m_avg[JAWLOWER]<jawDown)
	{
		m_mood = HAPPY;
	}
	//SAD
	else if(m_avg[STRETCHEDLIPS] < lipDownSad)
	{
		m_mood = SAD;
	}
	//ASTONISHED
	else if(m_avg[STRETCHEDLIPS] < lipDown && m_avg[JAWLOWER] > jawUp)
	{
		m_mood = ASTONISHED;
	}
	//SURPRISED
	else if(m_avg[STRETCHEDLIPS] > lipUp && m_avg[JAWLOWER] > jawUp)
	{
		m_mood = SURPRISED;
	}
	//NEUTRAL
	else
		m_mood = NEUTRAL;
	if(m_mood != m_prevMood)
		m_isTransition = true;		
	else
		m_isTransition = false;
}
FaceClassifier::~FaceClassifier(void)
{
	FILE *f = fopen("calibration.txt", "w");
	if(!f)
	{
		printf("Calibration not written!\n");
		return;
	}
	fprintf(f, "%d %d %d %d %d\n", m_StretchedLipUpTB, m_StretchedLipDownTB, m_JawLowerUpTB, m_JawLowerDownTB, m_StretchedLipDownSadTB);
	fclose(f);
}
