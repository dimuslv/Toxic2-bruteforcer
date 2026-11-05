constexpr uint STAND_COUNT = 22 * 3;

constexpr uint DUCK_NORMAL_COUNT = 22 * 3;
constexpr uint DUCK_SPECIAL_COUNT = 22 * 40 * 3;
constexpr uint DUCK_COUNT = 2*DUCK_NORMAL_COUNT + DUCK_SPECIAL_COUNT;

constexpr uint WALK_NORMAL_COUNT = 51 * 3;
constexpr uint WALK_COUNT = 2*WALK_NORMAL_COUNT;

constexpr uint JUMP_ONEVY_COUNT = 51 * 3;
constexpr uint JUMP_VY_COUNT = 28;
constexpr uint JUMP_COUNT = JUMP_VY_COUNT * JUMP_ONEVY_COUNT;

constexpr uint FALL_NORMAL_ONEVY_COUNT = 51 * 3;
constexpr uint FALL_FIRSTTWO_VYS = animAccuracy? 8 : 2;
constexpr uint FALL_VY_COUNT = FALL_FIRSTTWO_VYS + 10;
constexpr uint FALL_NORMAL_COUNT = FALL_VY_COUNT * FALL_NORMAL_ONEVY_COUNT;
constexpr uint FALL_SPECIAL_COUNT = 4 * 2;
constexpr uint FALL_COUNT = FALL_NORMAL_COUNT + FALL_SPECIAL_COUNT;

constexpr uint WALL_NORMAL_COUNT = 5;
constexpr uint WALL_SPECIAL_COUNT = 49 * 5;
constexpr uint WALL_COUNT = WALL_NORMAL_COUNT + WALL_SPECIAL_COUNT;


constexpr uint OFS_STAND= 0;
constexpr uint OFS_DUCK = OFS_STAND+STAND_COUNT;
constexpr uint OFS_WALK = OFS_DUCK + DUCK_COUNT;
constexpr uint OFS_JUMP = OFS_WALK + WALK_COUNT;
constexpr uint OFS_FALL = OFS_JUMP + JUMP_COUNT;
constexpr uint OFS_WALL = OFS_FALL + FALL_COUNT;
constexpr uint OFS_END  = OFS_WALL + WALL_COUNT;//9522 / 10440

State extractState(uint &info) {
	if (info >= OFS_JUMP) {
		if (info >= OFS_WALL) {
			info -= OFS_WALL;
			return WALL;
		} else if (info >= OFS_FALL) {
			info -= OFS_FALL;
			return FALL;
		} else {
			info -= OFS_JUMP;
			return JUMP;
		}
	} else {
		if (info >= OFS_WALK) {
			info -= OFS_WALK;
			return WALK;
		} else if (info >= OFS_DUCK) {
			info -= OFS_DUCK;
			return DUCK;
		} else {
			info -= OFS_STAND;
			return STAND;
		}
	}
}

class ToxicSet {
	static constexpr uint FLOOR_NORMAL_COUNT = STAND_COUNT + DUCK_NORMAL_COUNT + WALK_NORMAL_COUNT;
	static constexpr uint FLOOR_NOUPWARP_COUNT = DUCK_NORMAL_COUNT + WALK_NORMAL_COUNT;
	static constexpr uint WALL_NORMAL_FALL_SPECIAL_COUNT = WALL_NORMAL_COUNT + FALL_SPECIAL_COUNT;
	
	using headType = conditional<
		((ull)level_width*level_height*2*2*customSize > UINT_MAX),
		ull, uint
	>::type;
	
	bitsetMap<headType, 2*FLOOR_NORMAL_COUNT> floorNormal;
	bitsetMap<headType, 2*FLOOR_NOUPWARP_COUNT> floorNoupwarp;
	bitsetMap<headType, 2*JUMP_ONEVY_COUNT> jump[JUMP_VY_COUNT];
	bitsetMap<headType, 2*FALL_NORMAL_ONEVY_COUNT> fallNormal[FALL_VY_COUNT];
	bitsetMap<headType, 2*WALL_NORMAL_FALL_SPECIAL_COUNT> wallNormalFallSpecial;
	set<ull> duckSpecial;
	set<ull> wallSpecial;
	
	enum ContainerID {
		FLOOR_NORMAL,
		FLOOR_NOUPWARP,
		JUMP,
		FALL_NORMAL,
		WALL_NORMAL_FALL_SPECIAL,
		DUCK_SPECIAL,
		WALL_SPECIAL
	};
	
	ull sizeVal = 0;
	
	tuple<ContainerID, ull, uint, uc> getInfo (ull cState) {
		ull head = cState / OFS_END / 2;
		bool dir = cState / OFS_END % 2;
		uint tail = cState % OFS_END;
		ContainerID cid;
		uc vy = 0;
		
		State st = extractState(tail);
		
		uint wc = tail % 3;
		uint rTail = tail / 3;
		
		uint extra = 0;
		
		switch (st) {
			case STAND:
				cid = FLOOR_NORMAL;
				break;
			case DUCK:
				if (rTail % 42 >= 2) {
					cid = DUCK_SPECIAL;
					head = head * DUCK_COUNT + tail;
					break;
				}
				extra -= DUCK_NORMAL_COUNT;
			case WALK:
				extra += DUCK_NORMAL_COUNT;

				if (rTail % 2 == 0) {
					cid = FLOOR_NORMAL;
					tail = rTail / 2 * 3 + wc;
					tail += STAND_COUNT + extra;
				} else {
					cid = FLOOR_NOUPWARP;
					tail = rTail / 2 * 3 + wc;
					tail += extra;
				}
				break;
			case State::JUMP:
				cid = JUMP;
				vy = rTail % JUMP_VY_COUNT;
				tail = rTail / JUMP_VY_COUNT * 3 + wc;
				break;
			case FALL:
				if (tail >= FALL_NORMAL_COUNT) {
					cid = WALL_NORMAL_FALL_SPECIAL;
					tail = tail - FALL_NORMAL_COUNT + WALL_NORMAL_COUNT;
				} else {
					cid = FALL_NORMAL;
					vy = rTail % FALL_VY_COUNT;
					tail = rTail / FALL_VY_COUNT * 3 + wc;
				}
				break;
			case WALL:
				if (tail / 5 % 50 + head / 2 / customSize % 2 == 25) {
					cid = WALL_NORMAL_FALL_SPECIAL;
					tail %= 5;
				} else {
					cid = WALL_SPECIAL;
					head = head * WALL_COUNT + tail;
				}
				break;
		}
		
		if (cid == DUCK_SPECIAL || cid == WALL_SPECIAL) {
			head = head * 2 + dir;
		} else {
			tail = tail * 2 + dir;
		}
		
		return {cid, head, tail, vy};
	}
	
	public:
	
	bool count(ull cState) {
		auto [cid, head, tail, vy] = getInfo(cState);
		
		switch (cid) {
			case FLOOR_NORMAL:
				return floorNormal.count(head, tail);
			case FLOOR_NOUPWARP:
				return floorNoupwarp.count(head, tail);
			case JUMP:
				return jump[vy].count(head, tail);
			case FALL_NORMAL:
				return fallNormal[vy].count(head, tail);
			case WALL_NORMAL_FALL_SPECIAL:
				return wallNormalFallSpecial.count(head, tail);
			case DUCK_SPECIAL:
				return duckSpecial.count(head);
			case WALL_SPECIAL:
				return wallSpecial.count(head);
		}
		
		return false;
	}
	
	bool insert(ull cState) {
		sizeVal++;
		
		auto [cid, head, tail, vy] = getInfo(cState);
		
		switch (cid) {
			case FLOOR_NORMAL:
				return floorNormal.insert(head, tail);
			case FLOOR_NOUPWARP:
				return floorNoupwarp.insert(head, tail);
			case JUMP:
				return jump[vy].insert(head, tail);
			case FALL_NORMAL:
				return fallNormal[vy].insert(head, tail);
			case WALL_NORMAL_FALL_SPECIAL:
				return wallNormalFallSpecial.insert(head, tail);
			case DUCK_SPECIAL:
				return duckSpecial.insert(head).second;
			case WALL_SPECIAL:
				return wallSpecial.insert(head).second;
		}
		
		return false;
	}
	
	ull size() {
		return sizeVal;
	}
	
	void clear() {
		floorNormal.clear();
		floorNoupwarp.clear();
		for (auto c : jump) c.clear();
		for (auto c : fallNormal) c.clear();
		wallNormalFallSpecial.clear();
		duckSpecial.clear();
		wallSpecial.clear();
	}
};

map<ull, ull> states;
conditional<useToxicSet, ToxicSet, set<ull>>::type been;
set<ull> curBeen;
map<ull, us> shortMap;
map<ull, us> curShortMap;

ull compressState(const playerState &p) {
	ull a = 0;
	a += (p._y - 40);
	
	a *= level_width * 2;
	a += p._x.twiceValue;
	
	a *= customSize;
	a += p.custom;
	
	a *= 2;
	a += int(p.hit);
	
	a *= 2;
	a += p.dir;
	
	bool wallFall = p.anim == WALL && p.state == FALL && p.vy < 2;
	uint b = 0;
	
	switch (p.state) {
		case STAND:
		case DUCK: {
			b *= 22;
			int vx = p.vx.round();
			if (vx < -2) {
				b += vx + 12;
			} else if (vx < 3) {
				b += 10;
			} else {
				b += vx + 8;
			}
			break;
		}
		case FALL:
			if (wallFall) {
				b *= 4;
				b += min(abs(p.vx.twiceValue), 3);
				break;
			}
		case WALK:
		case JUMP:
			b *= 51;
			b += (p.vx + 12_5).twiceValue;
			break;
		case WALL:
			b *= 50;
			b += 25 - p._x.round();
			if (p.left_edge) {
				b += p.rx.truncate();
			} else if (p.right_edge) {
				b += p.lx.truncate();
			} else {
				b += p._x.truncate();
			}
			break;
	}
	
	switch (p.state) {
		case DUCK:
			b *= 42;
			if (p.vy < 0) {
				b++;
			} else if (p.right_edge || p.left_edge) {
				int d = dUp(p.right_edge? p.lx : p.rx, p._y);
				if (d > 1 && d <= 101) {
					b += 2;
					b += (p._x - p.oldX).truncate() + 19;
				} else {
					b++;
				}
			}
			
			/*b *= 2;
			if (p.right_edge || p.left_edge || p.vy < 0) {
				b++;
			}*/
			break;
		case WALK:
			b *= 2;
			b += p.vy < 0;
			break;
		case JUMP:
			b *= 28;
			b += min(p.vy, 11) + 16;
			break;
		case FALL:
			if (wallFall) {
				b *= 2;
			} else {
				b *= FALL_FIRSTTWO_VYS + 10;
				
				if (animAccuracy && p.vy < 2) {
					b += 2 * p.animDir;
					b += 4 * (p.anim == WALK);
				} else {
					b += FALL_FIRSTTWO_VYS - 2;
				}
			}
			
			b += min(p.vy, 11);
			break;
	}
	
	if (p.state == WALL) {
		b *= 5;
		b += p.fall_count;
	} else if (wallFall) {
		b += FALL_NORMAL_COUNT;
	} else {
		b *= 3;
		b += p.wall_count;
	}
	
	switch (p.state) {
		case STAND:
			b += OFS_STAND;
			break;
		case DUCK:
			b += OFS_DUCK;
			break;
		case WALK:
			b += OFS_WALK;
			break;
		case JUMP:
			b += OFS_JUMP;
			break;
		case FALL:
			b += OFS_FALL;
			break;
		case WALL:
			b += OFS_WALL;
			break;
	}
	
	a *= OFS_END;
	a += b;
	
	return a;
}

playerState uncompressState(ull c) {
	playerState p;
	
	uint temp = c % OFS_END;
	c /= OFS_END;
	
	p.dir = p.animDir = static_cast<Dir>(c % 2);
	c /= 2;
	
	p.hit = c % 2;
	c /= 2;
	
	p.custom = c % customSize;
	c /= customSize;
	
	p._x = div2(c % (level_width * 2));
	c /= level_width * 2;
	
	p._y = c + 40;
	
	p.state = p.anim = extractState(temp);
	
	if (p.state == WALL) {
		p.fall_count = temp % 5;
		temp /= 5;
	} else if (p.state == FALL && temp >= FALL_NORMAL_COUNT) {
		temp -= FALL_NORMAL_COUNT;
		p.anim = WALL;
	} else {
		p.wall_count = temp % 3;
		temp /= 3;
	}
	
	switch (p.state) {
		case DUCK:
			if (temp % 42 == 1) {
				p.vy = -1;
			} else if (temp % 42 > 1) {
				int oldXDisplacement = (temp % 42) - 2 - 19;
				p.anim = STAND;
				p._x -= oldXDisplacement;
				p.calculateDistance(false);
				p._x += oldXDisplacement;
				p.anim = DUCK;
			}
			temp /= 42;
			break;
		case WALK:
			p.vy = -(temp % 2);
			temp /= 2;
			break;
		case JUMP:
			p.vy = temp % 28 - 16;
			temp /= 28;
			break;
		case FALL:
			if (p.anim == WALL) {
				p.vy = p.fall_anim_count = temp % 2;
				temp /= 2;
			} else {
				uint temp2 = temp % (FALL_FIRSTTWO_VYS + 10);
				temp /= FALL_FIRSTTWO_VYS + 10;
				
				if (animAccuracy && temp2 < FALL_FIRSTTWO_VYS) {
					p.vy = p.fall_anim_count = temp2 % 2;
					temp2 /= 2;
					p.animDir = static_cast<Dir>(temp2 % 2);
					temp2 /= 2;
					p.anim = temp2? WALK : STAND;
				} else {
					p.vy = temp2 - FALL_FIRSTTWO_VYS + 2;
				}
			}
			break;
	}
	
	switch (p.state) {
		case STAND:
		case DUCK:
			if (temp % 22 < 10) {
				p.vx = temp % 22 - 12;
			} else if (temp % 22 == 10) {
				p.vx = 0;
			} else {
				p.vx = temp % 22 - 8;
			}
			temp /= 22;
			break;
		case FALL:
			if (p.anim == WALL) {
				p.vx = div2(temp % 4);
				temp /= 4;
				
				if (p.vx == 1_5) p.vx = 2;
				
				p.vx *= -1 + 2*p.dir;
				
				break;
			}
		case WALK:
		case JUMP:
			p.vx = div2(temp % 51) - 12_5;
			temp /= 51;
			break;
		case WALL:
			p.left_edge = true;
			p.rx = p._x.round() + temp % 50 - 25;
			temp /= 50;
			break;
	}
	
	return p;
}

ull compressRelative(const playerState &p1, const playerState &p2) {
	return compressState(p1);
}

/*playerState uncompressRelative (uint c, playerState p2) {
	playerState p = uncompressAllButXY(c);
	p._x = p2._x + (c / OFS_END / 2) % 93 - 46;
	p._y = p2._y + c / OFS_END / 2 / 93 - 20;
	p.rx += p._x;
	return p;
}*/


playerState uncompressRelative(ull c, const playerState &p2) {
	return uncompressState(c);
}
