#pragma once

#pragma warning(push,0)
#include <cv/cv.h>
#include <cv/highgui.h>
#undef EXIT //well, it is really ugly for opencv defining EXIT -> goto exit
#pragma warning(pop)
#pragma comment(lib, "highgui.lib")
#pragma comment(lib, "cv.lib")
#pragma comment(lib, "cxcore.lib")
