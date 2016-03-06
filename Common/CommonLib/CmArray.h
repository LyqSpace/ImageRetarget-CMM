#pragma once


/************************************************************************/
/* CmArray is a template for supplying an wrapper to 2D array.          */
/*		No memory would be allocated in this struct.					*/
/************************************************************************/
template<typename T> struct CmArray
{
	int n, m, widthStep;
	T* dataptr;
	
	CmArray() 
	{ 
		n = m = widthStep = 0; 
		dataptr = 0; 
	}

	T* operator [] (int rows) const 
	{ 
		return dataptr + rows * widthStep; 
	}

	CmArray(T* ptr, int n_, int m_, int step = -1)
	{
		n = n_; m = m_;
		if (step = -1) 
			step = m;
		widthStep = step;
		dataptr = ptr;
	}

	friend ostream& operator << (ostream& out, const CmArray<T>& a)
	{
		for (int i = 0; i < a.n; i++)
		{
			for (int j = 0; j < a.m; j++)
			{
				out << a[i][j] << " ";
			}
			out << endl;
		}
		return out;
	}

	friend istream& operator >> (istream& in, CmArray<T>& a)
	{
		for (int i = 0; i < a.n; i++)
		{
			for (int j = 0; j < a.m; j++)
			{
				in >> a[i][j];
			}
		}
		return in;
	}

	friend void save(const CmArray<T>& a, const char* name)
	{
		ofstream out(name);
		out << a;
	}

	friend void load(CmArray<T>& a, const char* name)
	{
		ifstream inp(name);
		inp >> a;
	}

	friend T maxof(const CmArray<T>& a)
	{
		if (a.n == 0 || a.m == 0) return T();
		T ret = a[0][0];
		for (int y = 0; y < a.n; y++)
		{
			for (int x = 0; x < a.m; x++)
			{
				T v = a[y][x];
				if (v > ret) ret = v;
			}
		}
		return ret;
	}

	// a = a + b;
	friend void add(CmArray<T>& a, const CmArray<T>& b)
	{
		for (int y = 0; y < a.n; y++)
		{
			for (int x = 0; x< a.m; x++)
			{
				a[y][x] += b[y][x];
			}
		}
	}

};

template<typename T1, typename T2>  void copyTo(const CmArray<T1>& src, CmArray<T2>& dst)
{
	for (int y=0; y<src.n; y++)
	{
		for (int x=0; x<src.m;x++)
		{
			dst[y][x] = src[y][x];
		}
	}
}

/************************************************************************/
/* CmArray2 is a template for supplying an wrapper to 2D array.			*/
/*		Memory would be self managed.									*/
/************************************************************************/
template<typename T> struct CmArray2 : public CmArray<T>
{
	vector<T> data;
	CmArray2() {}
	CmArray2(int _n, int _m) 
	{ 
		resize(_n, _m); 
	}

	void reset() 
	{ 
		dataptr = &data[0]; 
	}

	int sz() const 
	{ 
		return sizeof(T)*m*n; 
	}

	void zero() 
	{ 
		memset(dataptr, 0, sz()); 
	}

	void resize_fast(int _n, int _m) 
	{
		n = _n; 
		m = _m; 
		widthStep = m;
		data.resize(n*m);
		reset();
	}

	void resize(int _n, int _m) 
	{
		n = _n; 
		m = _m; 
		widthStep = m;
		data.clear();
		data.resize(n*m);
		reset();
	}

	void assign(const CmArray2<T>& another)
	{
		n = another.n;
		m = another.m;
		widthStep = another.widthStep;
		data = another.data;
		reset();
	}

	CmArray2(const CmArray2<T>& another)
	{
		assign(another);
	}

	void operator = (const CmArray2<T>& another)
	{
		assign(another);
	}
};

/************************************************************************/
/* CmImageWarp is a wrapper for IplImage enabling an IplImage to have   */
/*      properties as CmArray.											*/
/************************************************************************/

template<typename T> struct CmImageWrap : public CmArray<T>
{
	IplImage* ptr;

	CmImageWrap() 
	{ 
		ptr = 0; 
	}

	CmImageWrap(IplImage* im) 
	{ 
		reset(im); 
	}

	void reset(IplImage* im)
	{
		ptr = im;
		dataptr = (T*) im->imageData;
		widthStep = im->widthStep / sizeof(T);
		n = im->height;
		m = im->width;
	}

	void release() 
	{ 
		cvReleaseImage(&ptr); 
	}
};
