#pragma once
#include<opencv2\video\tracking.hpp>

#include<cmath>
#include<vector>
using namespace std;
typedef  pair<int, int> PII;
typedef	 pair<PII, PII> PIV;



struct bbox
{
	bbox(float _x1, float _y1, float _x2, float _y2) {
		x1 = _x1; y1 = _y1; x2 = _x2; y2 = _y2;
	}
	float x1, y1;		// box ���Ͻ�����, Ϊ�˾���ͳһ��Ϊ float����ʾע��תΪint
	float x2, y2;		// box ���½�����

	cv::Mat getMat();
};

struct zbox
{	
	zbox(float _x, float _y, float _s, float _r) {
		x = _x; y = _y; s = _s; r = _r;
	}
	float x, y;		// box ���ĵ����� 
	float s;			// box ���			w*h
	float r;			// box �ĺ��ݱ�		w/h

	cv::Mat getMat();
};




class Tracker {
	Tracker();				// Ĭ�Ϲ��캯��
	
	//~Tracker(void);		// ��������
	
public:
	Tracker(bbox box);		// ʹ�� bbox ����

	static int cnt;		// ��̬������ͳ�ƴ��ڵĸ��������������ڶ��壬�����ʼ�� 
		
	cv::KalmanFilter KF;	// Kalman Filter

	/* ��������������Ĳ��� */
	bool isInited = false;	// �Ƿ񱻳�ʼ��
	int id;
	int age = 0;			// ����
	int hits = 0;			// ���д���
	int hit_streak = 0;
	int time_since_update;	//
	vector<bbox> history; //�����ʷԤ��
	
	
	void update(bbox box);	// ����
	bbox predict();			// Ԥ��
	bbox get_state();		// ��ȡ��ǰ״̬��
};

zbox convert_bbox_to_zbox(const bbox& box);
bbox convert_z_to_bbox(const zbox& box);
zbox convert_mat_to_zbox(const cv::Mat& mat);
bbox convert_mat_to_bbox(const cv::Mat& mat);
cv::Mat convert_bbox_to_zmat(const bbox& box);
PIV bfs(int x1, int y1, const cv::Mat& mask);			// ����һ����ͨ����������½ǵ�
vector<bbox> getBboxs(const cv::Mat& mat,int area);		// �� ��Ĥ mask �л�ȡ������bbox
float iou(const bbox& box1,const bbox& box2);			// ������ box iou
void associate_detections_to_trackers(const vector<bbox>& detections, const vector<bbox>& trackers, vector<pair<int, int> >& _matches, vector<int>& _unmatched_detections, vector<int>& _unmatched_trackers, float iou_threshold);	// �������Ԥ������ƥ��
void drawBbox(cv::Mat& frame, const vector<bbox>& bboxes, bool isDet);		// �� frame �ϻ��� box  �� ������ʾ