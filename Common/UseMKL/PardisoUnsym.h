#ifndef _PARDISCO_UNSYM_H_
#define _PARDISCO_UNSYM_H_

#include <omp.h>
#include <mkl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <map>

extern int omp_get_max_threads();/* PARDISO prototype. */
#if defined(_WIN32) || defined(_WIN64)
#define pardiso_ PARDISO
#else
#define PARDISO pardiso_
#endif
extern int PARDISO        (void *, int *, int *, int *, int *, int *,        
						   double *, int *, int*, int *, int *, int *,        
						   int *, double *, double*, int *);

#pragma comment(lib, "libguide40.lib")
#pragma comment(lib, "mkl_c.lib")
#pragma comment(lib, "mkl_solver.lib")

using namespace std;

class PardisoUnsym {
private:

	vector< map<int, double> > A;
	vector<double> B;

	int n;        
	vector<int> ia;
	vector<int> ja;
	vector<double> a;
	vector<double> b;
	vector<double> x;

	void Translate();
public:	
	void Init(int n);	//	初始化行数

	void SetA(int r, int c, double d);		//	设置r行c列的数值	
	double GetA(int r, int c);
	void SetB(int r, double d);				//	设置r行的数值	
	double GetB(int r);
	
	int Solve();

	void GetData(vector<int> &result);

	void Test();
};

#endif//_PARDISCO_UNSYM_H_