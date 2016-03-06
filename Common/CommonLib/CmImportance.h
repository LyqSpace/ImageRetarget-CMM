#pragma once

class CmImportance
{
public:
	CmImportance();
	CmImportance(IplImage *base);
	~CmImportance(void);

	/************************************************************************/
	/* Calculate importance of image or video.                              */
	/* Input:																*/
	/*		im1_8U3C: current image											*/
	/*		im2_8U3C: nearby image used for calculate motion information	*/
	/*		weight[0]: weights of edge importance							*/
	/*		weight[1]: weights of face importance							*/
	/*		weight[2]: weights of motion importance							*/
	/*		weight[3]: weights of contrast importance						*/
	/*		weight[4]: minimal importance value								*/
	/* Output:																*/
	/*		An importance map with 32F1C format								*/
	/************************************************************************/
	IplImage* calcEnergy(const IplImage *im1_8U3C, double weight[5], IplImage *im2_8U3C = 0);

	void showEnergy();

	static void Demo(const char* videoFileName, double weights[5]);

private:

	IplImage *energymap8U3C;
	IplImage *sE32F, *sF32F, *sM32F, *sC32F, *s32F; // Edge, face, motion contrast and total difference
	IplImage *im32F3C, *tmp32F1, *tmp32F2, *gray8U;

	CmFaceDetect fd;

	void SetImage(const IplImage* srcImg8U3C);
	void release();

	// Fill edge difference energy to E
	void calcSE();
	void calcSF();

	void GetSalienceGray(const IplImage* inImg8U1C, IplImage* outImg32F1C);
};
