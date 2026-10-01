short dataUp[level_width * 32 + 64][level_height * 32 + 64];
short dataDown[level_width * 32 + 64][level_height * 32 + 64];
short dataLeft[level_width * 32 + 64][level_height * 32 + 64];
short dataRight[level_width * 32 + 64][level_height * 32 + 64];
const short MAX = 32767;
const short MIN = -32768;

bool collision(hint x, int y) {
	if (x < 0 || y < 0 || x >= level_width * 32 || y >= level_height * 32) {
		return false;
	}
	int xr = x.round();
	
	return collision_data[y * level_width + xr / 32] & (1 << (xr % 32));
}

void initializeData() {
	for (int y = -32; y < level_height * 32 + 32; y++) {
		short v = MIN;
		for (int x = -32; x < level_width * 32 + 32; x++) {
			if (x >= 0 && x < level_width * 32 && collision(x, y)) {
				if (v > 0) {
					v++;
				} else {
					v = 1;
				}
			} else {
				if (v > 0) {
					v = -1;
				} else if (v > MIN) {
					v--;
				}
			}
			dataLeft[x+32][y+32] = v;
		}
	}
	
	for (int y = -32; y < level_height * 32 + 32; y++) {
		short v = MIN;
		for (int x = level_width * 32 - 1 + 32; x >= -32; x--) {
			if (x >= 0 && x < level_width * 32 && collision(x, y)) {
				if (v > 0) {
					v++;
				} else {
					v = 1;
				}
			} else {
				if (v > 0) {
					v = -1;
				} else if (v > MIN) {
					v--;
				}
			}
			dataRight[x+32][y+32] = v;
		}
	}
	
	for (int x = -32; x < level_width * 32 + 32; x++) {
		short v = MIN;
		for (int y = -32; y < level_height * 32 + 32; y++) {
			if (y >= 0 && y < level_height * 32 && collision(x, y)) {
				if (v > 0) {
					v++;
				} else {
					v = 1;
				}
			} else {
				if (v > 0) {
					v = -1;
				} else if (v > MIN) {
					v--;
				}
			}
			dataUp[x+32][y+32] = v;
		}
	}
	
	for (int x = -32; x < level_width * 32 + 32; x++) {
		short v = MIN;
		for (int y = level_height * 32 - 1 + 32; y >= -32; y--) {
			if (y >= 0 && y < level_height * 32 && collision(x, y)) {
				if (v > 0) {
					v++;
				} else {
					v = 1;
				}
			} else {
				if (v > 0) {
					v = -1;
				} else if (v > MIN) {
					v--;
				}
			}
			dataDown[x+32][y+32] = v;
		}
	}
}

int dLeft(hint x, int y) {
	return dataLeft[x.roundDown()+32][y+32];
}

int dRight(hint x, int y) {
	return dataRight[x.roundDown()+32][y+32];
}

int dUp(hint x, int y) {
	return dataUp[x.roundDown()+32][y+32];
}

int dDown(hint x, int y) {
	return dataDown[x.roundDown()+32][y+32];
}
