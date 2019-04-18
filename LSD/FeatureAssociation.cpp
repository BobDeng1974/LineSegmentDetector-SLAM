#include"FeatureAssociation.h"
#define _USE_MATH_DEFINES
#include <math.h>

namespace myfa {
	double NormalizedLineDirection(double x1, double y1, double x2, double y2)
	{
		double ang = 0;
		double dy = y2 - y1, dx = x2 - x1;
		if (dy && !dx)
		{
			if (dy > 0)
				ang = 90;
			else
				ang = -90;
		}
		else if (!dy && dx)
		{
			if (dx > 0)
				ang = 0;
			else
				ang = 180;
		}
		else
			ang = atan(dy / dx) * 180 / M_PI;
		if (dx < 0)
		{
			if (ang < 0)
				ang += 180;
			else if (ang > 0)
				ang -= 180;
		}
		return ang;
	}

	void FeatureAssociation(
		const Mat& ScanlineIm,
		const vector<structLinesInfo>& ScanlinesInfo,
		const vector<structLinesInfo>& MaplinesInfo,
		const structMapParam& MapParam,
		const int* LidarPos,
		const Mat& MaplineIm,
		const Mat& MapCache,
		const Mat& MapValue,
		const vector<double>& ScanRanges,
		const vector<double>& ScanAngles,
		double* estimatePose_realworld,
		double* estimatePose,
		Mat& poseAll
	)
	{
		//��ȡֱ����Ϣ
		size_t Scanlinesize = ScanlinesInfo.size();
		vector<double> Scanlen(Scanlinesize);
		for (size_t i = 0; i < Scanlinesize; i++)
			Scanlen[i] = ScanlinesInfo[i].len;

		size_t Maplinesize = MaplinesInfo.size();
		vector<double> Maplen(Maplinesize);
		for (size_t i = 0; i < Maplinesize; i++)
			Maplen[i] = MaplinesInfo[i].len;

		//���ҳ������Ƶ�ֱ��
		double len_diff_m = 0.3;
		double len_diff = len_diff_m / MapParam.mapResol;
		vector<Mat> ScoresAll(Scanlinesize);
		for (unsigned i = 0; i < Scanlinesize; i++)
		{
			double target = Scanlen[i];		//ScanIm��ֱ�߳���
			vector<unsigned> Mapline_n;
			for (unsigned j = 0; j < Maplinesize; j++)
				if (Maplen[j] >= target - len_diff && Maplen[j] <= target + len_diff)
					Mapline_n.push_back(j);

			Mat Score(15, 4 * (int)Mapline_n.size(), CV_64F);
			for (unsigned j = 0; j < Mapline_n.size(); j++)
			{
				unsigned mapline_numi = Mapline_n[j];	//% mapline �� MaplinesInfo �еı��

				double Scan_x1 = ScanlinesInfo[i].x1;
				double Scan_y1 = ScanlinesInfo[i].y1;
				double Scan_x2 = ScanlinesInfo[i].x2;
				double Scan_y2 = ScanlinesInfo[i].y2;
				double map_x1 = MaplinesInfo[mapline_numi].x1;
				double map_y1 = MaplinesInfo[mapline_numi].y1;
				double map_x2 = MaplinesInfo[mapline_numi].x2;
				double map_y2 = MaplinesInfo[mapline_numi].y2;

				ScanToMapMatch(
					Scan_x1, Scan_y1, Scan_x2, Scan_y2,
					map_x1, map_y1, map_x2, map_y2,
					ScanlineIm,
					MaplineIm,
					MapCache,
					LidarPos,
					ScanRanges,
					ScanAngles,
					MapParam,
					i,
					mapline_numi
				).copyTo(Score.colRange(4 * j, 4 * j + 4));
			}
			ScoresAll[i] = Score;
		}

		// ��ȡ���й���λ�ˣ��ҵ���������λ��
		int t = 0;
		for (int i = 0; i < Scanlinesize; i++)
			t += ScoresAll[i].cols;
		poseAll.create(15, t, CV_64F);
		t = 0;
		for (int i = 0; i < Scanlinesize; i++)
			if (!ScoresAll[i].empty())
			{
				ScoresAll[i].copyTo(poseAll.colRange(t, t + ScoresAll[i].cols));
				t += ScoresAll[i].cols;
			}

		t = 0;
		for (int i = 0; i < poseAll.cols; i++)
			t = *poseAll.ptr<double>(3, i) < *poseAll.ptr<double>(3, t) ? i : t;
		estimatePose[0] = *poseAll.ptr<double>(0, t);
		estimatePose[1] = *poseAll.ptr<double>(1, t);
		estimatePose[2] = *poseAll.ptr<double>(2, t) / 180 * M_PI;

		// ������ʵ��������
		estimatePose_realworld[0] = estimatePose[0] * MapParam.mapResol + MapParam.mapOriX;
		estimatePose_realworld[1] = estimatePose[1] * MapParam.mapResol + MapParam.mapOriY;
		estimatePose_realworld[2] = estimatePose[2];
	}

	Mat ScanToMapMatch(
		double scan_x1,
		double scan_y1,
		double scan_x2,
		double scan_y2,
		double map_x1,
		double map_y1,
		double map_x2,
		double map_y2,
		const Mat& Scan_Im,
		const Mat& Map_Im,
		const Mat& MapCache,
		const int* LidarPos,
		const vector<double>& ScanRanges,
		const vector<double>& ScanAngles,
		const structMapParam& MapParam,
		unsigned scan_num_i,
		unsigned mapline_numi
	)
	{
		Mat Score(15, 4, CV_64F);
		double Mapline[3];
		double Scanline[3];
		double ScanPose[3];
		double Map_StaEnd_Point[4];
		double Scan_StaEnd_Point[4];

		for (int i = 0; i < 4; i++)
		{
			switch (i)
			{
			case 0:		// ��һ�� map���A �� scan���1 ����
				Map_StaEnd_Point[0] = map_x1; Map_StaEnd_Point[1] = map_y1; Map_StaEnd_Point[2] = map_x2; Map_StaEnd_Point[3] = map_y2;
				Scan_StaEnd_Point[0] = scan_x1; Scan_StaEnd_Point[1] = scan_y1; Scan_StaEnd_Point[2] = scan_x2; Scan_StaEnd_Point[3] = scan_y2;
				break;
			case 1:		//�ڶ��� �� map���A �� scan���2 ����
				Map_StaEnd_Point[0] = map_x1; Map_StaEnd_Point[1] = map_y1; Map_StaEnd_Point[2] = map_x2; Map_StaEnd_Point[3] = map_y2;
				Scan_StaEnd_Point[0] = scan_x2; Scan_StaEnd_Point[1] = scan_y2; Scan_StaEnd_Point[2] = scan_x1; Scan_StaEnd_Point[3] = scan_y1;
				break;
			case 2:		//������ �� map���B �� scan���1 ����
				Map_StaEnd_Point[0] = map_x2; Map_StaEnd_Point[1] = map_y2; Map_StaEnd_Point[2] = map_x1; Map_StaEnd_Point[3] = map_y1;
				Scan_StaEnd_Point[0] = scan_x1; Scan_StaEnd_Point[1] = scan_y1; Scan_StaEnd_Point[2] = scan_x2; Scan_StaEnd_Point[3] = scan_y2;
				break;
			case 3:		//������ �� map���B �� scan���2 ����
				Map_StaEnd_Point[0] = map_x2; Map_StaEnd_Point[1] = map_y2; Map_StaEnd_Point[2] = map_x1; Map_StaEnd_Point[3] = map_y1;
				Scan_StaEnd_Point[0] = scan_x2; Scan_StaEnd_Point[1] = scan_y2; Scan_StaEnd_Point[2] = scan_x1; Scan_StaEnd_Point[3] = scan_y1;
				break;
			}
			Mapline[0] = Map_StaEnd_Point[0];
			Mapline[1] = Map_StaEnd_Point[1];
			Mapline[2] = NormalizedLineDirection(Map_StaEnd_Point[0], Map_StaEnd_Point[1], Map_StaEnd_Point[2], Map_StaEnd_Point[3]);
			Scanline[0] = Scan_StaEnd_Point[0];
			Scanline[1] = Scan_StaEnd_Point[1];
			Scanline[2] = NormalizedLineDirection(Scan_StaEnd_Point[0], Scan_StaEnd_Point[1], Scan_StaEnd_Point[2], Scan_StaEnd_Point[3]);
			RotateScanIm(Scanline, Mapline, Scan_Im, Map_Im, LidarPos, ScanPose);
			for (int k = 0; k < 3; k++)
				*Score.ptr<double>(k, i) = ScanPose[k];
			*Score.ptr<double>(3, i) = ScanToMapMatchScore(Map_Im, MapCache, ScanPose, ScanRanges, ScanAngles, MapParam);
			for (int k = 0; k < 4; k++)
			{
				*Score.ptr<double>(4 + k, i) = Map_StaEnd_Point[k];
				*Score.ptr<double>(8 + k, i) = Scan_StaEnd_Point[k];
			}
			*Score.ptr<double>(12, i) = scan_num_i;
			*Score.ptr<double>(13, i) = mapline_numi;
			*Score.ptr<double>(14, i) = i;
		}
		return Score;
	}

	double ScanToMapMatchScore(
		const Mat& Map_Im,
		const Mat& MapCache,
		const double* ScanPose_Global,
		const vector<double>& ScanRanges,
		const vector<double>& ScanAngles,
		const structMapParam& MapParam
	) // ���� ƥ���׼ȷ�� Score ԽС˵��Խ׼ȷ
	{
		if (ScanPose_Global[0] > Map_Im.cols || ScanPose_Global[0] < 1 || ScanPose_Global[1] > Map_Im.rows || ScanPose_Global[1] < 1)
			return INFINITY;
		else
		{
			double Score_occ_dist = 0;
			double dist_count = 0;
			double Score_occ_count = 0;
			double Score_occ_max_count = 0;

			// ������ ������ռ�ݵ���������
			unsigned mapSizeY = Map_Im.rows;
			unsigned mapSizeX = Map_Im.cols;
			Mat Lidar_Global((int)ScanRanges.size(), 2, CV_64F);
			for (int i = 0; i < Lidar_Global.rows; i++)
			{
				*Lidar_Global.ptr<double>(i, 0) = floor(ScanRanges[i] * cos(ScanAngles[i] + ScanPose_Global[2] * M_PI / 180) / MapParam.mapResol) + ScanPose_Global[0] - 1;
				*Lidar_Global.ptr<double>(i, 1) = floor(ScanRanges[i] * sin(ScanAngles[i] + ScanPose_Global[2] * M_PI / 180) / MapParam.mapResol) + ScanPose_Global[1] - 1;
			}

			double Scanlen = 0;
			for (int i = 0; i < Lidar_Global.rows; i++)
			{
				if (*Lidar_Global.ptr<double>(i, 0) > 1 && *Lidar_Global.ptr<double>(i, 0) < mapSizeX
					&& *Lidar_Global.ptr<double>(i, 1) > 1 && *Lidar_Global.ptr<double>(i, 1) < mapSizeY)
				{
					Scanlen++;
					if (*Map_Im.ptr<uint8_t>((int)*Lidar_Global.ptr<double>(i, 1), (int)*Lidar_Global.ptr<double>(i, 0)) == 1)
						Score_occ_count++;
					if (*MapCache.ptr<double>((int)*Lidar_Global.ptr<double>(i, 1), (int)*Lidar_Global.ptr<double>(i, 0)) == 2)
						Score_occ_max_count++;
					else
					{
						Score_occ_dist += *MapCache.ptr<double>((int)*Lidar_Global.ptr<double>(i, 1), (int)*Lidar_Global.ptr<double>(i, 0));
						dist_count++;
					}
				}
			}
			if (Scanlen < ScanRanges.size() * 0.75)
				return INFINITY;
			return (Score_occ_dist + 7 * Score_occ_max_count) / (dist_count + Score_occ_max_count) + 10 * (Lidar_Global.rows - Scanlen) / Lidar_Global.rows;
		}
	}

	void RotateScanIm(
		const double* Scanline,
		const double* Mapline,
		const Mat& Scan_Im,
		const Mat& Map_Im,
		const int* ScanPosition,
		double* ScanPoseNew
	)
	{	// ��ת�����״����ͼ
		// Scan_Im �ϴ��� Scanline �� Map_Im �ϴ��� Mapline,
		// ��ת Scan_Im�� ʹ Scanline �� Mapline �غ�
		//unsigned mapSizeY = Map_Im.rows;
		//unsigned mapSizeX = Map_Im.cols;
		// �������߶εĽǶȲ�
		double ang_diff = Mapline[2] - Scanline[2];

		// �������״����ͼ��ӳ����ȫ�ֵ�ͼ�ϣ��Ƚ����ƶ���ȷ���ٸ���б�ʲ�����ͬ���ȵ��߶�
		//vector<double> ScanImY, ScanImX;
		//for (int i = 0; i < Scan_Im.rows; i++)
		//	for (int j = 0; j < Scan_Im.cols; j++)
		//		if (*Scan_Im.ptr<uint8_t>(i, j))
		//		{
		//			ScanImY.push_back(i + 1);
		//			ScanImX.push_back(j + 1);
		//		}
		// ����ת������ԭ��
		/*for (unsigned i = 0; i < ScanImX.size(); i++)
		{
			ScanImX[i] = ScanImX[i] - Scanline[0];
			ScanImY[i] = ScanImY[i] - Scanline[1];
		}*/

		// ��תͼ��
		double cosd_ang_diff = cos(ang_diff / 180 * M_PI);
		double sind_ang_diff = sin(ang_diff / 180 * M_PI);
		//vector<int> Scan_Im_GobalX_tmp(ScanImX.size()), Scan_Im_GobalY_tmp(ScanImX.size());
		//for (unsigned i = 0; i < ScanImX.size(); i++)
		//{
		//	Scan_Im_GobalX_tmp[i] = (int)floor(ScanImX[i] * cosd_ang_diff - ScanImY[i] * sind_ang_diff + Mapline[0]);
		//	Scan_Im_GobalY_tmp[i] = (int)floor(ScanImX[i] * sind_ang_diff + ScanImY[i] * cosd_ang_diff + Mapline[1]);
		//}

		ScanPoseNew[0] = floor((ScanPosition[0] - Scanline[0]) * cosd_ang_diff - (ScanPosition[1] - Scanline[1]) * sind_ang_diff + Mapline[0]);
		ScanPoseNew[1] = floor((ScanPosition[0] - Scanline[0]) * sind_ang_diff + (ScanPosition[1] - Scanline[1]) * cosd_ang_diff + Mapline[1]);
		ScanPoseNew[2] = Scanline[2] + ang_diff;
	}

	void samplePos(
		const Mat& realPos,
		const Mat& recored_Odom,
		Mat& sampleRealPos
	)
	{
		sampleRealPos = Mat::zeros(2, *recored_Odom.ptr<int>(0, recored_Odom.cols - 1),CV_64F);
		double x1, x2, y1, y2, k;
		double xHigh, xLow, yHigh, yLow;
		double xRange, yRange;
		int sampleN;
		Mat xx, yy;
		for (int i = 0; i < realPos.cols - 1; i++)
		{
			x1 = *realPos.ptr<double>(0, i);
			y1 = *realPos.ptr<double>(1, i);
			x2 = *realPos.ptr<double>(0, i + 1);
			y2 = *realPos.ptr<double>(1, i + 1);
			k = (y2 - y1) / (x2 - x1);
			sampleN = *recored_Odom.ptr<int>(0, i + 1) - *recored_Odom.ptr<int>(0, i);
			if (x1 > x2)
			{
				xHigh = x1;
				xLow = x2;
			}
			else
			{
				xHigh = x2;
				xLow = x1;
			}
			if (y1 > y2)
			{
				yHigh = y1;
				yLow = y2;
			}
			else
			{
				yHigh = y2;
				yLow = y1;
			}
			xRange = xHigh - xLow;
			yRange = yHigh - yLow;
			xx.create(1, sampleN + 1, CV_64F);
			yy.create(1, sampleN + 1, CV_64F);
			if (xRange > yRange)
			{
				*xx.ptr<double>(0, 0) = xLow;
				*xx.ptr<double>(0, sampleN) = xHigh;
				for (int j = 1; j < sampleN; j++)
					*xx.ptr<double>(0, j) = *xx.ptr<double>(0, j - 1) + xRange / sampleN;
				for (int j = 0; j < sampleN + 1; j++)
					*yy.ptr<double>(0, j) = (*xx.ptr<double>(0, j) - x1) * k + y1;
			}
			else
			{
				*yy.ptr<double>(0, 0) = yLow;
				*yy.ptr<double>(0, sampleN) = yHigh;
				for (int j = 1; j < sampleN; j++)
					*yy.ptr<double>(0, j) = *yy.ptr<double>(0, j - 1) + yRange / sampleN;
				for (int j = 0; j < sampleN + 1; j++)
					*xx.ptr<double>(0, j) = (*yy.ptr<double>(0, j) - y1) / k + x1;
			}
			xx.copyTo(sampleRealPos.colRange(*recored_Odom.ptr<int>(0, i) - 1, *recored_Odom.ptr<int>(0, i) + sampleN).row(0));
			yy.copyTo(sampleRealPos.colRange(*recored_Odom.ptr<int>(0, i) - 1, *recored_Odom.ptr<int>(0, i) + sampleN).row(1));
		}
	}
}

