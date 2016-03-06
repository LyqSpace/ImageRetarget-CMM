#pragma once
#include "CommonLib.h"

class CmWindow{
public:
	CmWindow(int flags = CV_WINDOW_AUTOSIZE);
	CmWindow(string wn, int flags = CV_WINDOW_AUTOSIZE);

	void Close(void);
	void Show(const IplImage* img, int wait = -1, char* title = NULL);

private:
	string m_name;
};