map<ull, ull> states;
set<ull> been;
set<ull> curBeen;
map<ull, us> shortMap;
map<ull, us> curShortMap;

const uint OFS_STAND= 0;
const uint OFS_DUCK = OFS_STAND+ 22 *  1 * 3;
const uint OFS_WALK = OFS_DUCK + 22 * 42 * 3;
const uint OFS_JUMP = OFS_WALK + 51 *  2 * 3;
const uint OFS_FALL = OFS_JUMP + 51 * 28 * 3;
const uint OFS_WALL = OFS_FALL + 51 * 12 * 5;
const uint OFS_END  = OFS_WALL + 99 *  1 * 5;//10983

string stateInfoLine(playerState t) {
	stringstream result;
	result << "x: " << (((double)t._x) / 2) << ", ";
	result << "y: " << t._y << ", ";
	result << "vx: " << (((double)t.vx) / 2) << ", ";
	result << "vy: " << t.vy << ", ";
	result << "state: " << t.state << ", ";
	result << "dir: " << t.dir << ", ";
	result << "wc: " << t.wall_count << endl;
	return result.str();
}

void writeToFile(string s) {
	ofstream file(filename);
	file << s;
	file.close();
}

ull compressState(const playerState &t) {
	ull a = 0;
	a += (t._y - 40);
	
	a *= level_width * 64;
	a += t._x;
	
	a *= customSize;
	a += t.custom;
	
	a *= 2;
	a += int(t.hit);
	
	a *= 2;
	a += t.dir;
	
	uint b = 0;
	
	if (t.state == STAND || t.state == DUCK) {
		b *= 22;
		int vx = roundUp(t.vx) / 2;
		if (vx < -2) {
			b += vx + 12;
		} else if (vx < 3) {
			b += 10;
		} else {
			b += vx + 8;
		}
	} else if (t.state == WALK || t.state == JUMP || t.state == FALL) {
		b *= 51;
		b += t.vx + 25;
	} else if (t.state == WALL) {
		b *= 99;
		if (t.left_edge) {
			b += (t.rx - t._x + 49);
		} else if (t.right_edge) {
			b += (t.lx - t._x + 49);
		} else {
			b += 49;
		}
	}
	
	if (t.state == JUMP) {
		b *= 28;
		b += min(t.vy + 16, 27);
	} else if (t.state == FALL) {
		b *= 12;
		b += min(t.vy, 11);
	} else if (t.state == DUCK) {
		b *= 42;
		if (t.vy < 0) {
			b++;
		} else if (t.right_edge || t.left_edge) {
			int d = dUp(t.right_edge? t.lx : t.rx, t._y);
			if (d > 1 && d <= 101) {
				b += 2;
				b += ((t._x - t.oldX) / 2) + 19;
			} else {
				b++;
			}
		}
		
		/*b *= 2;
		if (t.right_edge || t.left_edge || t.vy < 0) {
			b++;
		}*/
	} else if (t.state == WALK) {
		b *= 2;
		if (t.vy < 0) b++;
	}
	
	if (t.state == WALL) {
		b *= 5;
		b += t.fall_count;
	} else if (t.state == FALL) {
		b *= 5;
		if (t.anim == WALL && t.fall_anim_count < 2) {
			b += (3 + t.fall_anim_count);
		} else {
			b += t.wall_count;
		}
	} else {
		b *= 3;
		b += t.wall_count;
	}
	
	switch (t.state) {
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
	playerState t;
	ull temp;
	
	temp = c % OFS_END;
	c /= OFS_END;
	
	if (temp >= OFS_JUMP) {
		if (temp >= OFS_WALL) {
			t.state = WALL;
			temp -= OFS_WALL;
		} else if (temp >= OFS_FALL) {
			t.state = FALL;
			temp -= OFS_FALL;
		} else {
			t.state = JUMP;
			temp -= OFS_JUMP;
		}
	} else {
		if (temp >= OFS_WALK) {
			t.state = WALK;
			temp -= OFS_WALK;
		} else if (temp >= OFS_DUCK) {
			t.state = DUCK;
			temp -= OFS_DUCK;
		} else {
			t.state = STAND;
			temp -= OFS_STAND;
		}
	}
	
	if (t.state == WALL) {
		t.fall_count = temp % 5;
		temp /= 5;
	} else if (t.state == FALL) {
		if (temp % 5 < 3) {
			t.wall_count = temp % 5;
		} else {
			t.fall_anim_count = temp % 5 - 3;
			t.anim = WALL;
		}
		temp /= 5;
	} else {
		t.wall_count = temp % 3;
		temp /= 3;
	}
	
	int oldXDisplacement = -100;
	if (t.state == JUMP) {
		t.vy = temp % 28 - 16;
		temp /= 28;
	} else if (t.state == FALL) {
		t.vy = temp % 12;
		temp /= 12;
	} else if (t.state == DUCK) {
		if (temp % 42 == 1) {
			t.vy = -1;
		} else if (temp % 42 > 1) {
			oldXDisplacement = ((temp % 42) - 2 - 19) * 2;
		}
		temp /= 42;
	} else if (t.state == WALK) {
		t.vy = -(temp % 2);
		temp /= 2;
	}
	
	if (t.state == STAND || t.state == DUCK) {
		if (temp % 22 < 10) {
			t.vx = (temp % 22 - 12) * 2;
		} else if (temp % 22 == 10) {
			t.vx = 0;
		} else {
			t.vx = (temp % 22 - 8) * 2;
		}
		temp /= 22;
	} else if (t.state == WALK || t.state == JUMP || t.state == FALL) {
		t.vx = temp % 51 - 25;
		temp /= 51;
	} else if (t.state == WALL) {
		t.left_edge = true;
		t.rx = temp % 99 - 49;
		temp /= 99;
	}
	
	if (t.anim != WALL) {
		t.anim = t.state;
	}
	
	t.dir = c % 2;
	t.animDir = t.dir;
	c /= 2;
	
	t.hit = c % 2;
	c /= 2;
	
	t.custom = c % customSize;
	c /= customSize;
	
	t._x = c % (level_width * 64);
	t.rx += t._x;
	c /= level_width * 64;
	
	t._y = c + 40;
	
	if (t.state == DUCK && oldXDisplacement != -100) {
		t.anim = STAND;
		t._x -= oldXDisplacement;
		calculateDistance(t, false);
		t._x += oldXDisplacement;
		t.anim = DUCK;
	}
	
	return t;
}

ull compressRelative(const playerState &t1, const playerState &t2) {
	return compressState(t1);
}

/*playerState uncompressRelative (uint c, playerState t2) {
	playerState t = uncompressAllButXY(c);
	t._x = t2._x + (c / OFS_END / 2) % 93 - 46;
	t._y = t2._y + c / OFS_END / 2 / 93 - 20;
	t.rx += t._x;
	return t;
}*/


playerState uncompressRelative(ull c, const playerState &t2) {
	return uncompressState(c);
}

bool hasLostCustom(playerState &p);

bool hasLost(playerState &p) {
	return (p._x < 24 || p._x >= level_width * 64 - 24 || p._y > level_height * 32 - 64 || p._y < 46 || hasLostCustom(p));
}

bool hasWon(playerState &p);

playerState updateWith(playerState p, int inp) {
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
	
	update(p);
	return p;
}

playerState updateAndCheckFate(playerState p, int inp, bool &lost, bool &won) {
	//if (p.state != DUCK) {
		p = updateWith(p, inp);
		
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
			p = updateWith(p, 4);
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
	
	string line = solutionLine(ans, temp, endP);
	
	print(line);
	
	if (broken) {
		string rec = "";
		for (ull t3ci = p2ci; t3ci != 0;) {
			playerState temp3 = uncompressState(t3ci / e);
			stringstream info;
			info << stateInfoLine(temp3);
			info << t3ci << endl;
			rec = info.str() + rec;
			
			if (states[t3ci] == 0) break;
			
			t3ci = compressState(uncompressRelative(states[t3ci] / e, temp3)) * e + states[t3ci] % e;
		}
		print(rec);
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
	
	auto startTime = chrono::steady_clock::now();
	
	while (!border.empty()) {
		info << "Border size: " << border.size() << ", FPs: " << curFPs << endl;
		print();
		
		if (activeWrite) {
			writeToFile(getOutput());
		}
		
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
	
	auto endTime = chrono::steady_clock::now();
	
	info << "Elapsed: " << (chrono::duration_cast<chrono::seconds>(endTime - startTime)).count() << " seconds\n";
	info << "State map size: " << states.size() << endl;
	print();
	
	writeToFile(getOutput());
	
	states.clear();
}

void printHollowSolution(const playerState &startP, const playerState &endP, vector<char> &inputs) {
	stringstream ans;
	string letters[] = {"n", "a", "d", "w", "s"};
	
	for (int i = 0; i < inputs.size(); i++) {
		ans << letters[inputs.at(i)];
	}
	
	print(solutionLine(ans.str(), startP, endP));
}

us getFPs(us info) {
	return info >> 4;
}

us getInputs(us info) {
	return info & 15;
}

bool equalLength = true;

int inputCount = 5;

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
		been.insert(cState);
	}
	
	void insertWeak(ull cState, ull prevCState) {
		if (remember) {
			been.insert(cState);
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
		if (valueDescent && incomingInfo > minValue) {
			return true;
		}
		
		if (!remember && curShortMap.count(cState)) {
			return curShortMap[cState] <= incomingInfo;
		}
		
		return shortMap.count(cState) && shortMap[cState] <= incomingInfo;
	}
	
	void printSolution(ull cState) {
		playerState p2 = uncompressState(cState);
		p2.metaData = incomingInfo;
		printHollowSolution(curStartState, p2, currentInputs);
		
		if (incomingInfo < minValue) {
			minValue = incomingInfo;
		}
	}
	
	void insert(ull cState, ull prevCState) {
		shortMap[cState] = incomingInfo;
	}
	
	void insertWeak(ull cState, ull prevCState) {
		if (remember) {
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
		border.push(compressState(startStates[i]));
		bf.addStartState(compressState(startStates[i]));
	}
	
	curtime = 0;
	mintime = -1;
	
	auto startTime = chrono::steady_clock::now();
	
	while (border.size > 0) {
		info << "Border size: " << border.size << ", time: " << curtime << endl;
		print();
		
		if (activeWrite) {
			writeToFile(getOutput());
		}
		
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
					bf.insert(p2c, compressState(p));
					if (mintime == -1) {
						mintime = curtime;
					}
					bf.printSolution(p2c);
				} else {
					bf.insertWeak(p2c, compressState(p));
				}
				
				if (!won || !terminateOnWin) {
					border.push(p2c);
					bf.manageBorderPush();
				}
			}
		}
		
		curtime++;
		
		if (deviationFromPerfection != -1 && mintime != -1 && curtime > mintime + deviationFromPerfection) {
			break;
		}
	}
	
	auto endTime = chrono::steady_clock::now();
	
	info << "Elapsed: " << (chrono::duration_cast<chrono::seconds>(endTime - startTime)).count() << " seconds\n";
	info << "State container size: " << bf.getContainerSize() << endl;
	print();
	
	writeToFile(getOutput());
	
	bf.clearContainer();
}
