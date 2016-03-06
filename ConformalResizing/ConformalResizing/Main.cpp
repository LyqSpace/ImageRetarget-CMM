// ConformalResizing.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ConformalResizing.h"



IplImage* srcImg;
bool ispg;
bool exitNow;

bool FileIsExist(LPCTSTR szFileFullPathName)  
{  
    WIN32_FIND_DATA Win32_Find_Data;  
    HANDLE hFindFile;  
  
    hFindFile = FindFirstFile(szFileFullPathName,&Win32_Find_Data);  
  
    if(INVALID_HANDLE_VALUE == hFindFile)  
    {  
        //AfxMessageBox("Not Exist");  
        return false;  
    }  
    else  
    {  
        //AfxMessageBox("Have Exist");  
        FindClose(hFindFile);
        return true;  
    }
}

void work(int w, int h)
{
	IplImage* dstImg = ConformalResizing::Resize(srcImg, h, w, gSet["meshQuadSize"]);

	IplImage* pImgUni = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
	cvResize(srcImg, pImgUni);
	cvNamedWindow("Uniform scale");
	cvShowImage("Uniform scale", pImgUni);
	cvReleaseImage(&pImgUni);
	cvReleaseImage(&dstImg);
}

int main(int argc, char* argv[])
{
	if(argc>=2){
		//load
		char* section=NULL;
		if(argc>=3)
			section=argv[2];
		if(section==NULL)
			gSet.LoadSetting(argv[1]);
		else
			gSet.LoadSetting(argv[1],section);
		//exit?
		if(argc>=4)
		{
			if(strcmp(argv[3],"exit")==0)
				exitNow=true;
		}
	}else
		gSet.LoadSetting("ConformalResizing.ini");

	ispg=false;//(gSet["ispg"]>0);

	const char* wkDir = CmFile::GetWorkingDirectory();

	srcImg = cvLoadImage(gSet.Val("InputImage"), CV_LOAD_IMAGE_COLOR);
	cvNamedWindow("Source image");
	cvNamedWindow("Source image");
	cvShowImage("Source image", srcImg);

	FileNames::SetName(gSet.Val("InputImage"), gSet.Val("ImportanceImg"));

	work(gSet["newWidth"],gSet["newHeight"]);

	if(!exitNow)
	cvWaitKey(0);

	cvReleaseImage(&srcImg);
	return 0;
}
