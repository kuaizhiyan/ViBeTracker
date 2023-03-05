#pragma once
#include"ViBeT.h"
#include<algorithm>

ViBeT::ViBeT()
{
	this->max_age = 5;
	this->min_hits = 3;
	this->iou_threshold = 0.3;
	this->frame_count = 0;
}

ViBeT::ViBeT(int _max_age=5, int _min_hits=2, float _iou_threshold=0.3) {
	this->max_age = _max_age;
	this->min_hits = _min_hits;
	this->iou_threshold = _iou_threshold;
	this->frame_count = 0;
}

// �ж� box �Ƿ�Ϸ����жϺ���
bool ViBeT::isLeagal(const bbox& box) {
	if (box.x1 < 0 || box.y1 < 0 || box.x2 < 0 || box.y2 < 0)	return false;
	if (box.x1 >= box.x2 || box.y1 >= box.y2)	return false;
	
	if (box.x1 >= this->FRAME_MAX_ROW || box.y1 >= this->FRAME_MAX_COL)	return false; 
	if (box.x2 > this->FRAME_MAX_ROW || box.y2 > this->FRAME_MAX_COL)	return false;

}

vector<int> ViBeT::update(vector<bbox>& dets) {
/*
	ViBeT ��������һ֡�м����� ���� ��һ�θ��£�������
		1.����Ԥ���ƥ��
		2.�����µĸ�����
		3.ɾ���ɵĸ�����
		4.����ƥ��ɹ��ĸ�����
	input: 
		dets: frame �м����� ���� �б�
	output:
		trks:�������������Ի滭�� Ԥ��� �б�



*/
	
	this->frame_count++;		// ����֡��+1
	vector<int> to_del;			// ��ɾ���� tracker ����б�
	vector<int> ret;			// ������ʾ�� tracker ����б�

	vector<bbox> trks;			// ���Ԥ��� ���ٿ�
	
	///////    STEP 1 : ���еĸ��������б� ȫ����һ��Ԥ��
	for (int i = 0; i < this->trackers.size(); ++i) {
		bbox trk = this->trackers[i].predict(); 
		trks.push_back(trk);
		if (!isLeagal(trk)) {
			// ��Ԥ�������Ϸ�����ֱ�Ӽ����ɾ�б�
			to_del.push_back(i);
		}
	}

	///////    STEP 2 : ɾ�� to_del �ж�Ӧ�ĸ����� ???? Ӧ���ŵ����ɾ�������� trks ��������� trackers ��������Խ��
	// ��ȫɾ������֤�� vector ����ǰɾ
	/*sort(to_del.begin(), to_del.end());
	for (int i = to_del.size() - 1; i >= 0; --i) {
		this->trackers.erase(this->trackers.begin() + to_del[i]);

	}*/

	///////    STEP 3 : �����Ԥ���ƥ��
	vector<PII> matched;
	vector<int> unmatched_dets;
	vector<int> unmatched_trks;
	associate_detections_to_trackers(dets, trks, matched, unmatched_dets, unmatched_trks,this->iou_threshold);

	///////    STEP 4 : ����ƥ��ɹ��ĸ�����
	for (int i = 0; i < matched.size(); ++i) {
		int trk_id = matched[i].second;
		int det_id = matched[i].first;
		this->trackers[trk_id].update(dets[det_id]);
	}

	///////    STEP 5 : Ϊƥ��ʧ�ܵļ��򴴽��µĸ�����
	for (int i = 0; i < unmatched_dets.size(); ++i) {
		this->trackers.push_back(Tracker(dets[i]));
	}

	///////    STEP 6 : ɸѡ��������ʾ�� Ԥ���
	// ɾ������������޵ĸ�����
	//vector<int> to_del2;
	for (int i = 0; i < this->trackers.size(); ++i) {
		if (this->trackers[i].time_since_update > this->max_age)
			to_del.push_back(i);
	}
	sort(to_del.begin(), to_del.end());
	for (int i = to_del.size() - 1; i >= 0; --i) {
		this->trackers.erase(this->trackers.begin() + to_del[i]);
	}

	// �������� �� ���д�������ģ����� ret
	for (int i = this->trackers.size() - 1; i >= 0; --i) {
		if (this->trackers[i].time_since_update < 1 && this->trackers[i].hit_streak >= this->min_hits)
			ret.push_back(i);
	}

	return ret;

}