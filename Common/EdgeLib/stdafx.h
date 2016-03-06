// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers



// TODO: reference additional headers your program requires here
#include "../CommonLib/CommonLib.h"

void* xmalloc(size_t size);
void *xcalloc(size_t nelem, size_t elsize);
void *xrealloc(void* old_ptr, size_t size);
void print_error(const char* message, const char* info);
void compute_eigenvals(double dfdrr, double dfdrc, double dfdcc, double eigval[2], double eigvec[2][2]);

#define ERR_NOMEM "Out of memory"
#define ERR_FNF   "Could not open file"
#define ERR_NOPGM "Not a valid pgm file:"
#define ERR_SNS   "Sigma not specified"
#define ERR_SOR   "Sigma out of range:"
#define ERR_LNS   "Low not specified"
#define ERR_LOR   "Low out of range:"
#define ERR_HNS   "High not specified"
#define ERR_HOR   "High out of range:"
#define ERR_LGH   "Low > High"
#define ERR_CNW   "Line position correction impossible without line width"
#define ERR_INP   "Include-image option requires PostScript output"
#define ERR_UKO   "Unknown option:"
#define ERR_TMF   "Too many files specified:"


extern float const QUARTER_PI;
extern float const EIGHTH_PI;
extern float const PI2;
extern float const FLOAT_PI;
extern float const HALF_PI;

extern CPoint const DIRECTION8[8];
extern CPoint const DIRECTION16[16];
extern float const DRT_ANGLE[8];

//near position array
#define MAX_STEP 7
#define NEAR_POS_DATA_LEN 177

//计算两个方向之间的夹角。两个ornt都必须在[0, 2*PI)之间, 返回值在[0, PI/2)之间
float angle(float ornt1, float orn2); 
void refreshOrnt(float& ornt, float& newOrnt);
