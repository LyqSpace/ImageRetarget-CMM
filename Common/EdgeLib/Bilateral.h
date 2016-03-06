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

	float m_fDeltaD;// ����Ӱ�����Ӷ�Ӧ�ķ���
	float* m_pC; //���������Ӧ��c��

	float m_fDeltaR;// ���Ƴ̶�Ӱ�����Ӷ�Ӧ�ķ���
	float* m_pS;// ��ɫ������Ӧ��s��


	//������ͼ��ĳߴ�
	int m_nWidth, m_nHeight;

	//����ͼ���ڼ����ʱ�ռ�
	float* m_pSrc;
	float* m_pDst;
	int* m_pInt;
};
