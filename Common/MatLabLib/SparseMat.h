#pragma once



template<typename T>
class SparseMatT
{
public:
	int m,n;
	//bool sorted;
	struct Ele
	{
		int i;
		int j;
		T value;
		Ele()
		{}
		Ele(int i, int j, double value) : i(i), j(j), value(value)
		{}
		bool operator < (const Ele& r) const
		{
			if(j != r.j)
			{
				return j < r.j;
			}
			else
			{
				return i < r.i;
			}
		}
	};
	std::vector<Ele> elements;
#ifdef _DEBUG
	std::set<Ele> _dbgElements; //debug use only
#endif
	SparseMatT(int m, int n) : m(m), n(n)
	{}
	SparseMatT() : m(0),n(0)
	{}
	void SetDim(int m, int n)
	{
		this->m = m;
		this->n = n;
	}
	void Add(int i, int j, double val)
	{
		Ele e(i,j,val);
		elements.push_back(e);
#ifdef _DEBUG
		_ASSERT(_dbgElements.find(e) == _dbgElements.end());
		_ASSERT(i >= 0 && i < m && j >=0 && j < n);
		_dbgElements.insert(e);
#endif
	}
	void Clear()
	{
		m=0;
		n=0;
		elements.clear();
#ifdef _DEBUG
		_dbgElements.clear();
#endif
	}
	void Save(const char* fname)
	{
		ofstream fout(fname);
		fout << m << " " << n << " " << elements.size() << endl;
		BOOST_FOREACH(const Ele& e, elements)
		{
			fout << e.i << " " << e.j << " " << e.value << endl;
		}
	}
	//sort: no need to sort
	//void Sort();
};

class SparseMat : public SparseMatT<double>
{
public:
	SparseMat()
	{}
	SparseMat(int m, int n) : SparseMatT(m,n)
	{}
	//io
	SparseMat(const char* fname); //load from file
};

void SolveSparse(const SparseMat& A, const std::vector<double>& b, /*out*/std::vector<double>& x, bool useLsqr = false, double* initGuess = NULL);

