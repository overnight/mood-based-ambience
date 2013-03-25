#pragma once
#ifndef FACECLASSIFIER_H
#define FACECLASSIFIER_H
#include "Mood.h"
#include "SocketsHelper.h"
#define NUM_HISTORY 5
#include "core/core.hpp"
#include "highgui/highgui.hpp"
#define TB_WINDOW "Trackbars"

class FaceClassifier
{
public:
	FaceClassifier(void);
	~FaceClassifier(void);
	void populate(SocketDataType *pPacket);
	Mood getMood(){ calcMood(); return m_mood;}
	bool getTransition(){return m_isTransition;}
	FLOAT m_avg[6];
private:
	SocketDataType m_data[NUM_HISTORY];
	bool m_isTransition;
	int cursor;
	int m_StretchedLipUpTB;
	int m_StretchedLipDownTB;
	int m_JawLowerUpTB;
	int m_JawLowerDownTB;
	int m_StretchedLipDownSadTB;
	void calcMood();
	Mood m_mood, m_prevMood;
};

#endif