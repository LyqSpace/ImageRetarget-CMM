#pragma once

/************************************************************************/
/* Functions to compute the integral, and the 0th and 1st derivative of */
/*    the Gaussian function 1/(sqrt(2*PI)*sigma)*exp(-0.5*x^2/sigma^2)  */
/************************************************************************/
#ifndef PI
#define PI 3.14159265358979323846
#endif 

#ifndef SQRT2
#define SQRT2 1.41421356237309504880
#endif

#ifndef DERIV_R
#define DERIV_R  1  /* Derivative in row direction */
#define DERIV_C  2  /* Derivative in column direction */
#define DERIV_RR 3  /* Second derivative in row direction */
#define DERIV_RC 4  /* Second derivative in row and column direction */
#define DERIV_CC 5  /* Second derivative in column direction */
#endif

/* Mask sizes*/
#ifndef MAX_SIZE_MASK_0
#define MAX_SIZE_MASK_0  3.09023230616781    /* Size for Gaussian mask */
#define MAX_SIZE_MASK_1  3.46087178201605    /* Size for 1st derivative mask */
#define MAX_SIZE_MASK_2  3.82922419517181    /* Size for 2nd derivative mask */
#define MASK_SIZE(MAX,sigma) ceil(MAX*sigma) /* Maximum mask index */
#endif 

/* Translate row and column coordinates of an image into an index into its
one-dimensional array. */
#ifndef LINCOOR
#define LINCOOR(row,col,width) (int)((row)*(width)+(col))
#endif

/* Mirror the row coordinate at the borders of the image; height must be a
defined variable in the calling function containing the image height. */
#ifndef BR
#define BR(row) ((row) < 0 ? -(row) : \
	(row) >= height ? height - (row) + height - 2 : (row))
#endif

/* Mirror the column coordinate at the borders of the image; width must be a
defined variable in the calling function containing the image width. */
#ifndef BC
#define BC(col) ((col) < 0 ? -(col) : \
	(col) >= width ? width - (col) + width - 2 : (col))
#endif

/* Absolute value of x */
#ifndef ABS
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#endif

/* Sign of x */
#ifndef SGN
#define SGN(x) ((x) == 0 ? 0 : ((x) > 0 ? 1 : -1))
#endif

/* 1/sqrt(2*PI) */
#ifndef SQRT_2_PI_INV
#define SQRT_2_PI_INV 0.398942280401432677939946059935
#endif

class CGaussian
{
public:
	//正态分布函数的一种近似
	static double normal(double x); 

	/* Integral of the Gaussian function */
	static double phi0(double x, double sigma);

	/* The Gaussian function */
	static double phi1(double x,double sigma);

	/* First derivative of the Gaussian function */
	static double phi2(double x, double sigma);


	/* Functions to compute the one-dimensional convolution masks of the 0th, 1st,
	and 2nd derivative of the Gaussian kernel for a certain smoothing level
	given by sigma.  The mask is allocated by the function and given as the
	return value.  The caller must ensure that this memory is freed.  The
	output is intended to be used as an array with range [-num:num].  Therefore,
	the caller should add num to the return value.  Examples for the calling
	sequence can be found in convolve_gauss.  Examples for the usage of the
	masks are given in convolve_rows_gauss and convolve_cols_gauss. */

	/* Gaussian smoothing mask */
	static double *compute_gauss_mask_0(int* num, double sigma);

	/* First derivative of Gaussian smoothing mask */
	static double *compute_gauss_mask_1(int* num, double sigma);


	/* Second derivative of Gaussian smoothing mask */
	static double *compute_gauss_mask_2(int* num, double sigma);

	/* Convolve an image with the derivatives of a Gaussian smoothing kernel.
	Since all of the masks are separable, this is done in two steps in the
	function convolve_gauss.  Firstly, the rows of the image are convolved by
	an appropriate one-dimensional mask in convolve_rows_gauss, yielding an
	intermediate float-image h.  Then the columns of this image are convolved
	by another appropriate mask in convolve_cols_gauss to yield the final
	result k.  At the border of the image the gray values are mirrored. */

	/* Convolve the rows of an image with the derivatives of a Gaussian. */
	static void convolve_rows_gauss(float* image, double* mask, int n, float* h, int width, int height);

	/* Convolve the columns of an image with the derivatives of a Gaussian. */
	static void convolve_cols_gauss(float* h, double* mask, int n, float* k, int width, int height); 

	/* Convolve an image with a derivative of the Gaussian. */
	static void convolve_gauss(float* image, float* k, int width, int height, double sigma, int deriv_type);
};
