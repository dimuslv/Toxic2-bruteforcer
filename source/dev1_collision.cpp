enum Dir {
	NONE = -1,
	LEFT = 0,
	RIGHT = 1,
	UP = 2,
	DOWN = 3
};

constexpr int CPAD = 32;
short data[4][level_width + 2*CPAD][level_height + 2*CPAD];

constexpr bool getPixel(const int *pool, int x, int y, int cols = level_width / 32) {
	return pool[y * cols + x / 32] & (1 << (x % 32));
}

void setPixel(int *pool, int x, int y, int cols = level_width / 32) {
	pool[y * cols + x / 32] |= 1 << (x % 32);
}

void resetPixel(int *pool, int x, int y, int cols = level_width / 32) {
	pool[y * cols + x / 32] &= ~(1 << (x % 32));
}

bool getCollision(int x, int y) {
	if (x < 0 || y < 0 || x >= level_width || y >= level_height) {
		return false;
	}
	
	return getPixel(collision, x, y);
}

bool getCollision(hint x, int y) {
	return getCollision(x.truncate(), y);
}

void initializeData() {
	constexpr int bounds[]{-CPAD, level_width - 1 + CPAD,
		-CPAD, level_height - 1 + CPAD};
	
	for (int i = 0; i < 4; i++) {
		int sign = 1 - (i % 2) * 2;
		int majorStart = bounds[i];
		int majorEnd = bounds[i + sign] + sign;
		int minorStart = bounds[3 - i];
		int minorEnd = bounds[3 - i - sign] - sign;
		
		for (int minor = minorStart; minor != minorEnd; minor -= sign) {
			short v = SHRT_MIN;
			for (int major = majorStart; major != majorEnd; major += sign) {
				int x = i < 2? major : minor;
				int y = i < 2? minor : major;
				
				if (getCollision(x, y)) {
					if (v > 0) {
						v++;
					} else {
						v = 1;
					}
				} else {
					if (v > 0) {
						v = -1;
					} else if (v > SHRT_MIN) {
						v--;
					}
				}
				
				data[i][x+CPAD][y+CPAD] = v;
			}
		}
	}
}

void checkBounds(int x, int y) {
	return;
	
	if (x < -CPAD || y < -CPAD || x >= level_width + CPAD || y >= level_height + CPAD) {
		print << "ERROR: checked at " << x << ", " << y << '\n';
	}
}

inline int dLeft(int x, int y) {
	checkBounds(x, y);
	return data[LEFT][x+CPAD][y+CPAD];
}

inline int dLeft(hint x, int y) {
	return dLeft(x.truncate(), y);
}

inline int dRight(int x, int y) {
	checkBounds(x, y);
	return data[RIGHT][x+CPAD][y+CPAD];
}

inline int dRight(hint x, int y) {
	return dRight(x.truncate(), y);
}

inline int dUp(int x, int y) {
	checkBounds(x, y);
	return data[UP][x+CPAD][y+CPAD];
}

inline int dUp(hint x, int y) {
	return dUp(x.truncate(), y);
}

inline int dDown(int x, int y) {
	checkBounds(x, y);
	return data[DOWN][x+CPAD][y+CPAD];
}

inline int dDown(hint x, int y) {
	return dDown(x.truncate(), y);
}
