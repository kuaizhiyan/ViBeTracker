#include <opencv2/opencv.hpp>
#include <iostream>
#include "ViBe.h"

using namespace std;
using namespace cv;

int c_xoff[9] = { -1,  0,  1, -1, 1, -1, 0, 1, 0 };  //x���ھӵ㣬9����  
int c_yoff[9] = { -1,  0,  1, -1, 1, -1, 0, 1, 0 };  //y���ھӵ�

ViBe_BGS::ViBe_BGS(void)//���캯��
{

}
ViBe_BGS::~ViBe_BGS(void)//�����������Կ��ٵ��ڴ�����Ҫ��������  
{

}

/**************** Assign space and init ***************************/
void ViBe_BGS::init(const Mat _image)//��Ա������ʼ��  
{
	for (int i = 0; i < NUM_SAMPLES; i++)//#define NUM_SAMPLES 20������������⣬���һ֡ͼ�񣬽�����20֡��������  
	{
		m_samples[i] = Mat::zeros(_image.size(), CV_8UC1);//���ÿһ֡��������ÿһ�����س�ʼ��Ϊ8λ�޷���0����ͨ��  
	}
	m_mask = Mat::zeros(_image.size(), CV_8UC1);//��ʼ��  
	m_foregroundMatchCount = Mat::zeros(_image.size(), CV_8UC1);//ÿһ�����ر��ж�Ϊǰ���Ĵ�������ʼ��  
}

/**************** Init model from first frame ********************/
void ViBe_BGS::processFirstFrame(const Mat _image)//���õ�һ֡���н�ģ  
{
	RNG rng;//�����������           
	int row, col;//�У���

	for (int i = 0; i < _image.rows; i++)
	{
		for (int j = 0; j < _image.cols; j++)
		{
			for (int k = 0; k < NUM_SAMPLES; k++)
			{
				// Random pick up NUM_SAMPLES pixel in neighbourhood to construct the model
				int random = rng.uniform(0, 9);//�������0-9�����������Ҫ���ڶ�λ�������ص���������  

				row = i + c_yoff[random]; //��λ�������ص��������� 
				if (row < 0)//�����ľ���Ҫ�����ж��Ƿ񳬳��߽�  
					row = 0;
				if (row >= _image.rows)
					row = _image.rows - 1;

				col = j + c_xoff[random];
				if (col < 0)//�����ľ���Ҫ�����ж��Ƿ񳬳��߽�  
					col = 0;
				if (col >= _image.cols)
					col = _image.cols - 1;

				m_samples[k].at<uchar>(i, j) = _image.at<uchar>(row, col);//����Ӧ������ֵ���Ƶ���������  
			}
		}
	}
}

/**************** Test a new frame and update model ********************/
void ViBe_BGS::testAndUpdate(const Mat _image)//�ж�ǰ���뱳���������б�������  
{
	RNG rng;

	for (int i = 0; i < _image.rows; i++)
	{
		for (int j = 0; j < _image.cols; j++)
		{
			int matches(0), count(0);
			double dist;

			while (matches < MIN_MATCHES && count < NUM_SAMPLES)//��������жϣ���ƥ��������ڷ�ֵMIN_MATCHES�������������������������  
			{
				dist = abs(m_samples[count].at<uchar>(i, j) - _image.at<uchar>(i, j));//��ǰ֡����ֵ���������е�ֵ���ȡ����ֵ   
				if (dist < RADIUS) //������ֵС�ڷ�ֵ�ǣ���ʾ��ǰ֡����������ֵ�е�����  
					matches++;
				count++; //ȡ����ֵ����һ��Ԫ�����Ƚ�  
			}

			if (matches >= MIN_MATCHES) //ƥ��������ڷ�ֵMIN_MATCHES����ʱ����ʾ��Ϊ����  
			{
				// It is a background pixel
				m_foregroundMatchCount.at<uchar>(i, j) = 0; //�����Ϊǰ���ĸ�����ֵΪ0  

				// Set background pixel to 0
				m_mask.at<uchar>(i, j) = 0;//�����ص�ֵҲΪ0  

				// ���һ�������Ǳ����㣬��ô���� 1 / defaultSubsamplingFactor �ĸ���ȥ�����Լ���ģ������ֵ
				int random = rng.uniform(0, SUBSAMPLE_FACTOR);//��1 / defaultSubsamplingFactor���ʸ��±���  
				if (random == 0)
				{
					random = rng.uniform(0, NUM_SAMPLES);
					m_samples[random].at<uchar>(i, j) = _image.at<uchar>(i, j);
				}

				// ͬʱҲ�� 1 / defaultSubsamplingFactor �ĸ���ȥ���������ھӵ��ģ������ֵ
				random = rng.uniform(0, SUBSAMPLE_FACTOR);
				if (random == 0)
				{
					int row, col;
					random = rng.uniform(0, 9);
					row = i + c_yoff[random];
					if (row < 0)//�����ľ���Ҫ�����ж��Ƿ񳬳��߽�  
						row = 0;
					if (row >= _image.rows)
						row = _image.rows - 1;

					random = rng.uniform(0, 9);
					col = j + c_xoff[random];
					if (col < 0) //�����ľ���Ҫ�����ж��Ƿ񳬳��߽�  
						col = 0;
					if (col >= _image.cols)
						col = _image.cols - 1;

					random = rng.uniform(0, NUM_SAMPLES);
					m_samples[random].at<uchar>(row, col) = _image.at<uchar>(i, j);
				}
			}
			else//ƥ�����С�ڷ�ֵMIN_MATCHES����ʱ����ʾ��Ϊǰ��  
			{
				// It is a foreground pixel
				m_foregroundMatchCount.at<uchar>(i, j)++;

				// Set background pixel to 255
				m_mask.at<uchar>(i, j) = 255;

				//���ĳ�����ص�����N�α����Ϊǰ��������Ϊһ�龲ֹ��������Ϊ�˶����������Ϊ������
				if (m_foregroundMatchCount.at<uchar>(i, j) > 50)
				{
					int random = rng.uniform(0, SUBSAMPLE_FACTOR);
					if (random == 0)
					{
						random = rng.uniform(0, NUM_SAMPLES);
						m_samples[random].at<uchar>(i, j) = _image.at<uchar>(i, j);
					}
				}
			}
		}
	}
}
