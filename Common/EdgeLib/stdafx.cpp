// stdafx.cpp : source file that includes just the standard includes
// EdgeLib.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
#ifdef _DEBUG
#pragma comment(lib, "CommonLibd.lib")
#else
#pragma comment(lib, "Commonlib.lib")
#endif // _DEBUG


void* xmalloc(size_t size)
{
	void* ptr;

	if (size == 0)
		size = 1;
	ptr = malloc(size);
	if (ptr == NULL)
		print_error(ERR_NOMEM,NULL);
	return ptr;
}

void *xcalloc(size_t nelem, size_t elsize)
{
	void *ptr;

	if (elsize == 0)
		elsize = 1;
	if (nelem == 0)
		nelem = 1;
	ptr = calloc(nelem,elsize);
	if (ptr == NULL)
		print_error(ERR_NOMEM,NULL);
	return ptr;
}

void *xrealloc(void* old_ptr, size_t size)
{
	void *ptr;

	if (size == 0)
		size = 1;
	ptr = realloc(old_ptr,size);
	if (ptr == NULL)
		print_error(ERR_NOMEM,NULL);
	return ptr;
}

void print_error(const char* message, const char* info)
{
	if (info != NULL)
		fprintf(stderr,"%s: %s %s\n","detlines",message,info);
	else
		fprintf(stderr,"%s: %s\n","detlines",message);

	exit(1);
}

/* Compute the eigenvalues and eigenvectors of the Hessian matrix given by
dfdrr, dfdrc, and dfdcc, and sort them in descending order according to
their absolute values. */
void compute_eigenvals(double dfdrr, double dfdrc, double dfdcc, double eigval[2], double eigvec[2][2])
{
	double theta, t, c, s, e1, e2, n1, n2; /* , phi; */

	/* Compute the eigenvalues and eigenvectors of the Hessian matrix. */
	if (dfdrc != 0.0) {
		theta = 0.5*(dfdcc-dfdrr)/dfdrc;
		t = 1.0/(fabs(theta)+sqrt(theta*theta+1.0));
		if (theta < 0.0) t = -t;
		c = 1.0/sqrt(t*t+1.0);
		s = t*c;
		e1 = dfdrr-t*dfdrc;
		e2 = dfdcc+t*dfdrc;
	} else {
		c = 1.0;
		s = 0.0;
		e1 = dfdrr;
		e2 = dfdcc;
	}
	n1 = c;
	n2 = -s;

	/* If the absolute value of an eigenvalue is larger than the other, put that
	eigenvalue into first position.  If both are of equal absolute value, put
	the negative one first. */
	if (fabs(e1) > fabs(e2)) {
		eigval[0] = e1;
		eigval[1] = e2;
		eigvec[0][0] = n1;
		eigvec[0][1] = n2;
		eigvec[1][0] = -n2;
		eigvec[1][1] = n1;
	} else if (fabs(e1) < fabs(e2)) {
		eigval[0] = e2;
		eigval[1] = e1;
		eigvec[0][0] = -n2;
		eigvec[0][1] = n1;
		eigvec[1][0] = n1;
		eigvec[1][1] = n2;
	} else {
		if (e1 < e2) {
			eigval[0] = e1;
			eigval[1] = e2;
			eigvec[0][0] = n1;
			eigvec[0][1] = n2;
			eigvec[1][0] = -n2;
			eigvec[1][1] = n1;
		} else {
			eigval[0] = e2;
			eigval[1] = e1;
			eigvec[0][0] = -n2;
			eigvec[0][1] = n1;
			eigvec[1][0] = n1;
			eigvec[1][1] = n2;
		}
	}
}

float const static FLOAT_PI = 3.1415926535897932384626433832795f;
float const static QUARTER_PI = FLOAT_PI * 0.25f; 
float const static EIGHTH_PI = FLOAT_PI * 0.125f;
float const static PI2 = FLOAT_PI * 2.0f;
float const static HALF_PI = FLOAT_PI * 0.5f;


CPoint static const DIRECTION8[8] = {
	CPoint(1,  0), //Direction 0
	CPoint(1,  1), //Direction 1 
	CPoint(0,  1), //Direction 2
	CPoint(-1, 1), //Direction 3
	CPoint(-1, 0), //Direction 4
	CPoint(-1,-1), //Direction 5
	CPoint(0, -1), //Direction 6
	CPoint(1, -1)  //Direction 7
};  //format: {dx, dy}

CPoint static const DIRECTION16[16] = {
	CPoint(2,  0), //Direction 0
	CPoint(2,  1), //Direction 1 
	CPoint(2,  2), //Direction 2
	CPoint(1,  2), //Direction 3
	CPoint(0,  2), //Direction 4
	CPoint(-1, 2), //Direction 5
	CPoint(-2, 2), //Direction 6
	CPoint(-2, 1), //Direction 7
	CPoint(-2, 0), //Direction 8
	CPoint(-2,-1), //Direction 9 
	CPoint(-2,-2), //Direction 10
	CPoint(-1,-2), //Direction 11
	CPoint(0, -2), //Direction 12
	CPoint(1, -2), //Direction 13
	CPoint(2, -2), //Direction 14
	CPoint(2, -1)  //Direction 15
}; //format: {dx, dy}

float static const DRT_ANGLE[8] = {
	0.000000f,
	0.785398f,
	1.570796f,
	2.356194f,
	3.141593f,
	3.926991f,
	4.712389f,
	5.497787f
};

float angle(float ornt1, float orn2)
{//两个ornt都必须在[0, 2*PI)之间, 返回值在[0, PI/2)之间
	float agl = ornt1 - orn2;
	if (agl < 0)
		agl += PI2;
	if (agl >= FLOAT_PI)
		agl -= FLOAT_PI;
	if (agl >= HALF_PI)
		agl -= FLOAT_PI;
	return fabs(agl);
}

void refreshOrnt(float& ornt, float& newOrnt)
{
	static const float weightOld = 0.0f;
	static const float weightNew = 1.0f - weightOld;

	static const float largeBound = FLOAT_PI + HALF_PI;
	static const float smallBound = HALF_PI;

	if (newOrnt >= ornt + largeBound){
		newOrnt -= PI2;
		ornt = ornt * weightOld + newOrnt * weightNew;
		if (ornt < 0.0f)
			ornt += PI2;
	}
	else if (newOrnt + largeBound <= ornt){
		newOrnt += PI2;
		ornt = ornt * weightOld + newOrnt * weightNew;
		if (ornt >= PI2)
			ornt -= PI2;
	}
	else if (newOrnt >= ornt + smallBound){
		newOrnt -= FLOAT_PI;
		ornt = ornt * weightOld + newOrnt * weightNew;
		if (ornt < 0.0f)
			ornt += PI2;   
	}
	else if(newOrnt + smallBound <= ornt){
		newOrnt += FLOAT_PI;
		ornt = ornt * weightOld + newOrnt * weightNew;
		if (ornt >= PI2)
			ornt -= PI2;
	}
	else
		ornt = ornt * weightOld + newOrnt * weightNew;
	newOrnt = ornt;
}


