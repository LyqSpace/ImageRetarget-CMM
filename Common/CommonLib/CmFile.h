#pragma once

class CmFile
{
public:
	static bool ForceDirectory(const char*  path);

	// The first GetFileName solve the problem of those path name with ending space ' ';
	static const char* GetFileName(char* fullPath);
	static const char* GetFileName(const char* fullPath);

	// Test whether a file exist
	static bool FileExist(const char* filePath);

	static const char* GetWorkingDirectory();

	static const char*  BrowseFolder(const char* lpszTitle, const HWND hwndOwner/*=::AfxGetMainWnd->GetSafeHwnd()*/);   

	// Rename a group of images
	// Eg: RenameImages("D:/DogImages/*.jpg", "F:/Images", "dog", ".jpg");
	// Return the number of images has been processed
	static int RenameImages(const char* srcNames, const char* dstDir, const char* nameCommon, const char* nameExt);

	// clean files in a folder
	static void CleanFolder(const char* dir);
};