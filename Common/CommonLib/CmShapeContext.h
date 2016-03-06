#pragma once

const	double	SHAPE_CONTEXT_SECTOR  = CV_PI / 6;		// 每个扇区的角度
const	int	SHAPE_CONTEXT_REGION  =	5;		// 圆的区域数 <=0.125, 0.25, 0.5, 1, 2
const	int	SHAPE_CONTEXT_SECTION =	12;		// 扇区数目
const	int	N_SAMPLIING	  = 100;		// 边界取样点数
const	int	N_DUMMY		  = 25;
const	double	DUMMY_FRAC	  = 0.25;
const	int	N_TOT		  = N_SAMPLIING + N_DUMMY;
const	int	N_RANDOM_SAMPLING = 200;		// 为保证时间效率, 随机取样保留点数


const	double	EPS = 1e-8;				// Epsilon Value
const	double	INFINITY = 1e20;			// 无穷大

const	double 	SHAPE_CONTEXT_R_INNER =	0.125;
const	double	SHAPE_CONTEXT_R_OUTER =	2.0;

const	double	DUMMY_COST = 0.25;

struct	ShapeContext
{
	char	h[SHAPE_CONTEXT_REGION][SHAPE_CONTEXT_SECTION];
};

struct	Point 
{
	double	x, y;

	Point(){}
	Point(double x0, double y0) : x(x0), y(y0) {}
	Point(const Point& p0) : x(p0.x), y(p0.y) {}
};

class CmShapeContext
{
	static void boundaryExtract(const IplImage* pImg, vector<Point>& point, int nSample);

	static void	getSampling(const vector<Point>& src, vector<Point>& dst, int nSample); 

	static void calcSimilarity(vector<Point>& pA, vector<Point>& pB, double& sc_cost, double& aff_cost, bool showImage = 0);

	static double calcShapeContext(const vector<Point>& list, vector<Point>& target, vector<ShapeContext>& sc);

	static double findBestMatching(const double cost[N_TOT][N_TOT], int n, int cx[]);

	static bool KMextendPath(int u, const double cost[N_TOT][N_TOT], int n, double lx[], double ly[], int cx[], int cy[], bool mx[], bool my[]);
	
	static void KMmodify(const double cost[N_TOT][N_TOT], int n, double lx[], double ly[], bool mx[], bool my[]);

	static void showCompare(const vector<Point>& pA, const vector<Point>& pB);

	/************************************************************************/
	/*                        Affine cost block                             */
	/************************************************************************/
	static double calcAffineCost(const vector<Point>& pA, const vector<Point>& pB, double beta_k, double cx[], double cy[]);
	static void affineTransform(const vector<Point>& src, const vector<Point>& vp, vector<Point>& dst, double cx[] , double cy[]);
	static void affineTestMain();

public:
	static void ShapeMatch(const char* imgName1, const char* imgName2, double& scCost, 
		double& affCost, bool bShow = false);

	static void ShapeMatch(const IplImage* img1_8U, const IplImage* img2_8U, 
		double& scCost, double& affCost, bool bShow = false);
};
