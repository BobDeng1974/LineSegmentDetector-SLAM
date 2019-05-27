/////////////////////////////////////////////////////////////////////////
//@Copyright(C) Pyrokine
//All rights reserved
//���� http://www.cnblogs.com/Pyrokine/
//Github https://github.com/Pyrokine
//�������� 20190327
//�汾 1.0
//**********************************************************************
//V1.0
//����baseFunc���򻯲�ͬ��֮��������ݹ����Լ�ʵ�ֻ������Ǻ���
//
/////////////////////////////////////////////////////////////////////////

#ifndef _BASEFUNC_
#define _BASEFUNC_

#define debugMode

typedef struct _structMapParam {
	int oriMapCol;
	int oriMapRow;
	double mapResol;
	double mapOriX;
	double mapOriY;
} structMapParam;

typedef struct _structLinesInfo {
	double k;
	double b;
	double dx;
	double dy;
	double x1;
	double y1;
	double x2;
	double y2;
	double len;
	int orient;
} structLinesInfo;

typedef struct _structPosition {
	double x;
	double y;
	double ang;
} structPosition;
double sind(double x);
double cosd(double x);
double atand(double x);

//createMapCache ����
const double z_occ_max_dis = 1;

//ScanToMapMatch ���̲߳���
const int lenTHREAD = 25;
const int lenQUEUE = 50;
#endif // ! _BASEFUNC_
