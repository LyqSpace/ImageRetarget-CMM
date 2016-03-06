#include "StdAfx.h"
#include "CommonLib.h"
#include <shlobj.h>


bool CmFile::ForceDirectory(const char*  path)
{
	static char buffer[1024];
	strcpy(buffer, path);
	for (int i = 0; buffer[i] != 0; i ++)
	{
		if (buffer[i] == '\\')
		{
			buffer[i] = '\0';
			CreateDirectoryA(buffer, 0);
			buffer[i] = '\\';
		}
	}
#pragma warning(disable:4800)
	return CreateDirectoryA(path, 0);
#pragma warning(default:4800)
}

const char* CmFile::GetFileName(char* fullPath)
{
	int len = int(strlen(fullPath));
	while(fullPath[len - 1] == ' ' && len > 0)
		len--;
	fullPath[len] = '\0';

	while(fullPath[len - 1] != '\\' && fullPath[len - 1] != '/' && len > 0)
		len--;

	if (len > 0)
		return fullPath + len;
	else 
		return NULL;
}

const char* CmFile::GetFileName(const char* fullPath)
{
	int len = int(strlen(fullPath));

	while(fullPath[len - 1] != '\\' && fullPath[len - 1] != '/' && len > 0)
		len--;

	if (len > 0)
		return fullPath + len;
	else 
		return NULL;
}
// Test whether a file exist
bool CmFile::FileExist(const char* filePath)
{
	return  GetFileAttributesA(filePath) != INVALID_FILE_ATTRIBUTES; // ||  GetLastError() != ERROR_FILE_NOT_FOUND;
}

const char* CmFile::GetWorkingDirectory()
{
	static char wkDir[1024];
	GetCurrentDirectory(300, wkDir);
	return wkDir;
}

const char* CmFile::BrowseFolder(const char* lpszTitle,  const HWND hwndOwner)   
{
	static char Buffer[MAX_PATH];
	BROWSEINFO bi;//Initial bi 	
	bi.hwndOwner = hwndOwner; 
	bi.pidlRoot = NULL;
	bi.pszDisplayName = Buffer; // Dialog can't be shown if it's NULL
	bi.lpszTitle = "Find path for saving";
	bi.ulFlags = 0;
	bi.lpfn = NULL;
	bi.iImage = NULL;


	LPITEMIDLIST pIDList = SHBrowseForFolder(&bi); // Show dialog
	if(pIDList)
	{	
		SHGetPathFromIDList(pIDList, Buffer);
		if (Buffer[strlen(Buffer) - 1]  == '\\')
		{
			Buffer[strlen(Buffer) - 1] = 0;
		}
		return Buffer;
	}
	return NULL;   
}

int CmFile::RenameImages(const char *srcNames, const char *dstDir, const char *nameCommon, const char *nameExt)
{
	CFileFind finder;
	int counter = 0;
	CString srcImgNames(srcNames), srcImgName, dstImgName;
	BOOL bWorking = finder.FindFile(srcImgNames);
	while(bWorking)
	{
		bWorking = finder.FindNextFile();
		srcImgName = finder.GetFilePath();

		dstImgName.Format("%s\\%d%s.%s", dstDir, counter++, nameCommon, nameExt);
		::CopyFile(srcImgName, dstImgName, FALSE);
	}
	return counter;
}

void CmFile::CleanFolder(const char* dir)
{
	system(CmSprintf("del /q %s", dir));
}
