map<ull, ull> states;
set<ull> been;

const uint OFS_STAND = 0;
const uint OFS_DUCK = 66;
const uint OFS_WALK = 198;
const uint OFS_JUMP = 504;
const uint OFS_FALL = 4788;
const uint OFS_WALL = 7848;
const uint OFS_END = /*8343*/ OFS_WALL + 5 * ((24 + maxVx) * 2 + 1);

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
		b *= (24 + maxVx) * 2 + 1;
		if (t.left_edge) {
			b += (t.rx - t._x + 24 + maxVx);
		} else if (t.right_edge) {
			b += (t.lx - t._x + 24 + maxVx);
		} else {
			b += 24 + maxVx;
		}
	}
	
	if (t.state == JUMP) {
		b *= 28;
		b += min(t.vy + 16, 27);
	} else if (t.state == FALL) {
		b *= 12;
		b += min(t.vy, 11);
	} else if (t.state == DUCK) {
		b *= 2;
		if (t.right_edge || t.left_edge || t.vy < 0) {
			b++;
		}
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
	
	if (t.state == JUMP) {
		t.vy = temp % 28 - 16;
		temp /= 28;
	} else if (t.state == FALL) {
		t.vy = temp % 12;
		temp /= 12;
	} else if (t.state == DUCK || t.state == WALK) {
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
		t.rx = temp % ((24 + maxVx) * 2 + 1) - 24 - maxVx;
		temp /= (24 + maxVx) * 2 + 1;
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
	if (p.state != DUCK) {
		p = updateWith(p, inp);
		
		lost = hasLost(p);
		won = hasWon(p);
		
		if (p.state == DUCK) {
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
	}
	
	return p;
}

playerState updateWithFull(playerState p, int inp) {
	bool a;
	return updateAndCheckFate(p, inp, a, a);
}

string solutionLine(string inputs, playerState &startP, playerState &endP);

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
	
	auto startTime = std::chrono::steady_clock::now();
	
	while (!border.empty()) {
		info << "Border size: " << border.size() << ", FPs: " << curFPs << endl;
		print();
		
		if (activeWrite) {
			ofstream file(filename);
			file << getOutput();
			file.close();
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
	
	auto endTime = std::chrono::steady_clock::now();
	
	info << "Elapsed: " << (std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime)).count() << " seconds\n";
	info << "State map size: " << states.size() << endl;
	print();
	
	ofstream file(filename);
	file << getOutput();
	file.close();
	
	states.clear();
}

int curtime, mintime;

void bruteforceMinTime() {
	vector<playerState> startStates;
	getStartStates(startStates);
	
	vector<ull> border, newBorder;
	
	for (int i = 0; i < startStates.size(); i++) {
		states[compressState(startStates[i])] = 0;
		border.push_back(compressState(startStates[i]));
	}
	
	curtime = 0;
	mintime = -1;
	
	auto startTime = std::chrono::steady_clock::now();
	
	while (!border.empty()) {
		info << "Border size: " << border.size() << ", time: " << curtime << endl;
		print();
		
		if (activeWrite) {
			ofstream file(filename);
			file << getOutput();
			file.close();
		}
		
		for (int i = 0; i < border.size(); i++) {
			playerState p = uncompressState(border[i]);
			for (int j = 0; j < 5; j++) {
				bool lost, won;
				playerState p2 = updateAndCheckFate(p, j, lost, won);
				
				if (lost) {
					continue;
				}
				
				ull p2c = compressState(p2);
				if (states.count(p2c)) continue;
				
				states[p2c] = compressRelative(p, p2);
				
				if (won) {
					if (mintime == -1) {
						mintime = curtime;
					}
					printMinTimeSolution(p2c);
				} else {
					newBorder.push_back(p2c);
				}
			}
		}
		swap(border, newBorder);
		newBorder.clear();
		curtime++;
		
		if (deviationFromPerfection != -1 && mintime != -1 && curtime > mintime + deviationFromPerfection) {
			break;
		}
	}
	
	auto endTime = std::chrono::steady_clock::now();
	
	info << "Elapsed: " << (std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime)).count() << " seconds\n";
	info << "State map size: " << states.size() << endl;
	print();
	
	ofstream file(filename);
	file << getOutput();
	file.close();
	
	states.clear();
}

const char inputBits = 3;
typedef vector<bool> inputVector;

void pushInput(inputVector &v, char b) {
	for (int i = 0; i < inputBits; i++) {
		v.push_back(b & (1 << i));
	}
}

char getInput(inputVector &v, int ind) {
	char a;
	for (int i = 0; i < inputBits; i++) {
		a |= (1 << i) * v[inputBits * ind + i];
	}
	return a;
}

void resizeInputs(inputVector &v, int size) {
	v.resize(inputBits * size);
}

void printHollowSolution(playerState &startP, playerState &endP, inputVector &inputs, short length) {
	stringstream ans;
	string letters[] = {"n", "a", "d", "w", "s"};
	
	for (int i = 0; i < length; i++) {
		ans << letters[getInput(inputs, i)];
	}
	
	print(solutionLine(ans.str(), startP, endP));
}

bool equalLength = true;

void bruteforceMinTimeHollow() {
	vector<playerState> startStates;
	getStartStates(startStates);
	
	vector<ull> border, newBorder;
	inputVector inputs, newInputs, currentInputs;
	if (sizeof inputs != 40 || sizeof newInputs != 40 || sizeof currentInputs != 40) {
		print("Bool vector incorrect!\n");
		return;
	}
	vector<short> commonParts, newCommonParts;
	vector<short> lengths, newLengths;
	int startStateRegions[startStates.size()];
	
	for (int i = 0; i < startStates.size(); i++) {
		border.push_back(compressState(startStates[i]));
		commonParts.push_back(0);
		if (!equalLength) lengths.push_back(0);
		startStateRegions[i] = i;
	}
	
	curtime = 0;
	mintime = -1;
	
	auto startTime = std::chrono::steady_clock::now();
	
	while (!border.empty()) {
		info << "Border size: " << border.size() << ", time: " << curtime << endl;
		print();
		
		if (activeWrite) {
			ofstream file(filename);
			file << getOutput();
			file.close();
		}
		
		short minCommonPart = SHRT_MAX;
		int currentPos = 0;
		int nextStartStateRegion = 0;
		
		for (int i = 0; i < border.size(); i++) {
			playerState p = uncompressState(border[i]);
			
			while (nextStartStateRegion < startStates.size() && i == startStateRegions[nextStartStateRegion]) {
				startStateRegions[nextStartStateRegion++] = newBorder.size();
			}
			playerState curStartState = startStates[nextStartStateRegion - 1];
			
			resizeInputs(currentInputs, commonParts[i]);
			
			short curLength = equalLength? curtime : lengths[i];
			for (int k = commonParts[i]; k < curLength; k++) {
				pushInput(currentInputs, getInput(inputs, currentPos++));
			}
			
			bool isNew = true;
			for (int j = 0; j < 5; j++) {
				bool lost, won;
				playerState p2 = updateAndCheckFate(p, j, lost, won);
				
				if (lost) {
					continue;
				}
				
				ull p2c = compressState(p2);
				if (been.count(p2c)) continue;
				
				resizeInputs(currentInputs, curLength);
				pushInput(currentInputs, j);
				short newLength = curLength + 1;
				
				if (won) {
					been.insert(p2c);
					if (mintime == -1) {
						mintime = curtime;
					}
					printHollowSolution(curStartState, p2, currentInputs, newLength);
				} else {
					if ((curtime + i) % rememberPeriod == 0) {
						been.insert(p2c);
					}
					
					newBorder.push_back(p2c);
					short commonPart;
					
					if (isNew) {
						commonPart = min(commonParts[i], minCommonPart);
					} else {
						commonPart = curLength;
					}
					newCommonParts.push_back(commonPart);
					if (!equalLength) newLengths.push_back(newLength);
					
					for (int k = commonPart; k < newLength; k++) {
						pushInput(newInputs, getInput(currentInputs, k));
					}
					
					isNew = false;
				}
			}
			
			if (isNew) {
				minCommonPart = min(commonParts[i], minCommonPart);
			} else {
				minCommonPart = SHRT_MAX;
			}
		}
		
		swap(border, newBorder);
		newBorder.clear();
		swap(inputs, newInputs);
		newInputs.clear();
		swap(commonParts, newCommonParts);
		newCommonParts.clear();
		swap(lengths, newLengths);
		newLengths.clear();

		curtime++;
		
		if (deviationFromPerfection != -1 && mintime != -1 && curtime > mintime + deviationFromPerfection) {
			break;
		}
	}
	
	auto endTime = std::chrono::steady_clock::now();
	
	info << "Elapsed: " << (std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime)).count() << " seconds\n";
	info << "State set size: " << been.size() << endl;
	print();
	
	ofstream file(filename);
	file << getOutput();
	file.close();
	
	states.clear();
}

int main() {
	initializeData();
	
	switch (bruteforceType) {
		case 0:
			bruteforceMinTimeHollow();
			break;
		case 1:
			bruteforceMinInputTime();
			break;
	}
	
	system("pause");
	return 0;
}
