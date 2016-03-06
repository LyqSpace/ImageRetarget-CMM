#include "StdAfx.h"
#include "DetectEdge.h"

bool linePointGreater (const CLinePoint& elem1, const CLinePoint& elem2 )
{
	return elem1.value > elem2.value;
}

CDetectEdge::CDetectEdge(vector<CEdge>& edge, double sigma /* = 1 */)
: m_srcImg32FC1(NULL)
, m_sigma(sigma)
, m_vEdge(edge)
, m_pSecDer(NULL)
, m_pOrnt(NULL)
, m_pLineInd(NULL)
, m_pNext(NULL)
, m_nHeight(0)
, m_nWidth(0)
{
	CmAssert(sigma > 0.4);
	m_pTmp[0] = m_pTmp[1] = m_pTmp[2] = NULL;
}

CDetectEdge::~CDetectEdge(void)
{
	ReleaseMemmory();
}


// �߼�������Ϣͨ����ʼ��ʱ��edge���
void CDetectEdge::Initial(const IplImage* srcImg32FC1)
{
	// Manage memory
	ReleaseMemmory();
	m_srcImg32FC1 = cvCloneImage(srcImg32FC1);
	CvSize imgSize = cvGetSize(m_srcImg32FC1);
	m_nHeight = imgSize.height;
	m_nWidth = imgSize.width;
	m_pSecDer = cvCreateImage(imgSize, IPL_DEPTH_32F, 1);
	m_pOrnt = cvCreateImage(imgSize, IPL_DEPTH_32F, 1);
	m_pLineInd = cvCreateImage(imgSize, IPL_DEPTH_32S, 1);
	m_pNext = cvCreateImage(imgSize, IPL_DEPTH_32S, 1);
	m_pTmp[0] = cvCreateImage(imgSize, IPL_DEPTH_32F, 1);
	m_pTmp[1] = cvCreateImage(imgSize, IPL_DEPTH_32F, 1);
	m_pTmp[2] = cvCreateImage(imgSize, IPL_DEPTH_32F, 1);
}

void CDetectEdge::ReleaseMemmory(void)
{
	m_vEdge.clear();
	cvReleaseImage(&m_srcImg32FC1);
	cvReleaseImage(&m_pSecDer);
	cvReleaseImage(&m_pOrnt);
	cvReleaseImage(&m_pLineInd);
	cvReleaseImage(&m_pNext);
	cvReleaseImage(m_pTmp);
	cvReleaseImage(m_pTmp + 1);
	cvReleaseImage(m_pTmp + 2);
}

// ������׵�������
void CDetectEdge::CalSecDer(void)
{
	CmAssert(m_pSecDer != NULL);

	//pTmp[0], pTmp[1], pTmp[2]���DERIV_RR, DERIV_RC, DERIV_CC�����ϵĶ���ƫ��
	float* pSrcImg = (float*)m_srcImg32FC1->imageData;
	float* pTmp0 = (float*)(m_pTmp[0]->imageData);
	float* pTmp1 = (float*)(m_pTmp[1]->imageData);
	float* pTmp2 = (float*)(m_pTmp[2]->imageData);
	CGaussian::convolve_gauss(pSrcImg, pTmp0, m_srcImg32FC1->width, m_srcImg32FC1->height, m_sigma, DERIV_RR);
	CGaussian::convolve_gauss(pSrcImg, pTmp1, m_srcImg32FC1->width, m_srcImg32FC1->height, m_sigma, DERIV_RC);
	CGaussian::convolve_gauss(pSrcImg, pTmp2, m_srcImg32FC1->width, m_srcImg32FC1->height, m_sigma, DERIV_CC);


	int nPos, x, y;    
	double eigval[2], eigvec[2][2];
	float* pOrnt = (float*)(m_pOrnt->imageData);
	float* pSecDer = (float*)(m_pSecDer->imageData);
	for(y = 0; y < m_nHeight; y++){
		for(x = 0; x < m_nWidth; x++){
			nPos = x + y * m_nWidth;
			compute_eigenvals(pTmp0[nPos], pTmp1[nPos], pTmp2[nPos], eigval, eigvec);

			pOrnt[nPos] = (float)atan2(-eigvec[0][1], eigvec[0][0]); //���㷨�߷���
			if (pOrnt[nPos] < 0.0f)
				pOrnt[nPos] += PI2;
			pSecDer[nPos] = float(eigval[0] > 0.0f ? eigval[0] : 0.0f);//������׵���
		}
	}
}

// ����һ�׵���
void CDetectEdge::CalFirDer()
{
	CmAssert(m_pSecDer != NULL);

	//pTmp[0], pTmp[1] ���DERIV_R, DERIV_C �����ϵĶ���ƫ��
	float* pSrcImg = (float*)m_srcImg32FC1->imageData;
	float* pTmp0 = (float*)(m_pTmp[0]->imageData);
	float* pTmp1 = (float*)(m_pTmp[1]->imageData);
	CGaussian::convolve_gauss(pSrcImg, pTmp0, m_srcImg32FC1->width, m_srcImg32FC1->height, m_sigma, DERIV_R);
	CGaussian::convolve_gauss(pSrcImg, pTmp1, m_srcImg32FC1->width, m_srcImg32FC1->height, m_sigma, DERIV_C);

	int nPos = 0, x, y;    
	float* pOrnt = (float*)(m_pOrnt->imageData);
	float* pSecDer = (float*)(m_pSecDer->imageData);
	for(y = 0; y < m_nHeight; y++){
		for(x = 0; x < m_nWidth; x++){
			double dx = -pTmp0[nPos];
			double dy = pTmp1[nPos];

			//���㷨�߷���
			pOrnt[nPos] = (float)atan2(dy, dx); 
			if (pOrnt[nPos] < 0.0f)
				pOrnt[nPos] += PI2;

			pSecDer[nPos] = float(sqrt(dy*dy + dx*dx));//������׵���
			nPos++;
		}
	}
}

// �Ǽ���ֵ����
void CDetectEdge::NoneMaximalSuppress(float linkEndBound /* = 0.01f */, float linkStartBound /* = 0.04f */)
{
	CmAssert(m_pSecDer != NULL && m_pLineInd != NULL);
	m_vStartPoint.clear();
	m_vStartPoint.reserve(int(0.08 * m_nHeight * m_nWidth));
	CLinePoint linePoint;

	//���ڶ��׵�������һ��ֵ�ĵ㣬�������׵���ֵ���ȷ��߷������������С����ô������һ���ֲ����ֵ����������
	int r, c, nPosLine, nPos, xSgn, ySgn;
	float cosN, sinN;
	int* pLineInd = (int*)m_pLineInd->imageData;
	float* pSecDer = (float*)m_pSecDer->imageData;
	float* pOrnt = (float*)m_pOrnt->imageData;
	
	memset(pLineInd, 0, sizeof(int) * m_nWidth * m_nHeight);
	for (r = m_nHeight - 2; r > 0; r--){
		nPosLine = r * m_nWidth;
		for (c = m_nWidth - 2; c > 0; c--){
			nPos = nPosLine + c;
			if (pSecDer[nPos] < linkEndBound)  
				continue; //���С��һ��ֵ�Ͳ����ж���

			cosN = sin(pOrnt[nPos]);
			sinN = -cos(pOrnt[nPos]);
			xSgn = SGN(cosN);
			ySgn = SGN(sinN);
			cosN *= cosN;
			sinN *= sinN;
			if (pSecDer[nPos] >= (pSecDer[nPos + xSgn] * cosN + pSecDer[nPos + ySgn * m_nWidth] * sinN) 
				&& pSecDer[nPos] >= (pSecDer[nPos - xSgn] * cosN + pSecDer[nPos - ySgn * m_nWidth] * sinN)) {

					if (r < 100 && c < 100)
					{
						r *= 1;
					}
					pLineInd[nPos] = IND_NMS; //��ǳɷǼ���ֵ���ƺ�ʣ��ĵ㣬�����㱻����

					if (pSecDer[nPos] < linkStartBound)
						continue;

					//����Щ�����m_vStartPoint
					linePoint.c = c;
					linePoint.r = r;
					linePoint.value = pSecDer[nPos];
					m_vStartPoint.push_back(linePoint);
			}
		}
	}
}

void CDetectEdge::Link(int shortRemoveBound /* = 3 */)
{
	CmAssert(m_pSecDer != NULL && m_pLineInd != NULL);

	//�����׵��Ӵ�С��Ϊ��ʼ��link
	sort(m_vStartPoint.begin(), m_vStartPoint.end(), linePointGreater);
	vector<CLinePoint>::iterator itrStart;
	CEdge crtEdge;//��ǰ��
	int* pNext = (int*)m_pNext->imageData;
	int* pLineInd = (int*)m_pLineInd->imageData;
	float* pSecDer = (float*)m_pSecDer->imageData;
	memset(pNext, -1, sizeof(int) * m_nWidth * m_nHeight);
	m_vEdge.clear();
	m_vEdge.reserve(int(0.01 * m_nWidth * m_nHeight));
	int r, c;
	crtEdge.index = 1;
	for (itrStart = m_vStartPoint.begin(); itrStart != m_vStartPoint.end(); itrStart++)
	{
		r = itrStart->r;
		c = itrStart->c;
		if (pLineInd[r * m_nWidth + c] != IND_NMS)
			continue;

		findEdge(r, c, crtEdge, FALSE);
		findEdge(r, c, crtEdge, TRUE);
		if (crtEdge.pointNum <= shortRemoveBound) {
			CPoint point = crtEdge.start;
			int i, nextInd, nPos;
			for (i = 1; i < crtEdge.pointNum; i++) {
				nPos = point.x + point.y * m_nWidth;
				pLineInd[nPos] = IND_SHORT_REMOVED;
				nextInd = pNext[nPos];
				point += DIRECTION8[nextInd];
			}
			pLineInd[point.x + point.y * m_nWidth] = IND_SHORT_REMOVED;
		}
		else
		{
			m_vEdge.push_back(crtEdge);
			crtEdge.index++;
		}
	}
}

/************************************************************************/
/* ���isForwardΪTRUE������m_pOrnt����Ѱ��crtEdge, ������;���m_pNext */
/* ��Ӧֵ��Ϊ��;�ķ���ֵ, ͬʱ��m_pLineInd��ֵ��Ϊ��ǰ�ߵı��,�Ҳ���  */
/* ��һ���ʱ������һ�����������ΪcrtEdge��End����.                  */
/* ���isForwardΪFALSE������m_pOrnt������Ѱ��crtEdge, ������;���     */
/* m_pNext��Ӧֵ��Ϊ��;�ķ�����ֵ, ͬʱ��m_pLineInd��ֵ��Ϊ��ǰ�ߵ�  */
/* ���.�Ҳ�����һ���ʱ�����pointNum̫С��active��Ϊfalse���Ƴ�.����  */
/* �����һ�����������ΪcrtEdge��End����.                              */
/************************************************************************/
void CDetectEdge::findEdge(int seedR, int seedC, CEdge &crtEdge, bool isBackWard)
{
	float ornt = ((float*)m_pOrnt->imageData)[seedR * m_nWidth + seedC];
	if (isBackWard){
		ornt += FLOAT_PI;
		if (ornt >= PI2)
			ornt -= PI2;
	}
	else{
		crtEdge.active = true;
		crtEdge.pointNum = 1;
		((int*)m_pLineInd->imageData)[seedR * m_nWidth + seedC] = crtEdge.index;
	}

	int orntInd, nextInd1, nextInd2;
	int x = seedC, y = seedR;
	while (true) { 
		/*************�������ȼ�Ѱ����һ���㣬�������ϴ󲻼���**************/
		//��һ������DIRECTION16��ѷ�������
		orntInd = int(ornt/EIGHTH_PI + 0.5f) % 16;
		if (jumpNext(x, y, ornt, crtEdge, orntInd, isBackWard)) 
			continue;
		//��һ������DIRECTION8��ѷ�������
		orntInd = int(ornt/QUARTER_PI + 0.5f) % 8;
		if (goNext(x, y, ornt, crtEdge, orntInd, isBackWard)) 
			continue;
		//��һ������DIRECTION16���ŷ�������
		orntInd = int(ornt/EIGHTH_PI + 0.5f) % 16;
		nextInd1 = (orntInd + 1) % 16;
		nextInd2 = (orntInd + 15) % 16;
		if (angle(DRT_ANGLE[nextInd1], ornt) < angle(DRT_ANGLE[nextInd2], ornt)) {
			if(jumpNext(x, y, ornt, crtEdge, nextInd1, isBackWard))
				continue;
			if(jumpNext(x, y, ornt, crtEdge, nextInd2, isBackWard))
				continue;
		}
		else{//��һ������DIRECTION16��һ����������
			if(jumpNext(x, y, ornt, crtEdge, nextInd2, isBackWard))
				continue;
			if(jumpNext(x, y, ornt, crtEdge, nextInd1, isBackWard))
				continue;
		}
		//��һ������DIRECTION8���ŷ�������
		orntInd = int(ornt/QUARTER_PI + 0.5f) % 8;
		nextInd1 = (orntInd + 1) % 8;
		nextInd2 = (orntInd + 7) % 8;
		if (angle(DRT_ANGLE[nextInd1], ornt) < angle(DRT_ANGLE[nextInd2], ornt)) {
			if(goNext(x, y, ornt, crtEdge, nextInd1, isBackWard))
				continue;
			if(goNext(x, y, ornt, crtEdge, nextInd2, isBackWard))
				continue;
		}
		else{//��һ������DIRECTION8��һ����������
			if(goNext(x, y, ornt, crtEdge, nextInd2, isBackWard))
				continue;
			if(goNext(x, y, ornt, crtEdge, nextInd1, isBackWard))
				continue;
		}


		/*************�������ȼ�Ѱ����һ���㣬�������ϴ�Ҳ����**************/
		//��һ������DIRECTION16��ѷ�������
		orntInd = int(ornt/EIGHTH_PI + 0.5f) % 16;
		if (jumpNext(x, y, ornt, crtEdge, orntInd, isBackWard, false)) 
			continue;
		//��һ������DIRECTION8��ѷ�������
		orntInd = int(ornt/QUARTER_PI + 0.5f) % 8;
		if (goNext(x, y, ornt, crtEdge, orntInd, isBackWard, false)) 
			continue;
		//��һ������DIRECTION16���ŷ�������
		orntInd = int(ornt/EIGHTH_PI + 0.5f) % 16;
		nextInd1 = (orntInd + 1) % 16;
		nextInd2 = (orntInd + 15) % 16;
		if (angle(DRT_ANGLE[nextInd1], ornt) < angle(DRT_ANGLE[nextInd2], ornt)) {
			if(jumpNext(x, y, ornt, crtEdge, nextInd1, isBackWard, false))
				continue;
			if(jumpNext(x, y, ornt, crtEdge, nextInd2, isBackWard, false))
				continue;
		}
		else{//��һ������DIRECTION16��һ����������
			if(jumpNext(x, y, ornt, crtEdge, nextInd2, isBackWard, false))
				continue;
			if(jumpNext(x, y, ornt, crtEdge, nextInd1, isBackWard, false))
				continue;
		}
		//��һ������DIRECTION8���ŷ�������
		orntInd = int(ornt/QUARTER_PI + 0.5f) % 8;
		nextInd1 = (orntInd + 1) % 8;
		nextInd2 = (orntInd + 7) % 8;
		if (angle(DRT_ANGLE[nextInd1], ornt) < angle(DRT_ANGLE[nextInd2], ornt)) {
			if(goNext(x, y, ornt, crtEdge, nextInd1, isBackWard, false))
				continue;
			if(goNext(x, y, ornt, crtEdge, nextInd2, isBackWard, false))
				continue;
		}
		else{//��һ������DIRECTION8��һ����������
			if(goNext(x, y, ornt, crtEdge, nextInd2, isBackWard, false))
				continue;
			if(goNext(x, y, ornt, crtEdge, nextInd1, isBackWard, false))
				continue;
		}

		break;//���ornt���������������϶�û�еĻ�,����Ѱ��
	}

	if (isBackWard) {
		crtEdge.start.x = x;
		crtEdge.start.y = y;
		crtEdge.startOrnt = ornt;
	}
	else{
		crtEdge.end.x = x;
		crtEdge.end.y = y;
		crtEdge.endOrnt = ornt;
	} 
}

bool CDetectEdge::goNext(int &x, int &y, float &ornt, CEdge &crtEdge, int orntInd, bool isBackward, bool checkAngle)
{
	int nPos = y * m_nWidth + x;
	int tx = x + DIRECTION8[orntInd].x;
	int ty = y + DIRECTION8[orntInd].y;

	int* pLineInd = (int*)m_pLineInd->imageData;
	int* pNext = (int*)m_pNext->imageData;
	float* pOrnt = (float*)m_pOrnt->imageData;
	if (CHECK_IND(tx, ty)) {
		int nPos1 = ty * m_nWidth + tx;
		if (pLineInd[nPos1] == IND_NMS) {
			//����õ㷽���뵱ǰ�߷�����Ƚϴ��򲻼���/***********һ���ɱ���ֵ**********************/
			if (checkAngle && angle(ornt, pOrnt[nPos1]) > QUARTER_PI) { 
				//m_pLineInd[nPos1] = CHECKED_IND;
				return FALSE;
			}

			pLineInd[nPos1] = crtEdge.index;
			if (isBackward)
				pNext[nPos1] = (orntInd + 4) % 8;
			else
				pNext[nPos] = orntInd;
			crtEdge.pointNum++;

			//�������߷���
			refreshOrnt(ornt, pOrnt[nPos1]);
			x = tx;
			y = ty;
			return true;
		}
	}
	return false;

}

bool CDetectEdge::jumpNext(int &x, int &y, float &ornt, CEdge &crtEdge, int orntInd, bool isBackward, bool checkAngle)
{
	int* pLineInd = (int*)m_pLineInd->imageData;
	int* pNext = (int*)m_pNext->imageData;
	float* pOrnt = (float*)m_pOrnt->imageData;

	int nPos = y * m_nWidth + x;
	int tx = x + DIRECTION16[orntInd].x;
	int ty = y + DIRECTION16[orntInd].y;
	if (CHECK_IND(tx, ty)) {
		int nPos2 = ty * m_nWidth + tx;  //DIRECTION16�����ϵ���һ����
		if (pLineInd[nPos2] == IND_NMS) {
			//����õ㷽���뵱ǰ�߷�����Ƚϴ��򲻼���
			if (checkAngle && angle(ornt, pOrnt[nPos2]) > QUARTER_PI){
				//pLineInd[nPos2] = CHECKED_IND;
				return false;
			}


			/*************************************************************************/
			/* DIRECTION16�����ϵ�orntInd�൱��DIRECTION8����������orntInd1,orntInd2 */
			/* �ĵ���,����orntInd = orntInd1 + orntInd2.�˴�����ѡ��ʹ������ϵĵ�� */
			/* IND_NMS��ǵķ�����ϡ�(orntInd1,orntInd2��floor(orntInd/2)��         */
			/* ceil(orntInd/2)��ѡ��                                                 */
			/*************************************************************************/
			int orntInd1 = orntInd >> 1, orntInd2;
			int nPos1 = x + DIRECTION8[orntInd1].x + m_nWidth * (y + DIRECTION8[orntInd1].y);
			if (pLineInd[nPos1] != IND_NMS && orntInd % 2) { //
				orntInd1 = (orntInd + 1) >> 1;
				orntInd1 %= 8;
				nPos1 = x + DIRECTION8[orntInd1].x + m_nWidth * (y + DIRECTION8[orntInd1].y);
			}

			if (pLineInd[nPos1] != IND_NMS && pLineInd[nPos1] != 0) { //��ǰnPos1��Ϊ�������ϵĵ㣬���ܹ��뵱ǰ��
				return false;
			}
			orntInd2 = orntInd - orntInd1;
			orntInd2 %= 8;

			pLineInd[nPos1] = crtEdge.index;
			pLineInd[nPos2] = crtEdge.index;
			if (isBackward) {
				pNext[nPos1] = (orntInd1 + 4) % 8;
				pNext[nPos2] = (orntInd2 + 4) % 8;
			}
			else{
				pNext[nPos] = orntInd1;
				pNext[nPos1] = orntInd2;
			}
			crtEdge.pointNum += 2;

			refreshOrnt(ornt, pOrnt[nPos1]);
			refreshOrnt(ornt, pOrnt[nPos2]);
			x = tx;
			y = ty;
			return true;
		}
	}
	return false;

}