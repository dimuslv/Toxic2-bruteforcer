#include <bits/stdc++.h>
using namespace std;

typedef unsigned long long ull;
typedef unsigned long uint;
typedef unsigned short us;
typedef unsigned char uc;

#define arraySize(arr) (sizeof(arr) / sizeof(*(arr)))

stringstream output;
stringstream info;

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

template <typename T> class Half {
	public:
	
	T twiceValue = 0;
	
	constexpr Half() {}
	constexpr Half(T whole) : twiceValue(whole * 2) {}
	
	constexpr Half &operator+=(const Half &rhs) {
		twiceValue += rhs.twiceValue;
		return *this;
	}

	constexpr Half &operator-=(const Half &rhs) {
		twiceValue -= rhs.twiceValue;
		return *this;
	}

	constexpr Half &operator*=(const T &rhs) {
		twiceValue *= rhs;
		return *this;
	}

	constexpr Half &operator/=(const T &rhs) {
		twiceValue /= rhs;
		return *this;
	}
	
	friend constexpr Half operator+(Half lhs, const Half &rhs) {
		return lhs += rhs;
	}

	friend constexpr Half operator-(Half lhs, const Half &rhs) {
		return lhs -= rhs;
	}
	
	friend constexpr Half operator-(Half rhs) {
		return 0 - rhs;
	}

	friend constexpr Half operator*(Half lhs, const T &rhs) {
		return lhs *= rhs;
	}

	friend constexpr Half operator*(const T &lhs, Half rhs) {
		return rhs * lhs;
	}
	
	friend constexpr bool operator==(const Half&, const Half&) = default;
	friend constexpr auto operator<=>(const Half&, const Half&) = default;
	
	friend ostream &operator<<(ostream &output, const Half &value) {
		return output << (value.twiceValue / 2.0);
	}
	
	constexpr T round() const {
		return twiceValue / 2;
	}
	
	constexpr T roundDown() const {
		return (twiceValue - (twiceValue & 1)) / 2;
	}
	
	constexpr T roundUp() const {
		return (twiceValue + (twiceValue & 1)) / 2;
	}
	
	friend constexpr Half abs(Half value) {
		if (value.twiceValue < 0) {
			value.twiceValue = -value.twiceValue;
		}
		return value;
	}
};

typedef Half<int> hint;

consteval hint operator""_5(ull value) {
	hint ret{(int) value};
	ret.twiceValue++;
	return ret;
}

constexpr hint div2(int value) {
	hint ret;
	ret.twiceValue = value;
	return ret;
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
