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


// 边集索引信息通过初始化时的edge获得
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

// 计算二阶导数矩阵
void CDetectEdge::CalSecDer(void)
{
	CmAssert(m_pSecDer != NULL);

	//pTmp[0], pTmp[1], pTmp[2]存放DERIV_RR, DERIV_RC, DERIV_CC方向上的二阶偏导
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

			pOrnt[nPos] = (float)atan2(-eigvec[0][1], eigvec[0][0]); //计算法线方向
			if (pOrnt[nPos] < 0.0f)
				pOrnt[nPos] += PI2;
			pSecDer[nPos] = float(eigval[0] > 0.0f ? eigval[0] : 0.0f);//计算二阶导数
		}
	}
}

// 计算一阶导数
void CDetectEdge::CalFirDer()
{
	CmAssert(m_pSecDer != NULL);

	//pTmp[0], pTmp[1] 存放DERIV_R, DERIV_C 方向上的二阶偏导
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

			//计算法线方向
			pOrnt[nPos] = (float)atan2(dy, dx); 
			if (pOrnt[nPos] < 0.0f)
				pOrnt[nPos] += PI2;

			pSecDer[nPos] = float(sqrt(dy*dy + dx*dx));//计算二阶导数
			nPos++;
		}
	}
}

// 非极大值抑制
void CDetectEdge::NoneMaximalSuppress(float linkEndBound /* = 0.01f */, float linkStartBound /* = 0.04f */)
{
	CmAssert(m_pSecDer != NULL && m_pLineInd != NULL);
	m_vStartPoint.clear();
	m_vStartPoint.reserve(int(0.08 * m_nHeight * m_nWidth));
	CLinePoint linePoint;

	//对于二阶导数大于一定值的点，如果其二阶导数值不比法线方向上其它点的小，那么他就是一个局部最大值，否则被抑制
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
				continue; //如果小于一定值就不用判断了

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
					pLineInd[nPos] = IND_NMS; //标记成非极大值抑制后剩余的点，其它点被抑制

					if (pSecDer[nPos] < linkStartBound)
						continue;

					//把这些点加入m_vStartPoint
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

	//按二阶导从大到小作为起始点link
	sort(m_vStartPoint.begin(), m_vStartPoint.end(), linePointGreater);
	vector<CLinePoint>::iterator itrStart;
	CEdge crtEdge;//当前边
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
/* 如果isForward为TRUE则沿着m_pOrnt方向寻找crtEdge, 并将沿途点的m_pNext */
/* 相应值置为沿途的方向值, 同时把m_pLineInd的值置为当前线的编号,找不到  */
/* 下一点的时候把最后一个点的坐标置为crtEdge的End坐标.                  */
/* 如果isForward为FALSE则沿着m_pOrnt反方向寻找crtEdge, 并将沿途点的     */
/* m_pNext相应值置为沿途的方向反向值, 同时把m_pLineInd的值置为当前线的  */
/* 编号.找不到下一点的时候如果pointNum太小则active置为false并推出.否则  */
/* 把最后一个点的坐标置为crtEdge的End坐标.                              */
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
		/*************按照优先级寻找下一个点，方向差异较大不加入**************/
		//下一个点在DIRECTION16最佳方向上找
		orntInd = int(ornt/EIGHTH_PI + 0.5f) % 16;
		if (jumpNext(x, y, ornt, crtEdge, orntInd, isBackWard)) 
			continue;
		//下一个点在DIRECTION8最佳方向上找
		orntInd = int(ornt/QUARTER_PI + 0.5f) % 8;
		if (goNext(x, y, ornt, crtEdge, orntInd, isBackWard)) 
			continue;
		//下一个点在DIRECTION16次优方向上找
		orntInd = int(ornt/EIGHTH_PI + 0.5f) % 16;
		nextInd1 = (orntInd + 1) % 16;
		nextInd2 = (orntInd + 15) % 16;
		if (angle(DRT_ANGLE[nextInd1], ornt) < angle(DRT_ANGLE[nextInd2], ornt)) {
			if(jumpNext(x, y, ornt, crtEdge, nextInd1, isBackWard))
				continue;
			if(jumpNext(x, y, ornt, crtEdge, nextInd2, isBackWard))
				continue;
		}
		else{//下一个点在DIRECTION16另一个方向上找
			if(jumpNext(x, y, ornt, crtEdge, nextInd2, isBackWard))
				continue;
			if(jumpNext(x, y, ornt, crtEdge, nextInd1, isBackWard))
				continue;
		}
		//下一个点在DIRECTION8次优方向上找
		orntInd = int(ornt/QUARTER_PI + 0.5f) % 8;
		nextInd1 = (orntInd + 1) % 8;
		nextInd2 = (orntInd + 7) % 8;
		if (angle(DRT_ANGLE[nextInd1], ornt) < angle(DRT_ANGLE[nextInd2], ornt)) {
			if(goNext(x, y, ornt, crtEdge, nextInd1, isBackWard))
				continue;
			if(goNext(x, y, ornt, crtEdge, nextInd2, isBackWard))
				continue;
		}
		else{//下一个点在DIRECTION8另一个方向上找
			if(goNext(x, y, ornt, crtEdge, nextInd2, isBackWard))
				continue;
			if(goNext(x, y, ornt, crtEdge, nextInd1, isBackWard))
				continue;
		}


		/*************按照优先级寻找下一个点，方向差异较大也加入**************/
		//下一个点在DIRECTION16最佳方向上找
		orntInd = int(ornt/EIGHTH_PI + 0.5f) % 16;
		if (jumpNext(x, y, ornt, crtEdge, orntInd, isBackWard, false)) 
			continue;
		//下一个点在DIRECTION8最佳方向上找
		orntInd = int(ornt/QUARTER_PI + 0.5f) % 8;
		if (goNext(x, y, ornt, crtEdge, orntInd, isBackWard, false)) 
			continue;
		//下一个点在DIRECTION16次优方向上找
		orntInd = int(ornt/EIGHTH_PI + 0.5f) % 16;
		nextInd1 = (orntInd + 1) % 16;
		nextInd2 = (orntInd + 15) % 16;
		if (angle(DRT_ANGLE[nextInd1], ornt) < angle(DRT_ANGLE[nextInd2], ornt)) {
			if(jumpNext(x, y, ornt, crtEdge, nextInd1, isBackWard, false))
				continue;
			if(jumpNext(x, y, ornt, crtEdge, nextInd2, isBackWard, false))
				continue;
		}
		else{//下一个点在DIRECTION16另一个方向上找
			if(jumpNext(x, y, ornt, crtEdge, nextInd2, isBackWard, false))
				continue;
			if(jumpNext(x, y, ornt, crtEdge, nextInd1, isBackWard, false))
				continue;
		}
		//下一个点在DIRECTION8次优方向上找
		orntInd = int(ornt/QUARTER_PI + 0.5f) % 8;
		nextInd1 = (orntInd + 1) % 8;
		nextInd2 = (orntInd + 7) % 8;
		if (angle(DRT_ANGLE[nextInd1], ornt) < angle(DRT_ANGLE[nextInd2], ornt)) {
			if(goNext(x, y, ornt, crtEdge, nextInd1, isBackWard, false))
				continue;
			if(goNext(x, y, ornt, crtEdge, nextInd2, isBackWard, false))
				continue;
		}
		else{//下一个点在DIRECTION8另一个方向上找
			if(goNext(x, y, ornt, crtEdge, nextInd2, isBackWard, false))
				continue;
			if(goNext(x, y, ornt, crtEdge, nextInd1, isBackWard, false))
				continue;
		}

		break;//如果ornt附近的三个方向上都没有的话,结束寻找
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
			//如果该点方向与当前线方向差别比较大则不加入/***********一个可变域值**********************/
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

			//更新切线方向
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
		int nPos2 = ty * m_nWidth + tx;  //DIRECTION16方向上的下一个点
		if (pLineInd[nPos2] == IND_NMS) {
			//如果该点方向与当前线方向差别比较大则不加入
			if (checkAngle && angle(ornt, pOrnt[nPos2]) > QUARTER_PI){
				//pLineInd[nPos2] = CHECKED_IND;
				return false;
			}


			/*************************************************************************/
			/* DIRECTION16方向上的orntInd相当于DIRECTION8方向上两个orntInd1,orntInd2 */
			/* 的叠加,满足orntInd = orntInd1 + orntInd2.此处优先选择使得组合上的点具 */
			/* IND_NMS标记的方向组合。(orntInd1,orntInd2在floor(orntInd/2)和         */
			/* ceil(orntInd/2)中选择                                                 */
			/*************************************************************************/
			int orntInd1 = orntInd >> 1, orntInd2;
			int nPos1 = x + DIRECTION8[orntInd1].x + m_nWidth * (y + DIRECTION8[orntInd1].y);
			if (pLineInd[nPos1] != IND_NMS && orntInd % 2) { //
				orntInd1 = (orntInd + 1) >> 1;
				orntInd1 %= 8;
				nPos1 = x + DIRECTION8[orntInd1].x + m_nWidth * (y + DIRECTION8[orntInd1].y);
			}

			if (pLineInd[nPos1] != IND_NMS && pLineInd[nPos1] != 0) { //当前nPos1点为其它线上的点，不能归入当前线
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