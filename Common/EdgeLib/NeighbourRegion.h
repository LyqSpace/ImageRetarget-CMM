#pragma once

class CNeighbourRegion
{
public:
	CNeighbourRegion(int radius = 0);
	~CNeighbourRegion(void);
public:
	int m_nRadius, m_nRgnPntNum;
	void SetRgnRadius(int radius);
	CPoint* m_pNbrRgn;
};
