#pragma once

template<typename T> struct Array
{
	int n, m, widthStep;
	T* dataptr;
	Array() { n = m = widthStep = 0; dataptr = 0; }
	T * operator [] (int rows) const { return dataptr + rows * widthStep; }

	Array(T* ptr, int n_, int m_, int step = -1)
	{
		n = n_; m = m_;
		if (step = -1) step = m;
		widthStep = step;
		dataptr = ptr;
	}
	friend ostream& operator << (ostream& out, const Array<T>& a)
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

	friend istream& operator >> (istream& in, Array<T>& a)
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

	friend void save(const Array<T>& a, const char* name)
	{
		ofstream out(name);
		out << a;
	}

	friend void load(Array<T>& a, const char* name)
	{
		ifstream inp(name);
		inp >> a;
	}

	friend T maxof(const Array<T>& a)
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
	friend void add(Array<T>& a, const Array<T>& b)
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

template<typename T1, typename T2>  void copyTo(const Array<T1>& src, Array<T2>& dst)
{
	for (int y=0; y<src.n; y++)
	{
		for (int x=0; x<src.m;x++)
		{
			dst[y][x] = src[y][x];
		}
	}
}

template<typename T> struct Array2 : public Array<T>
{
	vector<T> data;
	Array2() { }
	Array2(int _n, int _m) { resize(_n, _m); }
	void reset() { dataptr = &data[0]; }
	int sz() const { return sizeof(T)*m*n; }
	void zero() { memset(dataptr, 0, sz()); }
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
	void assign(const Array2<T>& another)
	{
		n = another.n;
		m = another.m;
		widthStep = another.widthStep;
		data = another.data;
		reset();
	}
	Array2(const Array2<T>& another)
	{
		assign(another);
	}
	void operator = (const Array2<T>& another)
	{
		assign(another);
	}
};

template<typename T> struct ImageWrap : public Array<T>
{
	IplImage* ptr;
	ImageWrap() { ptr = 0; }
	ImageWrap(IplImage* im) { reset(im); }
	void reset(IplImage* im)
	{
		ptr = im;
		dataptr = (T*) im->imageData;
		widthStep = im->widthStep / sizeof(T);
		n = im->height;
		m = im->width;
	}
	void release() { cvReleaseImage(&ptr); }
};