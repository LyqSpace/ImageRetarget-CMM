#pragma once
#include "array2.h"
#include "facedetect.h"

// ”√¿¥º∆À„ Importance Map
struct FastImportance
{
	IplImage *im, *gray, *energymap;
	ImageWrap<double> E;
	Array2<double> SF;
	Array2<double> SD; //difference
	FaceDetector fd;

	FastImportance(IplImage *base) 
	{
		im = cvCloneImage(base);
		energymap = cvCloneImage(base);
		gray = cvCreateImage(cvGetSize(im), IPL_DEPTH_8U, 1);
		SF.resize(im->height, im->width);
		SD.resize(im->height, im->width);
		E.reset(cvCreateImage(cvSize(im->width, im->height), IPL_DEPTH_64F, 1));
	}

	~FastImportance() 
	{ 
		E.release(); 
		cvReleaseImage(&energymap);
		cvReleaseImage(&im);
		cvReleaseImage(&gray);
	}

	double f(int x) { return abs(x); }

	void calcSE()
	{
		uchar* d = (uchar*)gray->imageData;
		for (int y = 0; y < gray->height; y++)
		{
			for (int x = 0; x < gray->width; x++)
			{
				const int c = 1;
				uchar *left = (x == 0) ? d+x*c : d+(x-1)*c;
				uchar *right = (x == gray->width-1) ? d+x*c : d+(x+1)*c;
				uchar *up = (y == 0) ? d+x*c : d+x*c-gray->widthStep;
				uchar *down = (y == gray->height-1) ? d+x*c : d+x*c+gray->widthStep;
				double v = sqrt(double((left[0]-right[0])*(left[0]-right[0])+(up[0]-down[0])*(up[0]-down[0])))/255;
				E[y][x] = v; //MODIFY
			}
			d += gray->widthStep;
		}
	}

	void calcSF()
	{
		vector<CvRect> ret;
		fd.detect(gray, ret);
		vector<CvPoint2D32f> center;
		vector<int> radius;
		vector<double> r1, r2;
		for (int i = 0; i < (int)ret.size(); i++)
		{
			center.push_back(cvPoint2D32f(ret[i].x+0.5*ret[i].width, ret[i].y+0.5*ret[i].height));
			radius.push_back(max(ret[i].width, ret[i].height));
			r1.push_back(-radius[i]*radius[i]*radius[i]+0.5*radius[i]*radius[i]);
			double rs = radius[i]/max(im->width, im->height);
			r2.push_back(1 - 2.5*rs*rs*rs*rs - 1.5*rs*rs);
		}
		SF.zero();
		for (int y = 0; y < im->height; y++)
		{
			for (int x = 0; x < im->width; x++)
			{
				for (int k = 0; k < (int)ret.size(); k++)
				{
					double dx = x-center[k].x, dy = y-center[k].y;
					double d = sqrt((dx*dx+dy*dy));
					double v = max(1 - (-d*d*d + 0.5*d*d)/r1[k], 0);
					SF[y][x] += v;
				}
			}
		}
	}

	void calcEnergy(IplImage *im1, IplImage *im2 = 0, bool DetectFace = true)
	{
		cvConvertImage(im1, gray);
		calcSE();
		if (DetectFace) 
			calcSF();
		double minc = 0.5/256;
		for (int y = 0; y < E.n; y++)
		{
			for (int x = 0; x < E.m; x++)
			{
				double v = E[y][x] + SF[y][x];
				if (v > 1) v = 1; else if (v < minc) v = minc;
				E[y][x] = v;
			}
		}
		if (im2 != 0)
		{
			for (int y = 0; y < E.n; y++)
			{
				for (int x = 0; x < E.m; x++)
				{
					double s = 0;
					for (int z = 0; z < 3; z++)
					{
						double v = im1->imageData[y*im1->widthStep+x*3+z] -
							im2->imageData[y*im2->widthStep+x*3+z];
						s+=v*v;
					}
					SF[y][x] = s;
				}
			}
		}
	}

	void showColorMap(IplImage* im, const char* name = "color")
	{
		ImageWrap<uchar> gray(cvCreateImage(cvGetSize(im), 8, 1));
		if (im->nChannels == 1) cvCopy(im, gray.ptr); 
		else cvConvertImage(im, gray.ptr);
		ImageWrap<uchar> hsv(cvCreateImage(cvGetSize(im), 8, 3));
		ImageWrap<uchar> rgb(cvCreateImage(cvGetSize(im), 8, 3));
		for (int y = 0; y < im->height; y++)
		{
			for (int x = 0; x < im->width; x++)
			{
				hsv[y][x*3+0] = gray[y][x];
				hsv[y][x*3+1] = 255;
				hsv[y][x*3+2] = 128;
			}
		}

		cvCvtColor(hsv.ptr, rgb.ptr, CV_HSV2BGR);
		cvNamedWindow(name); cvShowImage(name, rgb.ptr);
		cvSaveImage("e.bmp", rgb.ptr);
		rgb.release();
		gray.release();
		hsv.release();
	}

	void showEnergy()
	{
		ImageWrap<uchar> g(energymap);
		double m = maxof(E);
		for (int y = 0; y < im->height; y++)
		{
			for (int x = 0; x < im->width; x++)
			{
				g[y][x*3]=g[y][x*3+1]=g[y][x*3+2]=(uchar)(E[y][x]/m*255);
			}
		}
		cvNamedWindow("energy");
		cvShowImage("energy", energymap);

		showColorMap(g.ptr, "ecolor");
	}
};

