#include <bits/stdc++.h>
using namespace std;

typedef unsigned long long ull;
typedef unsigned long uint;
typedef unsigned short us;

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

void writeToFile(string s) {
	ofstream file(filename);
	file << s;
	file.close();
}

template <typename T> class vQueue {
	T *arr;
	
	int start;
	int end;
	int capacity;
	
	public:
	
	int size;
	
	vQueue() {
		arr = new T[8];
		capacity = 8;
		start = 0;
		end = 0;
		size = 0;
	}
	
	~vQueue() {
		delete[] arr;
	}
	
	int next(int index) {
		return (index + 1) % capacity;
	}
	
	void checkResize() {
		if (!(size == capacity || capacity > 8 && size < capacity / 4)) {
			return;
		}
		
		int newCapacity;
		if (size == capacity) {
			newCapacity = capacity * 2;
		} else {
			newCapacity = capacity / 2;
		}
		
		T *temp = new T[newCapacity];
		
		for (int i = 0, ind = start; i < size; i++, ind = next(ind)) {
			temp[i] = arr[ind];
		}
		
		start = 0;
		end = size;
		
		delete[] arr;
		capacity = newCapacity;
		arr = temp;
	}
	
	void push(T value) {
		checkResize();
		
		arr[end] = value;
		end = next(end);
		size++;
	}
	
	T pop() {
		T value = arr[start];
		start = next(start);
		size--;
		
		checkResize();
		
		return value;
	}
	
	T read() {
		return arr[start];
	}
};

class cQueue {
	vQueue<ull> q;
	ull startInd;
	ull endInd;
	ull endValue;
	ull limit;
	
	public:
	
	int r;
	
	cQueue(int gr) {
		r = gr;
		startInd = 1;
		endInd = 1;
		endValue = 0;
		limit = ULLONG_MAX / r;
	}
	
	bool empty() {
		return q.size == 0 && startInd == endInd;
	}
	
	void push(int value) {
		endValue += endInd * value;
		endInd *= r;
		
		if (endInd > limit) {
			q.push(endValue);
			endValue = 0;
			endInd = 1;
		}
	}
	
	int pop() {
		ull startValue = endValue;
		if (q.size != 0) {
			startValue = q.read();
		}
		
		int value = (startValue / startInd) % r;
		startInd *= r;
		
		if (startInd > limit) {
			q.pop();
			startInd = 1;
		}
		
		return value;
	}
};

class cVector {
	vector<ull> v;
	int fitNum;
	ull endInd;
	ull endValue;
	ull limit;
	
	public:
	
	int r;
	
	cVector(int gr) {
		r = gr;
		limit = ULLONG_MAX / r;
		fitNum = 0;
		for (ull i = 1; i <= limit; i *= r) {
			fitNum++;
		}
		endInd = 1;
		endValue = 0;
	}
	
	int at(int ind) {
		ull value = endValue;
		if (ind / fitNum < v.size()) {
			value = v[ind / fitNum];
		}
		for (int i = 0; i < ind % fitNum; i++) {
			value /= r;
		}
		return value % r;
	}
	
	void push(int value) {
		endValue += endInd * value;
		endInd *= r;
		
		if (endInd > limit) {
			v.push_back(endValue);
			endValue = 0;
			endInd = 1;
		}
	}
	
	void resize(int length) {
		if (length / fitNum < v.size()) {
			endValue = v[length / fitNum];
		}
		v.resize(length / fitNum);
		
		endInd = 1;
		for (int i = 0; i < length % fitNum; i++) {
			endInd *= r;
		}
		endValue %= endInd;
	}
};
