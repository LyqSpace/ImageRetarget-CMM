#include "StdAfx.h"
#include "MatMesh.h"
#include "MatEngine.h"

/************************************************************************/
/* Draw mesh using matlab functions and save image if necessary.        */
/* Input:																*/
/*		points: m*n*2 matrix representing nodes of mesh					*/
/*		saveName: name of image without extension.						*/
/************************************************************************/
void DrawMesh(const double* pointsX, const double* pointsY, int m, int n, const char* saveName /* = NULL */)
{
	mxArray* X = mxCreateDoubleMatrix(m, n, mxREAL);
	mxArray* Y = mxCreateDoubleMatrix(m, n, mxREAL);
	memcpy(mxGetPr(X), pointsX, m * n * sizeof(double));
	memcpy(mxGetPr(Y), pointsY, m * n * sizeof(double));

	MatEngine eg = MatEngineMan::GetEngine();
	eg.PutVariable("X", X);
	eg.PutVariable("Y", Y);
	
}