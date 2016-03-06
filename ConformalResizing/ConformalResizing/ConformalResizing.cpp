#include "StdAfx.h"
#include "importance.h"
#include "ConformalResizing.h"
#include <EdgeLib.h>

ConformalResizing::ConstrainUnits::ConstrainUnits()
{
	n = 0;
	pnts = NULL;
	ind = NULL;
}

void ConformalResizing::ConstrainUnits::Destory()
{
	n=0;
	delete []pnts;
	delete []ind;
	pnts = NULL;
	ind = NULL;
}

void ConformalResizing::ConstrainUnits::SetNumber(int _n)
{
	Destory();
	n = _n;
	CmAssert(n != 0);
	pnts = new CvPoint2D64f[n];
	ind = new int[n];
}

ConformalResizing::ConstrainUnits& ConformalResizing::ConstrainUnits::operator =(const ConstrainUnits& r)
{
	Destory();
	n = r.n;
	if (n > 0)
	{
		pnts = new CvPoint2D64f[n];
		ind = new int[n];
		memcpy(pnts, r.pnts, sizeof(CvPoint2D64f) * n);
		memcpy(ind, r.ind, sizeof(int) * n);
		imp = r.imp;
	}
	else
	{
		pnts = NULL;
		ind = NULL;
	}

	return *this;			
}

ConformalResizing::ConstrainUnits::~ConstrainUnits()
{
	Destory();
}

CvSize ConformalResizing::GetConstrainUnits(const IplImage* srcImg32F,  
										 const IplImage* img8U3C,
										 const CvSize szGrid, 
										 vector<ConstrainUnits>& quads, 
										 vector<ConstrainUnits>& qaud5s,
										 vector<ConstrainUnits>& edges,
										 vector<double>& ppos,/*added 2009.08.16*/
										 int meshQuadSize)
{

	// Get importance map
	//IplImage* impImg32F = cvCreateImage(cvGetSize(srcImg32F), IPL_DEPTH_32F, 1);
	//cvScale(srcImg32F, impImg32F);

	IplImage* impImg32F = NULL;
	if (strlen(FileNames::impName) > 0)
	{
		IplImage* impMap = cvLoadImage(FileNames::impName, CV_LOAD_IMAGE_GRAYSCALE);
		//if (impMap != NULL)
		{
			NormalizeImg(impMap, meshQuadSize);
			impImg32F = cvCreateImage(cvGetSize(impMap), IPL_DEPTH_32F, 1);
			cvScale(impMap, impImg32F, 1/255.0);
			cvReleaseImage(&impMap);
			cvNamedWindow("Importance");
			cvShowImage("Importance", impImg32F);
			cvAddS(impImg32F, cvScalarAll(gSet("minWeight")), impImg32F);
		}
	}

	CmImportance imp;
	if (impImg32F == NULL)
	{
		double weights[5];
		weights[0] = gSet("edgeWeight");
		weights[1] = gSet("faceWeight");
		weights[2] = gSet("motionWeight");
		weights[3] = gSet("contrastWeight"); 
		weights[4] = gSet("minWeight");

		impImg32F = imp.calcEnergy(img8U3C, weights);
		imp.showEnergy();
	}

	{
		IplImage* impSave = cvCreateImage(cvGetSize(impImg32F), IPL_DEPTH_8U, 1);
		cvScale(impImg32F, impSave, 255);
		//cvSaveImage(FileNames::outImp, impSave);
		cvReleaseImage(&impSave);
	}

//#ifdef _DEBUG
//	cvSave("impd.xml", img8U3C, "impImg32F");
//#else
//	cvSave("imp.xml", img8U3C, "impImg32F");
//#endif // _DEBUG

	//
	IplImage *pGridNodeX64F, *pGridNodeY64F;
	CmCvHelper::MeshGrid(pGridNodeX64F, pGridNodeY64F, 0, srcImg32F->width, 0, srcImg32F->height, meshQuadSize, meshQuadSize);
	double (*pGridPos)[2] = new double[szGrid.width * szGrid.height][2]; //Original edge point position within each grid. (x, y)
	int *pGridIdx = new int[szGrid.width * szGrid.height];  // Index of grid point variable
	int *pGridIdxE = new int[szGrid.width * szGrid.height]; // Index of edge contain this grid point
	typedef vector<pair<int, int>> EdgePos;  // Position of edge point in grid
	vector<EdgePos> edgePntPos;
	int varaInd = (szGrid.height + 1) * (szGrid.width + 1);
	/*added 2009.08.16*/
	{
		ppos.reserve(varaInd*2);
		for(int y=0;y<=szGrid.height;y++)
			for(int x=0;x<=szGrid.width;x++)
			{
				ppos.push_back(x*meshQuadSize);//x
				ppos.push_back(y*meshQuadSize);//y
			}
	}
	{
		//Get Edges
		const IplImage* pLineInd;
		vector<CEdge> edge;
		CDetectEdge detEdge(edge, gSet("Sigma"));
		detEdge.Initial(srcImg32F);
		detEdge.CalFirDer();
		detEdge.NoneMaximalSuppress((float)gSet("LinkEndBound"), (float)gSet("LinkStartBound"));
		detEdge.Link(gSet["ShortRemoveBound"]);
		pLineInd = detEdge.LineIdx();

		int* pTmp = pGridIdx;  // Borrow memory inside
		memset(pTmp, 0xff, szGrid.width * szGrid.height * sizeof(int));
		memset(pGridIdxE, 0xff, szGrid.width * szGrid.height * sizeof(int));

		for (int y = 0; y < srcImg32F->height; y++)
		{
			int* lineIdx = (int*)(pLineInd->imageData + pLineInd->widthStep * y); 
			for (int x = 0; x < srcImg32F->width; x++)
			{
				if (lineIdx[x] > 0) // it's an edge point
				{
					int dx = x % meshQuadSize;
					dx = min(dx, meshQuadSize - dx);
					int dy = y % meshQuadSize;
					dy = min(dy, meshQuadSize - dy);
					dx = min(dx, dy);
					int gridPos = y / meshQuadSize * szGrid.width + x / meshQuadSize;

					if (dx > pTmp[gridPos] && dx > gSet("minEdgeRatio") * meshQuadSize)
					{
						pGridPos[gridPos][0] = x;
						pGridPos[gridPos][1] = y;
						pGridIdxE[gridPos] = lineIdx[x];
						pTmp[gridPos] = dx;
					}
				}
			}
		}

		map<int, EdgePos> edgePntPosMap;
		for (int y = 0; y < szGrid.height; y++)
		{
			for (int x = 0; x < szGrid.width; x++)
			{
				int gridPos = y * szGrid.width + x;
				int idx = pGridIdxE[gridPos];
				if (idx > 0) // an edge point within grid
				{
					edgePntPosMap[idx].push_back(pair<int, int>(x, y));
				}
			}
		}

		for (map<int, EdgePos>::iterator it = edgePntPosMap.begin(); it != edgePntPosMap.end(); it++)
		{
			EdgePos& edPos = it->second;
			if (edPos.size() >= 3)
				edgePntPos.push_back(edPos);
			else
			{
				for (size_t i = 0; i < edPos.size(); i++)
					pGridIdxE[edPos[i].first + edPos[i].second * szGrid.width] = -1;
			}
		}

		for (int y = 0; y < szGrid.height; y++)
		{
			for (int x = 0; x < szGrid.width; x++)
			{
				int gridPos = y * szGrid.width + x;
				int idx = pGridIdxE[gridPos];
				if (idx > 0) // an edge point within grid
				{
					pGridIdx[gridPos] = varaInd;
					varaInd++;
					ppos.push_back(pGridPos[gridPos][0]);
					ppos.push_back(pGridPos[gridPos][1]);
				}
				else
					pGridIdx[gridPos] = -1;
			}
		}

		for (size_t i = 0; i < edgePntPos.size(); i++)
		{
			for (size_t j = 0; j < edgePntPos[i].size(); j++)
			{
				int gridPos = edgePntPos[i][j].first + szGrid.width * edgePntPos[i][j].second;
				pGridIdxE[gridPos] = i;
			}
		}

		CmShow::Labels(pLineInd, "Labels", 1); // Show Line Idx
		//CmShow::MixedMesh(img8U3C, pGridNodeX64F, pGridNodeY64F, szGrid, pGridPos, pGridIdxE, "Mixed", 1);
	}

	CvSize szConstrainA = cvSize(varaInd, 0);

	// Get constrain units
	{
		IplImage* gridImp32F = cvCreateImage(szGrid, IPL_DEPTH_32F, 1);
		cvResize(impImg32F, gridImp32F, CV_INTER_AREA/*added 2009-7-27*/);

		double* pNodeX = (double*)(pGridNodeX64F->imageData);
		double* pNodeY = (double*)(pGridNodeY64F->imageData);

		// Quads constrains and qaud5 constrains
		for (int y = 0; y < szGrid.height; y++)
		{
			for (int x = 0; x < szGrid.width; x++)
			{
				int gridpos = szGrid.width * y + x;

				ConstrainUnits unit;
				unit.SetNumber(pGridIdxE[gridpos] >= 0 ? 5 : 4);

				unit.ind[0] = y * (szGrid.width + 1) + x;
				unit.ind[1] = unit.ind[0] + szGrid.width + 1;
				unit.ind[2] = unit.ind[0] + 1;
				unit.ind[3] = unit.ind[0] + szGrid.width + 2;

				if (pGridIdxE[gridpos] >= 0)
				{
					unit.ind[4] = pGridIdx[gridpos];
					unit.pnts[4].x = pGridPos[gridpos][0];
					unit.pnts[4].y = pGridPos[gridpos][1];
				}

				for (int i = 0; i < 4; i++)
				{
					unit.pnts[i].x = pNodeX[unit.ind[i]];
					unit.pnts[i].y = pNodeY[unit.ind[i]];
				}

				unit.imp = CV_IMAGE_ELEM(gridImp32F, float, y, x);
				if (pGridIdxE[gridpos] >= 0)
				{
					//unit.imp *=1.2;
					qaud5s.push_back(unit);					
				}
				else
					quads.push_back(unit);
			}
		}

		szConstrainA.height = quads.size() * 8 + qaud5s.size() * 10;

		// Edge constrains
		for (size_t i = 0; i < edgePntPos.size(); i++)
		{
			ConstrainUnits unit;
			unit.SetNumber(edgePntPos[i].size());
			double imp = 0;
			for (size_t j = 0; j < edgePntPos[i].size(); j++)
			{
				int gridPos = edgePntPos[i][j].first + edgePntPos[i][j].second * szGrid.width;
				unit.ind[j] = pGridIdx[gridPos];
				unit.pnts[j].x = pGridPos[gridPos][0];
				unit.pnts[j].y = pGridPos[gridPos][1];
				imp += ((float*)(gridImp32F->imageData))[gridPos];
			}
			unit.imp = imp/unit.n * gSet("EdgeConstrainRation");
			edges.push_back(unit);
			szConstrainA.height += unit.n * 2;
		}

		cvReleaseImage(&gridImp32F);
		ShowConstrains(img8U3C, quads, qaud5s, edges, "Constrains", 1, FileNames::srcMeshName);
	}
	delete []pGridIdxE;
	delete []pGridIdx;
	delete []pGridPos;
	cvReleaseImage(&pGridNodeY64F);
	cvReleaseImage(&pGridNodeX64F);
	//cvReleaseImage(&impImg32F);

	szConstrainA.width *= 2;
	return szConstrainA;
}

IplImage* ConformalResizing::Resize(IplImage*& img8U3C, int h, int w, int meshQuadSize /* = 10 */)
{
	//fix me
	if(w%4!=0)
		w = w-(w%4);
	// Normalize image
	IplImage* srcImg32F = cvCreateImage(cvGetSize(img8U3C), IPL_DEPTH_32F, 1);
	{
		IplImage* imgG = cvCreateImage(cvGetSize(img8U3C), img8U3C->depth, 1);
		cvCvtColor(img8U3C, imgG, CV_BGR2GRAY);
		cvScale(imgG, srcImg32F, 1.0/250.0);
		cvReleaseImage(&imgG);
		NormalizeImg(srcImg32F, meshQuadSize);
		NormalizeImg(img8U3C, meshQuadSize);
	}

	// Get constrain units
	vector<double> ppos;
	CvSize szGrid = cvSize(srcImg32F->width / meshQuadSize, srcImg32F->height / meshQuadSize); // Number of quads 
	vector<ConstrainUnits> quads; // Quads units
	vector<ConstrainUnits> qaud5s;  // Constrain unit of qaud5s
	vector<ConstrainUnits> edges; // Constrain unit of edges
	CvSize szA = GetConstrainUnits(srcImg32F, img8U3C, szGrid, quads, qaud5s, edges, ppos, meshQuadSize);

	// Least square problem of min |A*X|^2 with boundary conditions X(ind) = val
	SparseMat A(0, szA.width);
	vector<double> val;
	vector<int> ind;
	vector<double> X;
	ind.reserve((szA.width+szA.height)*3);
	val.reserve(ind.size());

	//for(int i=0;i<10;i++)
	//{
	//	A.Clear();
	//	A.n=szA.width;
	//	ind.clear();
	//	val.clear();
	//	BuildConstrainsEquation(quads, qaud5s, edges, szGrid, cvSize(w, h), A, ind, val);
	//}
	BuildConstrainsEquation(quads, qaud5s, edges, szGrid, cvSize(w, h), A, ind, val);
	//init values
	double xratio=double(w)/srcImg32F->width;
	double yratio=double(h)/srcImg32F->height;
	for(int i=0;i<szA.width;i+=2)
	{
		ppos[i] *= xratio;
		ppos[i+1] *= yratio;
	}
	CmAssert(A.m == szA.height);

	cout << "quads.size=" << quads.size() << endl;
	cout << "quad5s.size=" << qaud5s.size() << endl;
	cout << "edges.size=" << edges.size() << endl;
	cout << "A.m=" << A.m << " A.n=" <<A.n<< endl;

	// Solve least square problem with boundary condition
	
	SolveConstrainedLeastSquare(A, ind, val, X, ppos);

	vector<ConstrainUnits> newQuads = quads;
	vector<ConstrainUnits> newqaud5s = qaud5s;
	vector<ConstrainUnits> newEdges = edges;

	RefreshPositions(newQuads, X);
	RefreshPositions(newqaud5s, X);
	RefreshPositions(newEdges, X);

	//SaveConstrainUnits(newQuads, "quads.txt");
	//SaveConstrainUnits(newQuads, "qaud5s.txt");
	//SaveConstrainUnits(newQuads, "edges.txt");

	IplImage* dstImg = Warpping(img8U3C, meshQuadSize, X, cvSize(w, h));
	if (dstImg == NULL)
	{
		dstImg = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
	}
	ShowConstrains(dstImg, newQuads, newqaud5s, newEdges, "Reshaped constrains", 1, FileNames::dstMeshName);
	cvReleaseImage(&srcImg32F);
	return dstImg;
}

IplImage* ConformalResizing::Warpping_dsp(const IplImage* pSrc8U3C, int meshQuadSize, const vector<double>& X, const CvSize dstSize)
{
	DeleteFile("1.bmp");
	ofstream out("config");
	cvSaveImage("out_img.bmp", pSrc8U3C);
	out<<"out_img.bmp"<<endl;
	out << meshQuadSize << endl;
	out<<dstSize.width<<" "<<dstSize.height<<endl;
	int num = (pSrc8U3C->width/meshQuadSize+1)*(pSrc8U3C->height/meshQuadSize+1);
	out<<pSrc8U3C->width/meshQuadSize<<" "<<pSrc8U3C->height/meshQuadSize<<endl;
	for (int i = 0; i < 2*num; i+=2)
	{
		out<<X[i]<<" ";
		out<<X[i+1]<<endl;
	}
	//QString program = "D:\\program\\papercode\\ConformalResizing\\Debug\\interpolation.exe";
	out.close();
	QString program = "interpolation.exe";
	QProcess qp;
	QStringList arguments;
	arguments << "config" << "save";
	qp.start(program, arguments);
	qp.waitForFinished();
	IplImage* img = cvLoadImage("1.bmp");
	DeleteFile("1.bmp");
	DeleteFile("1_mesh.bmp");
	DeleteFile("out_img.bmp");
	DeleteFile("config");
	return img;
}

IplImage* ConformalResizing::Warpping_old(const IplImage *pSrc8U3C, int meshQuadSize, const std::vector<double> &X, const CvSize dstSize)
{	
	CmAssert(pSrc8U3C->widthStep == pSrc8U3C->width * 3);
	CmAssert(dstSize.width % 4 == 0);
	IplImage* dstImg = cvCreateImage(dstSize, IPL_DEPTH_8U, 3);
	MatEngine eg = MatEngineMan::GetEngine();

	// Prepare variables
	mwSize nDim = 3;
	mwSize dims[] = {3, pSrc8U3C->width, pSrc8U3C->height};
	mxArray* srcImg = mxCreateNumericArray(nDim, dims, mxUINT8_CLASS, mxREAL);
	memcpy(mxGetPr(srcImg), pSrc8U3C->imageData, 3 * pSrc8U3C->width * pSrc8U3C->height);
	eg.PutVariable("srcImg", srcImg);
	mxArray* quadSize = mxCreateDoubleMatrix(1, 1, mxREAL);
	double* p = mxGetPr(quadSize);
	p[0] = meshQuadSize;
	eg.PutVariable("quadSize", quadSize);
	eg.EvalString("clear warpedImg;");
	mxArray* dstImgArr = NULL;
	if(gSet["useWarp0"] == 1)
	{
		eg.EvalString("WarpImage");
		dstImgArr = eg.GetVariable("warpedImg");
		if(dstImgArr == NULL){
			printf("Failed to execute matlab code WarpImage.m \n");
			eg.EvalString("WarpImage1");
			dstImgArr = eg.GetVariable("warpedImg");
		}
	}else{//default
		eg.EvalString("WarpImage1");
		dstImgArr = eg.GetVariable("warpedImg");
		if (dstImgArr == NULL)
		{
			printf("Failed to execute matlab code WarpImage1.m \n");
			eg.EvalString("WarpImage");
			dstImgArr = eg.GetVariable("warpedImg");
		}
	}
	if (dstImgArr == NULL)
	{
		printf("Run matlab code WarpImage1.m failed\n");
		return NULL;
	}
	const mwSize* dstDims = mxGetDimensions(dstImgArr);
	//CmAssert(dstDims[0] == 3 && dstDims[1] == 
	printf("Dims = [%d, %d, %d]\n", dstDims[0], dstDims[1], dstDims[2]);
	memcpy(dstImg->imageData, mxGetPr(dstImgArr), dstImg->widthStep * dstImg->height);

	mxDestroyArray(srcImg);
	mxDestroyArray(quadSize);
	mxDestroyArray(dstImgArr);

	return dstImg;
}

IplImage* ConformalResizing::Warpping(const IplImage* pSrc8U3C, int meshQuadSize, const vector<double>& X, const CvSize dstSize)
{
	int method = gSet["useWarp0"];
	IplImage *dstImg = NULL;
	//if(method == 0 || method == 1)
	//	dstImg = Warpping_old(pSrc8U3C,meshQuadSize,X,dstSize);
	//else
	//{
		dstImg = Warpping_dsp(pSrc8U3C, meshQuadSize, X, dstSize);
	//}
	CmWindow win("Dst image");
	win.Show(dstImg, 1);
	cvSaveImage(FileNames::dstName, dstImg);
	return dstImg;
}


IplImage* ConformalResizing::WarppingMLS(const IplImage* pSrc8U3C, int meshQuadSize, const vector<double>& X)
{
	return NULL;
}

void ConformalResizing::RefreshPositions(vector<ConstrainUnits>& units, vector<double>& X)
{
	for (size_t i = 0; i < units.size(); i++)
	{
		for (int j = 0; j < units[i].n; j++)
		{
			units[i].pnts[j].x = X[units[i].ind[j] * 2];
			units[i].pnts[j].y = X[units[i].ind[j] * 2 + 1];
		}
	}

}

void ConformalResizing::NormalizeImg(IplImage*& img, int meshQuadSize /* = 10 */)
{
	CvSize sz = cvGetSize(img), szNew;
	szNew.height = (int)(double(sz.height) / meshQuadSize + 0.5) * meshQuadSize;
	meshQuadSize = meshQuadSize % 2 ? meshQuadSize * 2 : meshQuadSize;
	meshQuadSize = meshQuadSize % 4 ? meshQuadSize * 2 : meshQuadSize;
	szNew.width = (int)(double(sz.width) / meshQuadSize + 0.5) * meshQuadSize;
	if (szNew.height != sz.height || sz.width != szNew.width)
	{
		IplImage* newImg = cvCreateImage(szNew, img->depth, img->nChannels);
		cvResize(img, newImg);
		cvReleaseImage(&img);
		img = newImg;
	}
}

CvScalar GenColor(int i)
{
	int step=5;
	int ii=i%CM_SHOW_COLOR_NUM;
	int ci=(i/CM_SHOW_COLOR_NUM)*step;
	CvScalar bs=CmShow::gColors[ii];
	int k=0;
	while(ci>0)
	{
		int m=255-bs.val[k];
		if(m>=ci)
		{
			bs.val[k]+=ci;
			ci=0;
		}
		else{
			bs.val[k]=255;
			ci-=m;
		}
		k++;
		if(k>2) break;
	}
	return bs;
}

CvScalar GenColor1(int i)
{
	if(i>10) CmAssert(false);
	const static CvScalar ss[]={
		{10, 10, 10},//0
		{237, 28, 36},//1
		{255, 126, 0},//2
		{255, 194, 14},//3
		{255, 247, 20},//4
		{177,218,43},//5
		{31,179,231},//6
		{108,108,222},//7
		{53,57,146},//8
		{107,54,141},//9
		{153,85,56},//10
	};
	return ss[i];
}
extern bool ispg;
void ConformalResizing::ShowConstrains(const IplImage *pBackGround, 
									   const vector<ConstrainUnits>& quads, 
									   const vector<ConstrainUnits>& qaud5s, 
									   const vector<ConstrainUnits>& edges, 
									   const char *winName /* =  */, 
									   const int waite /* = 0 */,
									   const char* saveName /* = NULL */)
{
	IplImage* pMixedImg = cvCloneImage(pBackGround);
	cvNamedWindow(winName);

	// Show quads
	for (size_t i = 0; i < quads.size(); i++)
	{
		CvPoint pnts[4];
		for (int j = 0; j < 4; j++)
		{
			pnts[j].x = (int)(quads[i].pnts[j].x);
			pnts[j].y = (int)(quads[i].pnts[j].y);
		}
		
		cvLineAA(pMixedImg, pnts[0], pnts[1], 255);
		cvLineAA(pMixedImg, pnts[0], pnts[2], 255);
		cvLineAA(pMixedImg, pnts[3], pnts[1], 255);
		cvLineAA(pMixedImg, pnts[3], pnts[2], 255);
	}

	// Show qaud5s
	for (size_t i = 0; i < qaud5s.size(); i++)
	{
		CvPoint pnts[5];
		for (int j = 0; j < 5; j++)
		{
			pnts[j].x = (int)(qaud5s[i].pnts[j].x);
			pnts[j].y = (int)(qaud5s[i].pnts[j].y);
		}

		cvLineAA(pMixedImg, pnts[0], pnts[1], 255);
		cvLineAA(pMixedImg, pnts[0], pnts[2], 255);
		cvLineAA(pMixedImg, pnts[3], pnts[1], 255);
		cvLineAA(pMixedImg, pnts[3], pnts[2], 255);
		cvLineAA(pMixedImg, pnts[0], pnts[4], 128);
		cvLineAA(pMixedImg, pnts[1], pnts[4], 128);
		cvLineAA(pMixedImg, pnts[2], pnts[4], 128);
		cvLineAA(pMixedImg, pnts[3], pnts[4], 128);
	}

	// Show edges
	for (size_t i = 0; i < edges.size(); i++)
	{
		CvScalar color;
		if(ispg)
			color=GenColor1(i);
		else
			color=GenColor(i);
		//swap(color.val[0],color.val[2]);
		for (int j = 0; j < edges[i].n; j++)
		{
			CvPoint point = cvPoint((int)(edges[i].pnts[j].x + 0.5), (int)(edges[i].pnts[j].y + 0.5));
			//cvCircle(pMixedImg, point, 3, CmShow::gColors[i % CM_SHOW_COLOR_NUM], 2);
			cvCircle(pMixedImg, point, 3, color, 2);
			//cvCircle(pMixedImg, point, 3, CmShow::gColors[edges[i].ind[j] % CM_SHOW_COLOR_NUM], 2);
		}
	}

	cvNamedWindow(winName);
	cvShowImage(winName, pMixedImg);
	if (saveName != NULL)
		cvSaveImage(saveName, pMixedImg);
	cvReleaseImage(&pMixedImg);
}

void ConformalResizing::BuildConstrainsEquation(
	const std::vector<ConstrainUnits> &quads,
	const std::vector<ConstrainUnits> &qaud5s, 
	const std::vector<ConstrainUnits> &edges, 
	CvSize szGrid,
	const CvSize newSize,
	SparseMat &A,
	std::vector<int> &ind, 
	std::vector<double> &val
	)
{
	// Find A
	A.elements.reserve(3000000);
	//timer.Start("quads");
	AddConstrain(quads, A, false);
	//timer.End();
	//timer.Start("quad5s");
	AddConstrain(qaud5s, A);
	//timer.End();
	//timer.Start("edges");
	AddConstrain(edges, A);
	//timer.End();
	// Find boundary conditions
	szGrid.height++;
	szGrid.width++;
	int upY = 1;
	int downY = szGrid.width * 2 * (szGrid.height - 1) + 1;
	for (int i = 0; i < szGrid.width; i++)
	{
		ind.push_back(upY);
		val.push_back(0);

		ind.push_back(downY);
		val.push_back(newSize.height);

		upY+=2;
		downY+= 2;
	}

	int leftX = 0;
	int rightX = szGrid.width * 2 - 2;
	for (int j = 0; j < szGrid.height; j++)
	{
		ind.push_back(leftX);
		val.push_back(0);

		ind.push_back(rightX);
		val.push_back(newSize.width);

		leftX += szGrid.width * 2;
		rightX += szGrid.width * 2;
	}
}

void ConformalResizing::SaveConstrainUnits(const vector<ConstrainUnits>& units, const char* fileName)
{
	FILE* f = fopen(fileName, "w");
	CmAssert(f != NULL);

	fprintf(f, "Unites number = %d\n", units.size());
	for (size_t i = 0; i < units.size();  i++)
	{
		fprintf(f, "\n\n\nn = %d\n", units[i].n);
		for (int j = 0; j < units[i].n; j++)
		{
			fprintf(f, "%d\t", units[i].ind[j]);
		}
		fprintf(f, "\n");
		for (int j = 0; j < units[i].n; j++)
		{
			fprintf(f, "(%g, %g)\t", units[i].pnts[j].x, units[i].pnts[j].y);
		}
		fprintf(f, "\nImp = %g", units[i].imp);
	}

	fclose(f);
}

void ConformalResizing::AddConstrain(const vector<ConstrainUnits>& units, SparseMat& A, bool recalculateM)
{
	CvMat* M = NULL;
	//timer.Start("testaaa");
	for (size_t i = 0; i < units.size(); i++)
	{
		if (recalculateM == true || i == 0)
		{
			if (M != NULL)
				cvReleaseMat(&M);
			Constrian(units[i], M);
		}

		int* ind = new int[units[i].n * 2];

		for (int j = 0; j < units[i].n; j++)
		{
			ind[j*2] = units[i].ind[j]*2;
			ind[j*2 + 1] = units[i].ind[j]*2 + 1;
		}

		int nPos = 0;
		for (int y = 0; y < M->height; y++)
		{
			A.m++;
			for (int x = 0; x < M->width; x++, nPos++)
			{
				A.Add(A.m-1, ind[x], M->data.db[nPos] * units[i].imp);
				//debug
				//double d=0;//M->data.db[nPos] * units[i].imp;
				//A.elements.push_back(SparseMat::Ele(A.m-1, ind[x], d));
			}
		}
		delete []ind;
	}
	//timer.End();
	cvReleaseMat(&M);
}

void ConformalResizing::Constrian(const ConstrainUnits& unit, CvMat*& M)
{
	// Preprocess unit to make Matrix M less singular
	double meanX(0), meanY(0);
	for (int i = 0; i < unit.n; i++)
	{
		meanX += unit.pnts[i].x;
		meanY += unit.pnts[i].y;
	}
	meanX /= unit.n;
	meanY /= unit.n;

	int n = unit.n * 2;
	M = cvCreateMat(n, n, CV_64F);
	CvMat* A = cvCreateMat(n, 4, CV_64F);
	CvMat* Q = cvCreateMat(n, 4, CV_64F);
	CvMat* P = cvCreateMat(4, 4, CV_64F);

	// Initial A
	cvZero(A);
	for (int i = 0; i < unit.n; i++)
	{
		double x = unit.pnts[i].x - meanX;
		double y = unit.pnts[i].y - meanY;
		CV_MAT_ELEM(*A, double, 2*i, 0) = x;
		CV_MAT_ELEM(*A, double, 2*i, 1) = -y;
		CV_MAT_ELEM(*A, double, 2*i, 2) = 1;

		CV_MAT_ELEM(*A, double, 2*i+1, 0) = y;
		CV_MAT_ELEM(*A, double, 2*i+1, 1) = x;
		CV_MAT_ELEM(*A, double, 2*i+1, 3) = 1;
	}
	cvMulTransposed(A, P, 1); // P = (A^T * A)
	cvInvert(P, P, CV_SVD_SYM); // P = (A^T * A)^(-1)
	cvMatMul(A, P, Q); 
	cvGEMM(Q, A, 1, NULL, 0, M, CV_GEMM_B_T);

	// M = M - I
	double* d = M->data.db;
	for (int i = 0; i < n; i++, d += n+1)
	{
		*d -= 1;
	}

	cvReleaseMat(&A);
	cvReleaseMat(&Q);
	cvReleaseMat(&P);
}

char* GetModuleDictionary()
{
	static char path[MAX_PATH];
	if(path[0]==0){
		::GetModuleFileNameA(NULL, path, MAX_PATH);
		int c=MAX_PATH-1;
		while(path[c]!='\\')
			c--;
		for(int i=c;i<MAX_PATH;i++)
			path[i]=0;
	}
	return path;
}

void ConformalResizing::SolveConstrainedLeastSquare(
	SparseMat &A, std::vector<int> &ind, std::vector<double> &val, std::vector<double> &X, const vector<double>& initVals)
{
	char* dir=GetModuleDictionary();
	MatEngine eg = MatEngineMan::GetEngine();

	mwSize m = A.m;
	mwSize n = A.n;
	mwSize nz = A.elements.size();
	eg.EvalString("clear;");
	eg.EvalString("rehash path;");
	//init values
	{
		mxArray* arr = mxCreateDoubleMatrix(n, 1, mxREAL);
		double* pr = mxGetPr(arr);
		memcpy(pr, &initVals[0], n*sizeof(double));
		eg.PutVariable("initVals", arr);
		mxDestroyArray(arr);
	}
	// A
	{
		mwSize dims[] = {nz, 1};
		mxArray* iv = mxCreateDoubleMatrix(nz, 1, mxREAL);
		mxArray* jv = mxCreateDoubleMatrix(nz, 1, mxREAL);
		mxArray* sv = mxCreateDoubleMatrix(nz, 1, mxREAL);

		double* ivr = mxGetPr(iv);
		double* jvr = mxGetPr(jv);
		double* svr = mxGetPr(sv);
		for (size_t i = 0; i < nz; i++)
		{
			ivr[i] = A.elements[i].i + 1;
			jvr[i] = A.elements[i].j + 1;
			svr[i] = A.elements[i].value;
		}
		eg.PutVariable("iv", iv);
		eg.PutVariable("jv", jv);
		eg.PutVariable("sv", sv);
		char buf[1024];
		sprintf(buf, "A=sparse(iv,jv,sv,%d,%d);", m,n);
		eg.EvalString(buf);
		mxDestroyArray(iv);
		mxDestroyArray(jv);
		mxDestroyArray(sv);
	}

	// ind & val
	mxArray* indArr = mxCreateDoubleMatrix(val.size(), 1, mxREAL);
	mxArray* valArr = mxCreateDoubleMatrix(val.size(), 1, mxREAL);
	//memcpy(mxGetPr(indArr), &(ind[0]), ind.size() * sizeof(double));
	double* pInd = mxGetPr(indArr);
	for (size_t i = 0; i < ind.size(); i++)
		pInd[i] = ind[i] + 1;
	memcpy(mxGetPr(valArr), &(val[0]), val.size() * sizeof(double));
	eg.PutVariable("ind", indArr);
	eg.PutVariable("vals", valArr);

	if(gSet["UseFastSolver"]>0)
	{
		int iten = gSet["FastSolverIteN"];
		if(iten==0) iten=60;
		char tmp[1024];
		sprintf(tmp,"x = LeastSquareBoundary(A, zeros(size(A, 1), 1), ind, vals, initVals, %d);",iten);
		if (eg.EvalMatCode(dir, tmp) != 0) 
		{
			printf("Error occurred when running code. %s:%s\n", __FILE__, __LINE__);
		}
	}else{
		if (eg.EvalMatCode(dir, "x = LeastSquareBoundary(A, zeros(size(A, 1), 1), ind, vals);") != 0)
		{
			printf("Error occurred when running code. %s:%s\n", __FILE__, __LINE__);
		}
	}

	X.resize(A.n);
	mxArray* xr = eg.GetVariable("x");
	CmAssert(xr != NULL);
	memcpy(&X[0], mxGetPr(xr), n*sizeof(double));
	mxDestroyArray(valArr);
	mxDestroyArray(indArr);
	mxDestroyArray(xr);
	eg.EvalString("clear;");
}
