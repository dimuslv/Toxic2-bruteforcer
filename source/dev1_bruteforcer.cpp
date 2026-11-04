map<ull, ull> states;
set<ull> been;
set<ull> curBeen;
map<ull, us> shortMap;
map<ull, us> curShortMap;

constexpr uint STAND_COUNT = 22 * 3;

constexpr uint DUCK_NORMAL_COUNT = 22 * 3;
constexpr uint DUCK_NOUPWARP_COUNT = 22 * 3;
constexpr uint DUCK_SPECIAL_COUNT = 22 * 40 * 3;
constexpr uint DUCK_COUNT = DUCK_NORMAL_COUNT + DUCK_NOUPWARP_COUNT + DUCK_SPECIAL_COUNT;

constexpr uint WALK_NORMAL_COUNT = 51 * 3;
constexpr uint WALK_NOUPWARP_COUNT = 51 * 3;
constexpr uint WALK_COUNT = WALK_NORMAL_COUNT + WALK_NOUPWARP_COUNT;

constexpr uint JUMP_ONEVY_COUNT = 51 * 3;
constexpr uint JUMP_COUNT = 28 * JUMP_ONEVY_COUNT;

constexpr uint FALL_NORMAL_ONEVY_COUNT = 51 * 3;
constexpr uint FALL_FIRSTTWO_VYS = animAccuracy? 8 : 2;
constexpr uint FALL_NORMAL_COUNT = (FALL_FIRSTTWO_VYS + 10) * FALL_NORMAL_ONEVY_COUNT;
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

string stateInfoLine(const playerState &p) {
	stringstream result;
	result << "x: " << p._x << ", ";
	result << "y: " << p._y << ", ";
	result << "vx: " << p.vx << ", ";
	result << "vy: " << p.vy << ", ";
	result << "state: " << p.state << ", ";
	result << "dir: " << p.dir << ", ";
	result << "wc: " << p.wall_count << '\n';
	return result.str();
}

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
	
	if (temp >= OFS_JUMP) {
		if (temp >= OFS_WALL) {
			p.state = WALL;
			temp -= OFS_WALL;
		} else if (temp >= OFS_FALL) {
			p.state = FALL;
			temp -= OFS_FALL;
		} else {
			p.state = JUMP;
			temp -= OFS_JUMP;
		}
	} else {
		if (temp >= OFS_WALK) {
			p.state = WALK;
			temp -= OFS_WALK;
		} else if (temp >= OFS_DUCK) {
			p.state = DUCK;
			temp -= OFS_DUCK;
		} else {
			p.state = STAND;
			temp -= OFS_STAND;
		}
	}
	
	p.anim = p.state;
	
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

bool hasLostCustom(const playerState &p);

bool hasLost(const playerState &p) {
	return (p._x < 12 || p._x >= level_width - 12 || p._y > level_height - 64 || p._y < 46 || hasLostCustom(p));
}

bool hasWon(const playerState &p);

void updateWith(playerState &p, int inp) {
	switch (inp) {
		case 0:
			break;
		case 1:
			p.DIR_PRESSED = LEFT;
			break;
		case 2:
			p.DIR_PRESSED = RIGHT;
			break;
		case 3:
			p.UP_PRESSED = true;
			break;
		case 4:
			p.DOWN_PRESSED = true;
			break;
	}
	
	p.update();
}

playerState updateAndCheckFate(playerState p, int inp, bool &lost, bool &won) {
	//if (p.state != DUCK) {
		updateWith(p, inp);
		
		lost = hasLost(p);
		if (!lost) won = hasWon(p);
		
		/*if (p.state == DUCK) {
			update(p);
		}
	} else {
		lost = hasLost(p);
		won = hasWon(p);
		
		if (inp == 0) {
			p.state = STAND;
		} else if (inp == 4) {
			updateWith(p, 4);
		} else {
			lost = true;
		}
	}*/
	
	return p;
}

playerState updateWithFull(playerState p, int inp) {
	bool a;
	return updateAndCheckFate(p, inp, a, a);
}

bool skipInput(playerState &p, int &inp) {
	switch (p.state) {
		case JUMP:
		case FALL:
		case WALL:
			if (inp == 3) {
				inp = 4;
				return true;
			}
			break;
		case DUCK:
			if (inp == 1) {
				inp = 4;
			}
			break;
	}
	
	return false;
}

string solutionLine(string inputs, const playerState &startP, const playerState &endP);

void printSolution(ull p2ci, int e) {
	string ans = "";
	string letters[] = {"n", "a", "d", "w", "s"};
	bool broken = false;
	
	playerState endP = uncompressState(p2ci/e);
	ull pci = compressState(uncompressRelative(states[p2ci]/e, endP)) * e + (states[p2ci] % e);
	
	playerState temp, temp2;
	
	for (ull tci = pci, t2ci = p2ci; tci != 0;) {
		temp = uncompressState(tci / e);
		temp2 = uncompressState(t2ci / e);
		
		for (int k = 0; k < 6; k++) {
			if (k == 5) {
				ans = "f" + ans;
				broken = true;
				break;
			}
			
			if (compressState(updateWithFull(temp, k)) == t2ci / e) {
				ans = letters[k] + ans;
				break;
			}
		}
		
		if (states[tci] == 0) break;
		
		t2ci = tci;
		tci = compressState(uncompressRelative(states[tci] / e, temp)) * e + states[tci] % e;
	}
	
	print << solutionLine(ans, temp, endP);
	
	if (broken) {
		string rec = "";
		for (ull t3ci = p2ci; t3ci != 0;) {
			playerState temp3 = uncompressState(t3ci / e);
			stringstream info;
			info << stateInfoLine(temp3);
			info << t3ci << '\n';
			rec = info.str() + rec;
			
			if (states[t3ci] == 0) break;
			
			t3ci = compressState(uncompressRelative(states[t3ci] / e, temp3)) * e + states[t3ci] % e;
		}
		print << rec;
	}
}

void printMinInputTimeSolution(ull p2ci) {
	printSolution(p2ci, 4);
}

void printMinTimeSolution(ull p2ci) {
	printSolution(p2ci, 1);
}

void getStartStates(vector<playerState> &startStates);

int curFPs, minFPs;

void frameMinInputTime(playerState &p, int oldInp, int inp, vector<ull> &next, bool didntCheckAll) {
	if (inp >= 3 && p.state != STAND && p.state != WALK && p.state != DUCK) {
		return;
	}
	
	bool lost, won;
	playerState p2 = updateAndCheckFate(p, inp, lost, won);
	
	if (lost) {
		return;
	}
	
	ull p2c = compressState(p2);
	
	if (advancedMinInput) {
		if (compressState(p) == p2c) {
			if (didntCheckAll)
				for (int i = 0; i < 5; i++) {
					if (i == inp) continue;
					frameMinInputTime(p, oldInp, i, next, false);
				}
			return;
		}
	}
	
	ull p2ci = p2c * 4;
	
	if (inp < 3) {
		p2ci += inp;
	}
	
	/*uint*/ull pci = compressRelative(p, p2) * 4 + oldInp;
	
	if (won) {
		if (states.count(p2ci)) return;
		states[p2ci] = pci;
		if (minFPs == -1) {
			minFPs = curFPs;
		}
		
		printMinInputTimeSolution(p2ci);
		return;
	}
	
	if (!states.count(p2ci)) {
		states[p2ci] = pci;
		next.push_back(p2ci);
	}
	
	if (advancedMinInput && (p.state == JUMP || p.state == FALL) && (p2.state == WALK || p2.state == STAND)) {
		if (!states.count(p2c * 4 + 3)) {
			states[p2c * 4 + 3] = pci;
			next.push_back(p2c * 4 + 3);
		}
		if (p.state == JUMP) {
			for (int i = 0; i < 3; i++) {
				if (!states.count(p2c * 4 + i)) {
					states[p2c * 4 + i] = pci;
					next.push_back(p2c * 4 + i);
				}
			}
		}
	} else if (p.state == DUCK && p2.state == STAND) {
		for (int i = 1; i < 4; i++) {
			if (!states.count(p2c * 4 + i)) {
				states[p2c * 4 + i] = pci;
				next.push_back(p2c * 4 + i);
			}
		}
	}
}

void bruteforceMinInputTime() {
	vector<playerState> startStates;
	getStartStates(startStates);
	
	vector<ull> border, newBorder;
	vector<int> ends, newEnds;
	
	for (int i = 0; i < startStates.size(); i++) {
		states[compressState(startStates[i]) * 4] = 0;
		border.push_back(compressState(startStates[i]) * 4);
	}
	
	ends.push_back(border.size());
	
	curFPs = 0;
	minFPs = -1;
	
	auto startTime = std::chrono::steady_clock::now();
	
	while (!border.empty()) {
		print << "Border size: " << border.size() << ", FPs: " << curFPs << '\n';
		
		for (int i = 0; true; i++) {
			if (i < ends.size()) {
				for (int j = ((i > 0)? ends[i-1] : 0); j < ends[i]; j++) {
					playerState p = uncompressState(border[j] / 4);
					int oldInp = border[j] % 4;
					
					for (int k = 0; k < 5; k++) {
						frameMinInputTime(p, oldInp, k, newBorder, false);
					}
				}
			}
			
			if (i > 0) {
				for (int j = ((i > 1)? newEnds[i-2] : 0); j < newEnds[i-1]; j++) {
					playerState p = uncompressState(newBorder[j] / 4);
					int oldInp = newBorder[j] % 4;
					
					if (p.vy == -16) {
						for (int k = 0; k < 3; k++) {
							frameMinInputTime(p, oldInp, k, newBorder, false);
						}
					} else if (p.state == DUCK) {
						frameMinInputTime(p, oldInp, 4, newBorder, true);
					} else if (oldInp == 3) {
						frameMinInputTime(p, oldInp, 3, newBorder, false);
						frameMinInputTime(p, oldInp, 4, newBorder, false);
					} else {
						frameMinInputTime(p, oldInp, oldInp, newBorder, true);
					}
				}
				
				if (i + 1 >= ends.size() && newEnds[i-1] == newBorder.size()) {
					break;
				}
			}
			
			newEnds.push_back(newBorder.size());
		}
		
		swap(border, newBorder);
		swap(ends, newEnds);
		newBorder.clear();
		newEnds.clear();
		
		curFPs++;
		
		if (deviationFromPerfection != -1 && minFPs != -1 && curFPs > minFPs + deviationFromPerfection) {
			break;
		}
	}
	
	auto endTime = std::chrono::steady_clock::now();
	
	print << "Elapsed: " << (duration_cast<std::chrono::seconds>(endTime - startTime)).count() << " seconds\n";
	print << "State map size: " << states.size() << '\n';
	
	states.clear();
}

void printHollowSolution(const playerState &startP, const playerState &endP, vector<char> &inputs) {
	stringstream ans;
	string letters[] = {"n", "a", "d", "w", "s"};
	
	for (int i = 0; i < inputs.size(); i++) {
		ans << letters[inputs.at(i)];
	}
	
	print << solutionLine(ans.str(), startP, endP);
}

#ifndef DO_VISUALIZATION
	void savedVisit(const playerState &p) {}
	void borderVisit(const playerState &p) {}
	void winVisit(const playerState &p) {}
	void updateVisualization() {}
	void windowUpdate() {}
#endif

us getFPs(us info) {
	return info >> 4;
}

us getInputs(us info) {
	return info & 15;
}

constexpr bool equalLength = true;

constexpr int inputCount = 5;

int curtime, mintime;

class MinTimeBF {
	public:
	
	void addStartState(ull cState) {
		states[cState] = 0;
	}
	
	void prepareNewBorderPass() {}
	
	void prepareStateData(int i, int newBorderSize, vector<playerState> &startStates, playerState &p) {}
	
	bool hasBeenTo(ull cState) {
		return states.count(cState);
	}
	
	void insert(ull cState, ull prevCState) {
		savedVisit(uncompressState(cState));
		//states[cState] = compressRelative(uncompressState(prevCState), uncompressState(cState));
		states[cState] = prevCState;
	}
	
	void insertWeak(ull cState, ull prevCState) {
		insert(cState, prevCState);
	}
	
	void setLastInput(int inp, playerState &p2) {}
	
	void printSolution(ull cState) {
		printMinTimeSolution(cState);
	}
	
	void manageBorderPush() {}
	
	int getContainerSize() {
		return states.size();
	}
	
	void clearContainer() {
		states.clear();
	}
};

class MinTimeHollowBF {
	protected:
	
	cQueue inputs = cQueue(inputCount);
	vector<char> currentInputs;	
	vQueue<short> commonParts;
	vQueue<short> lengths;
	vector<int> startStateRegions/*(startStates.size())*/;
	int nextStartStateRegion = 0;
	bool remember;
	playerState curStartState;
	short curCommonPart;
	short newCommonPart;
	short curLength;
	
	void manageHollowStartState() {
		commonParts.push(0);
		if (!equalLength) lengths.push(0);
		startStateRegions.push_back(nextStartStateRegion++);
	}
	
	public:
	
	void addStartState(ull cState) {
		been.insert(cState);
		manageHollowStartState();
	}
	
	void prepareNewBorderPass() {
		startStateRegions.resize(nextStartStateRegion);
		nextStartStateRegion = 0;
		
		newCommonPart = SHRT_MAX;
		
		curBeen.clear();
		remember = curtime % rememberPeriod == 0;
	}
	
	void prepareStateData(int i, int newBorderSize, vector<playerState> &startStates, playerState &p) {
		curCommonPart = commonParts.pop();
		curLength = equalLength? curtime : lengths.pop();
		
		currentInputs.resize(curCommonPart);
		
		for (int k = curCommonPart; k < curLength; k++) {
			currentInputs.push_back(inputs.pop());
		}
		
		newCommonPart = min(newCommonPart, curCommonPart);
		
		while (nextStartStateRegion < startStateRegions.size() && i == startStateRegions[nextStartStateRegion]) {
			startStateRegions[nextStartStateRegion++] = newBorderSize;
		}
		curStartState = startStates[nextStartStateRegion - 1];
	}
	
	bool hasBeenTo(ull cState) {
		return been.count(cState) || (!remember && curBeen.count(cState));
	}
	
	void insert(ull cState, ull prevCState) {
		savedVisit(uncompressState(cState));
		been.insert(cState);
	}
	
	void insertWeak(ull cState, ull prevCState) {
		if (remember) {
			insert(cState, prevCState);
		} else {
			curBeen.insert(cState);
		}
	}
	
	void setLastInput(int inp, playerState &p2) {
		currentInputs.resize(curLength);
		currentInputs.push_back(inp);
	}
	
	void printSolution(ull cState) {
		printHollowSolution(curStartState, uncompressState(cState), currentInputs);
	}
	
	void manageBorderPush() {
		commonParts.push(newCommonPart);
		if (!equalLength) lengths.push(currentInputs.size());
		
		for (int k = newCommonPart; k < currentInputs.size(); k++) {
			inputs.push(currentInputs.at(k));
		}
		
		newCommonPart = curLength;
	}
	
	int getContainerSize() {
		return been.size();
	}
	
	void clearContainer() {
		been.clear();
		curBeen.clear();
	}
};

class MinTimeOptimizerBF: public MinTimeHollowBF {
	protected:
	vQueue<us> borderInfo;
	us initValue = USHRT_MAX;
	us minValue = maxValue;
	us incomingInfo;
	
	public:
	
	MinTimeOptimizerBF() {}
	
	MinTimeOptimizerBF(us init) {
		initValue = init;
	}
	
	void addStartState(ull cState) {
		shortMap[cState] = initValue;
		borderInfo.push(initValue);
		manageHollowStartState();
	}
	
	void prepareNewBorderPass() {
		startStateRegions.resize(nextStartStateRegion);
		nextStartStateRegion = 0;
		
		newCommonPart = SHRT_MAX;
		
		curShortMap.clear();
		remember = curtime % rememberPeriod == 0;
	}
	
	void prepareStateData(int i, int newBorderSize, vector<playerState> &startStates, playerState &p) {
		MinTimeHollowBF::prepareStateData(i, newBorderSize, startStates, p);
		
		p.metaData = borderInfo.pop();
	}
	
	void setLastInput(int inp, playerState &p2) {
		MinTimeHollowBF::setLastInput(inp, p2);
		
		incomingInfo = p2.metaData;
	}
	
	bool hasBeenTo(ull cState) {
		// We're depending on the fact that updated info gets inserted in shortMap no matter what
		// if there already was info there, as there's no memory downside, though there
		// might be performance downside of accessing a larger map...
		
		if (shortMap.count(cState)) {
			return shortMap[cState] <= incomingInfo;
		}
		
		return !remember && curShortMap.count(cState) && curShortMap[cState] <= incomingInfo;
	}
	
	void printSolution(ull cState) {
		if (valueDescent && incomingInfo > minValue) {
			return;
		}
		
		playerState p2 = uncompressState(cState);
		p2.metaData = incomingInfo;
		printHollowSolution(curStartState, p2, currentInputs);
		
		if (incomingInfo < minValue) {
			minValue = incomingInfo;
		}
	}
	
	void insert(ull cState, ull prevCState) {
		savedVisit(uncompressState(cState));
		shortMap[cState] = incomingInfo;
	}
	
	void insertWeak(ull cState, ull prevCState) {
		if (remember || shortMap.count(cState)) {
			insert(cState, prevCState);
		} else {
			curShortMap[cState] = incomingInfo;
		}
	}
	
	void manageBorderPush() {
		MinTimeHollowBF::manageBorderPush();
		
		borderInfo.push(incomingInfo);
	}
	
	int getContainerSize() {
		return shortMap.size();
	}
	
	void clearContainer() {
		shortMap.clear();
		curShortMap.clear();
	}
};

class SpectralBF: public MinTimeOptimizerBF {
	us sourceFPs;
	bool sourceInputs[5];
	us incomingFPs;
	us incomingInputs;
	playerState p;
	
	public:
	
	SpectralBF() : MinTimeOptimizerBF(15) {}
	
	void prepareStateData(int i, int newBorderSize, vector<playerState> &startStates, playerState &pI) {
		MinTimeHollowBF::prepareStateData(i, newBorderSize, startStates, pI);
		
		p = pI;
		us sourceInfo = borderInfo.pop();
		sourceFPs = getFPs(sourceInfo);
		for (char i = 0; i < 4; i++) {
			sourceInputs[i] = sourceInfo & (1 << i);
		}
		sourceInputs[4] = sourceInfo & (1 << 3);
		
		// The bit that's used to show that we've been holding down also signifies that we've
		// been holding up, as that's useful in other states, but when ducking, we don't want
		// that! So as an exception, we set the boolean for whether we've been holding up
		// false for duck state.
		if (p.state == DUCK) {
			sourceInputs[3] = false;
		}
	}
	
	void setLastInput(int inp, playerState &p2) {
		MinTimeHollowBF::setLastInput(inp, p2);
		
		incomingFPs = sourceFPs;
		
		if (!sourceInputs[inp]) {
			incomingFPs++;
		}
		
		incomingInputs = 1 << min(inp, 3);
		if (p2.vy == -16) {
			incomingInputs = 15;
		} else if (advancedMinInput && (p.state == JUMP || p.state == FALL || p.state == WALL) && (p2.state == WALK || p2.state == STAND)) {
			if (p.state == JUMP) {
				incomingInputs = 15;
			} else {
				incomingInputs |= 1 << 3;
			}
		} else if (p.state == DUCK && p2.state == STAND) {
			incomingInputs = 15;
		}
	}
	
	bool hasBeenTo(ull cState) {
		if (valueDescent && incomingFPs > minValue) {
			return true;
		}
		
		incomingInfo = incomingInputs | (incomingFPs << 4);
		
		us destInfo;
		
		// This one used to be broken, now it isn't! (I hope)
		if (shortMap.count(cState)) {
			destInfo = shortMap[cState];
		} else if (!remember && curShortMap.count(cState)) {
			destInfo = curShortMap[cState];
		} else {
			return false;
		}
		
		us destFPs = getFPs(destInfo);
		us destInputs = getInputs(destInfo);
		
		if (destFPs < incomingFPs) {
			return true;
		}
		
		if (destFPs > incomingFPs) {
			return false;
		}
		
		incomingInfo |= destInputs;
		
		if (incomingInputs & ~destInputs) {
			return false;
		}
		
		if (advancedMinInput && compressState(p) == cState && destInputs != 15) {
			incomingInfo |= 15;
			return false;
		}
		
		return true;
	}
	
	void printSolution(ull cState) {
		playerState p2 = uncompressState(cState);
		p2.metaData = incomingFPs;
		printHollowSolution(curStartState, p2, currentInputs);
		
		if (incomingFPs < minValue) {
			minValue = incomingFPs;
		}
	}
};

template <class C> void bruteforce() {
	C bf;
	
	vector<playerState> startStates;
	getStartStates(startStates);
	
	vQueue<ull> border;
	
	for (int i = 0; i < startStates.size(); i++) {
		borderVisit(startStates[i]);
		savedVisit(startStates[i]);
		border.push(compressState(startStates[i]));
		bf.addStartState(compressState(startStates[i]));
	}
	
	auto startTime = std::chrono::steady_clock::now();
	
	for (curtime = 0, mintime = -1; border.size > 0 &&
		(deviationFromPerfection == -1 || mintime == -1 || curtime <= mintime + deviationFromPerfection);
		curtime++) {
		
		updateVisualization();
		print << "Border size: " << border.size << ", time: " << curtime << '\n';
		//file.flush();
		
		bf.prepareNewBorderPass();
		
		int borderSize = border.size;
		for (int i = 0; i < borderSize; i++) {
			playerState p = uncompressState(border.pop());
			
			bf.prepareStateData(i, border.size - (borderSize - i) + 1, startStates, p);
			
			for (int j = 0; j < 5; j++) {
				if (skipInput(p, j)) {
					continue;
				}
				
				bool lost, won;
				playerState p2 = updateAndCheckFate(p, j, lost, won);
				
				if (lost) {
					continue;
				}
				
				bf.setLastInput(j, p2);
				
				ull p2c = compressState(p2);
				if (bf.hasBeenTo(p2c)) continue;
				
				if (won) {
					winVisit(p2);
					bf.insert(p2c, compressState(p));
					if (mintime == -1) {
						mintime = curtime;
					}
					bf.printSolution(p2c);
				} else {
					bf.insertWeak(p2c, compressState(p));
				}
				
				if (!won || !terminateOnWin) {
					borderVisit(p2);
					border.push(p2c);
					bf.manageBorderPush();
				}
			}
		}
	}
	
	auto endTime = std::chrono::steady_clock::now();
	
	updateVisualization();
	
	print << "Elapsed: " << (duration_cast<std::chrono::seconds>(endTime - startTime)).count() << " seconds\n";
	print << "State container size: " << bf.getContainerSize() << '\n';
	
	file.flush();
	
	bf.clearContainer();
}
