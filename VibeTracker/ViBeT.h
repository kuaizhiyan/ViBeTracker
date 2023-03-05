#pragma once
#include<vector>
#include"Tracker.h"
using namespace std;

class ViBeT {
	/*
		���� ViBe ʵ�ָ��ٵ���(��� Sort ��)
	*/

	

public:
	ViBeT();													// Ĭ�Ϲ��캯��
	ViBeT(int _max_age, int _min_hits, float _iou_threshold);	// �Զ��幹�캯��

	int max_age;				// tracker ���������
	int min_hits;				// ��Сƥ�����
	float iou_threshold;		// ƥ����ֵ
	int frame_count;

	int FRAME_MAX_COL;			// ��¼�� frame �ĳߴ�
	int FRAME_MAX_ROW;

	vector<Tracker> trackers;	// ������� �ĸ�����

	vector<int> update( vector<bbox>& dets);		// �������м�����һ�θ��£����ؿ�����ʾ�� tracker ����
	bool isLeagal(const bbox& box);					// �ж� box �Ƿ�Ϸ�
};
