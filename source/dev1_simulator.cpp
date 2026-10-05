enum Dir {
	NONE = -1,
	LEFT = 0,
	RIGHT = 1,
	UP = 2,
	DOWN = 3
};

enum State {
	START,
	STAND,
	DUCK,
	WALK,
	JUMP,
	FALL,
	WALL,
	HIT,
	DIE,
	END
};

bool getOnGround(hint x, int y) {
	return collision(x,y) && !collision(x,y-1);
}

bool getInGround(hint x, int y) {
	return collision(x,y) && collision(x,y-1);
}

bool getInAir(hint x, int y) {
	return !collision(x,y) && !collision(x,y-1);
}

bool getInWall(hint x, int y) {
	return collision(x,y);
}

struct playerState {
	hint _x;
	int _y;
	hint vx = 0;
	int vy = 0;
	State anim = STAND;
	Dir animDir;
	State state = STAND;
	Dir dir;
	hint lx;
	hint rx;
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
	Dir DIR_PRESSED = NONE;
	
	int custom = 0;
	
	hint oldX;
	int metaData;
	
	bool getOnWall(hint x, int y) {
		if (dir == LEFT) {
			return collision(x,y) && !collision(x+1,y);
		} else {
			return collision(x,y) && !collision(x-1,y);
		}
	}
	
	void finishHit() {
		return;
	}
	
	void updateAnim() {
		if (state == FALL) {
			if (dir != animDir || state != anim) {
				fall_anim_count++;
				if (fall_anim_count >= 3) {
					anim = state;
					animDir = dir;
				}
			}
		} else {
			anim = state;
			animDir = dir;
		}
		if (hit) {
			anim = HIT;
		}
	}
	
	void calculateDistance(bool b) {
		if (anim == STAND || anim == WALK || anim == JUMP || anim == FALL || anim == HIT) {
			lx = _x - 12;
			rx = _x + 12;
		} else if (anim == DUCK) {
			if (dir == LEFT) {
				lx = _x - 26;
				rx = _x + 22;
			} else {
				lx = _x - 22;
				rx = _x + 26;
			}
		} else {
			lx = 0;
			rx = 0;
			right_edge = false;
			left_edge = false;
			return;
		}
		
		//int lh = 35;
		//int rh = 35;
		
		/*if (getInGround(lx, _y)) {
			lh = 0;
		}
		if (getInGround(rx, _y)) {
			rh = 0;
		}
		
		for (int i = 0; i <= 34; i++) {
			if (getOnGround(lx, _y + i)) {
				lh = i;
				break;
			}
		}
		for (int i = 0; i <= 34; i++) {
			if (getOnGround(rx, _y + i)) {
				rh = i;
				break;
			}
		}*/
		
		oldX = _x;
		
		int lh = 35;
		int rh = 35;
		
		int d = dDown(lx, _y-1);
		if (d < 0) {
			lh = min(35, -d-1);
		} else {
			int d2 = d - dDown(lx, _y-1+d);
			if (d2 <= 35) {
				lh = d2 - 1;
			} else if (d > 1) {
				lh = 0;
			}
		}
		
		d = dDown(rx, _y-1);
		if (d < 0) {
			rh = min(35, -d-1);
		} else {
			int d2 = d - dDown(rx, _y-1+d);
			if (d2 <= 35) {
				rh = d2 - 1;
			} else if (d > 1) {
				rh = 0;
			}
		}
		
		if (!b) {
			left_edge = lh >= 32 && rh == 0;
			right_edge = rh >= 32 && lh == 0;
		} else {
			left_edge = lh >= 32 && rh < lh;
			right_edge = rh >= 32 && lh < rh;
		}
	}

	void adjustToFloor() {
		if (vy < 0) return;
		
		/*if (left_edge) {
			if (getInGround(rx, _y)) {
				for (int i = 1; i <= 100; i++) {
					if (getOnGround(rx, _y - i)) {
						_y -= i;
						break;
					}
				}
			}
		} else if (right_edge) {
			if (getInGround(lx, _y)) {
				for (int i = 1; i <= 100; i++) {
					if (getOnGround(lx, _y - i)) {
						_y -= i;
						break;
					}
				}
			}
		} else if (getInGround(_x, _y)) {
			for (int i = 1; i <= 100; i++) {
				if (getOnGround(_x, _y - i)) {
					_y -= i;
					break;
				}
			}
		}*/
		
		int d;
		
		if (left_edge) {
			d = dUp(rx, _y);
		} else if (right_edge) {
			d = dUp(lx, _y);
		} else {
			d = dUp(_x, _y);
		}
		
		if (d > 1 && d <= 101) {
			_y -= d - 1;
		}
	}

	hint checkWalls(hint v, bool duck) {
		
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
			vx = 0;
			return 0;
		}
		
		if (anim == WALL || anim == START || anim == END || anim == DIE) {
			if (leftSideType == 1) return 0;
			if (leftSideType == 2) return v;
			
			if (getInWall(0, _y + offsets[0]) || getInWall(0, _y + offsets[1]) || getInWall(0, _y + offsets[2]) || getInWall(0, _y + offsets[3]) || getInWall(0, _y + offsets[4])) {
				return 0;
			}
			return v;
		}
		
		if (v > 0) {
			int w;
			if (anim == DUCK) {
				w = (animDir == RIGHT)? 26 : 22;
			} else {
				w = 12;
			}
			
			if (!(getInWall(_x + w + v, _y + offsets[0]) || getInWall(_x + w + v, _y + offsets[1]) || getInWall(_x + w + v, _y + offsets[2]) || getInWall(_x + w + v, _y + offsets[3]) || getInWall(_x + w + v, _y + offsets[4]))) {
				/*for (int i = 1; i <= 12; i++) {
					for (int j = 0; j < 5; j++) {
						if (getInWall(_x + w + i, _y + offsets[j])) {
							wall_jump = true;
							if (v < i) {
								return v;
							}
							vx = 0;
							return i - 1;
						}
					}
				}*/
				int d = -dRight(_x + w + 1, _y + offsets[0]);
				for (int i = 1; i < 5; i++) {
					d = min(d, -dRight(_x + w + 1, _y + offsets[i]));
				}
				
				if (d < 0) d = 0;
				
				if (d <= 11) {
					wall_jump = true;
					if (v < d + 1) {
						return v;
					}
					vx = 0;
					return d;
				}
				
				return v;
			}
			
			/*for (int i = 1; i <= 12; i++) {
				int freeCount = 0;
				for (int j = 0; j < 5; j++) {
					if (!getInWall(_x + w + v - i, _y + offsets[j])) {
						freeCount++;
					}
				}
				if (freeCount == 5) {
					vx = v - i;
					wall_jump = true;
					return v - i;
				}
			}*/
			
			int i = 0;
			bool wasUpdated;
			
			do {
				wasUpdated = false;
				
				for (int j = 0; j < 5; j++) {
					int d = dLeft(_x + w + v - i, _y + offsets[j]);
					if (d > 0) {
						i += d;
						if (i > 12) {
							return 0;
						}
						
						wasUpdated = true;
					}
				}
			} while (wasUpdated);
			
			vx = v - i;
			wall_jump = true;
			return v - i;
			
		} else if (v < 0) {
			int w;
			if (anim == DUCK) {
				w = (animDir == LEFT)? -26 : -22;
			} else {
				w = -12;
			}
			
			if (!(getInWall(_x + w + v, _y + offsets[0]) || getInWall(_x + w + v, _y + offsets[1]) || getInWall(_x + w + v, _y + offsets[2]) || getInWall(_x + w + v, _y + offsets[3]) || getInWall(_x + w + v, _y + offsets[4]))) {
				/*for (int i = 1; i <= 12; i++) {
					for (int j = 0; j < 5; j++) {
						if (getInWall(_x + w - i, _y + offsets[j])) {
							wall_jump = true;
							if (abs(v) < i) {
								return v;
							}
							vx = 0;
							return -(i - 1);
						}
					}
				}*/
				
				int d = -dLeft(_x + w - 1, _y + offsets[0]);
				for (int i = 1; i < 5; i++) {
					d = min(d, -dLeft(_x + w - 1, _y + offsets[i]));
				}
				
				if (d < 0) d = 0;
				
				if (d <= 11) {
					wall_jump = true;
					if (-v < d + 1) {
						return v;
					}
					vx = 0;
					return -d;
				}
				
				return v;
			}
			
			/*for (int i = 1; i <= 12; i++) {
				int freeCount = 0;
				for (int j = 0; j < 5; j++) {
					if (!getInWall(_x + w + v + i, _y + offsets[j])) {
						freeCount++;
					}
				}
				if (freeCount == 5) {
					vx = v + i;
					wall_jump = true;
					return v + i;
				}
			}*/
			
			int i = 0;
			bool wasUpdated;
			
			do {
				wasUpdated = false;
				
				for (int j = 0; j < 5; j++) {
					int d = dRight(_x + w + v + i, _y + offsets[j]);
					if (d > 0) {
						i += d;
						if (i > 12) {
							return 0;
						}
						
						wasUpdated = true;
					}
				}
			} while (wasUpdated);
			
			vx = v + i;
			wall_jump = true;
			return v + i;
		}
		
		return 0;
	}

	int checkCeiling(int v) {
		int m;
		if (dir == LEFT) {
			m = -1;
		} else if (dir == RIGHT) {
			m = 1;
		}
		if (v >= 0) {
			return v;
		}
		
		int h;
		if (anim == JUMP || anim == HIT) {
			h = -46;
		} else {
			return v;
		}
		
		if (getInWall(_x + 6 * m, _y + h)) {
			/*for (int i = 1; i < 16; i++) {
				if (!getInWall(_x + 6 * m, _y + h + i)) {
					vy = 0;
					return v + i;
				}
			}*/
			
			int d = dDown(_x + 6 * m, _y + h);
			
			if (d < 16) {
				vy = 0;
				return v + d;
			}
			
		} else if (getInWall(_x + 1 * m, _y + h)) {
			/*for (int i = 1; i < 16; i++) {
				if (!getInWall(_x + 1 * m, _y + h + i)) {
					vy = 0;
					return v + i;
				}
			}*/
			
			int d = dDown(_x + 1 * m, _y + h);
			
			if (d < 16) {
				vy = 0;
				return v + d;
			}
			
		} else if (getInWall(_x + 12 * m, _y + h)) {
			/*for (int i = 1; i < 16; i++) {
				if (!getInWall(_x + 12 * m, _y + h + i)) {
					vy = 0;
					return v + i;
				}
			}*/
			
			int d = dDown(_x + 12 * m, _y + h);
			
			if (d < 16) {
				vy = 0;
				return v + d;
			}
			
		} else {
			int d = min(min(-dUp(_x + 6 * m, _y + h), -dUp(_x + 1 * m, _y + h)), -dUp(_x + 12 * m, _y + h));
			
			if (d < -v) {
				vy = 0;
				hit_ceiling = true;
				return - d - 2;
			}
			
			/*for (int i = 1; i < abs(v); i++) {
				if (getInWall(_x + 6 * m, _y + h - i) ||
				getInWall(_x + 1 * m, _y + h - i) ||
				getInWall(_x + 12 * m, _y + h - i)) {
					vy = 0;
					hit_ceiling = true;
					return - i - 2;
				}
			}*/
		}
		
		return v;
	}

	int checkFloor(int v) {
		//v = Math.round(v);
		/*for (int i = 1; i <= v; i++) {
			if (getInWall(_x, _y + i)) {
				return i;
			}
		}
		return v;*/
		return min(v, max(0, -dDown(_x, _y + 1)) + 1);
	}

	void startJump() {
		//if (can_jump) {
			vy = -16;
			state = JUMP;
			//can_jump = false;
		//}
	}

	void startWall() {
		state = WALL;
		wall_count = 0;
		fall_count = 0;
		vy = 0;
		vx = 0;
		//can_jump = true;
	}

	void startWallJump() {
		//if (can_jump) {
			vy = -12;
			state = JUMP;
			//can_jump = false;
		//}
	}

	void startHit() {
		hit = true;
		vy = -8;
		state = JUMP;
		updateAnim();
	}

	void doStand() {
		if (hit) {
			finishHit();
		}
		calculateDistance(false);
		vy = 0;
		
		if (vx != 0) {
			vx = vx.roundUp();
			if (vx > 0) {
				vx -= 2;
				if (vx <= 0) {
					vx = 0;
				}
			} else {
				vx += 2;
				if (vx >= 0) {
					vx = 0;
				}
			}
			_x += checkWalls(vx, false);
		}
		
		if (DOWN_PRESSED) {
			state = DUCK;
			return;
		}
		if (UP_PRESSED) {
			startJump();
			return;
		}
		if (DIR_PRESSED != NONE) {
			if (left_edge) {
				if (getOnGround(rx, _y)) {
					state = WALK;
					dir = DIR_PRESSED;
				}
			} else if (right_edge) {
				if (getOnGround(lx, _y)) {
					state = WALK;
					dir = DIR_PRESSED;
				}
			} else if (getOnGround(_x, _y)) {
				state = WALK;
				dir = DIR_PRESSED;
			}
		} else {
			if (left_edge) {
				if (!getOnGround(rx, _y)) {
					fall_anim_count = 0;
					vy = 0;
					state = FALL;
					return;
				}
			} else if (right_edge) {
				if (!getOnGround(lx, _y)) {
					fall_anim_count = 0;
					vy = 0;
					state = FALL;
					return;
				}
			} else if (!getOnGround(_x, _y)) {
				fall_anim_count = 0;
				vy = 0;
				state = FALL;
				return;
			}
			adjustToFloor();
		}
	}

	void doDuck() {
		if (vx != 0) {
			vx = vx.roundUp();
			if (vx > 0) {
				vx -= 2;
				if (vx <= 0) {
					vx = 0;
				}
			} else if (vx < 0) {
				vx += 2;
				if (vx >= 0) {
					vx = 0;
				}
			} else {
				vx = 0;
			}
			_x += checkWalls(vx, true);
		}
		adjustToFloor();
		if (!DOWN_PRESSED) {
			state = STAND;
		}
	}

	void doWalk() {
		if (hit) {
			finishHit();
		}
		calculateDistance(false);
		if (DOWN_PRESSED) {
			state = DUCK;
			return;
		}
		if (UP_PRESSED) {
			startJump();
			return;
		}
		if (left_edge && !getOnGround(rx, _y)) {
			fall_anim_count = 0;
			vy = 0;
			state = FALL;
			return;
		}
		if (right_edge && !getOnGround(lx, _y)) {
			fall_anim_count = 0;
			vy = 0;
			state = FALL;
			return;
		}
		if (!left_edge && !right_edge && !getOnGround(_x, _y)) {
			fall_anim_count = 0;
			vy = 0;
			state = FALL;
			return;
		}
		
		if (DIR_PRESSED == LEFT) {
			if (abs(vx) < 12) {
				if (vx > 0) {
					vx -= 1_5;
				} else {
					vx -= 1;
				}
			} else {
				if (vx > 0) {
					vx -= 1_5;
				}
				if (abs(vx) > 12) {
					vx = -12;
				}
			}
			state = WALK;
			dir = DIR_PRESSED;
		} else if (DIR_PRESSED == RIGHT) {
			if (abs(vx) < 12) {
				if (vx < 0) {
					vx += 1_5;
				} else {
					vx += 1;
				}
			} else {
				if (vx < 0) {
					vx += 1_5;
				}
				if (abs(vx) > 12) {
					vx = 12;
				}
			}
			state = WALK;
			dir = DIR_PRESSED;
		} else {
			state = STAND;
		}
		
		_x += checkWalls(vx, false);
		adjustToFloor();
	}

	void doJump() {
		calculateDistance(true);
		hit_ceiling = false;
		vy++;
		if (vy > 12) {
			vy = 12;
		}
		if (vy < 0) {
			_y += checkCeiling(vy);
		} else {
			_y += checkFloor(vy);
		}
		
		if (DIR_PRESSED == LEFT) {
			if (abs(vx) < 12) {
				if (vx > 0) {
					vx -= 1_5;
				} else {
					vx -= 1;
				}
			} else {
				if (vx > 0) {
					vx -= 1_5;
				}
				if (abs(vx) > 12) {
					vx = -12;
				}
			}
			
			dir = DIR_PRESSED;
		} else if (DIR_PRESSED == RIGHT) {
			if (abs(vx) < 12) {
				if (vx < 0) {
					vx += 1_5;
				} else {
					vx += 1;
				}
			} else {
				if (vx < 0) {
					vx += 1_5;
				}
				if (abs(vx) > 12) {
					vx = 12;
				}
			}
			
			dir = DIR_PRESSED;
		} else if (vx > 0) {
			vx -= 1;
		} else if (vx < 0) {
			vx += 1;
		}
		
		wall_jump = false;
		_x += checkWalls(vx, false);
		if (wall_jump) {
			if (dir == LEFT && DIR_PRESSED == LEFT ||
			dir == RIGHT && DIR_PRESSED == RIGHT) {
				wall_count++;
				if (wall_count >= 3) {
					startWall();
					return;
				}
			}
		}
		
		if (vy > 0 && !hit_ceiling) {
			adjustToFloor();
		}
		if (left_edge) {
			if (getOnGround(rx, _y)) {
				if (!UP_PRESSED) {
					//can_jump = true;
				}
				if (vx == 0) {
					state = STAND;
				} else {
					state = WALK;
				}
			}
		} else if (right_edge) {
			if (getOnGround(lx, _y)) {
				if (!UP_PRESSED) {
					//can_jump = true;
				}
				if (vx == 0) {
					state = STAND;
				} else {
					state = WALK;
				}
			}
		} else if (getOnGround(_x, _y)) {
			if (!UP_PRESSED) {
				//can_jump = true;
			}
			if (vx == 0) {
				state = STAND;
			} else {
				state = WALK;
			}
		}
	}

	void doFall() {
		calculateDistance(false);
		vy++;
		if (vy > 12) {
			vy = 12;
		}
		_y += checkFloor(vy);
		
		if (DIR_PRESSED == LEFT) {
			if (abs(vx) < 12) {
				if (vx > 0) {
					vx -= 1_5;
				} else {
					vx -= 1;
				}
			} else {
				if (vx > 0) {
					vx -= 1_5;
				}
				if (abs(vx) > 12) {
					vx = -12;
				}
			}
			
			dir = DIR_PRESSED;
		} else if (DIR_PRESSED == RIGHT) {
			if (abs(vx) < 12) {
				if (vx < 0) {
					vx += 1_5;
				} else {
					vx += 1;
				}
			} else {
				if (vx < 0) {
					vx += 1_5;
				}
				if (abs(vx) > 12) {
					vx = 12;
				}
			}
			
			dir = DIR_PRESSED;
		} else if (vx > 0) {
			vx -= 1;
		} else if (vx < 0) {
			vx += 1;
		}
		
		wall_jump = false;
		_x += checkWalls(vx, false);
		if (wall_jump) {
			if (dir == LEFT && DIR_PRESSED == LEFT ||
			dir == RIGHT && DIR_PRESSED == RIGHT) {
				wall_count++;
				if (wall_count >= 3) {
					startWall();
					return;
				}
			}
		}
		
		if (vy > 0) {
			adjustToFloor();
		}
		if (left_edge) {
			if (getOnGround(rx, _y)) {
				if (!UP_PRESSED) {
					//can_jump = true;
				}
				if (vx == 0) {
					state = STAND;
				} else {
					state = WALK;
				}
			}
		} else if (right_edge) {
			if (getOnGround(lx, _y)) {
				if (!UP_PRESSED) {
					//can_jump = true;
				}
				if (vx == 0) {
					state = STAND;
				} else {
					state = WALK;
				}
			}
		} else if (getOnGround(_x, _y)) {
			if (!UP_PRESSED) {
				//can_jump = true;
			}
			if (vx == 0) {
				state = STAND;
			} else {
				state = WALK;
			}
		}
	}

	void doWall() {
		if (hit) {
			finishHit();
		}
		_y++;
		
		if (dir == LEFT && DIR_PRESSED == LEFT) {
			fall_count = 0;
			if (getOnGround(_x, _y)) {
				state = STAND;
				return;
			}
			if (getInGround(_x, _y)) {
				state = STAND;
				adjustToFloor();
				return;
			}
			if (!getOnWall(_x - 13, _y - 30)) {
				fall_anim_count = 0;
				state = FALL;
				return;
			}
		} else if (dir == RIGHT && DIR_PRESSED == RIGHT) {
			fall_count = 0;
			if (getOnGround(_x, _y)) {
				state = STAND;
				return;
			}
			if (getInGround(_x, _y)) {
				state = STAND;
				adjustToFloor();
				return;
			}
			if (!getOnWall(_x + 13, _y - 30)) {
				fall_anim_count = 0;
				state = FALL;
				return;
			}
		} else if (dir == LEFT && DIR_PRESSED == RIGHT) {
			vx = 6;
			startWallJump();
		} else if (dir == RIGHT && DIR_PRESSED == LEFT) {
			vx = -6;
			startWallJump();
		} else {
			fall_count++;
			if (fall_count >= 5) {
				fall_anim_count = 0;
				state = FALL;
				if (dir == LEFT) {
					dir = RIGHT;
				} else if (dir == RIGHT) {
					dir = LEFT;
				}
			}
		}
	}
	
	friend void doSpecial(playerState &p);

	void update() {
		updateAnim();
		//checkScreenBounds();
		
		switch (state) {
			case STAND:
				doStand();
				break;
			case DUCK:
				doDuck();
				break;
			case WALK:
				doWalk();
				break;
			case JUMP:
				doJump();
				break;
			case FALL:
				doFall();
				break;
			case WALL:
				doWall();
				break;
			case DIE:
				_y++;
				adjustToFloor();
				break;
			default:
				break;
		}
		
		doSpecial(*this);
		
		/*if (hit_count > 0) {
			hit_count--;
		}*/
	}
};
