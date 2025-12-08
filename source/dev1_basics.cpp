#include <bits/stdc++.h>
using namespace std;

typedef unsigned long long ull;
typedef unsigned long uint;

int roundDown(int a) {
	if (a & 1) return a - 1;
	return a;
}

int roundUp(int a) {
	if (a & 1) return a + 1;
	return a;
}
