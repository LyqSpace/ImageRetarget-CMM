#pragma once

#include <matlablib.h>

class ConformalResizing
{
public:
	/************************************************************************/
	/* Resize an input image to new size h*w                                */
	/* Quads, qaud5s and edges 
	/************************************************************************/
	static IplImage* Resize(IplImage*& img8U, int h, int w, int meshQuadSize = 10);

private:
	/************************************************************************/
	/* A constrain unit with n points                                       */
	/************************************************************************/
	struct ConstrainUnits
	{
		ConstrainUnits();

		~ConstrainUnits();

		ConstrainUnits(const ConstrainUnits& r) :n(0),pnts(NULL),ind(NULL)
		{ *this = r;}
		ConstrainUnits& operator = (const ConstrainUnits& r);

		void Destory();

		void SetNumber(int _n);

		// Number of points in this constrain units. n = 4 for quad, n = 3 for 
		// qaud5, and n >= 3 for edge
		int n; 
		
		// Coordinates for each points
		CvPoint2D64f* pnts;

		// Points index
		int* ind;

		// Importance value of this unite
		double imp;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	static void ShowConstrains(const IplImage *pBackGround, 
		const vector<ConstrainUnits>& quads,
		const vector<ConstrainUnits>& qaud5s,
		const vector<ConstrainUnits>& edges, 
		const char *winName = "Constrains", 
		const int waite = 0,
		const char* saveName = NULL);

	/************************************************************************/
	/* Normalize image to have with and height be multiples of meshQuadSize */
	/************************************************************************/
	static inline void NormalizeImg(IplImage*& img, int meshQuadSize = 10);

	/************************************************************************/
	/* Get each constrain units												*/
	/* Output variable indicates the number of elements in constrain matrix	*/
	/************************************************************************/
	static CvSize GetConstrainUnits(const IplImage* impImg64F, //Importance map
		const IplImage* img8U3C,
		const CvSize szGrid, // Size of mesh represent by grid of each dimension
		vector<ConstrainUnits>& quads, // Quads units
		vector<ConstrainUnits>& qaud5s,  // Constrain unit of qaud5s
		vector<ConstrainUnits>& edges, // Constrain unit of edges
		vector<double>& ppos,/*added 2009.08.16*/
		int meshQuadSize
	); 

	/************************************************************************/
	/* Build constrained minimization problem								*/
	/*						min |A*X|^2.									*/
	/*						s.t. X(ind) = val								*/
	/************************************************************************/
	static void BuildConstrainsEquation(
		const vector<ConstrainUnits>& quads, // Quads units
		const vector<ConstrainUnits>& qaud5s,  // Constrain unit of qaud5s
		const vector<ConstrainUnits>& edges, // Constrain unit of edges
		CvSize szGrid,  // Size of mesh grid
		const CvSize newSize, // New image size
		SparseMat& A, 
		vector<int>&ind, 
		vector<double>& val
		);

	static void AddConstrain(const vector<ConstrainUnits>& units, SparseMat& A,
		bool recalculateM = true // Not need to be recalculated for square quads);
		);

	/************************************************************************/
	/* Solve constrained minimization problem								*/
	/*						min |A*X|^2.									*/
	/*						s.t. X(ind) = val								*/
	/************************************************************************/
	static void SolveConstrainedLeastSquare(SparseMat& A, 
		vector<int>& ind, 
		vector<double>& val,
		vector<double>& X, 
		const vector<double>& initval
	);

	/************************************************************************/
	/* This function finds constrains that an optimal constrained similarity*/
	/* transform of point set OrgPnts and its result b should be least square*/
	/* solution of															*/
	/*                                M*b=0									*/
	/* M should be released outside											*/
	/************************************************************************/
	static void Constrian(const ConstrainUnits& unit, CvMat*& M);

	static void RefreshPositions(vector<ConstrainUnits>& units, vector<double>& X);

	static void SaveConstrainUnits(const vector<ConstrainUnits>& units, const char* fileName);

	static IplImage* Warpping_dsp(const IplImage* pSrc8U3C, int meshQuadSize, const vector<double>& X, const CvSize dstSize);

	static IplImage* Warpping_old(const IplImage* pSrc8U3C, int meshQuadSize, const vector<double>& X, const CvSize dstSize);

	static IplImage* WarppingMLS(const IplImage* pSrc8U3C, int meshQuadSize, const vector<double>& X);

	static IplImage* Warpping(const IplImage* pSrc8U3C, int meshQuadSize, const vector<double>& X, const CvSize dstSize);
};
