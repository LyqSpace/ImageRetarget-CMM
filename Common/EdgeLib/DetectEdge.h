#pragma once
#include "EdgeLib.h"

#define IND_NMS 0xffffffff
#define IND_SHORT_REMOVED 0xfffffffe

typedef struct CLinePoint
{
	int r, c;
	float value; 
}CLinePoint; //排序后用于link过程中起始点的寻找

typedef struct CLineInd{
	int ind, pointNum;
}CLineInd;

typedef struct CEdge{
	CEdge(void): active(true){}
	~CEdge(void){}

	int index; //边的序号
	CPoint start, end; //起始和终止节点
	float startOrnt, endOrnt; //start和end点处边线的延伸方向
	int pointNum; //边线的节点数目
	bool active; //边线状态标记, false表示该线已经被删除
	double width; //线条的宽度
}CEdge;

class CDetectEdge
{
public:
	CDetectEdge(vector<CEdge>& edge, double sigma = 1);
	~CDetectEdge(void);

	// 边集索引信息通过初始化时的edge获得
	void Initial(const IplImage* srcImg32FC1);

	// 计算二阶导数矩阵
	void CalSecDer(void);

	// 计算一阶导数
	void CalFirDer();

	// 非极大值抑制
	void NoneMaximalSuppress(float linkEndBound = 0.01f, float linkStartBound = 0.04f);

	// 连接
	void Link(int shortRemoveBound = 3);

	// 获取内部数据的指针
	const IplImage* GetDer(){ return m_pSecDer; }
	const IplImage* LineIdx() { return m_pLineInd; }
	const IplImage* NextMap() { return m_pNext; }
	const IplImage* Ornt() { return m_pOrnt; }

private:
	// 原图灰度矩阵
	IplImage* m_srcImg32FC1; 
	
	// 二阶导数矩阵, 32FC1
	IplImage* m_pSecDer; 

	// 切向矩阵, 32FC1
	IplImage* m_pOrnt;

	//点所属线编号矩阵, 32SC1, (accurly 16UC1 is enough)
	IplImage* m_pLineInd; 

	//下一点的方位编码, 32SC1, (accurly 8UC1 is enough)
	IplImage* m_pNext; 

	// 临时空间, 32FC1
	IplImage* m_pTmp[3];
	

	// 排序后用于link过程中起始点的寻找
	vector<CLinePoint> m_vStartPoint;
	
	// 边集
	vector<CEdge>& m_vEdge;

	//高斯过程中用
	double m_sigma;

	// 图像大小
	int m_nHeight, m_nWidth;

	//
	void ReleaseMemmory(void);

	void findEdge(int seedR, int seedC, CEdge& crtEdge, bool isBackWard);
	bool goNext(int& x, int& y, float& ornt, CEdge& crtEdge, int orntInd, bool isBackward, bool checkAngle = true);
	bool jumpNext(int& x, int& y, float& ornt, CEdge& crtEdge, int orntInd, bool isBackward, bool checkAngle = true);
};