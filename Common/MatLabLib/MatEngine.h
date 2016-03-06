#pragma once
#include <matlab/engine.h>


class MatEngine
{
	Engine* eg;
	MatEngine(Engine* eg) : eg(eg) {}
public:
	friend class MatEngineMan;

	int EvalString(const char *string)
	{
		return engEvalString(eg, string);
	}

	int EvalMatCode(const char* pathStr, const char* name)
	{
		// Change working directory
		mxArray* wkDir = mxCreateString(pathStr);
		engPutVariable(eg, "pathStr__", wkDir);
		engEvalString(eg, "cd(pathStr__)");

		return engEvalString(eg, name);
	}

	mxArray *GetVariable(const char *name)
	{
		return engGetVariable(eg, name);
	}
	int PutVariable(const char *var_name, const mxArray *ap)
	{
		return engPutVariable(eg, var_name, ap);
	}
	int OutputBuffer(char* buffer, int buflen)
	{
		return engOutputBuffer(eg, buffer, buflen);
	}
	
};

class MatEngineMan
{
public:
	static void CloseWhenExit(bool close)
	{
		inst.__CloseWhenExit(close);
	}
	static MatEngine GetEngine()
	{
		return inst.__GetEngine();
	}
private:
	bool closeEng;
	MatEngineMan()
	{
		eg = NULL;
		closeEng = false;
	}
	~MatEngineMan()
	{
		if(eg != NULL && closeEng)
			engClose(eg);
	}
	static MatEngineMan inst;
	MatEngine __GetEngine()
	{
		if(eg == NULL)
			eg = engOpen("\0");
		return MatEngine(eg);
	}
	void __CloseWhenExit(bool close)
	{
		closeEng = close;
	}

	Engine* eg;
};

