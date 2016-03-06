#include <vector>
#include <iostream>
#include "PardisoUnsym.h"

PardisoUnsym test;

using namespace std;
int main() {
	vector<int> result;
	test.Init(3);
	test.SetA(0, 0, 1);
	test.SetA(0, 2, 2);
	test.SetB(0, 7);

	test.SetA(1, 0, 1);
	test.SetA(1, 1, 1);
	test.SetB(1, 3);

	test.SetA(2, 0, 2);
	test.SetA(2, 2, 1);
	test.SetB(2, 5);

	test.Solve();
	test.GetData(result);

	for (int i = 0; i < 3; i++) {
		cout << result[i] << endl;
	}
}