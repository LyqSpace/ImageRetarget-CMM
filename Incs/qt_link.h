#pragma once

#ifdef _DEBUG
#ifndef DONOT_LINK_QTMAIN
	#pragma comment(lib, "qtmaind.lib")
#endif
#pragma comment(lib, "QtGuid4.lib")
#pragma comment(lib, "QtCored4.lib")
#pragma comment(lib, "QtOpenGLd4.lib")
#pragma comment(lib, "QtScriptd4.lib")
#else
#ifndef DONOT_LINK_QTMAIN
	#pragma comment(lib, "qtmain.lib")
#endif
#pragma comment(lib, "QtGui4.lib")
#pragma comment(lib, "QtCore4.lib")
#pragma comment(lib, "QtOpenGL4.lib")
#pragma comment(lib, "QtScript4.lib")
#endif

