#pragma once
#include "EdgeLib.h"

#define IND_NMS 0xffffffff
#define IND_SHORT_REMOVED 0xfffffffe

typedef struct CLinePoint
{
	int r, c;
	float value; 
}CLinePoint; //���������link��������ʼ���Ѱ��

typedef struct CLineInd{
	int ind, pointNum;
}CLineInd;

typedef struct CEdge{
	CEdge(void): active(true){}
	~CEdge(void){}

	int index; //�ߵ����
	CPoint start, end; //��ʼ����ֹ�ڵ�
	float startOrnt, endOrnt; //start��end�㴦���ߵ����췽��
	int pointNum; //���ߵĽڵ���Ŀ
	bool active; //����״̬���, false��ʾ�����Ѿ���ɾ��
	double width; //�����Ŀ��
}CEdge;

class CDetectEdge
{
public:
	CDetectEdge(vector<CEdge>& edge, double sigma = 1);
	~CDetectEdge(void);

	// �߼�������Ϣͨ����ʼ��ʱ��edge���
	void Initial(const IplImage* srcImg32FC1);

	// ������׵�������
	void CalSecDer(void);

	// ����һ�׵���
	void CalFirDer();

	// �Ǽ���ֵ����
	void NoneMaximalSuppress(float linkEndBound = 0.01f, float linkStartBound = 0.04f);

	// ����
	void Link(int shortRemoveBound = 3);

	// ��ȡ�ڲ����ݵ�ָ��
	const IplImage* GetDer(){ return m_pSecDer; }
	const IplImage* LineIdx() { return m_pLineInd; }
	const IplImage* NextMap() { return m_pNext; }
	const IplImage* Ornt() { return m_pOrnt; }

private:
	// ԭͼ�ҶȾ���
	IplImage* m_srcImg32FC1; 
	
	// ���׵�������, 32FC1
	IplImage* m_pSecDer; 

	// �������, 32FC1
	IplImage* m_pOrnt;

	//�������߱�ž���, 32SC1, (accurly 16UC1 is enough)
	IplImage* m_pLineInd; 

	//��һ��ķ�λ����, 32SC1, (accurly 8UC1 is enough)
	IplImage* m_pNext; 

	// ��ʱ�ռ�, 32FC1
	IplImage* m_pTmp[3];
	

	// ���������link��������ʼ���Ѱ��
	vector<CLinePoint> m_vStartPoint;
	
	// �߼�
	vector<CEdge>& m_vEdge;

	//��˹��������
	double m_sigma;

	// ͼ���С
	int m_nHeight, m_nWidth;

	//
	void ReleaseMemmory(void);

	void findEdge(int seedR, int seedC, CEdge& crtEdge, bool isBackWard);
	bool goNext(int& x, int& y, float& ornt, CEdge& crtEdge, int orntInd, bool isBackward, bool checkAngle = true);
	bool jumpNext(int& x, int& y, float& ornt, CEdge& crtEdge, int orntInd, bool isBackward, bool checkAngle = true);
};