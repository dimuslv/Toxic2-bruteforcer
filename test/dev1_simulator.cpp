const int LEFT = 0;
const int RIGHT = 1;

const int START = 0;
const int STAND = 1;
const int DUCK = 2;
const int WALK = 3;
const int JUMP = 4;
const int FALL = 5;
const int WALL = 6;
const int HIT = 7;
const int DIE = 8;
const int END = 9;

struct playerState {
	int _x;
	int _y;
	int vx = 0;
	int vy = 0;
	int anim = STAND;
	int animDir;
	int state = STAND;
	int dir;
	int lx;
	int rx;
	int wall_count = 0;
	int fall_count;
	bool wall_jump;
	int fall_anim_count;
	bool left_edge = false;
	bool right_edge = false;
	bool hit = false;
	bool hit_ceiling;
	//bool can_jump = true;
	bool DOWN_PRESSED = false;
	bool UP_PRESSED = false;
	int DIR_PRESSED = -1;
	int custom = 0;
};

bool getOnWall(playerState &t, int x, int y) {
	if (t.dir == LEFT) {
		return collision(x,y) && !collision(x+2,y);
	} else {
		return collision(x,y) && !collision(x-2,y);
	}
}

bool getOnGround(int x, int y) {
	return collision(x,y) && !collision(x,y-1);
}

bool getInGround(int x, int y) {
	return collision(x,y) && collision(x,y-1);
}

bool getInAir(int x, int y) {
	return !collision(x,y) && !collision(x,y-1);
}

bool getInWall(int x, int y) {
	return collision(x,y);
}

void finishHit(playerState &t) {
	return;
}

void updateAnim(playerState &t) {
	if (t.state == FALL) {
		if (t.dir != t.animDir || t.state != t.anim) {
			t.fall_anim_count++;
			if (t.fall_anim_count >= 3) {
				t.anim = t.state;
				t.animDir = t.dir;
			}
		}
	} else {
		t.anim = t.state;
		t.animDir = t.dir;
	}
	if (t.hit) {
		t.anim = HIT;
	}
}

void calculateDistance(playerState &t, bool b) {
	if (t.anim == STAND || t.anim == WALK || t.anim == JUMP || t.anim == FALL || t.anim == HIT) {
		t.lx = t._x - (12 << 1);
		t.rx = t._x + (12 << 1);
	} else if (t.anim == DUCK) {
		if (t.dir == LEFT) {
			t.lx = t._x - (26 << 1);
			t.rx = t._x + (22 << 1);
		} else {
			t.lx = t._x - (22 << 1);
			t.rx = t._x + (26 << 1);
		}
	} else {
		t.lx = 0;
		t.rx = 0;
		t.right_edge = false;
		t.left_edge = false;
		return;
	}
	
	//int lh = 35;
	//int rh = 35;
	
	/*if (getInGround(t.lx, t._y)) {
		lh = 0;
	}
	if (getInGround(t.rx, t._y)) {
		rh = 0;
	}
	
	for (int i = 0; i <= 34; i++) {
		if (getOnGround(t.lx, t._y + i)) {
			lh = i;
			break;
		}
	}
	for (int i = 0; i <= 34; i++) {
		if (getOnGround(t.rx, t._y + i)) {
			rh = i;
			break;
		}
	}*/
	
	int lh = 35;
	int rh = 35;
	
	int d = dDown(t.lx, t._y-1);
	if (d < 0) {
		lh = min(35, -d-1);
	} else {
		int d2 = d - dDown(t.lx, t._y-1+d);
		if (d2 <= 35) {
			lh = d2 - 1;
		} else if (d > 1) {
			lh = 0;
		}
	}
	
	d = dDown(t.rx, t._y-1);
	if (d < 0) {
		rh = min(35, -d-1);
	} else {
		int d2 = d - dDown(t.rx, t._y-1+d);
		if (d2 <= 35) {
			rh = d2 - 1;
		} else if (d > 1) {
			rh = 0;
		}
	}
	
	if (!b) {
		t.left_edge = lh >= 32 && rh == 0;
		t.right_edge = rh >= 32 && lh == 0;
	} else {
		t.left_edge = lh >= 32 && rh < lh;
		t.right_edge = rh >= 32 && lh < rh;
	}
}

void adjustToFloor(playerState &t) {
	if (t.vy < 0) return;
	
	/*if (t.left_edge) {
		if (getInGround(t.rx, t._y)) {
			for (int i = 1; i <= 100; i++) {
				if (getOnGround(t.rx, t._y - i)) {
					t._y -= i;
					break;
				}
			}
		}
	} else if (t.right_edge) {
		if (getInGround(t.lx, t._y)) {
			for (int i = 1; i <= 100; i++) {
				if (getOnGround(t.lx, t._y - i)) {
					t._y -= i;
					break;
				}
			}
		}
	} else if (getInGround(t._x, t._y)) {
		for (int i = 1; i <= 100; i++) {
			if (getOnGround(t._x, t._y - i)) {
				t._y -= i;
				break;
			}
		}
	}*/
	
	int d;
	
	if (t.left_edge) {
		d = dUp(t.rx, t._y);
	} else if (t.right_edge) {
		d = dUp(t.lx, t._y);
	} else {
		d = dUp(t._x, t._y);
	}
	
	if (d > 1 && d <= 101) {
		t._y -= d - 1;
	}
}

int checkWalls(playerState &t, int v, bool duck) {
	
	int offsets[5];
	
	if (duck) {
		offsets[0] = -2;
		offsets[1] = -5;
		offsets[2] = -10;
		offsets[3] = -15;
		offsets[4] = -20;
	} else {
		offsets[0] = -16;
		offsets[1] = -20;
		offsets[2] = -25;
		offsets[3] = -30;
		offsets[4] = -35;
	}
	
	if (v == 0) {
		t.vx = 0;
		return 0;
	}
	
	if (t.anim == WALL || t.anim == START || t.anim == END || t.anim == DIE) {
		if (leftSideType == 1) return 0;
		if (leftSideType == 2) return v;
		
		if (getInWall(0, t._y + offsets[0]) || getInWall(0, t._y + offsets[1]) || getInWall(0, t._y + offsets[2]) || getInWall(0, t._y + offsets[3]) || getInWall(0, t._y + offsets[4])) {
			return 0;
		}
		return v;
	}
	
	if (v > 0) {
		int w;
		if (t.anim == DUCK) {
			w = ((t.animDir == RIGHT)? 26 : 22) << 1;
		} else {
			w = 12 << 1;
		}
		
		if (!(getInWall(t._x + w + v, t._y + offsets[0]) || getInWall(t._x + w + v, t._y + offsets[1]) || getInWall(t._x + w + v, t._y + offsets[2]) || getInWall(t._x + w + v, t._y + offsets[3]) || getInWall(t._x + w + v, t._y + offsets[4]))) {
			/*for (int i = 2; i <= 12 << 1; i += 2) {
				for (int j = 0; j < 5; j++) {
					if (getInWall(t._x + w + i, t._y + offsets[j])) {
						t.wall_jump = true;
						if (v < i) {
							return v;
						}
						t.vx = 0;
						return i - 2;
					}
				}
			}*/
			int d = -dRight(t._x + w + 2, t._y + offsets[0]);
			for (int i = 1; i < 5; i++) {
				d = min(d, -dRight(t._x + w + 2, t._y + offsets[i]));
			}
			
			if (d < 0) d = 0;
			
			if (d <= 22) {
				t.wall_jump = true;
				if (v < d + 2) {
					return v;
				}
				t.vx = 0;
				return d;
			}
			
			return v;
		}
		
		/*for (int i = 2; i <= 12 << 1; i += 2) {
			int freeCount = 0;
			for (int j = 0; j < 5; j++) {
				if (!getInWall(t._x + w + v - i, t._y + offsets[j])) {
					freeCount++;
				}
			}
			if (freeCount == 5) {
				t.vx = v - i;
				t.wall_jump = true;
				return v - i;
			}
		}*/
		
		int i = 0;
		bool wasUpdated;
		
		do {
			wasUpdated = false;
			
			for (int j = 0; j < 5; j++) {
				int d = dLeft(t._x + w + v - i, t._y + offsets[j]);
				if (d > 0) {
					i += d;
					if (i > 12 << 1) {
						return 0;
					}
					
					wasUpdated = true;
				}
			}
		} while (wasUpdated);
		
		t.vx = v - i;
		t.wall_jump = true;
		return v - i;
		
	} else if (v < 0) {
		int w;
		if (t.anim == DUCK) {
			w = ((t.animDir == LEFT)? -26 : -22) << 1;
		} else {
			w = (-12) << 1;
		}
		
		if (!(getInWall(t._x + w + v, t._y + offsets[0]) || getInWall(t._x + w + v, t._y + offsets[1]) || getInWall(t._x + w + v, t._y + offsets[2]) || getInWall(t._x + w + v, t._y + offsets[3]) || getInWall(t._x + w + v, t._y + offsets[4]))) {
			/*for (int i = 2; i <= 12 << 1; i += 2) {
				for (int j = 0; j < 5; j++) {
					if (getInWall(t._x + w - i, t._y + offsets[j])) {
						t.wall_jump = true;
						if (abs(v) < i) {
							return v;
						}
						t.vx = 0;
						return -(i - 2);
					}
				}
			}*/
			
			int d = -dLeft(t._x + w - 2, t._y + offsets[0]);
			for (int i = 1; i < 5; i++) {
				d = min(d, -dLeft(t._x + w - 2, t._y + offsets[i]));
			}
			
			if (d < 0) d = 0;
			
			if (d <= 22) {
				t.wall_jump = true;
				if (-v < d + 2) {
					return v;
				}
				t.vx = 0;
				return -d;
			}
			
			return v;
		}
		
		/*for (int i = 2; i <= 12 << 1; i += 2) {
			int freeCount = 0;
			for (int j = 0; j < 5; j++) {
				if (!getInWall(t._x + w + v + i, t._y + offsets[j])) {
					freeCount++;
				}
			}
			if (freeCount == 5) {
				t.vx = v + i;
				t.wall_jump = true;
				return v + i;
			}
		}*/
		
		int i = 0;
		bool wasUpdated;
		
		do {
			wasUpdated = false;
			
			for (int j = 0; j < 5; j++) {
				int d = dRight(t._x + w + v + i, t._y + offsets[j]);
				if (d > 0) {
					i += d;
					if (i > 12 << 1) {
						return 0;
					}
					
					wasUpdated = true;;
				}
			}
		} while (wasUpdated);
		
		t.vx = v + i;
		t.wall_jump = true;
		return v + i;
	}
	
	return 0;
}

int checkCeiling(playerState &t, int v) {
	int m;
	if (t.dir == LEFT) {
		m = -2;
	} else if (t.dir == RIGHT) {
		m = 2;
	}
	if (v >= 0) {
		return v;
	}
	
	int h;
	if (t.anim == JUMP || t.anim == HIT) {
		h = -46;
	} else {
		return v;
	}
	
	if (getInWall(t._x + 6 * m, t._y + h)) {
		/*for (int i = 1; i < 16; i++) {
			if (!getInWall(t._x + 6 * m, t._y + h + i)) {
				t.vy = 0;
				return v + i;
			}
		}*/
		
		int d = dDown(t._x + 6 * m, t._y + h);
		
		if (d < 16) {
			t.vy = 0;
			return v + d;
		}
		
	} else if (getInWall(t._x + 1 * m, t._y + h)) {
		/*for (int i = 1; i < 16; i++) {
			if (!getInWall(t._x + 1 * m, t._y + h + i)) {
				t.vy = 0;
				return v + i;
			}
		}*/
		
		int d = dDown(t._x + 1 * m, t._y + h);
		
		if (d < 16) {
			t.vy = 0;
			return v + d;
		}
		
	} else if (getInWall(t._x + 12 * m, t._y + h)) {
		/*for (int i = 1; i < 16; i++) {
			if (!getInWall(t._x + 12 * m, t._y + h + i)) {
				t.vy = 0;
				return v + i;
			}
		}*/
		
		int d = dDown(t._x + 12 * m, t._y + h);
		
		if (d < 16) {
			t.vy = 0;
			return v + d;
		}
		
	} else {
		int d = min(min(-dUp(t._x + 6 * m, t._y + h), -dUp(t._x + 1 * m, t._y + h)), -dUp(t._x + 12 * m, t._y + h));
		
		if (d < -v) {
			t.vy = 0;
			t.hit_ceiling = true;
			return - d - 2;
		}
		
		/*for (int i = 1; i < abs(v); i++) {
			if (getInWall(t._x + 6 * m, t._y + h - i) ||
			getInWall(t._x + 1 * m, t._y + h - i) ||
			getInWall(t._x + 12 * m, t._y + h - i)) {
				t.vy = 0;
				t.hit_ceiling = true;
				return - i - 2;
			}
		}*/
	}
	
	return v;
}

int checkFloor(playerState &t, int v) {
	//v = Math.round(v);
	/*for (int i = 1; i <= v; i++) {
		if (getInWall(t._x, t._y + i)) {
			return i;
		}
	}*/
	return min(v, max(0, -dDown(t._x, t._y + 1)) + 1);
	return v;
}

void startJump(playerState &t) {
	//if (t.can_jump) {
		t.vy = -16;
		t.state = JUMP;
		//t.can_jump = false;
	//}
}

void startWall(playerState &t) {
	t.state = WALL;
	t.wall_count = 0;
	t.fall_count = 0;
	t.vy = 0;
	t.vx = 0;
	//t.can_jump = true;
}

void startWallJump(playerState &t) {
	//if (t.can_jump) {
		t.vy = -12;
		t.state = JUMP;
		//t.can_jump = false;
	//}
}

void startHit(playerState &t) {
	t.hit = true;
	t.vy = -8;
	t.state = JUMP;
	updateAnim(t);
}

void doStand(playerState &t) {
	if (t.hit) {
		finishHit(t);
	}
	calculateDistance(t, false);
	t.vy = 0;
	
	if (t.vx != 0) {
		t.vx = roundUp(t.vx);
		if (t.vx > 0) {
			t.vx -= 4;
			if (t.vx <= 0) {
				t.vx = 0;
			}
		} else {
			t.vx += 4;
			if (t.vx >= 0) {
				t.vx = 0;
			}
		}
		t._x += checkWalls(t, t.vx, false);
	}
	
	if (t.DOWN_PRESSED) {
		t.state = DUCK;
		return;
	}
	if (t.UP_PRESSED) {
		startJump(t);
		return;
	}
	if (t.DIR_PRESSED != -1) {
		if (t.left_edge) {
			if (getOnGround(t.rx, t._y)) {
				t.state = WALK;
				t.dir = t.DIR_PRESSED;
			}
		} else if (t.right_edge) {
			if (getOnGround(t.lx, t._y)) {
				t.state = WALK;
				t.dir = t.DIR_PRESSED;
			}
		} else if (getOnGround(t._x, t._y)) {
			t.state = WALK;
			t.dir = t.DIR_PRESSED;
		}
	} else {
		if (t.left_edge) {
			if (!getOnGround(t.rx, t._y)) {
				t.fall_anim_count = 0;
				t.vy = 0;
				t.state = FALL;
				return;
			}
		} else if (t.right_edge) {
			if (!getOnGround(t.lx, t._y)) {
				t.fall_anim_count = 0;
				t.vy = 0;
				t.state = FALL;
				return;
			}
		} else if (!getOnGround(t._x, t._y)) {
			t.fall_anim_count = 0;
			t.vy = 0;
			t.state = FALL;
			return;
		}
		adjustToFloor(t);
	}
}

void doDuck(playerState &t) {
	if (t.vx != 0) {
		t.vx = roundUp(t.vx);
		if (t.vx > 0) {
			t.vx -= 4;
			if (t.vx <= 0) {
				t.vx = 0;
			}
		} else if (t.vx < 0) {
			t.vx += 4;
			if (t.vx >= 0) {
				t.vx = 0;
			}
		} else {
			t.vx = 0;
		}
		t._x += checkWalls(t, t.vx, true);
	}
	adjustToFloor(t);
	if (!t.DOWN_PRESSED) {
		t.state = STAND;
	}
}

void doWalk(playerState &t) {
	if (t.hit) {
		finishHit(t);
	}
	calculateDistance(t, false);
	if (t.DOWN_PRESSED) {
		t.state = DUCK;
		return;
	}
	if (t.UP_PRESSED) {
		startJump(t);
		return;
	}
	if (t.left_edge && !getOnGround(t.rx, t._y)) {
		t.fall_anim_count = 0;
		t.vy = 0;
		t.state = FALL;
		return;
	}
	if (t.right_edge && !getOnGround(t.lx, t._y)) {
		t.fall_anim_count = 0;
		t.vy = 0;
		t.state = FALL;
		return;
	}
	if (!t.left_edge && !t.right_edge && !getOnGround(t._x, t._y)) {
		t.fall_anim_count = 0;
		t.vy = 0;
		t.state = FALL;
		return;
	}
	
	if (t.DIR_PRESSED == LEFT) {
		if (abs(t.vx) < 12 << 1) {
			if (t.vx > 0) {
				t.vx -= 3;
			} else {
				t.vx -= 2;
			}
		} else {
			if (t.vx > 0) {
				t.vx -= 3;
			}
			if (abs(t.vx) > 12 << 1) {
				t.vx = - (12 << 1);
			}
		}
		t.state = WALK;
		t.dir = t.DIR_PRESSED;
	} else if (t.DIR_PRESSED == RIGHT) {
		if (abs(t.vx) < 12 << 1) {
			if (t.vx < 0) {
				t.vx += 3;
			} else {
				t.vx += 2;
			}
		} else {
			if (t.vx < 0) {
				t.vx += 3;
			}
			if (abs(t.vx) > 12 << 1) {
				t.vx = 12 << 1;
			}
		}
		t.state = WALK;
		t.dir = t.DIR_PRESSED;
	} else {
		t.state = STAND;
	}
	
	t._x += checkWalls(t, t.vx, false);
	adjustToFloor(t);
}

void doJump(playerState &t) {
	calculateDistance(t, true);
	t.hit_ceiling = false;
	t.vy++;
	if (t.vy > 12) {
		t.vy = 12;
	}
	if (t.vy < 0) {
		t._y += checkCeiling(t, t.vy);
	} else {
		t._y += checkFloor(t, t.vy);
	}
	
	if (t.DIR_PRESSED == LEFT) {
		if (abs(t.vx) < 12 << 1) {
			if (t.vx > 0) {
				t.vx -= 3;
			} else {
				t.vx -= 2;
			}
		} else {
			if (t.vx > 0) {
				t.vx -= 3;
			}
			if (abs(t.vx) > 12 << 1) {
				t.vx = - (12 << 1);
			}
		}
		
		t.dir = t.DIR_PRESSED;
	} else if (t.DIR_PRESSED == RIGHT) {
		if (abs(t.vx) < 12 << 1) {
			if (t.vx < 0) {
				t.vx += 3;
			} else {
				t.vx += 2;
			}
		} else {
			if (t.vx < 0) {
				t.vx += 3;
			}
			if (abs(t.vx) > 12 << 1) {
				t.vx = 12 << 1;
			}
		}
		
		t.dir = t.DIR_PRESSED;
	} else if (t.vx > 0) {
		t.vx -= 2;
	} else if (t.vx < 0) {
		t.vx += 2;
	}
	
	t.wall_jump = false;
	t._x += checkWalls(t, t.vx, false);
	if (t.wall_jump) {
		if (t.dir == LEFT && t.DIR_PRESSED == LEFT ||
		t.dir == RIGHT && t.DIR_PRESSED == RIGHT) {
			t.wall_count++;
			if (t.wall_count >= 3) {
				startWall(t);
				return;
			}
		}
	}
	
	if (t.vy > 0 && !t.hit_ceiling) {
		adjustToFloor(t);
	}
	if (t.left_edge) {
		if (getOnGround(t.rx, t._y)) {
			if (!t.UP_PRESSED) {
				//t.can_jump = true;
			}
			if (t.vx == 0) {
				t.state = STAND;
			} else {
				t.state = WALK;
			}
		}
	} else if (t.right_edge) {
		if (getOnGround(t.lx, t._y)) {
			if (!t.UP_PRESSED) {
				//t.can_jump = true;
			}
			if (t.vx == 0) {
				t.state = STAND;
			} else {
				t.state = WALK;
			}
		}
	} else if (getOnGround(t._x, t._y)) {
		if (!t.UP_PRESSED) {
			//t.can_jump = true;
		}
		if (t.vx == 0) {
			t.state = STAND;
		} else {
			t.state = WALK;
		}
	}
}

void doFall(playerState &t) {
	calculateDistance(t, false);
	t.vy++;
	if (t.vy > 12) {
		t.vy = 12;
	}
	t._y += checkFloor(t, t.vy);
	
	if (t.DIR_PRESSED == LEFT) {
		if (abs(t.vx) < 12 << 1) {
			if (t.vx > 0) {
				t.vx -= 3;
			} else {
				t.vx -= 2;
			}
		} else {
			if (t.vx > 0) {
				t.vx -= 3;
			}
			if (abs(t.vx) > 12 << 1) {
				t.vx = - (12 << 1);
			}
		}
		
		t.dir = t.DIR_PRESSED;
	} else if (t.DIR_PRESSED == RIGHT) {
		if (abs(t.vx) < 12 << 1) {
			if (t.vx < 0) {
				t.vx += 3;
			} else {
				t.vx += 2;
			}
		} else {
			if (t.vx < 0) {
				t.vx += 3;
			}
			if (abs(t.vx) > 12 << 1) {
				t.vx = 12 << 1;
			}
		}
		
		t.dir = t.DIR_PRESSED;
	} else if (t.vx > 0) {
		t.vx -= 2;
	} else if (t.vx < 0) {
		t.vx += 2;
	}
	
	t.wall_jump = false;
	t._x += checkWalls(t, t.vx, false);
	if (t.wall_jump) {
		if (t.dir == LEFT && t.DIR_PRESSED == LEFT ||
		t.dir == RIGHT && t.DIR_PRESSED == RIGHT) {
			t.wall_count++;
			if (t.wall_count >= 3) {
				startWall(t);
				return;
			}
		}
	}
	
	if (t.vy > 0) {
		adjustToFloor(t);
	}
	if (t.left_edge) {
		if (getOnGround(t.rx, t._y)) {
			if (!t.UP_PRESSED) {
				//t.can_jump = true;
			}
			if (t.vx == 0) {
				t.state = STAND;
			} else {
				t.state = WALK;
			}
		}
	} else if (t.right_edge) {
		if (getOnGround(t.lx, t._y)) {
			if (!t.UP_PRESSED) {
				//t.can_jump = true;
			}
			if (t.vx == 0) {
				t.state = STAND;
			} else {
				t.state = WALK;
			}
		}
	} else if (getOnGround(t._x, t._y)) {
		if (!t.UP_PRESSED) {
			//t.can_jump = true;
		}
		if (t.vx == 0) {
			t.state = STAND;
		} else {
			t.state = WALK;
		}
	}
}

void doWall(playerState &t) {
	if (t.hit) {
		finishHit(t);
	}
	t._y++;
	
	if (t.dir == LEFT && t.DIR_PRESSED == LEFT) {
		t.fall_count = 0;
		if (getOnGround(t._x, t._y)) {
			t.state = STAND;
			return;
		}
		if (getInGround(t._x, t._y)) {
			t.state = STAND;
			adjustToFloor(t);
			return;
		}
		if (!getOnWall(t, t._x - (13 << 1), t._y - 30)) {
			t.fall_anim_count = 0;
			t.state = FALL;
			return;
		}
	} else if (t.dir == RIGHT && t.DIR_PRESSED == RIGHT) {
		t.fall_count = 0;
		if (getOnGround(t._x, t._y)) {
			t.state = STAND;
			return;
		}
		if (getInGround(t._x, t._y)) {
			t.state = STAND;
			adjustToFloor(t);
			return;
		}
		if (!getOnWall(t, t._x + (13 << 1), t._y - 30)) {
			t.fall_anim_count = 0;
			t.state = FALL;
			return;
		}
	} else if (t.dir == LEFT && t.DIR_PRESSED == RIGHT) {
		t.vx = 6 << 1;
		startWallJump(t);
	} else if (t.dir == RIGHT && t.DIR_PRESSED == LEFT) {
		t.vx = -(6 << 1);
		startWallJump(t);
	} else {
		t.fall_count++;
		if (t.fall_count >= 5) {
			t.fall_anim_count = 0;
			t.state = FALL;
			if (t.dir == LEFT) {
				t.dir = RIGHT;
			} else if (t.dir == RIGHT) {
				t.dir = LEFT;
			}
		}
	}
}

void doSpecial(playerState &t);

void update(playerState &t) {
	updateAnim(t);
	//checkScreenBounds(t);
	
	switch (t.state) {
		case STAND:
			doStand(t);
			break;
		case DUCK:
			doDuck(t);
			break;
		case WALK:
			doWalk(t);
			break;
		case JUMP:
			doJump(t);
			break;
		case FALL:
			doFall(t);
			break;
		case WALL:
			doWall(t);
			break;
		case DIE:
			t._y++;
			adjustToFloor(t);
			break;
		default:
			break;
	}
	
	doSpecial(t);
	
	/*if (t.hit_count > 0) {
		t.hit_count--;
	}*/
}
