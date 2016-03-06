#include "StdAfx.h"
#include "CmAviHelper.h"

CmAviHelper::CmAviHelper(const char *fileName)
: m_input(NULL)
, m_output(NULL)
{
	m_name = fileName;
}

CmAviHelper::~CmAviHelper(void)
{
	CloseAVI();
}

void CmAviHelper::OpenAVI(void)
{
	if (m_input == NULL)
	{
		m_input = cvCaptureFromFile(m_name.c_str());
		if (m_input == NULL)
		{
			assert(m_input != NULL);
			throw invalid_argument("Can't open avi file");
			return;
		}
	}
}

void CmAviHelper::CloseAVI(void)
{
	if (m_input)
		cvReleaseCapture(&m_input);
	if (m_output)
		cvReleaseVideoWriter(&m_output);

}

int CmAviHelper::FrameNumber(void)
{
	OpenAVI();
	cvSetCaptureProperty(m_input, CV_CAP_PROP_POS_AVI_RATIO, 1);
	return (int)cvGetCaptureProperty(m_input, CV_CAP_PROP_POS_FRAMES);
}

CvSize CmAviHelper::GetSize(void)
{
	CvSize sz;
	OpenAVI();
	sz.height = (int)cvGetCaptureProperty(m_input, CV_CAP_PROP_FRAME_HEIGHT);
	sz.width = (int)cvGetCaptureProperty(m_input, CV_CAP_PROP_FRAME_WIDTH);
	return sz;
}

void CmAviHelper::SetSize(CvSize size, int fourcc /* = 0 */, int fps /* = 24 */)
{
	assert(m_output == 0);
	if (m_output != 0) 
		throw "Can not reopen";
	if (fourcc == 0) fourcc = CV_FOURCC('I','Y','U','V');
	m_output = cvCreateVideoWriter(m_name.c_str(), fourcc, fps, size);
}

//The IplImage returned have to be deleted outside
IplImage* CmAviHelper::CreateImage(int nChanels /* = 3 */, int depth /* = IPL_DEPTH_8U */)
{
	return cvCreateImage(GetSize(), depth, nChanels);
}


IplImage* CmAviHelper::GetFrame(int frameNumber, IplImage* buf)
{
	IplImage* img = buf ? buf : CreateImage(3, IPL_DEPTH_8U);
	if (frameNumber < FrameNumber() && frameNumber >= 0)
	{
		cvSetCaptureProperty(m_input, CV_CAP_PROP_POS_FRAMES, frameNumber);
		cvCopyImage(cvQueryFrame(m_input), img);
		//cvConvertImage(cvQueryFrame(m_input), img, CV_CVTIMG_FLIP);
	}
	return img;
}

void CmAviHelper::WriteFrame(IplImage* img, int fourcc)
{
	if (m_output == NULL)
	{
		SetSize(cvSize(img->width, img->height), fourcc);
	}
	cvWriteFrame(m_output, img);
}

