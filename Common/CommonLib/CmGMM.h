#pragma once
/*
* Modified from this implementation:
* GrabCut implementation source code Copyright(c) 2005-2006 Justin Talbot
*
* All Rights Reserved.
* For educational use only; commercial use expressly forbidden.
* NO WARRANTY, express or implied, for this software.
*/

struct CmGaussian 
{
	Real mean[3];			// mean value
	Real covar[3][3];		// covariance matrix of the Gaussian
	Real det;				// determinant of the covariance matrix
	Real inv[3][3];			// inverse of the covariance matrix
	Real w;					// weighting of this Gaussian in the GMM.

	// These are only needed during Orchard and Bouman clustering.
	Real eValues[3];		// eigenvalues of covariance matrix
	Real eVectors[3][3];	// eigenvectors
};


/************************************************************************/
/* Gaussian mixture models                                              */
/************************************************************************/
class CmGMM
{
public:
	// Initialize GMM with the number of guassians desired
	CmGMM(int _K);
	~CmGMM(void);

	int K() const {return m_K; }
	const CmGaussian* GetGaussians() {return m_Guassians;}

	// Returns the probability density of color c in this GMM
	Real P(Real c[3]);

	// Returns the probability density of color c in just Gaussian k
	Real P(int i, Real c[3]);

	// Build the initial GMMs using the Orchard and Bouman color clustering algorithm
	// mask8U == NULL indicates all pixels need to be fitted
	void BuildGMMs(IN const IplImage* img32F3C, IN const IplImage* mask8U, OUT IplImage* components32S);

	// Iteratively refine GMM
	// mask8U == NULL indicates all pixels need to be fitted
	void RefineGMMs(IN const IplImage* img32F3C, IN const IplImage* mask8U, OUT IplImage* components32S);

	// Show GMM images
	void Show(const IplImage* components32S, const char* title = "GMM components", int waite = 0);

	// Show components
	void ShowComponents(const IplImage* src32F3C, const IplImage* components32S, const char* title = "GMM components", int waite = 0);

	static void Demo();

private:
	int m_K; // Number of guassians
	CmGaussian* m_Guassians; // An array of K guassians
};

