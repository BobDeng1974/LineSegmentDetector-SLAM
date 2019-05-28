#include <myFA.h>

using namespace cv;
using namespace std;

namespace myfa {
	//���̵߳������������������
	int num_tasks = 0;
	int num_done = 0;
	//pthread�Ļ�����
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	structScore FeatureAssociation(structFAInput *FAInput) {
		//���룺LSD��RDP�㷨�õ����߶������Լ������ͼ
		//�������λ���
		int sizeScanLine = (int)FAInput->scanLinesInfo.size();
		int sizeMapLine = (int)FAInput->mapLinesInfo.size();
		//��ʼ����������
		num_tasks = 0;
		num_done = 0;
		//��ʼ���̳߳�
		threadpool_t *pool = threadpool_create(numTHREAD, lenQUEUE, 0);
		//��ʼ��Score
		vector<structScore> Score;

		int cntScanLine = 0;
		//��RDP�߶���Ϊ��׼����ƥ��
		for (cntScanLine = 0; cntScanLine < sizeScanLine; cntScanLine++) {
			double lenScanLine = FAInput->scanLinesInfo[cntScanLine].len;
			//���Թ��̵��߶�
			if (lenScanLine < ignoreScanLength)
				continue;

			double lenDiff = FAInput->scanLinesInfo[cntScanLine].len * scanToMapDiff;
			int cntMapLine = 0;
			for (cntMapLine = 0; cntMapLine < sizeMapLine; cntMapLine++) {
				double lenMapLine = FAInput->mapLinesInfo[cntMapLine].len;
				//�Գ��Ȳ���һ����Χ�ڵ��߶ν���ƥ��
				if (lenMapLine < lenScanLine - lenDiff || lenMapLine > lenScanLine + lenDiff)
					continue;

				//���߳�����
				//ScanToMapMatch(FAInput, cntMapLine, cntScanLine, &Score);

				//���̲߳�������
				//�ȴ�����ճ�
				while (num_tasks - num_done > lenQUEUE);
				//����
				structThreadSTMM *argSTMM = (structThreadSTMM*)malloc(sizeof(structThreadSTMM));
				argSTMM->cntMapLine = cntMapLine;
				argSTMM->cntScanLine = cntScanLine;
				argSTMM->FAInput = FAInput;
				argSTMM->Score = &Score;
				//�������
				threadpool_add(pool, &thread_ScanToMapMatch, argSTMM, 0);
				pthread_mutex_lock(&mutex);
				num_tasks++;
				pthread_mutex_unlock(&mutex);
			}
		}
		//�ȴ�������������������̣߳���֪BUG����һ��������ܲ�������
		//�������ӵȴ�ʱ���Խ����ȴ�
		while (num_tasks - num_done > 1);
		threadpool_destroy(pool, 0);

		int lenScore = 0;
		structScore *poseAll;
		structScore poseBase;
		//�ж��Ƿ���ƥ�������������򴴽��µ�����Ʒ���
		if (Score.empty()) {
			
			return poseBase;
		}
		else {
			lenScore = (int)Score.size();
			poseAll = new structScore[lenScore * sizeof(structScore)];
			memcpy(poseAll, &Score[0], lenScore * sizeof(structScore));
		}
		//ȷ��Scorre��͵Ľ��Ϊ��һ��ƥ��Ļ�׼��
		qsort(poseAll, lenScore, sizeof(structScore), CompScore);
		poseBase = poseAll[0];
		printf("Score:%f\n", poseBase.score);

		free(poseAll);
		return poseBase;
	}

	void thread_ScanToMapMatch(void *arg) {
		//���룺��ƥ��������߶ε����
		//���������ƥ�����ļ����״�λ�ú͵÷�
		structThreadSTMM *argSTMM = (structThreadSTMM*) arg;

		int lenScore = 0;
		int i = 0;
		//����ƥ�䷽ʽ
		for (i = 1; i <= 4; i++) {
			structStaEnd mapStaEndPoint, scanStaEndPoint;
			if (i == 1) {
				mapStaEndPoint.staX = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].x1;
				mapStaEndPoint.staY = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].y1;
				mapStaEndPoint.endX = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].x2;
				mapStaEndPoint.endY = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].y2;
				scanStaEndPoint.staX = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].x1;
				scanStaEndPoint.staY = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].y1;
				scanStaEndPoint.endX = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].x2;
				scanStaEndPoint.endY = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].y2;
			}
			else if (i == 2) {
				mapStaEndPoint.staX = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].x1;
				mapStaEndPoint.staY = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].y1;
				mapStaEndPoint.endX = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].x2;
				mapStaEndPoint.endY = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].y2;
				scanStaEndPoint.staX = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].x2;
				scanStaEndPoint.staY = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].y2;
				scanStaEndPoint.endX = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].x1;
				scanStaEndPoint.endY = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].y1;
			}
			else if (i == 3) {
				mapStaEndPoint.staX = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].x2;
				mapStaEndPoint.staY = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].y2;
				mapStaEndPoint.endX = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].x1;
				mapStaEndPoint.endY = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].y1;
				scanStaEndPoint.staX = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].x1;
				scanStaEndPoint.staY = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].y1;
				scanStaEndPoint.endX = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].x2;
				scanStaEndPoint.endY = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].y2;
			}
			else {
				mapStaEndPoint.staX = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].x2;
				mapStaEndPoint.staY = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].y2;
				mapStaEndPoint.endX = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].x1;
				mapStaEndPoint.endY = argSTMM->FAInput->mapLinesInfo[argSTMM->cntMapLine].y1;
				scanStaEndPoint.staX = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].x2;
				scanStaEndPoint.staY = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].y2;
				scanStaEndPoint.endX = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].x1;
				scanStaEndPoint.endY = argSTMM->FAInput->scanLinesInfo[argSTMM->cntScanLine].y1;
			}

			//����ƥ���׼��ͻ�׼�߶νǶ�
			structPosition mapPose, scanPose;
			mapPose.x = mapStaEndPoint.staX;
			mapPose.y = mapStaEndPoint.staY;
			mapPose.ang = NormalizedLineDirection(mapStaEndPoint);
			scanPose.x = scanStaEndPoint.staX;
			scanPose.y = scanStaEndPoint.staY;
			scanPose.ang = NormalizedLineDirection(scanStaEndPoint);

			structRotateScanIm RSI = rotateScanIm(argSTMM->FAInput, mapPose, scanPose);

			structScore tempScore;
			tempScore.pos = RSI.rotateLidarPos;
			tempScore.score = CalcScore(argSTMM->FAInput, RSI);
			free(RSI.rotateScanImPoint);

			//д����
			pthread_mutex_lock(&mutex);
			argSTMM->Score->push_back(tempScore);
			pthread_mutex_unlock(&mutex);
		}
		//��������������ͷŲ���
		pthread_mutex_lock(&mutex);
		num_done++;
		pthread_mutex_unlock(&mutex);
		free(argSTMM);
	}

	void ScanToMapMatch(structFAInput *FAInput, int cntMapLine, int cntScanLine, vector<structScore> *Score) {
		//���߳�����ƥ�䣨�����ã�
		int lenScore = 0;
		int i = 0;
		for (i = 1; i <= 4; i++) {
			structStaEnd mapStaEndPoint, scanStaEndPoint;
			if (i == 1) {
				mapStaEndPoint.staX = FAInput->mapLinesInfo[cntMapLine].x1;
				mapStaEndPoint.staY = FAInput->mapLinesInfo[cntMapLine].y1;
				mapStaEndPoint.endX = FAInput->mapLinesInfo[cntMapLine].x2;
				mapStaEndPoint.endY = FAInput->mapLinesInfo[cntMapLine].y2;
				scanStaEndPoint.staX = FAInput->scanLinesInfo[cntScanLine].x1;
				scanStaEndPoint.staY = FAInput->scanLinesInfo[cntScanLine].y1;
				scanStaEndPoint.endX = FAInput->scanLinesInfo[cntScanLine].x2;
				scanStaEndPoint.endY = FAInput->scanLinesInfo[cntScanLine].y2;
			}
			else if (i == 2) {
				mapStaEndPoint.staX = FAInput->mapLinesInfo[cntMapLine].x1;
				mapStaEndPoint.staY = FAInput->mapLinesInfo[cntMapLine].y1;
				mapStaEndPoint.endX = FAInput->mapLinesInfo[cntMapLine].x2;
				mapStaEndPoint.endY = FAInput->mapLinesInfo[cntMapLine].y2;
				scanStaEndPoint.staX = FAInput->scanLinesInfo[cntScanLine].x2;
				scanStaEndPoint.staY = FAInput->scanLinesInfo[cntScanLine].y2;
				scanStaEndPoint.endX = FAInput->scanLinesInfo[cntScanLine].x1;
				scanStaEndPoint.endY = FAInput->scanLinesInfo[cntScanLine].y1;
			}
			else if (i == 3) {
				mapStaEndPoint.staX = FAInput->mapLinesInfo[cntMapLine].x2;
				mapStaEndPoint.staY = FAInput->mapLinesInfo[cntMapLine].y2;
				mapStaEndPoint.endX = FAInput->mapLinesInfo[cntMapLine].x1;
				mapStaEndPoint.endY = FAInput->mapLinesInfo[cntMapLine].y1;
				scanStaEndPoint.staX = FAInput->scanLinesInfo[cntScanLine].x1;
				scanStaEndPoint.staY = FAInput->scanLinesInfo[cntScanLine].y1;
				scanStaEndPoint.endX = FAInput->scanLinesInfo[cntScanLine].x2;
				scanStaEndPoint.endY = FAInput->scanLinesInfo[cntScanLine].y2;
			}
			else {
				mapStaEndPoint.staX = FAInput->mapLinesInfo[cntMapLine].x2;
				mapStaEndPoint.staY = FAInput->mapLinesInfo[cntMapLine].y2;
				mapStaEndPoint.endX = FAInput->mapLinesInfo[cntMapLine].x1;
				mapStaEndPoint.endY = FAInput->mapLinesInfo[cntMapLine].y1;
				scanStaEndPoint.staX = FAInput->scanLinesInfo[cntScanLine].x2;
				scanStaEndPoint.staY = FAInput->scanLinesInfo[cntScanLine].y2;
				scanStaEndPoint.endX = FAInput->scanLinesInfo[cntScanLine].x1;
				scanStaEndPoint.endY = FAInput->scanLinesInfo[cntScanLine].y1;
			}

			structPosition mapPose, scanPose;
			mapPose.x = mapStaEndPoint.staX;
			mapPose.y = mapStaEndPoint.staY;
			mapPose.ang = NormalizedLineDirection(mapStaEndPoint);
			scanPose.x = scanStaEndPoint.staX;
			scanPose.y = scanStaEndPoint.staY;
			scanPose.ang = NormalizedLineDirection(scanStaEndPoint);

			structRotateScanIm RSI = rotateScanIm(FAInput, mapPose, scanPose);
			
			structScore tempScore;
			tempScore.pos = RSI.rotateLidarPos;
			tempScore.score = CalcScore(FAInput, RSI);
			free(RSI.rotateScanImPoint);
			Score->push_back(tempScore);
		}
	}

	double NormalizedLineDirection(structStaEnd lineStaEnd) {
		//����б�ʣ� ��һ���߶η���, �㣨x1, y1�� Ϊ��ʼ��
		//angleΪ�߶νǶ�, ��λΪy�ȣ���СΪ[-180��180]
		double angle;
		if (lineStaEnd.staX == lineStaEnd.endX && lineStaEnd.staY != lineStaEnd.endY) {
			if (lineStaEnd.staY < lineStaEnd.endY)
				angle = 90;
			else
				angle = -90;
		}
		else if (lineStaEnd.staX != lineStaEnd.endX && lineStaEnd.staY == lineStaEnd.endY) {
			if (lineStaEnd.staX < lineStaEnd.endX)
				angle = 0;
			else
				angle = 180;
		}
		else
			angle = (lineStaEnd.endY - lineStaEnd.staY) / (lineStaEnd.endX - lineStaEnd.staX);

		if (angle < 0 && lineStaEnd.staX > lineStaEnd.endX)
			angle += 180;

		if (angle > 0 && lineStaEnd.staX > lineStaEnd.endX)
			angle -= 180;

		return angle;
	}

	structRotateScanIm rotateScanIm(structFAInput *FAInput, structPosition mapPose, structPosition scanPose) {
		//��תRDP�����Լ���Score
		//LSD�߶κ�RDP�߶εĽǶȲ�
		double angDiff = mapPose.ang - scanPose.ang;

		int numScanImPoint = (int)FAInput->scanImPoint.size();
		int cnt;
		//������ԭ��ƽ�Ƶ���ƥ���RDP�߶εĻ�׼��
		structPosition *oriScanImPoint = (structPosition*)malloc(numScanImPoint * sizeof(structPosition));
		for (cnt = 0; cnt < numScanImPoint; cnt++) {
			oriScanImPoint[cnt].x = FAInput->scanImPoint[cnt].x - scanPose.x;
			oriScanImPoint[cnt].y = FAInput->scanImPoint[cnt].y - scanPose.y;
		}
		//���ǶȲ���תͼ���ԭ��ƽ�Ƶ�LSD�߶λ�׼��
		structPosition *rotateScanImPoint = (structPosition*)malloc(numScanImPoint * sizeof(structPosition));
		for (cnt = 0; cnt < numScanImPoint; cnt++) {
			rotateScanImPoint[cnt].x = oriScanImPoint[cnt].x * cosd(angDiff) - oriScanImPoint[cnt].y * sind(angDiff) + mapPose.x;
			rotateScanImPoint[cnt].y = oriScanImPoint[cnt].x * sind(angDiff) + oriScanImPoint[cnt].y * cosd(angDiff) + mapPose.y;
		}
		//�������״�������������ͬ�任
		structPosition rotateLidarPos;
		rotateLidarPos.x = (FAInput->lidarPos[0] - scanPose.x) * cosd(angDiff) - (FAInput->lidarPos[1] - scanPose.y) * sind(angDiff) + mapPose.x;
		rotateLidarPos.y = (FAInput->lidarPos[0] - scanPose.x) * sind(angDiff) + (FAInput->lidarPos[1] - scanPose.y) * cosd(angDiff) + mapPose.y;
		rotateLidarPos.ang = scanPose.ang + angDiff;

		//���ǶȲ������[-180,180]
		while (angDiff <= -180)
			angDiff += 360;
		while (angDiff > 180)
			angDiff -= 360;

		structRotateScanIm RSI;
		RSI.rotateScanImPoint = rotateScanImPoint;
		RSI.numScanImPoint = numScanImPoint;
		RSI.angDiff = angDiff;
		RSI.rotateLidarPos = rotateLidarPos;

		free(oriScanImPoint);
		return RSI;
	}

	double CalcScore(structFAInput *FAInput, structRotateScanIm RSI) {
		//���룺��ת���RDP���ƺ;���ͼmapCache
		//�������������Score��ԽСԽ��
		//sumDistΪ����ͣ�ValidΪ��mapCache��С�ڲ���z_occ_max_dis������
		double sumValidDist = 0, sumMaxDist = 0, numValidDistPoint = 0, numMaxDistPoint = 0;
		//AllΪ�������أ�ValidΪ�ڵ�ͼ�ڵ�����
		double numAllPoint = 0, numValidPoint = 0;

		int cnt;
		for (cnt = 0; cnt < RSI.numScanImPoint; cnt++) {
			int y = (int)round(RSI.rotateScanImPoint[cnt].y);
			int x = (int)round(RSI.rotateScanImPoint[cnt].x);
			//printf("%d %d %d %d\n", FAInput->mapIm.size[0], FAInput->mapIm.size[1], y, x);
			//printf("%f %f\n", RSI.rotateScanImPoint[cnt].y, RSI.rotateScanImPoint[cnt].x);
			if (y >= 0 && y < FAInput->mapCache.size[0] && x >= 0 && x < FAInput->mapCache.size[1]) {
				numValidPoint += 1;
				if (FAInput->mapCache.ptr<double>(y)[x] >= z_occ_max_dis)
					numMaxDistPoint += 1;
				else {
					sumValidDist += FAInput->mapCache.ptr<double>(y)[x];
					numValidDistPoint += 1;
				}
			}
		}
		numAllPoint = RSI.numScanImPoint;
		//���Ȩ�أ��ͷ�Max����
		sumMaxDist = 7 * numMaxDistPoint;
		double Score;
		if (numValidPoint == 0)
			Score = INFINITY;
		else
			Score = (sumValidDist + sumMaxDist) / (numValidPoint) + 10 * (numAllPoint - numValidPoint) / numValidPoint;
		//double Score = (sumValidDist + sumMaxDist) / numValidPoint;
		
		return Score;
	}

	int CompScore(const void *p1, const void *p2)
	{
		//��С��������
		return(*(structScore*)p2).score < (*(structScore*)p1).score ? 1 : -1;
	}

}