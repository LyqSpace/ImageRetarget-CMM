#pragma once

struct FaceDetector
{
	CvMemStorage* storage;
	CvHaarClassifierCascade* cascade;
	IplImage* gray;

	FaceDetector()
	{
		const char* xmls[] = {
			"haarcascade_frontalface_alt.xml",
			"haarcascade_frontalface_alt2.xml",
			"haarcascade_frontalface_alt_tree.xml",
			"haarcascade_frontalface_default.xml",
			"haarcascade_fullbody.xml",
			"haarcascade_lowerbody.xml",
			"haarcascade_profileface.xml",
			"haarcascade_upperbody.xml"
		};
		char buffer[1024];
		int ind = 1;
		sprintf(buffer, "D:\\library\\OpenCV\\data\\haarcascades\\%s", xmls[ind]);
		cascade = (CvHaarClassifierCascade*)cvLoad(buffer, 0, 0, 0);
		storage = cvCreateMemStorage(0);
		gray = 0;
	}

	~FaceDetector()
	{
		cvReleaseImage(&gray);
		cvReleaseHaarClassifierCascade(&cascade);
		cvReleaseMemStorage(&storage);
	}

	void detect(IplImage* img, vector<CvRect>& ret)
	{
		if (gray == 0 || gray->width != img->width || gray->height != img->height)
		{
			cvReleaseImage(&gray);
			gray = cvCreateImage(cvSize(img->width,img->height), 8, 1);
		}
		if (img->nChannels == 1)
		{
			cvCopy(img, gray);
		}
		else
		{
			cvConvertImage(img, gray);
		}
		//cvCvtColor(img, gray, CV_BGR2GRAY);
		cvEqualizeHist(gray, gray);
		cvClearMemStorage(storage);

		CvSeq* faces;
		
		const bool fast = false;
		if (fast)
		{
			faces = cvHaarDetectObjects( gray, cascade, storage,
				1.2, 2, CV_HAAR_DO_CANNY_PRUNING,
				cvSize(100, 100) );
		}
		else
		{
			/*
			faces = cvHaarDetectObjects( gray, cascade, storage,
				1.1, 2, 0,
				cvSize(30, 30) );
			*/
			faces = cvHaarDetectObjects( gray, cascade, storage,
				1.1, 2, CV_HAAR_DO_CANNY_PRUNING,
				cvSize(50, 50) );
		}
		ret.clear();
		for(int i = 0; i < (faces ? faces->total : 0); i++)
		{
			CvRect* r = (CvRect*)cvGetSeqElem(faces, i);
			if ( r->y+r->height/2 > 150 ) continue;
			if (r->height > 160) r->height=r->width=160; //MODIFY
			ret.push_back(*r);
		}
	}

	void draw(IplImage* img, vector<CvRect>& ret)
	{
		for(int i = 0; i < (int)ret.size(); i++)
		{
			CvRect *r = &ret[i];
			CvBox2D box;
			box.angle = 0;
			box.center = cvPoint2D32f((r->x + r->width*0.5), (r->y + r->height*0.5));
			box.size = cvSize2D32f(r->width, r->height);
			cvEllipseBox( img, box, CV_RGB(255, 0, 0) );
		}

	}

	void detect_and_draw( IplImage* img )
	{
		vector<CvRect> ret;
		detect(img, ret);
		draw(img, ret);
		cvNamedWindow( "result" );
		cvShowImage( "result", img );
	}


};