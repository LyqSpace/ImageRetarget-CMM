#include "StdAfx.h"
#include "NeighbourRegion.h"

bool moreNear(CPoint& pnt1, CPoint& pnt2)
{
	return (pnt1.x * pnt1.x + pnt1.y * pnt1.y) < (pnt2.x * pnt2.x + pnt2.y * pnt2.y);
}

CNeighbourRegion::CNeighbourRegion(int radius)
: m_nRgnPntNum(0)
, m_pNbrRgn(NULL)
{
	SetRgnRadius(radius);
}

CNeighbourRegion::~CNeighbourRegion(void)
{
	if (m_pNbrRgn) {
		delete []m_pNbrRgn;
		m_pNbrRgn = NULL;
	}
}

void CNeighbourRegion::SetRgnRadius(int radius)
{
	if (radius == m_nRadius)
		return;

	int r, c;
	vector<CPoint> positions;
	m_nRadius = radius;
	for (r = -m_nRadius; r <= m_nRadius; r++)
		for (c = -m_nRadius; c <= m_nRadius; c++) 
			if ((int)(sqrt(float(r * r + c * c)) + 0.5) <= radius) 
				positions.push_back(CPoint(c, r));

	sort(positions.begin(), positions.end(), moreNear);
	m_nRgnPntNum = (int)positions.size();
	if (m_pNbrRgn) 
		delete []m_pNbrRgn;
	m_pNbrRgn = new CPoint[m_nRgnPntNum];
	CmAssert(m_pNbrRgn);
	for(r = 0; r < m_nRgnPntNum; r++)
		m_pNbrRgn[r] = positions[r];
}

