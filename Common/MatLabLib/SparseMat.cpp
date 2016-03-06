#include "stdafx.h"
#include "SparseMat.h"
#include "MatEngine.h"
//#include "base/ns.h"
//#include "base/debug_tool_c.h"
//BEGIN_NS(Base)
using namespace std;
//SparseMat
SparseMat::SparseMat(const char* fname) //load from file
{
	ifstream fin(fname);
	int ec;
	fin >> m >> n >> ec;
	elements.reserve(ec);
	for(int i = 0; i < ec; i++)
	{
		int _i,_j;
		double val;
		fin >> _i >> _j >> val;
		Add(_i,_j,val);
	}
}


//solve
//A must be sorted first!
//not thread safe!!!
// A has n rows and m columns
//
//      n
//    ©³©¥©·
//   m©§A ©§
//    ©»©¥©¿
//
void SolveSparse(const SparseMat& A, const vector<double>& b, /*out*/vector<double>& x, bool useLsqr, double* initGuess)
{
	//GD_ASSERTE(A.sorted);
	MatEngine eg = MatEngineMan::GetEngine();
	mwSize m = A.m;
	mwSize n = A.n;
	mwSize nz = A.elements.size();
	_ASSERTE(m == b.size());
	mwSize nzmax = A.elements.size();
	//b
	mxArray* barr = mxCreateDoubleMatrix(m, 1, mxREAL);
	double* pr = mxGetPr(barr);
	memcpy(pr, &b[0], m*sizeof(double));
	//init x
	mxArray* ixarr = NULL;
	if(initGuess != NULL)
	{
		ixarr = mxCreateDoubleMatrix(n, 1, mxREAL);
		double* pr = mxGetPr(ixarr);
		memcpy(pr, initGuess, n*sizeof(double));
	}
	//A
	{
		mwSize dims[] = {nz, 1};
		mxArray* iv = mxCreateDoubleMatrix(nz, 1, mxREAL);//mxCreateNumericArray(2, dims, mxINT32_CLASS, mxREAL);
		mxArray* jv = mxCreateDoubleMatrix(nz, 1, mxREAL);//mxCreateNumericArray(2, dims, mxINT32_CLASS, mxREAL);
		mxArray* sv = mxCreateDoubleMatrix(nz, 1, mxREAL);
		double* ivr =  mxGetPr(iv);//(int*)mxGetData(iv);
		double* jvr =  mxGetPr(jv);//(int*)mxGetData(jv);
		double* svr = mxGetPr(sv);
		for(size_t i = 0; i < nz; i++)
		{
			ivr[i] = A.elements[i].i+1;
			jvr[i] = A.elements[i].j+1;
			svr[i] = A.elements[i].value;
		}
		eg.PutVariable("iv", iv);
		eg.PutVariable("jv", jv);
		eg.PutVariable("sv", sv);
		char buf[1024];
		sprintf(buf, "A=sparse(iv,jv,sv,%d,%d);", m,n);
		eg.EvalString(buf);
		mxDestroyArray(iv);
		mxDestroyArray(jv);
		mxDestroyArray(sv);
	}
	
	//now open an engine
	eg.PutVariable("b", barr);
	if(useLsqr)
	{
		if(initGuess)
		{
			eg.PutVariable("ix", ixarr);
			eg.EvalString("x = lsqr(A,b,0,50,[],[],ix);");
		}
		else
		{
			eg.EvalString("x = lsqr(A,b,0,50,[],[]);");
		}
	}
	else
		eg.EvalString("x = A\\b;");
	mxArray* xarr = eg.GetVariable("x");
	double* xr = mxGetPr(xarr);
	x.resize(n);
	memcpy(&x[0], xr, n*sizeof(double));
	mxDestroyArray(barr);
	if(ixarr != NULL)
		mxDestroyArray(ixarr);
}
