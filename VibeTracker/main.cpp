#include"Vibe.h"
#include"Tracker.h"
#include"ViBeT.h"
#include<iostream>
#include<cstdio>
using namespace std;

int FRAME_MAX_COL;			// 
int FRAME_MAX_ROW;

int main() {

		
	Mat frame, gray, mask;//һ֡ͼ�񣬻ҶȻ���ǰ��
	VideoCapture capture;
	//capture.open("D:\\MOT_LOCAL\\dataset\\kftest.mp4");//����˿�
	//capture.open("D:\\MOT_LOCAL\\Radar\\visiontraffic--good.avi");//����˿�
	//capture.open("D:\\MOT_LOCAL\\Radar\\test2.mp4");// MOT1 ����˿�
	capture.open("D:\\MOT_LOCAL\\Radar\\car.avi");//����˿�

	if (!capture.isOpened())
	{

		cout << " ====== No camera or video input! =======\n" << endl;
		return -1;
	}

	ViBe_BGS Vibe_Bgs;			//����һ��������ֶ���  
	int count = 0;				//֡��������ͳ��Ϊ�ڼ�֡   

	float start, time;

	vector<bbox> dets;			// ���� Vibe ��⵽ bboxes   "����"

	ViBeT mot_tracker;		// ʵ���� VibeT ����
	int flag = 0;			// ���Ե�Ŀ���õı��

	int total = 0;			// ͳ�ƴ����֡��


	// ѭ������
	while (1)
	{
		start = static_cast<float>(getTickCount());
		count++;
		capture >> frame;

		mot_tracker.FRAME_MAX_COL = frame.cols;
		mot_tracker.FRAME_MAX_ROW = frame.rows;

		if (frame.empty())//ֱ����Ƶ���һ֡�˳�ѭ��
			break;
		cvtColor(frame, gray, COLOR_RGB2GRAY);//ת��Ϊ�Ҷ�ͼ��   

		namedWindow("masked", 0);
		//resizeWindow("masked", frame.cols, frame.rows);
		namedWindow("origin", 0);
		//resizeWindow("origin", frame.cols, frame.rows);


		/////////////                   STEP 1 : ʹ�� Vibe ��õ�ǰ֡���� ����            //////////////////////////
		dets.clear();

		if (count == 1)//��Ϊ��һ֡  
		{
			Vibe_Bgs.init(gray);// ��ʼ��
			Vibe_Bgs.processFirstFrame(gray);//����ģ�ͳ�ʼ��  ,���õ�һ֡���н�ģ  
			cout << " Training ViBe complete!" << endl;
		}
		else
		{
			Vibe_Bgs.testAndUpdate(gray);	//�ж�ǰ���뱳���������б�������  

			mask = Vibe_Bgs.getMask();		//�õ�ǰ�� 

			Mat kernel = cv::getStructuringElement(MORPH_RECT, Size(7, 7));
			morphologyEx(mask, mask, MORPH_CLOSE, kernel);		// �����ͺ�ʴ����ͨ��ͨ��,Ĭ�� [3*3]
			morphologyEx(mask, mask, MORPH_CLOSE, kernel);		// �����ͺ�ʴ����ͨ��ͨ��,Ĭ�� [3*3]
			morphologyEx(mask, mask, MORPH_CLOSE, kernel);		// �����ͺ�ʴ����ͨ��ͨ��,Ĭ�� [3*3]
			morphologyEx(mask, mask, MORPH_CLOSE, kernel);		// �����ͺ�ʴ����ͨ��ͨ��,Ĭ�� [3*3]
			morphologyEx(mask, mask, MORPH_CLOSE, kernel);		// �����ͺ�ʴ����ͨ��ͨ��,Ĭ�� [3*3]
			morphologyEx(mask, mask, MORPH_CLOSE, kernel);		// �����ͺ�ʴ����ͨ��ͨ��,Ĭ�� [3*3]


			dets = getBboxs(mask,1200);
			//cout << mask << endl;
			imshow("masked", mask);
		}

		//bbox statePredict(0.0,0.0,0.0,0.0);


		/////////////                   STEP 2 : ���� VibeT���� ��һ�θ���          //////////////////////////
		vector<int> trks = mot_tracker.update(dets);		// ���ؿ�����ʾ�� Ԥ��� ��mot_trakcer �б��е��±�����



		/////////////                   STEP 3 : ���ӻ���ʾ�������Ԥ���          //////////////////////////
		vector<bbox> trks_draw;
		for (int i = 0; i < trks.size(); ++i) {
			trks_draw.push_back(mot_tracker.trackers[trks[i]].get_state());
		}
		if(!dets.empty()) drawBbox(frame, dets, true);
		if(!trks_draw.empty())	drawBbox(frame, trks_draw, false);
		imshow("origin", frame);

		

		time = ((float)getTickCount() - start) / getTickFrequency() * 1000;
		//cout << "Time of processing one frame: " << time << " ms " << endl;

		if (waitKey(10) == 'q')
			break;
		//1.��waitKey(10)Ϊ�ȴ�10ms;
		//2.����whileѭ������Ϊ���޵ȴ�
	}

	cv::destroyAllWindows();
	capture.release();
	
	
		


	return 0;
}