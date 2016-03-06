#pragma once

/************************************************************************/
/* CmFaceDetect detect human face using cascade algorithm with Haar		*/ 
/*		feature in OpenCV.                                              */
/************************************************************************/

class CmFaceDetect
{
public:
	CmFaceDetect(void);
	~CmFaceDetect(void);

	void detect(IplImage* img, vector<CvRect>& ret);
	void draw(IplImage* img, vector<CvRect>& ret);
	void detect_and_draw( IplImage* img );

	static void Demo();

private:
	CvMemStorage* storage;
	CvHaarClassifierCascade* cascade;
	IplImage* gray;
};
