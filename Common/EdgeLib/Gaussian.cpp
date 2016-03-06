/************************************************************************/
/* Functions to compute the integral, and the 0th and 1st derivative of */
/*    the Gaussian function 1/(sqrt(2*PI)*sigma)*exp(-0.5*x^2/sigma^2)  */
/************************************************************************/
#include "StdAfx.h"
#include "Gaussian.h"
#include <stdlib.h>


#ifdef HAVE_ERFC

double CGaussian::normal(double x)
{
	return 0.5 * erfc(-x/SQRT2);
}

#else /* not HAVE_ERFC */

/* Compute the integral of the Gaussian, i.e., the normal distribution. */

#define SQRTPI 1.772453850905516027

#define UPPERLIMIT 20.0 

#define P10 242.66795523053175
#define P11 21.979261618294152
#define P12 6.9963834886191355
#define P13 -.035609843701815385
#define Q10 215.05887586986120
#define Q11 91.164905404514901
#define Q12 15.082797630407787
#define Q13 1.0

#define P20 300.4592610201616005
#define P21 451.9189537118729422
#define P22 339.3208167343436870
#define P23 152.9892850469404039
#define P24 43.16222722205673530
#define P25 7.211758250883093659
#define P26 .5641955174789739711
#define P27 -.0000001368648573827167067
#define Q20 300.4592609569832933
#define Q21 790.9509253278980272
#define Q22 931.3540948506096211
#define Q23 638.9802644656311665
#define Q24 277.5854447439876434
#define Q25 77.00015293522947295
#define Q26 12.78272731962942351
#define Q27 1.0

#define P30 -.00299610707703542174
#define P31 -.0494730910623250734
#define P32 -.226956593539686930
#define P33 -.278661308609647788
#define P34 -.0223192459734184686
#define Q30 .0106209230528467918
#define Q31 .191308926107829841
#define Q32 1.05167510706793207
#define Q33 1.98733201817135256
#define Q34 1.0

//正态分布函数的一种近似
double CGaussian::normal(double x)
{
	int    sn;
	double R1, R2, y, y2, y3, y4, y5, y6, y7;
	double erf, erfc, z, z2, z3, z4;
	double phi;

	if (x < -UPPERLIMIT) return 0.0;
	if (x > UPPERLIMIT) return 1.0;

	y = x / SQRT2;
	if (y < 0) {
		y = -y;
		sn = -1;
	} 
	else
		sn = 1;

	y2 = y * y;
	y4 = y2 * y2;
	y6 = y4 * y2;

	if (y < 0.46875) {
		R1 = P10 + P11 * y2 + P12 * y4 + P13 * y6;
		R2 = Q10 + Q11 * y2 + Q12 * y4 + Q13 * y6;
		erf = y * R1 / R2;
		if (sn == 1)
			phi = 0.5 + 0.5*erf;
		else 
			phi = 0.5 - 0.5*erf;
	} 
	else if (y < 4.0) {
		y3 = y2 * y;
		y5 = y4 * y;
		y7 = y6 * y;
		R1 = P20 + P21 * y + P22 * y2 + P23 * y3 + P24 * y4 + P25 * y5 + P26 * y6 + P27 * y7;
		R2 = Q20 + Q21 * y + Q22 * y2 + Q23 * y3 + Q24 * y4 + Q25 * y5 + Q26 * y6 + Q27 * y7;
		erfc = exp(-y2) * R1 / R2;
		if (sn == 1)
			phi = 1.0 - 0.5*erfc;
		else
			phi = 0.5*erfc;
	} 
	else {
		z = y4;
		z2 = z * z;
		z3 = z2 * z;
		z4 = z2 * z2;
		R1 = P30 + P31 * z + P32 * z2 + P33 * z3 + P34 * z4;
		R2 = Q30 + Q31 * z + Q32 * z2 + Q33 * z3 + Q34 * z4;
		erfc = (exp(-y2)/y) * (1.0 / SQRTPI + R1 / (R2 * y2));
		if (sn == 1)
			phi = 1.0 - 0.5*erfc;
		else 
			phi = 0.5*erfc;
	} 

	return phi;
}

#endif /* not HAVE_ERFC */


/* Integral of the Gaussian function */
double CGaussian::phi0(double x, double sigma)
{
	return normal(x/sigma);
}

/* The Gaussian function */
double CGaussian::phi1(double x,double sigma)
{
	double t;

	t = x/sigma;
	return SQRT_2_PI_INV/sigma*exp(-0.5*t*t);
}

/* First derivative of the Gaussian function */
double CGaussian::phi2(double x, double sigma)
{
	double t;

	t = x/sigma;
	return -x*SQRT_2_PI_INV/pow(sigma,3)*exp(-0.5*t*t);
}

/* Functions to compute the one-dimensional convolution masks of the 0th, 1st,
and 2nd derivative of the Gaussian kernel for a certain smoothing level
given by sigma.  The mask is allocated by the function and given as the
return value.  The caller must ensure that this memory is freed.  The
output is intended to be used as an array with range [-num:num].  Therefore,
the caller should add num to the return value.  Examples for the calling
sequence can be found in convolve_gauss.  Examples for the usage of the
masks are given in convolve_rows_gauss and convolve_cols_gauss. */

/* Gaussian smoothing mask */
double* CGaussian::compute_gauss_mask_0(int* num, double sigma)
{
	int   i, n;
	double *h, *mask;

	n = (int)MASK_SIZE(MAX_SIZE_MASK_0,sigma); /* Error < 0.001 on each side */
	h = (double*)xcalloc(2*n+1,sizeof(double));
	mask = h + n;
	for (i = -n+1;i < n;i++)
		mask[i] = phi0(-i+0.5,sigma) - phi0(-i-0.5,sigma);
	mask[-n] = 1.0 - phi0(n-0.5,sigma);
	mask[n] = phi0(-n+0.5,sigma);
	*num = n;
	return h;
}

/* First derivative of Gaussian smoothing mask */
double* CGaussian::compute_gauss_mask_1(int* num, double sigma)
{
	int   i, n;
	double limit;
	double *h, *mask;

	limit = MASK_SIZE(MAX_SIZE_MASK_1,sigma); /* Error < 0.001 on each side */
	n = (int)limit;
	h = (double*)xcalloc(2*n+1,sizeof(double));
	mask = h + n;
	for (i=-n+1;i<=n-1;i++)
		mask[i] = phi1(-i+0.5,sigma) - phi1(-i-0.5,sigma);
	mask[-n] = -phi1(n-0.5,sigma);
	mask[n] = phi1(-n+0.5,sigma);
	*num = n;
	return h;
}

/* Second derivative of Gaussian smoothing mask */
double* CGaussian::compute_gauss_mask_2(int* num, double sigma)
{
	int   i, n;
	double limit;
	double *h, *mask;

	limit = MASK_SIZE(MAX_SIZE_MASK_2,sigma); /* Error < 0.001 on each side */
	n = (int)limit;
	h = (double*)xcalloc(2*n+1,sizeof(double));
	mask = h + n;
	for (i=-n+1;i<=n-1;i++)
		mask[i] = phi2(-i+0.5,sigma) - phi2(-i-0.5,sigma);
	mask[-n] = -phi2(n-0.5,sigma);
	mask[n] = phi2(-n+0.5,sigma);
	*num = n;
	return h;
}


/* Convolve an image with the derivatives of a Gaussian smoothing kernel.
Since all of the masks are separable, this is done in two steps in the
function convolve_gauss.  Firstly, the rows of the image are convolved by
an appropriate one-dimensional mask in convolve_rows_gauss, yielding an
intermediate float-image h.  Then the columns of this image are convolved
by another appropriate mask in convolve_cols_gauss to yield the final
result k.  At the border of the image the gray values are mirrored. */

/* Convolve the rows of an image with the derivatives of a Gaussian. */
void CGaussian::convolve_rows_gauss(float* image, double* mask, int n, float* h, int width, int height)
{
	int      j, r, c, l;
	double    sum;

	/* Inner region */
	for (r=n; r<height-n; r++) {
		for (c=0; c<width; c++) {
			l = LINCOOR(r,c,width);
			sum = 0.0;
			for (j=-n;j<=n;j++)
				sum += (double)(image[l+j*width])*mask[j];
			h[l] = (float)sum;
		}
	}
	/* Border regions */
	for (r=0; r<n; r++) {
		for (c=0; c<width; c++) {
			l = LINCOOR(r,c,width);
			sum = 0.0;
			for (j=-n;j<=n;j++)
				sum += (double)(image[LINCOOR(BR(r+j),c,width)])*mask[j];
			h[l] = (float)sum;
		}
	}
	for (r=height-n; r<height; r++) {
		for (c=0; c<width; c++) {
			l = LINCOOR(r,c,width);
			sum = 0.0;
			for (j=-n;j<=n;j++)
				sum += (double)(image[LINCOOR(BR(r+j),c,width)])*mask[j];
			h[l] = (float)sum;
		}
	}
}



/* Convolve the columns of an image with the derivatives of a Gaussian. */
void CGaussian::convolve_cols_gauss(float* h, double* mask, int n, float* k, int width, int height)
{
	int      j, r, c, l;
	double    sum;

	/* Inner region */
	for (r=0; r<height; r++) {
		for (c=n; c<width-n; c++) {
			l = LINCOOR(r,c,width);
			sum = 0.0;
			for (j=-n;j<=n;j++)
				sum += h[l+j]*mask[j];
			k[l] = (float)sum;
		}
	}
	/* Border regions */
	for (r=0; r<height; r++) {
		for (c=0; c<n; c++) {
			l = LINCOOR(r,c,width);
			sum = 0.0;
			for (j=-n;j<=n;j++)
				sum += h[LINCOOR(r,BC(c+j),width)]*mask[j];
			k[l] = (float)sum;
		}
	}
	for (r=0; r<height; r++) {
		for (c=width-n; c<width; c++) {
			l = LINCOOR(r,c,width);
			sum = 0.0;
			for (j=-n;j<=n;j++)
				sum += h[LINCOOR(r,BC(c+j),width)]*mask[j];
			k[l] = (float)sum;
		}
	}
}



/* Convolve an image with a derivative of the Gaussian. */
void CGaussian::convolve_gauss(float* image, float* k, int width, int height, double sigma, int deriv_type)
{
	double  *hr, *hc;
	double  *maskr, *maskc;
	int    nr, nc;
	float   *h;

	h = (float*)xcalloc(width*height,sizeof(float));

	switch (deriv_type) {
	case DERIV_R:
		hr = compute_gauss_mask_1(&nr,sigma);
		hc = compute_gauss_mask_0(&nc,sigma);
		break;
	case DERIV_C:
		hr = compute_gauss_mask_0(&nr,sigma);
		hc = compute_gauss_mask_1(&nc,sigma);
		break;
	case DERIV_RR:
		hr = compute_gauss_mask_2(&nr,sigma);
		hc = compute_gauss_mask_0(&nc,sigma);
		break;
	case DERIV_RC:
		hr = compute_gauss_mask_1(&nr,sigma);
		hc = compute_gauss_mask_1(&nc,sigma);
		break;
	case DERIV_CC:
		hr = compute_gauss_mask_0(&nr,sigma);
		hc = compute_gauss_mask_2(&nc,sigma);
		break;
	}

	maskr = hr + nr;
	maskc = hc + nc;


	convolve_rows_gauss(image,maskr,nr,h,width,height);
	convolve_cols_gauss(h,maskc,nc,k,width,height);

	free(h);
	free(hr);
	free(hc);
}
