#include "StdAfx.h"
#include "CmShapeContext.h"

inline bool doubleEqual(double a, double b) { return fabs(a - b) < EPS;}

inline double sqr(double x){ return x * x; }

inline	double	sqrdist(const Point& A, const Point& B) { return  sqr(A.x - B.x) + sqr(A.y - B.y);}

inline	double	dist(const Point& A, const Point& B) { return sqrt( (double)sqrdist(A, B) ); }

inline	int	round(double x){ return int(x + 0.5); }


// 计算两个ShapeContext的对应cost
inline	double	shapeContextCost(const ShapeContext& A, const ShapeContext& B)
{
	double	ret = 0;
	double	ca = EPS, cb = EPS;
	for (int r = 0; r < SHAPE_CONTEXT_REGION; r ++)
		for (int s = 0; s < SHAPE_CONTEXT_SECTION; s ++)
			ca += A.h[r][s], cb += B.h[r][s];

	// Both no neighbors
	if (ca < 1 - EPS && cb < 1 - EPS) return 0;

	for (int r = 0; r < SHAPE_CONTEXT_REGION; r ++)
		for (int s = 0; s < SHAPE_CONTEXT_SECTION; s ++)
			if (A.h[r][s] || B.h[r][s])
				ret += sqr(A.h[r][s] / ca - B.h[r][s] / cb) / (A.h[r][s] / ca + B.h[r][s] / cb);
	return ret / 2.0;
}


void CmShapeContext::ShapeMatch(const char *imgName1, const char *imgName2, double &scCost, 
								double &affCost, bool bShow)
{
	IplImage* img1 = cvLoadImage(imgName1, CV_LOAD_IMAGE_GRAYSCALE);
	IplImage* img2 = cvLoadImage(imgName2, CV_LOAD_IMAGE_GRAYSCALE);

	ShapeMatch(img1, img2, scCost, affCost, bShow);

	cvReleaseImage(&img1);
	cvReleaseImage(&img2);
}

void CmShapeContext::ShapeMatch(const IplImage* img1_8U, const IplImage* img2_8U, double& scCost, 
								double& affCost, bool bShow)
{
	vector<Point> pA, pB;

	boundaryExtract(img1_8U, pA, N_SAMPLIING);	
	boundaryExtract(img1_8U, pB, N_SAMPLIING);

	if (pA.size() != N_SAMPLIING || pB.size() != N_SAMPLIING)
	{
		printf("#A = %d, #B = %d\n", pA.size(), pB.size());
		scCost = affCost = 1e8;
		return;
	}
	
	if (bShow)
	{
		IplImage* pImg1 = cvCreateImage(cvGetSize(img1_8U), IPL_DEPTH_8U, 3);
		IplImage* pImg2 = cvCreateImage(cvGetSize(img1_8U), IPL_DEPTH_8U, 3);
		cvZero(pImg1);
		cvZero(pImg2);
		cvNamedWindow("Sample points 1");
		cvNamedWindow("Sample points 2");

		for (unsigned int i = 0; i < pA.size(); i++)
		{
			cvCircle(pImg1, cvPoint(round(pA[i].x), round(pA[i].y)), 3, cvScalar(0, 0, 255));
			cvCircle(pImg2, cvPoint(round(pB[i].x), round(pB[i].y)), 3, cvScalar(0, 0, 255));
		}	

		cvShowImage("Sample points 1", pImg1);
		cvShowImage("Sample points 2", pImg2);
	}

	calcSimilarity(pA, pB , scCost, affCost, bShow);
}

void CmShapeContext::boundaryExtract(const IplImage *pImg, vector<Point> &point, int nSample)
{
	IplImage*	pCannyImg = cvCreateImage(cvGetSize(pImg), IPL_DEPTH_8U, 1);	
	cvCanny(pImg, pCannyImg, 200, 400, 3); // 边缘检测

	vector<Point> edgeList;
	for (int y = 0; y < pCannyImg->height; y++)
	{
		byte* data = (byte*)(pCannyImg->imageData + pCannyImg->widthStep * y);
		for (int x = 0; x < pCannyImg->width; x++)
		{
			if (data[x])
				edgeList.push_back(Point(x, y));
		}
	}

	int nStart = min(3 * nSample, (int)(edgeList.size()));
	if (edgeList.size() > (size_t)nStart)
	{
		random_shuffle(edgeList.begin(), edgeList.end());
		edgeList.resize(nStart);
	}

	cvReleaseImage(&pCannyImg);

	getSampling(edgeList, point, nSample);

}


inline	double	PointDist(const Point& A, const Point& B)
{
	return   sqr(A.x - B.x) + sqr(A.y - B.y);
}

void CmShapeContext::getSampling(const vector<Point>& src, vector<Point>& dst, int n)
{
	if (src.size() <= size_t(n))
	{
		dst = src;
		return;
	}

	int	m = src.size();
	int	i, j, cnt = 0;
	vector< pair<double, pair<int,int> > >	pwdist(m * m / 2);	//pairwise-distance	

	for (i = 0; i < m; i ++)
		for (j = i + 1; j < m; j ++)
			pwdist[cnt ++] = make_pair( PointDist(src[i], src[j]), make_pair(i,j) );

	pwdist.resize(cnt);
	sort(pwdist.begin(), pwdist.end());

	vector<bool>	alive(m, true);

	srand((unsigned int) time(0) );

	//printf("n = %d , src.size() = %d\n" , n , src.size());

	int	p = 0;
	//每次选择最近的点, 随机删掉一个
	for (i = src.size() - n; i > 0; i --)
	{
		while (! alive[pwdist[p].second.first] || ! alive[pwdist[p].second.second])
			p ++;
		if (rand() % 2)
			alive[pwdist[p].second.first] = false;
		else	alive[pwdist[p].second.second] = false;
	}

	dst.resize(n);
	p = 0;
	for (i = 0; i < m; i ++)
		if (alive[i])
			dst[p ++] = src[i];

}

void CmShapeContext::calcSimilarity(vector<Point> &pA, std::vector<Point> &pB, double &sc_cost, 
									double &aff_cost, bool showImage)
{

	//	Shape Context Cost --------------------------------
	vector<double> tA, tB;
	vector<ShapeContext>	scA, scB;
	vector<Point>	qA, qB;

	double	meanDistA = calcShapeContext(pA, qA, scA);
	double	meanDistB = calcShapeContext(pB, qB, scB);

	double	cost[N_TOT][N_TOT];

	int	i, j;	
	int	n = pA.size();
	int	m = pB.size();

	int	tot = round( (n > m ? n : m) * (1 + DUMMY_FRAC) );	// 用于匹配的总点数

	// calc cost matrix
	for (i = 0; i < tot; i ++)
		for (j = 0; j < tot; j ++)
			if (i >= n || j >= m)
				cost[i][j] = DUMMY_COST;
			else
				cost[i][j] = shapeContextCost(scA[i], scB[j]);

#ifdef SHOW_LOG
	// Show information for debugging
	printf("n = %d, m = %d, tot = %d\n" , n , m , tot);

	double	low, upp;
	low = upp = 0;
	for (i = 0; i < tot; i ++)
	{
		double	t = INFINITY;
		for (j = 0; j < tot; j ++)
			if (cost[i][j] < t) t = cost[i][j];
		low += t;
		upp += cost[i][i];
	}
	printf("Matching value : lower = %.5lf , upper = %.5lf\n" , low, upp);
#endif //SHOW_LOG

	int	perm[N_TOT];

	sc_cost = findBestMatching(cost, tot, perm);
	//	showCorrespondence(pA, pB, perm);

	double	row_lower, row_min;
	double	col_lower, col_min;

	row_lower = 0;
	for (i = 0; i < n; i ++)
	{
		row_min = INFINITY;
		for (j = 0; j < m; j ++)
			if (cost[i][j] < row_min) row_min = cost[i][j];
		row_lower += row_min;
	}
	col_lower = 0;
	for (j = 0; j < m; j ++)
	{
		col_min = INFINITY;
		for (i = 0; i < n; i ++)
			if (cost[i][j] < col_min) col_min = cost[i][j];
		col_lower += col_min;
	}
	row_lower /= n; col_lower /= m;
	sc_cost = max(row_lower, col_lower);

	//printf("sc_cost = %.3lf\n", sc_cost);


	//	Affine Cost --------------------------------
	vector<Point>	vpA, vpB;	//non-outerlier points(corresponding)
	for (i = 0; i < n; i ++)
	{
		if (perm[i] < m)
		{
			vpA.push_back( pA[i] );
			vpB.push_back( pB[ perm[i] ] );
		}
	}

	double	acx[N_SAMPLIING + 3];
	double	acy[N_SAMPLIING + 3];
	aff_cost = calcAffineCost( vpA, vpB , meanDistB * meanDistB * 1000.0, acx , acy);

	//printf("aff_cost = %.3lf\n" , aff_cost);

	if (showImage)
	{
		showCompare(pA, pB);
	}

	vector<Point>	target_pA;
	affineTransform(pA, vpA, target_pA, acx, acy);

	if (showImage)
	{
		showCompare(target_pA, pB);
	}

}

double CmShapeContext::calcShapeContext(const std::vector<Point> &list, std::vector<Point> &target, std::vector<ShapeContext> &sc)
{
	int	n = list.size();
	target.resize( n );
	sc.resize( n );

	double	meanDist = 0;
	int	i, j;

	for (i = 0; i < n; i ++)
		for (j = i + 1; j < n; j ++)
			meanDist += dist(list[i], list[j]);
	meanDist /= ( n * n / 2.0 );

	for (i = 0; i < n; i ++)
		target[i] = Point(list[i].x / meanDist, list[i].y / meanDist);

	double	d, lv;
	int	r, s;

	for (i = 0; i < n; i ++)
	{
		memset(sc[i].h, 0, sizeof(sc[i].h));
		for (j = 0; j < n; j ++)
		{
			if (i == j) continue;
			d = dist(target[i], target[j]);
			for (r = 0, lv = SHAPE_CONTEXT_R_INNER; r < SHAPE_CONTEXT_REGION; r ++, lv *= 2.0)
				if (d <= lv) break;
			if (r < SHAPE_CONTEXT_REGION)
			{
				double dy = target[j].y - target[i].y;
				double dx = target[j].x - target[i].x;
				d = atan2(dy, dx);
				for (s = 0, lv = - CV_PI + SHAPE_CONTEXT_SECTOR; s + 1 < SHAPE_CONTEXT_SECTION; s ++, lv += SHAPE_CONTEXT_SECTOR)
					if (d <= lv) break;

				sc[i].h[r][s] ++;
			}
		}
	}

	return meanDist;

}

double CmShapeContext::findBestMatching(const double cost[][N_TOT], int n, int cx[])
{
	int	cy[N_TOT];
	double	lx[N_TOT], ly[N_TOT];
	bool	mx[N_TOT], my[N_TOT];
	int	i, j;

	memset(ly, 0, sizeof(ly));
	for (i = 0; i < n; i ++)
	{
		lx[i] = INFINITY;
		for (j = 0; j < n; j ++)
			if (cost[i][j] < lx[i])
				lx[i] = cost[i][j];
	}

	memset(cx, 0xff, sizeof(cx));
	memset(cy, 0xff, sizeof(cy));
	for (i = 0; i < n; i ++)
	{
		memset(mx, 0, sizeof(mx));
		memset(my, 0, sizeof(my));
		while (! KMextendPath(i, cost, n, lx, ly, cx, cy, mx, my))
		{
			KMmodify(cost, n, lx, ly, mx, my);
			memset(mx, 0, sizeof(mx));
			memset(my, 0, sizeof(my));
		}
	}

	double	cost = 0;
	for (i = 0; i < n; i ++)
		cost += lx[i] + ly[i];

#ifdef SHOW_LOG
	printf("best Matching = %.6lf\n" , cost);
#endif

	return cost;
}

bool CmShapeContext::KMextendPath(int u, const double cost[][N_TOT], int n, double lx[], double ly[], int cx[], int cy[], bool mx[], bool my[])
{
	mx[u] = 1;
	for (int v = 0; v < n; v ++)
	{		
		if (doubleEqual(lx[u] + ly[v], cost[u][v]))
		{
			if (my[v]) continue;
			my[v] = 1;
			if (cy[v] < 0 || KMextendPath(cy[v], cost, n, lx, ly, cx, cy, mx, my))
				return cx[u] = v, cy[v] = u, true;
		}
	}
	return false;
}

void CmShapeContext::KMmodify(const double cost[][N_TOT], int n, double lx[], double ly[], bool mx[], bool my[])
{
	double	alpha = 1e20;

	int	i, j;
	for (i = 0; i < n; i ++)
		if (mx[i])
			for (j = 0; j < n; j ++)
				if (! my[j])
					if (cost[i][j] - lx[i] - ly[j] < alpha) alpha = cost[i][j] - lx[i] - ly[j];
	//printf("fix alpha = %.5lf\n" , alpha);
	for (i = 0; i < n; i ++)
		if (mx[i]) lx[i] += alpha;
	for (j = 0; j < n; j ++)
		if (my[j]) ly[j] -= alpha;
}

double CmShapeContext::calcAffineCost(const std::vector<Point> &pA, const std::vector<Point> &pB, double beta_k, double cx[], double cy[])
{
	if (pA.size() != pB.size())
	{
		fprintf(stderr, "Point set don't match!\n");
		return 0;
	}

	int	n = pA.size();

	CvMat	*L = cvCreateMat(n + 3, n + 3, CV_32FC1);
	CvMat	*invL = cvCreateMat(n + 3, n + 3, CV_32FC1);
	CvMat	*V = cvCreateMat(n + 3, 2, CV_32FC1);

	int	i, j;

	// Set L
	for (i = 0; i < n; i ++)
	{
		for (j = 0; j < n; j ++)
		{
			if (i == j)
			{
				cvmSet(L, i, j, beta_k);
			}
			else
			{
				double	d = sqrdist(pA[i], pB[i]);
				cvmSet(L, i, j, d * log(d));
			}
		}
	}

	for (i = 0; i < n; i ++)
	{
		cvmSet(L, i, n, 1.0);
		cvmSet(L, i, n+1, pA[i].x);
		cvmSet(L, i, n+2, pA[i].y);

		cvmSet(L, n, i, 1.0);
		cvmSet(L, n+1, i, pA[i].x);
		cvmSet(L, n+2, i, pA[i].y);
	}

	for (i = n; i < n + 3; i ++)
		for (j = n; j < n + 3; j ++)
			cvmSet(L, i, j, 0);

	// Set V
	for (i = 0; i < n; i ++)
	{
		cvmSet(V, i, 0, pB[i].x);
		cvmSet(V, i, 1, pB[i].y);
	}

	for (i = n; i < n + 3; i ++)
	{
		cvmSet(V, i, 0, 0);
		cvmSet(V, i, 1, 0);
	}

	cvInvert(L, invL);

	CvMat *C = cvCreateMat(n+3, 2, CV_32FC1);
	cvMatMul(invL, V, C);

	double	E = 0;
	for (int p = 0; p < 2; p ++)
	{
		double	val = 0;
		for (i = 0; i < n; i ++)
			for (j = 0; j < n; j ++)
				if (i != j)
					val += cvmGet(C, i, p) * cvmGet(L, i, j) * cvmGet(C, j, p);
		E += val;
	}
	E /= 2.0;

	CvMat	*A = cvCreateMat(2, 2, CV_32FC1);
	cvmSet(A, 0, 0, cvmGet(C, n + 1 , 0));
	cvmSet(A, 1, 0, cvmGet(C, n + 2 , 0));
	cvmSet(A, 0, 1, cvmGet(C, n + 1 , 1));
	cvmSet(A, 1, 1, cvmGet(C, n + 2 , 1));

	CvMat	*S = cvCreateMat(2, 1, CV_32FC1);
	cvSVD(A, S);

	double aff_cost = log(cvmGet(S, 0, 0) / cvmGet(S, 1, 0));

	for (i = 0; i < n + 3; i ++)
	{
		cx[i] = cvmGet(C, i, 0);
		cy[i] = cvmGet(C, i, 1);
	}

	return aff_cost;
}

void CmShapeContext::affineTransform(const std::vector<Point> &src, const std::vector<Point> &vp, std::vector<Point> &dst, double cx[], double cy[])
{
	double	aff	[N_SAMPLIING];
	double	wrp	[N_SAMPLIING];

	int	i , j;
	int	n = src.size();
	int	n_good = vp.size();
	double	d2, u;

	dst.resize(n);


	for (i = 0; i < n; i ++)
		aff[i] = cx[n_good] + cx[n_good + 1] * src[i].x + cx[n_good + 2] * src[i].y;

	memset(wrp, 0, sizeof(wrp));
	for (i = 0; i < n_good; i ++)
		for (j = 0; j < n; j ++)
		{
			d2 = sqrdist(vp[i], src[j]);
			u = d2 * log(d2 + EPS);
			wrp[j] += cx[i] * u;
		};
	for (i = 0; i < n; i ++)
		dst[i].x = aff[i] + wrp[i];

	for (i = 0; i < n; i ++)
		aff[i] = cy[n_good] + cy[n_good + 1] * src[i].x + cy[n_good + 2] * src[i].y;

	memset(wrp, 0, sizeof(wrp));
	for (i = 0; i < n_good; i ++)
		for (j = 0; j < n; j ++)
		{
			d2 = sqrdist(vp[i], src[j]);
			u = d2 * log(d2 + EPS);
			wrp[j] += cy[i] * u;
		};
	for (i = 0; i < n; i ++)
		dst[i].y = aff[i] + wrp[i];

}

void CmShapeContext::affineTestMain()
{
	CvMat* A=cvCreateMat(1,2,CV_32FC1);
	CvMat* B=cvCreateMat(2,1,CV_32FC1);
	cvmSet(A,0,0,1);
	cvmSet(A,0,1,2);
	cvmSet(B,0,0,3);
	cvmSet(B,1,0,4);
	CvMat* C = cvCreateMat(1,1,CV_32FC1);;

	cvMatMul(A,B,C);
	printf("%.3lf\n" , cvmGet(C,0,0));
}

void CmShapeContext::showCompare(const std::vector<Point> &pA, const std::vector<Point> &pB)
{
	int	size = 0;
	for (size_t i = 0; i < pA.size(); i ++)
	{
		size = (int)max(size, pA[i].x);
		size = (int)max(size, pA[i].y);
	}
	for (size_t i = 0; i < pB.size(); i ++)
	{
		size = (int)max(size, pB[i].x);
		size = (int)max(size, pB[i].y);
	}

	size += 10;	
	IplImage* img = cvCreateImage( cvSize(size, size), IPL_DEPTH_8U, 3 );

#ifdef SHOW_LOG
	printf("[log] Show Correspondence(size=%d), height=%d, width=%d\n", pA.size(), size, size);
#endif

	memset(img->imageData, 255, img->widthStep * img->height);

	for (size_t i = 0; i < pA.size() && i < pB.size(); i++)
		cvLineAA(img, cvPoint(round(pA[i].x), round(pA[i].y)), cvPoint(round(pB[i].x), round(pB[i].y)), 128);

	for (size_t i = 0; i < pA.size(); i ++)
		cvCircle(img, cvPoint(round(pA[i].x), round(pA[i].y)), 2, cvScalar(0,0,255,0), 2);
	for (size_t i = 0; i < pB.size(); i ++)
		cvCircle(img, cvPoint(round(pB[i].x), round(pB[i].y)), 2, cvScalar(0,0,0,0));

	cvNamedWindow("Correponsdence", CV_WINDOW_AUTOSIZE);
	cvShowImage( "Correponsdence", img ); 
	cvWaitKey(0);
	cvDestroyWindow( "Correponsdence" );
	cvReleaseImage( &img );
}
