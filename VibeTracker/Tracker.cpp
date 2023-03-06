#pragma once
#include"Tracker.h"
#include<iostream>
#include<opencv2/video/tracking.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<queue>
#include<cstring>
#include <dlib/optimization/max_cost_assignment.h>

#define INF 0x3fffffff
using namespace std; 


short visit[4000][700];				// ���� bfs �з��ʿ��ƣ�Ϊ�����ظ������ڴ�����Ϊȫ�ֱ���


cv::Mat bbox::getMat()
{
	return (cv::Mat_<float>(4, 1) << x1, y1, x2, y2);
}
cv::Mat zbox::getMat()
{
	return (cv::Mat_<float>(4, 1) << x, y, s, r);
}

/* bbox / zbox / cv::Mat ֮���ת�� */
zbox convert_bbox_to_zbox(const bbox& box) {
	int w = box.x2 - box.x1;
	int h = box.y2 - box.y1;
	int x = box.x1 + w / 2.0;
	int y = box.y1 + h / 2.0;
	float s = w * h;
	float r = w / (float)h;
	return zbox(x, y, s, r);
}

bbox convert_z_to_bbox(const zbox& box) {
	float w = sqrt(box.s * box.r);		// sqrt(w*h * w/h)
	float h = box.s / w;				// w*h / w
	return bbox(box.x - w / 2.0, box.y - h / 2.0, box.x + w / 2.0, box.y + h / 2.0);
}

zbox convert_mat_to_zbox(const cv::Mat& mat) {
	// �� (4,1) ��������תΪ zbox
	return zbox(*mat.ptr<float>(0), *mat.ptr<float>(1), *mat.ptr<float>(2), *mat.ptr<float>(3));
}

bbox convert_mat_to_bbox(const cv::Mat& mat)
{
	return convert_z_to_bbox(convert_mat_to_zbox(mat));
}


cv::Mat convert_bbox_to_zmat(const bbox& box) {
	/* �� bbox ת��Ϊ zbox �� mat ��ʽ */
	return (convert_bbox_to_zbox(box)).getMat();
}


/* ���ߺ����� Mask ��ȡ�����е� bbox */

PIV bfs(int input_x, int input_y, const cv::Mat& mask) {
	/* ��һ��������� mask �� BFS ������ͨ�򣬼�¼�½ǵ� */

	// ���������mask ֵ Ϊ 255 �����ص�ſɽ��뱾�����������С�����Ϊ 1 �����ص�
	// ��ü�һ���Ƿ��жϣ����� ��255 ��������
	if (mask.at<uchar>(input_x, input_y) != 255) {
		cout << "Not a valid point !" << endl;
		return make_pair(make_pair(-1, -1), make_pair(-1, -1));
	}

	int x1 = 0x3fffffff, y1 = 0x3fffffff;	// ���ϵ�
	int x2 = -1, y2 = -1;	// ���µ�
	//short visit[4000][700];				// �����ļ�ͷ����Ϊȫ�ֱ���

	int dir[4][2] = { {0,-1},{1,0},{0,1},{0,-1} };



	// ��������
	queue<PII> qu;
	qu.push(make_pair(input_x, input_y));

	while (!qu.empty()) {
		// ��ȡ����Ԫ��
		int topx = (qu.front()).first;
		int topy = (qu.front()).second;

		// �жϸ��½ǵ�
		if (topx < x1) x1 = topx;
		if (topy < y1) y1 = topy;
		if (topx > x2) x2 = topx;
		if (topy > y2) y2 = topy;


		// ѭ�����
		for (int i = 0; i < 4; ++i) {
			int tmpx = topx + dir[i][0];
			int tmpy = topy + dir[i][1];

			if (tmpx >= 0 && tmpx < mask.rows && tmpy >= 0 && tmpy < mask.cols && !visit[tmpx][tmpy] && mask.at<uchar>(tmpx, tmpy) == 255) {
				qu.push(make_pair(tmpx, tmpy));
				visit[tmpx][tmpy] = 1;
			}
		};

		qu.pop();
	}


	return  make_pair(make_pair(x1, y1), make_pair(x2, y2));

};



vector<bbox> getBboxs(const cv::Mat& mat, int area) {

	vector<bbox> bboxs;
	memset(visit, 0, sizeof(visit));			// ���� visit

	for (int i = 0; i < mat.rows; ++i)
		for (int j = 0; j < mat.cols; ++j) {
			if (mat.at<uchar>(i, j) == 255 && !visit[i][j]) {
				PIV res = bfs(i, j, mat);
				if ((res.second.first - res.first.first) * (res.second.second - res.first.second) >= area)
					bboxs.push_back(bbox(res.first.first, res.first.second, res.second.first, res.second.second));
				//printf("LU:(%d,%d)\nRD:(%d,%d)\n\n", res.first.first, res.first.second, res.second.first, res.second.second);
			}
		}
	return bboxs;
}

float iou(const bbox& box1, const bbox& box2)
{
	float xx1 = max(box1.x1, box2.x1);
	float yy1 = max(box1.y1, box2.y1);
	float xx2 = min(box1.x2, box2.x2);
	float yy2 = min(box1.y2, box2.y2);
	float w = max((float)0, xx2 - xx1);
	float h = max((float)0, yy2 - yy1);
	float wh = w * h;						//�󽻼����
	//cout << "wh: " << wh << endl;
	float o = (float)wh / ((box1.x2 - box1.x1) * (box1.y2 - box1.y1) + (box2.x2 - box2.x1) * (box2.y2 - box2.y1) - wh);

	if (o <= 1e-9) return 0.0;
	return o;
}




int Tracker::cnt = 0;		//��̬��Ա��ʼ��

Tracker::Tracker() {

};
//
//Tracker::~Tracker(void) {
//
//};

Tracker::Tracker(bbox box) {
	/*
		ʹ�� bbox ����һ��������������
	*/


	const int stateNum = 7;		// ״̬����ά��Ϊ 7 [u,v,s,r,u',v',s']
	const int measureNum = 4;	// �۲�����ά��Ϊ 4 [u,v,s,r]
	this->KF = cv::KalmanFilter(stateNum, measureNum, 0);

	/*  ������������ʼ�� */
	float F[7][7] = {
		1,0,0,0,1,0,0,
		0,1,0,0,0,1,0,
		0,0,1,0,0,0,1,
		0,0,0,1,0,0,0,
		0,0,0,0,1,0,0,
		0,0,0,0,0,1,0,
		0,0,0,0,0,0,1
	};
	// ״̬ת�ƾ��� F
	this->KF.transitionMatrix = (cv::Mat_<float>(7, 7) << 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1);
	//cout << "F matrix in creater:\n" << KF.transitionMatrix << endl;
	cv::setIdentity(KF.measurementMatrix);			// �������� H ,����Ϊ�Խ���Ĭ��Ϊ 1
	//cout << "H matrix in creater:\n" << KF.measurementMatrix << endl;

	// ���²����ɽ�һ������
	cv::setIdentity(KF.processNoiseCov, cv::Scalar::all(1e-2));		// ϵͳ����������� Q
	cv::setIdentity(KF.measurementNoiseCov, cv::Scalar::all(10));	// ��������������� R

	cv::setIdentity(KF.errorCovPost, cv::Scalar::all(1));			// ����������Э������� P
	for (int i = 4; i < 7; ++i)	KF.errorCovPost.at<float>(i, i) = 1000.0;	// ����ʼ������

	/*  ����ԭʼ״̬�� */
	//KF.statePost = convert_bbox_to_zbox(box).getMat();
	cv::Mat tmp = convert_bbox_to_zbox(box).getMat();
	for (int i = 0; i < 4; ++i)
		KF.statePost.at<float>(i, 0) = tmp.at<float>(i, 0);


	/* ����������ʼ�� */
	this->time_since_update = 0;
	this->id = Tracker::cnt + 1;
	Tracker::cnt++;
	this->hits = this->hit_streak = this->age = 0;

}

bbox Tracker::predict() {
	// �������Ԥ��ֵС�� 0
	if (this->KF.statePost.at<float>(6, 0) + this->KF.statePost.at<float>(2, 0) <= 0.0)
		this->KF.statePost.at<float>(6, 0) *= 0.0;

	//cout << "KF.transitionMatrix\n" << KF.transitionMatrix << KF.transitionMatrix.type() << endl;
	//cout << "KF.statePost \n" << KF.statePost << KF.statePost.type() << endl;
	//cout << "KF.statePre\n" << KF.statePre << KF.statePre.type() << endl;
	//cout << "KF.errorCovPost\n" << KF.errorCovPost << KF.errorCovPost.type() << endl;
	//cout << "KF.processNoiseCov\n" << KF.processNoiseCov << KF.processNoiseCov.type() << endl;

	cv::Mat statePre = this->KF.predict();		// ���Ԥ���� 

	if (this->time_since_update > 0)	this->hit_streak = 0;		// ???
	this->time_since_update++;

	this->history.push_back(convert_z_to_bbox(convert_mat_to_zbox(statePre)));

	return history[history.size() - 1];

}

void Tracker::update(bbox box) {
	// ���ø��º�������ζ��ƥ��ɹ�
	this->time_since_update = 0;	// ����
	this->history.clear();

	this->hits++;
	this->hit_streak++;

	this->KF.correct(convert_bbox_to_zmat(box));	// update
}

bbox Tracker::get_state() {
	zbox z = zbox(*this->KF.statePost.ptr<float>(0), *this->KF.statePost.ptr<float>(1), *this->KF.statePost.ptr<float>(2), *this->KF.statePost.ptr<float>(3));
	return convert_z_to_bbox(z);
}


vector<vector<double> >  iou_batch(const vector<bbox>& detections, const vector<bbox>& trackers) {
	/*
	���� detections �� trackers  bboxs ���� iou ��������
	ͬʱȡ���ά�ȹ��� nr == nc �����飬���㲹�㣨Ϊ��ʹ�� max_cost_assignment ������

	input:
		detections: ����bbox vector
		trackers: Ԥ��� bbox vector
	output:
		iou ���������󣬶�ά vector

	*/


	int num_det = detections.size();
	int num_tra = trackers.size();
	int max_num = max(num_det, num_tra);

	// ������һΪ��ʱ���޷����� iou ƥ��ľ���ֱ�ӷ��ؿ�
	if (!num_det || !num_tra) {
		vector<vector<double> >  vec;
		return vec;
	}


	vector<vector<double> > vec(max_num, vector<double>(max_num));			// �����ձ�
	for (int i = 0; i < max_num; ++i)
		for (int j = 0; j < max_num; ++j) {
			if (i >= num_det) vec[i][j] = 0;
			else if (j >= num_tra) vec[i][j] = 0;
			else {
				vec[i][j] = iou(detections[i], trackers[j]);		// ��������֮��� iou ֵ
			}

		}

	return vec;
}

void associate_detections_to_trackers(const vector<bbox>& detections, const vector<bbox>& trackers, vector<pair<int, int> >& _matches, vector<int>& _unmatched_detections, vector<int>& _unmatched_trackers, float iou_threshold = 0.3) {

	/*
	������ bbox �� Ԥ�����й���ƥ��

	input:
		detections: ���� bbox ����
		trackers:	���ٿ� bbox ����
	output:
		ͨ�����õķ�ʽ����
		_matches: ƥ��ɹ��� detection/tracker ��Ŷ�
		_unmatched_deteciotns ƥ��ʧ�ܵ� detection ���
		_unmatched_trackers ƥ��ʧ�ܵ� tracker ���

	*/

	vector<pair<int, int> > vec;

	// �����ص���������
	vector<pair<int, int> > matched;			// max_sum_assignment ƥ��� detection/tracker ��Ŷ� 
	vector<pair<int, int> > matches;			// ������� detection/tracker ��Ŷ�
	vector<int> unmatched_detections;			// δƥ��ɹ��� detection ���
	vector<int> unmatched_trackers;				// δƥ��ɹ��� tracker ���

	/////////////////////////////////////////////////////
	if (trackers.empty()) {
		// û��Ԥ���ʱ��ֱ��ƥ��ʧ��
		// matches Ϊ��
		// unmatched_detections Ϊȫ��
		for (int i = 0; i < detections.size(); ++i)	unmatched_detections.push_back(i);
		// unmatched_trackers Ϊ��

		_matches = matches;
		_unmatched_detections = unmatched_detections;
		_unmatched_trackers = unmatched_trackers;

		return;

	}

	// ������ iou ���� ����Ϊ���Σ�
	vector<vector<double> > iou_matrix = iou_batch(detections, trackers);
	// ���� ��ֵ ɸѡ
	int max_num = iou_matrix.size();
	vector<int> vec2;


	/////////////                STEP 1 : ���� iou cost matrix               ///////////////

	// ����ƥ��ɹ������	
	if (max_num > 0) {

		// ������ֵ����ɸѡ
		/*for (int i = 0; i < max_num; ++i)
			for (int j = 0; j < max_num; ++j)
				if (iou_matrix[i][j] < iou_threshold) iou_matrix[i][j] = 0;*/

				// ʹ���������㷨���� cost ����dlib �м�����������ۣ�python �� linear_sum_assignment ����С��
		dlib::matrix<long> cost(max_num, max_num);

		// dlib �� max_cost_assignment ֻ�ܽ������ͣ���˽� iou * 1000 ��ȡ��
		for (int i = 0; i < max_num; ++i)
			for (int j = 0; j < max_num; ++j) {
				cost(i, j) = int(iou_matrix[i][j] * 1000);
			}

		// ���� 0,1,2.. ���ض�Ӧ���±�
		std::vector<long> assignment = max_cost_assignment(cost);





		/////////////                STEP 2 : matched  (����ƥ����)             /////////////
		// �� detection �ı�ż��룬���� (N,2) ������
		for (int i = 0; i < assignment.size(); ++i)	matched.push_back(make_pair(i, assignment[i]));




		/////////////                STEP 3 : unmatched_detections               /////////////
	/*
		����Ŀ�꼴Ϊƥ��ʧ�ܵ� ���� (unmatched_detections)
		�жϷ�����ƥ�䵽 �Ƿ� tracker �� detection ; iou С����ֵ�� detection
	*/
		for (int i = 0; i < detections.size(); ++i)
			if (matched[i].second >= trackers.size() || iou_matrix[matched[i].first][matched[i].second] < iou_threshold)
				unmatched_detections.push_back(i);






		/////////////                STEP 4 : unmatched_trackers               /////////////
		/*
			ƥ��ʧ�ܵĸ��ٿ� unmatched_trackers
			�жϷ�����ƥ�䵽�Ƿ� detection �� tracker;iou С����ֵ�� tracker

		*/
		for (int i = 0; i < matched.size(); ++i) {
			if (matched[i].second < trackers.size()) {
				if (matched[i].first >= detections.size() || iou_matrix[matched[i].first][matched[i].second] < iou_threshold)
					unmatched_trackers.push_back(matched[i].second);
			}
		}



		/////////////                STEP 5 : matches               /////////////
		/*
			�Ϸ���ƥ��� matches

		*/
		for (int i = 0; i < matched.size(); ++i) {
			if (matched[i].first < detections.size() && matched[i].second < trackers.size() && iou_matrix[matched[i].first][matched[i].second] >= iou_threshold)
				matches.push_back(matched[i]);
		}




	}
	else {
		// û��ƥ��ɹ����� matched ֱ��Ϊ��



		// detections ȫΪ unmatched_detections 
		for (int i = 0; i < detections.size(); ++i)	unmatched_detections.push_back(i);

		// trackers ȫΪ unmatched_trackers
		for (int i = 0; i < trackers.size(); ++i)	unmatched_trackers.push_back(i);

	}


	_matches = matches;
	_unmatched_detections = unmatched_detections;
	_unmatched_trackers = unmatched_trackers;

}

// ��ʾ
//int fontface = FONT_HERSHEY_COMPLEX_SMALL;
//const char* GTname = "GT";
//const char* Predictname = "Predict";
//for (int i = 0; i < vec.size(); ++i) {
//	cv::rectangle(frame, cv::Point(vec[i].y1, vec[i].x1), cv::Point(vec[i].y2, vec[i].x2), Scalar(255, 0, 0), 1, 1, 0);
//	cv::putText(frame, GTname, cv::Point(vec[i].y1, vec[i].x1 - 10), fontface, 0.5, cv::Scalar(255, 0, 0));
//}
//
//if (statePredict.x1 != 0.0) {
//	cv::rectangle(frame, cv::Point(statePredict.y1, statePredict.x1), cv::Point(statePredict.y2, statePredict.x2), Scalar(0, 255, 0), 1, 1, 0);
//	cv::rectangle(mask, cv::Point(statePredict.y1, statePredict.x1), cv::Point(statePredict.y2, statePredict.x2), Scalar(0, 255, 0), 1, 1, 0);
//	cv::putText(frame, Predictname, cv::Point(statePredict.y1, statePredict.x1 - 10), fontface, 0.5, cv::Scalar(0, 255, 0));
//	cv::putText(mask, Predictname, cv::Point(statePredict.y1, statePredict.x1 - 10), fontface, 0.5, cv::Scalar(0, 255, 0));
//}

void drawBbox(cv::Mat& frame, const vector<bbox>& bboxes, bool isDet) {
	/*
		�� bboxes ���� frame ��
		input:
			frame:��ǰ֡����������frame
			bboxes:�������� ����/Ԥ���
			isDet:�����־���������ּ���/Ԥ�����ɫ

	*/

	for (int i = 0; i < bboxes.size(); ++i) {
		int fontface = cv::FONT_HERSHEY_COMPLEX_SMALL;
		if (isDet) {
			const char* GTname = "GT";
			cv::rectangle(frame, cv::Point(bboxes[i].y1, bboxes[i].x1), cv::Point(bboxes[i].y2, bboxes[i].x2), cv::Scalar(255, 0, 0), 1, 1, 0);
			cv::putText(frame, GTname, cv::Point(bboxes[i].y1, bboxes[i].x1 - 10), fontface, 0.5, cv::Scalar(255, 0, 0));
		}
		else { 
			const char* GTname = "Predict";
			cv::rectangle(frame, cv::Point(bboxes[i].y1, bboxes[i].x1), cv::Point(bboxes[i].y2, bboxes[i].x2), cv::Scalar(0, 255 , 0), 1, 1, 0);
			cv::putText(frame, GTname, cv::Point(bboxes[i].y1, bboxes[i].x1 - 10), fontface, 0.5, cv::Scalar( 0,255, 0));
		}

	}


}