/////////////////////////////////////////////////////////////////////////
//@Copyright(C) Pyrokine
//All rights reserved
//���� http://www.cnblogs.com/Pyrokine/
//Github https://github.com/Pyrokine
//�������� 20190327
//�汾 1.2
//**********************************************************************
//V1.0
//��LSD�㷨��RDP�㷨��ȡ�ɵ����ļ������Ե������ã����������ռ�mylsd��
//myrdp������baseFunc���򻯲�ͬ��֮��������ݹ����Լ�ʵ�ֻ�������
//
//V1.1
//�ں���FeatureAssociation�㷨�����������ռ�myfa����������ת�������ͽṹ��
//����λ�ü���lineIm
//
//V1.2
//�޸����ļ��ṹ��ɾ����main_with_disp.cpp��ֻ���ڷ����汾1.3֮ǰ���Լ���
//���ļ����޸����ļ���ȡ��ʽ������ʹ���ļ�����ȡ��ʽ
/////////////////////////////////////////////////////////////////////////

#ifndef _BASEFUNC_
#define _BASEFUNC_

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

double sind(double x);
double cosd(double x);
double atand(double x);

#endif // ! _BASEFUNC_
