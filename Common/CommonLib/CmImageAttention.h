#pragma once

class CmImageAttention
{
public:
	/************************************************************************/
	/* [CVPR 2007] Saliency Detection: A Spectral Residual Approach			*/	
	/* In place operation is supported										*/
	/* Return time used. Seconds											*/
	/************************************************************************/
	static double SpectralResidual(IN const IplImage* img32F, OUT IplImage* saliencyMap32F);

	/************************************************************************/
	/* [CVPR07] Learning to detect a salient object                         */
	/* Color spatial distribution											*/
	/* Return time used. Seconds											*/
	/************************************************************************/
	static double ColorSpatialDistribution(IN const IplImage* img32F3C, OUT IplImage* saliencyMap32F);

	/************************************************************************/
	/* [CVPR07] Learning to detect a salient object                         */
	/* Multi-scale contrast													*/
	/* Return time used. Seconds											*/
	/************************************************************************/
	static double MultiScaleContrast(IN const IplImage* img32F3C, OUT IplImage* saliencyMap32F);

	/************************************************************************/
	/* [CVPR07] Learning to detect a salient object                         */
	/* Center surround histogram											*/
	/* Return time used. Seconds											*/
	/************************************************************************/
	static double CenterSurroundHistogram(IN const IplImage* img8U3C, OUT IplImage* saliencyMap32F);

	/************************************************************************/
	/* Get saliency map by linear combine different features. The weight of */
	/* are defined by: w[]. The following features are used.				*/
	/* 0: Spectral residual													*/
	/* 1: Multi scale contrast												*/
	/* 2: Center-surround histogram											*/
	/* 3: Color spatial distribution                                        */
	/* Return time used. Seconds											*/
	/************************************************************************/
	static const int FEA_NUM = 4;
	static double GetSaliencyMap(IN const IplImage* img8U3C, OUT IplImage* saliencyMap32F
		, double w[4]);

	static double GetSaliencyMap(const char* inImgNames, const char* outDir, 
		double w[4]);

	static CvRect GetSaliencyRegion(IN const IplImage* saliency32F, IN double ratio = 1, 
		IN double threshold = 2, IN int step = 5);

	static void Demo();

private:
	static CvPoint GetBaryCenter(IN const IplImage* saliency32F);

};
