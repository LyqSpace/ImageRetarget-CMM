#define PRINT_PROCESS
#undef PRINT_PROCESS
#include "PardisoUnsym.h"

void PardisoUnsym::Init(int n) {
	this->n = n;
	A.resize(n);
	B.resize(n);
	x.resize(n);	

	ia.clear();
	ja.clear();
	a.clear();
	b.clear();
}

void PardisoUnsym::Translate() {	
	ia.resize(n+1);
	int s = 0;
	for (int i = 0; i < n; i++)
	{
		ia[i] = s + 1;
		A[i][i] = A[i][i]; // make sure a(i,i) exists
		s += A[i].size();
	}
	ia[n] = s + 1;
	ja.resize(s);
	a.resize(s);
	for (int i = 0; i < n; i++)
	{
		int k = ia[i] - 1;
		for (map<int, double>::iterator it = A[i].begin(); it != A[i].end(); it++, k++)
		{
			ja[k] = it->first + 1;
			a[k] = it->second;
		}
	}

	b.resize(n);
	for (int i = 0; i < B.size(); i++)
		b[i] = B[i];	
}

void PardisoUnsym::SetA(int r, int c, double d) {
	A[r][c] = d;
}

double PardisoUnsym::GetA(int r, int c) {
	return A[r][c];
}

void PardisoUnsym::SetB(int r, double d) {
	B[r] = d;
}

double PardisoUnsym::GetB(int r) {
	return B[r];
}

int PardisoUnsym::Solve() {
	this->Translate();

	int mtype = 11; /* Real unsymmetric matrix */    
	int nrhs = 1; /* Number of right hand sides. */        
	/* Internal solver memory pointer pt, */        
	/* 32-bit: int pt[64]; 64-bit: long int pt[64] */        
	/* or void *pt[64] should be OK on both architectures */        
	void *pt[64];        /* Pardiso control parameters.*/        
	int iparm[64];        
	int maxfct, mnum, phase, error, msglvl; 	/* Auxiliary variables.*/        
	int i;        
	double ddum; /* Double dummy */        
	int idum; /* Integer dummy. */

	/* --------------------------------------------------------------------*/
	/* .. Setup Pardiso control parameters.*/
	/* --------------------------------------------------------------------*/        
	for (i = 0; i < 64; i++) {                
		iparm[i] = 0;        
	}        
	iparm[0] = 1; /* No solver default */        
	iparm[1] = 2; /* Fill-in reordering from METIS */        
	/* Numbers of processors, value of OMP_NUM_THREADS */        
	iparm[2] = omp_get_max_threads();        
	iparm[3] = 0; /* No iterative-direct algorithm */        
	iparm[4] = 0; /* No user fill-in reducing permutation */        
	iparm[5] = 0; /* Write solution into x */        
	iparm[6] = 0; /* Not in use */        
	iparm[7] = 2; /* Max numbers of iterative refinement steps */        
	iparm[8] = 0; /* Not in use */        
	iparm[9] = 13; /* Perturb the pivot elements with 1E-13 */        
	iparm[10] = 1; /* Use nonsymmetric permutation and scaling MPS */        
	iparm[11] = 0; /* Not in use */        
	iparm[12] = 0; /* Not in use */        
	iparm[13] = 0; /* Output: Number of perturbed pivots */        
	iparm[14] = 0; /* Not in use */        
	iparm[15] = 0; /* Not in use */        
	iparm[16] = 0; /* Not in use */        
	iparm[17] = -1; /* Output: Number of nonzeros in the factor LU */        
	iparm[18] = -1; /* Output: Mflops for LU factorization */        
	iparm[19] = 0; /* Output: Numbers of CG Iterations */        
	maxfct = 1; /* Maximum number of numerical factorizations. */        
	mnum = 1; /* Which factorization to use. */        
	msglvl = 0; /* Print statistical information in file */        
	error = 0; /* Initialize error flag */
	/* --------------------------------------------------------------------*/
	/* .. Initialize the internal solver memory pointer. This is only */
	/* necessary for the FIRST call of the PARDISO solver. */
	/* --------------------------------------------------------------------*/       
	for (i = 0; i < 64; i++) {                
		pt[i] = 0;        
	}
	/* --------------------------------------------------------------------*/
	/* .. Reordering and Symbolic Factorization. This step also allocates */
	/* all memory that is necessary for the factorization. */
	/* --------------------------------------------------------------------*/        
	phase = 11;        
	PARDISO (pt, &maxfct, &mnum, &mtype, &phase,                
		&n, &a[0], &ia[0], &ja[0], &idum, &nrhs,                
		iparm, &msglvl, &ddum, &ddum, &error);        
	if (error != 0) {    
#ifdef PRINT_PROCESS
		printf("\nERROR during symbolic factorization: %d", error);                
#endif
		return 1;        
	}        
#ifdef PRINT_PROCESS
	printf("\nReordering completed ... ");        
	printf("\nNumber of nonzeros in factors = %d", iparm[17]);        
	printf("\nNumber of factorization MFLOPS = %d", iparm[18]);
#endif
	/* --------------------------------------------------------------------*/
	/* .. Numerical factorization.*/
	/* --------------------------------------------------------------------*/        
	phase = 22;        
	PARDISO (pt, &maxfct, &mnum, &mtype, &phase,                
		&n, &a[0], &ia[0], &ja[0], &idum, &nrhs,                
		iparm, &msglvl, &ddum, &ddum, &error);        
	if (error != 0) {                
#ifdef PRINT_PROCESS
		printf("\nERROR during numerical factorization: %d", error);                
#endif
		return 2;        
	} 
#ifdef PRINT_PROCESS
	printf("\nFactorization completed ... ");
#endif
	/* --------------------------------------------------------------------*/
	/* .. Back substitution and iterative refinement. */
	/* --------------------------------------------------------------------*/        
	phase = 33;        
	iparm[7] = 2; /* Max numbers of iterative refinement steps. */        
	/* Set right hand side to one. */        
	PARDISO (pt, &maxfct, &mnum, &mtype, &phase,                
		&n, &a[0], &ia[0], &ja[0], &idum, &nrhs,                
		iparm, &msglvl, &b[0], &x[0], &error);        
	if (error != 0) {                
#ifdef PRINT_PROCESS
		printf("\nERROR during solution: %d", error);                
#endif
		return 3;        
	} 

#ifdef PRINT_PROCESS
	printf("\nSolve completed ... ");        
	printf("\nThe solution of the system is: ");        
	for (i = 0; i < n; i++) {                
		printf("\n x [%d] = % f", i, x[i] );        
	}        
	printf ("\n");
#endif
	return 0;
}

void PardisoUnsym::Test() {
	Init(3);
	SetA(0, 0, 1);
	SetA(0, 2, 2);
	SetB(0, 3);

	SetA(1, 1, 1);
	SetB(1, 1);

	SetA(2, 0, 2);
	SetA(2, 2, 1);
	SetB(2, 3);

	Solve();
}

void PardisoUnsym::GetData(vector<int> &result) {
	result.resize(n);
	for (int i = 0; i < n; i++)
		result[i] = x[i];
}