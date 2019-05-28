/////////////////////////////////////////////////////////////////////////
//@Copyright(C) Pyrokine
//All rights reserved
//���� http://www.cnblogs.com/Pyrokine/
//Github https://github.com/Pyrokine
//�������� 20190528
//�汾 1.2
//**********************************************************************
//V1.0
//ʵ����FetureAssociation�Ļ����㷨
//
//V1.1
//����ע��������RDP���Ƶ���ȡ�޸ĵ�RDP�㷨����ȡ���򻯼�����
//
//V1.2
//�Լ���Score���������˻���pthread���̳߳أ�������
//https://github.com/mbrossard/threadpool
//
//
/////////////////////////////////////////////////////////////////////////
#pragma comment(lib,"pthreadVC2.lib")  
#ifndef _MYFA_
#define _MYFA_

#include <opencv.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <threadpool.h>
#include <baseFunc.h>

using namespace cv;
using namespace std;

namespace myfa {
	typedef struct _structFAInput {
		vector<structLinesInfo> scanLinesInfo;
		vector<structLinesInfo> mapLinesInfo;
		vector<structPosition> scanImPoint;
		int lidarPos[2];
		Mat mapCache;
	} structFAInput;

	typedef struct _structScore {
		structPosition pos;
		double score;
		struct _structScore *next = NULL;
	} structScore;

	typedef struct _structStaEnd {
		double staX;
		double staY;
		double endX;
		double endY;
	} structStaEnd;

	typedef struct _structRotateScanIm {
		structPosition rotateLidarPos;
		structPosition *rotateScanImPoint;
		int numScanImPoint;
		double angDiff;
	} structRotateScanIm;

	typedef struct _structThreadSTMM {
		structFAInput *FAInput;
		int cntMapLine;
		int cntScanLine;
		vector<structScore> *Score;
	} structThreadSTMM;

	structScore FeatureAssociation(structFAInput *FAInput);
	void thread_ScanToMapMatch(void *arg);
	void ScanToMapMatch(structFAInput *FAInput, int cntMapLine, int cntScanLine, vector<structScore> *Score);
	double NormalizedLineDirection(structStaEnd lineStaEnd);
	structRotateScanIm rotateScanIm(structFAInput *FAInput, structPosition mapPose, structPosition scanPose);
	double CalcScore(structFAInput *FAInput, structRotateScanIm RSI);
	int CompScore(const void *p1, const void *p2);
}


#endif