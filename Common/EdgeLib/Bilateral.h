#pragma once

class CBilateral
{
public:
	~CBilateral(void);
	CBilateral();

	void initial(float deltaD, float deltaR, int imgHeight, int imgWidth, int size);
	void bilateralSmooth(const float* src, float* dst, int times);

private:
	int m_nSize;

	float m_fDeltaD;// 距离影响因子对应的方差
	float* m_pC; //距离差所对应的c项

	float m_fDeltaR;// 相似程度影响因子对应的方差
	float* m_pS;// 颜色差所对应的s项


	//待处理图像的尺寸
	int m_nWidth, m_nHeight;

	//处理图像期间的临时空间
	float* m_pSrc;
	float* m_pDst;
	int* m_pInt;
};
