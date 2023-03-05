#pragma once
#pragma once
#include <iostream>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

#define NUM_SAMPLES 20		//ÿ�����ص����������
#define MIN_MATCHES 2		//#minָ��
#define RADIUS 20		//Sqthere�뾶
#define SUBSAMPLE_FACTOR 16	//�Ӳ������ʣ������������µĸ���  


class ViBe_BGS
{
public:
	ViBe_BGS(void); //���캯��  
	~ViBe_BGS(void);//�����������Կ��ٵ��ڴ�����Ҫ��������  

	void init(const Mat _image);   //��ʼ��
	void processFirstFrame(const Mat _image); //���õ�һ֡���н�ģ   
	void testAndUpdate(const Mat _image);  //�ж�ǰ���뱳���������б�������  
	Mat getMask(void) { return m_mask; };//�õ�ǰ��  


	
private:
	Mat m_samples[NUM_SAMPLES];  //ÿһ֡ͼ���ÿһ�����ص�������  
	Mat m_foregroundMatchCount; //ͳ�����ر��ж�Ϊǰ���Ĵ��������ڸ���  
	Mat m_mask;					//ǰ����ȡ�����Ĥ���Ҷ�ͼ 0 Ϊ������255 Ϊǰ��
};