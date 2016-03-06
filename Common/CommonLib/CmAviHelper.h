#pragma once
#include "CommonLib.h"

class CmAviHelper
{
public:
	CmAviHelper(const char *fileName);
	~CmAviHelper(void);

private:
	CvCapture* m_input;
	CvVideoWriter* m_output;
	string m_name;

	void OpenAVI(void);
	void CloseAVI(void);
public:
	int FrameNumber(void);
	CvSize GetSize(void);
	void SetSize(CvSize size, int fourcc = 0, int fps = 24);

	//The IplImage returned have to be deleted outside
	IplImage* CreateImage(int nChanels = 3, int depth = IPL_DEPTH_8U);
	IplImage* GetFrame(int frameNumber, IplImage* buf = NULL);
	void WriteFrame(IplImage* img, int fourcc = 0);

};