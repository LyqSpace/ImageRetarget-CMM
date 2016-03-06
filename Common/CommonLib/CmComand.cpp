#include "StdAfx.h"
#include "CmComand.h"

void CmComand::RunComand(std::string parameters, bool waiteTillFlished/* = false*/)
{
	// Whether close console after finished: '/k'=NO, '/c'=YES 
	parameters = "/k" + parameters;

	SHELLEXECUTEINFOA  ShExecInfo  =  {0};  
	ShExecInfo.cbSize  =  sizeof(SHELLEXECUTEINFO);  
	ShExecInfo.fMask  =  SEE_MASK_NOCLOSEPROCESS;  
	ShExecInfo.hwnd  =  NULL;  
	ShExecInfo.lpVerb  =  NULL;  
	ShExecInfo.lpFile  =  "cmd.exe";                          
	ShExecInfo.lpParameters  =  parameters.c_str();         
	ShExecInfo.lpDirectory  =  NULL;  
	ShExecInfo.nShow  =  SW_SHOW;  
	ShExecInfo.hInstApp  =  NULL;              
	ShellExecuteExA(&ShExecInfo); 

	if (waiteTillFlished)
		WaitForSingleObject(ShExecInfo.hProcess,INFINITE);
}

void CmComand::RunProgram(string fileName, string parameters, bool waiteTillFlished/**/)
{
	bool ProgramFileExist = CmFile::FileExist(fileName.c_str());

	if (!ProgramFileExist)
	{
#ifdef _DEBUG
		fileName.insert(0, "..\\Debug\\");
#else
		fileName.insert(0, "..\\Release\\");
#endif // _DEBUG
		ProgramFileExist = CmFile::FileExist(fileName.c_str());
	}
	CmAssertM("Program file not exist", ProgramFileExist == true);

	SHELLEXECUTEINFOA  ShExecInfo  =  {0};  
	ShExecInfo.cbSize  =  sizeof(SHELLEXECUTEINFO);  
	ShExecInfo.fMask  =  SEE_MASK_NOCLOSEPROCESS;  
	ShExecInfo.hwnd  =  NULL;  
	ShExecInfo.lpVerb  =  NULL;  
	ShExecInfo.lpFile  =  fileName.c_str();                          
	ShExecInfo.lpParameters  =  parameters.c_str();         
	ShExecInfo.lpDirectory  =  NULL;  
	ShExecInfo.nShow  =  SW_SHOW;  
	ShExecInfo.hInstApp  =  NULL;              
	ShellExecuteExA(&ShExecInfo);  
	
	if (waiteTillFlished)
		WaitForSingleObject(ShExecInfo.hProcess,INFINITE);
	
}

void CmComand::Demo()
{
	printf("A simple demo: show path environment\n");
	RunComand("path");
}
