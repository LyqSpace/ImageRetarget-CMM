#pragma once
#include <matlab/engine.h>
#include <matlab/mat.h>

#ifdef LINK_MATLAB
#pragma comment(lib, "libeng.lib")
#pragma comment(lib, "libmat.lib")
#pragma comment(lib, "libmx.lib")
#endif
