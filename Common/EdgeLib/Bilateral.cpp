#include "StdAfx.h"
#include "Bilateral.h"
#include "EdgeLib.h"

CBilateral::CBilateral()
: m_fDeltaD(0)
, m_fDeltaR(0)
, m_pC(NULL)
, m_nSize(0)
, m_nWidth(0)
, m_nHeight(0)
, m_pSrc(NULL)
, m_pDst(NULL)
, m_pInt(NULL)
{
	m_pS = new float[255 * 2 + 1];
	CmAssert(m_pS);
	m_pS += 255;
}

CBilateral::~CBilateral(void)
{
	if (m_pC) {
		delete []m_pC;
		delete []m_pSrc;
		delete []m_pDst;
		delete []m_pInt;
	}
	m_pS -= 255; 
	delete m_pS;
}

//����֪��deltaֵ���г�ʼ����Ϊ����˫�����˲��ṩ�˲�����ֵ
void CBilateral::initial(float deltaD, float deltaR, int imgHeight, int imgWidth, int size)
{
	m_nSize = size;
	CmAssert(m_nSize <= min(imgHeight, imgWidth));
	m_nHeight = imgHeight;
	m_nWidth = imgWidth;

	//Ϊ��ʱͼ�����ڴ�ռ�
	int tmpImgSize = (imgHeight + (m_nSize - 1) * 2) * (imgWidth + (m_nSize - 1) * 2);
	if (m_pC){
		delete []m_pC;
		delete []m_pSrc;
		delete []m_pDst;
		delete []m_pInt;
	}
	m_pC = new float[(m_nSize * 2 - 1) * (m_nSize * 2 - 1)];//Ϊcolor similar��������ռ�
	m_pSrc = new float[tmpImgSize];
	m_pDst = new float[tmpImgSize];
	m_pInt = new int[tmpImgSize];
	memset(m_pSrc, 0, tmpImgSize * sizeof(float));
	memset(m_pDst, 0, tmpImgSize * sizeof(float));
	memset(m_pInt, 0, tmpImgSize * sizeof(int));

	//c(��-x)=e^(-1/2 ��(����-x��/��_d )��^2 )
	int r, c, k;
	double minVal = 1e8;
	for ( r = - m_nSize + 1, k = 0; r < m_nSize; r++)
		for (c = - m_nSize + 1; c < m_nSize; c++, k++)
			m_pC[k] = exp(-0.5f * (r * r + c * c) / (deltaD * deltaD));

	//s(f(��)-f(x) )=e^(-1/2 ��(|f(��)-f(x)|/��_r )��^2 ), �˴� r = f(��)-f(x)
	for (r = -255; r <= 255; r++)
		m_pS[r] = exp(-0.5f * r * r / (deltaR * deltaR));
}

//h(x)=k^(-1) ��_(-��)^�ޡ�_(-��)^�ޡ�f(��)c(��-x)s(f(��)-f(x) ) d_�� ��
//k(x)=��_(-��)^�ޡ�_(-��)^�ޡ�c(��-x)s(f(��)-f(x) ) d_�� ��
void CBilateral::bilateralSmooth(const float* src, float* dst, int times)
{
	CmAssert(times > 0);
	int r, c, i, j, nPos, nNewPos, innerPos;
	int addLen = m_nSize - 1;
	int addLen2 = addLen * 2;
	int newWidth = m_nWidth + addLen2;
	int newHeight = m_nHeight + addLen2;
	float *pS, *pTmp;
	float result, k, similar;

	//��ԭͼ�񿽱���m_pSrc��
	nNewPos = newWidth * addLen + addLen;
	for (r = 0, nPos = 0; r < m_nHeight; r++, nNewPos += addLen2)
		for (c = 0; c < m_nWidth; c++, nPos++, nNewPos++)
			m_pSrc[nNewPos] = src[nPos];

	while (times--) {


		//����m_pSrc��256ɫ��ʽ���Ա�ʵ��m_pS�Ŀ����±����
		for (nPos = newHeight * newWidth - 1; nPos >= 0; nPos--)
			m_pInt[nPos] = int(m_pSrc[nPos] * 255.0f + 0.5f);

		nNewPos = newWidth * addLen + addLen;
		//x(height, width) = (r, c)
		for (r = 0; r < m_nHeight; r++, nNewPos += addLen2) {
			for (c = 0; c < m_nWidth; c++, nNewPos++){
				pS = m_pS - m_pInt[nNewPos];
				innerPos = nNewPos - addLen * newWidth - addLen;
				result = k = 0.0f;
				//��(height, width) = (i, j)
				for (i = - addLen, nPos = 0; i <= addLen; i++, innerPos += m_nWidth - 1) {
					for (j = - addLen; j <= addLen; j++, nPos++, innerPos++) {
						similar = m_pC[nPos] * pS[m_pInt[innerPos]];
						k += similar;
						result += similar * m_pSrc[innerPos];
					}
				}
				m_pDst[nNewPos] = result/k;
			}
		}

		//swap(m_pSrc, m_pDst);
		pTmp = m_pSrc;
		m_pSrc = m_pDst;
		m_pDst = pTmp;
	}

	//��m_pSrc�е�ͼ�񿽱���ԭͼ��
	nNewPos = newWidth * addLen + addLen;
	for (r = 0, nPos = 0; r < m_nHeight; r++, nNewPos += addLen2)
		for (c = 0; c < m_nWidth; c++, nPos++, nNewPos++)
			dst[nPos] = m_pSrc[nNewPos];
}
