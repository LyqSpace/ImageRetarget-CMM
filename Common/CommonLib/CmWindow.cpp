#include "StdAfx.h"
#include "CmWindow.h"

CmWindow::CmWindow(int flags /* =  CV_WINDOW_AUTOSIZE*/)
{
	static int number = 0;
	number++;
	ostringstream oss;
	oss << "figure" << number;
	m_name = oss.str();
	cvNamedWindow(m_name.c_str(), flags);
}

CmWindow::CmWindow(std::string wn, int flags/* =  CV_WINDOW_AUTOSIZE*/)
{
	m_name = wn;
	cvNamedWindow(m_name.c_str(), flags);
}

void CmWindow::Close(void)
{
	cvDestroyWindow(m_name.c_str());
}

void CmWindow::Show(const IplImage* img, int wait /* = -1 */, char* title /* = NULL */)
{
	if (title != NULL)
	{
		printf("Titile = %s\n", title);
	}

	cvShowImage(m_name.c_str(), img);

	cvWaitKey(wait);

}

