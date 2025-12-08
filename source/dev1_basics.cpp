#include <bits/stdc++.h>
using namespace std;

typedef unsigned long long ull;
typedef unsigned long uint;

stringstream output;
stringstream info;

int roundDown(int a) {
	if (a & 1) return a - 1;
	return a;
}

int roundUp(int a) {
	if (a & 1) return a + 1;
	return a;
}

void print() {
	cout << info.str();
	output << info.str();
	info.str("");
	info.clear();
}

void print(string s) {
	cout << s;
	output << s;
}

string getOutput() {
	return output.str();
}
