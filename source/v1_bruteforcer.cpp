map<ull, uint> states;
stringstream result;

const uint OFS_STAND = 0;
const uint OFS_DUCK = 66;
const uint OFS_WALK = 198;
const uint OFS_JUMP = 504;
const uint OFS_FALL = 4788;
const uint OFS_WALL = 7848;
const uint OFS_END = 8343;

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

uint compressAllButXY(playerState &t) {
	uint a = t.dir * OFS_END;
	
	int vx;
	switch (t.state) {
		case STAND:
			a += OFS_STAND;
			vx = roundUp(t.vx) / 2;
			
			if (vx < -2) {
				a += vx + 12;
			} else if (vx < 3) {
				a += 10;
			} else {
				a += vx + 8;
			}
			
			a += t.wall_count * 22;
			break;
		case DUCK:
			a += OFS_DUCK;
			vx = roundUp(t.vx) / 2;
			
			if (vx < -2) {
				a += vx + 12;
			} else if (vx < 3) {
				a += 10;
			} else {
				a += vx + 8;
			}
			
			a += t.wall_count * 22;
			if (t.right_edge || t.left_edge || t.vy < 0) {
				a += 66;
			}
			break;
		case WALK:
			a += OFS_WALK;
			a += t.vx + 25;
			a += t.wall_count * 51;
			if (t.vy < 0) a += 51*3;
			break;
		case JUMP:
			a += OFS_JUMP;
			a += t.vx + 25;
			a += min(t.vy + 16, 27) * 51;
			a += t.wall_count * 51 * 28;
			break;
		case FALL:
			a += OFS_FALL;
			a += t.vx + 25;
			a += min(t.vy, 11) * 51;
			if (t.anim == WALL && t.fall_anim_count < 2) {
				a += (3 + t.fall_anim_count) * 51 * 12;
			} else {
				a += t.wall_count * 51 * 12;
			}
			break;
		case WALL:
			a += OFS_WALL;
			a += t.fall_count;
			if (t.left_edge) {
				a += (t.rx - t._x + 49) * 5;
			} else if (t.right_edge) {
				a += (t.lx - t._x + 49) * 5;
			} else {
				a += 49 * 5;
			}
			break;
	}
	
	return a;
}

ull compressState(playerState &t) {
	ull a = t._y - 40;
	a = a * level_width * 64 + t._x;
	a *= 2 * OFS_END;
	a += compressAllButXY(t);
	
	return a;
}

ull compressState2(playerState t) {
	return compressState(t);
}

uint compressRelative(playerState &t, playerState &t2) {
	uint a = t._y - t2._y + 20;
	a = a * 93 + t._x - t2._x + 46;
	a *= 2 * OFS_END;
	a += compressAllButXY(t);
	
	return a;
}

uint compressRelative2(playerState t, playerState t2) {
	return compressRelative(t, t2);
}

playerState uncompressAllButXY (ull c) {
	playerState t;
	t.anim = -1;
	ull temp = c % OFS_END;
	if (temp >= OFS_WALL) {
		t.state = WALL;
		temp -= OFS_WALL;
		t.fall_count = temp % 5;
		temp /= 5;
		if (temp != 49) {
			t.left_edge = true;
			t.rx = temp - 49;
		}
	} else if (temp >= OFS_FALL) {
		t.state = FALL;
		temp -= OFS_FALL;
		t.vx = temp % 51 - 25;
		temp /= 51;
		t.vy = temp % 12;
		temp /= 12;
		if (temp < 3) {
			t.wall_count = temp;
		} else {
			t.fall_anim_count = temp - 3;
			t.anim = WALL;
			t.prev_state = WALL;
		}
	} else if (temp >= OFS_JUMP) {
		t.state = JUMP;
		temp -= OFS_JUMP;
		t.vx = temp % 51 - 25;
		temp /= 51;
		t.vy = temp % 28 - 16;
		temp /= 28;
		t.wall_count = temp;
	} else if (temp >= OFS_WALK) {
		t.state = WALK;
		temp -= OFS_WALK;
		t.vx = temp % 51 - 25;
		temp /= 51;
		t.wall_count = temp % 3;
		if (temp >= 3) t.vy = -1;
	} else {
		t.state = STAND;
		if (temp >= OFS_DUCK) {
			t.state = DUCK;
			temp -= OFS_DUCK;
			if (temp >= 66) {
				t.vy = -1;
				temp -= 66;
			}
		}
		t.wall_count = temp / 22;
		temp %= 22;
		if (temp < 10) {
			t.vx = (temp - 12) * 2;
		} else if (temp == 10) {
			t.vx = 0;
		} else {
			t.vx = (temp - 8) * 2;
		}
	}
	
	if (t.anim == -1) {
		t.anim = t.state;
		t.prev_state = t.state;
	}
	
	c /= OFS_END;
	
	t.dir = c & 1;
	t.prev_dir = t.dir;
	t.animDir = t.dir;
	
	return t;
}

playerState uncompressState(ull c) {
	playerState t = uncompressAllButXY(c);
	t._x = (c / OFS_END / 2) % (level_width * 64);
	t._y = c / OFS_END / 2 / (level_width * 64) + 40;
	t.rx += t._x;
	return t;
}

playerState uncompressRelative (uint c, playerState &t2) {
	playerState t = uncompressAllButXY(c);
	t._x = t2._x + (c / OFS_END / 2) % 93 - 46;
	t._y = t2._y + c / OFS_END / 2 / 93 - 20;
	t.rx += t._x;
	return t;
}

playerState uncompressRelative2(uint c, playerState t2) {
	return uncompressRelative(c, t2);
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
	ull pci = compressState2(uncompressRelative(states[p2ci]/e, endP)) * e + (states[p2ci] % e);
	
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
			
			if (compressState2(updateWithFull(temp, k)) == t2ci / e) {
				ans = letters[k] + ans;
				break;
			}
		}
		
		if (states[tci] == 0) break;
		
		t2ci = tci;
		tci = compressState2(uncompressRelative(states[tci] / e, temp)) * e + states[tci] % e;
	}
	
	string line = solutionLine(ans, temp, endP);
	
	cout << line;
	result << line;
	
	if (broken) {
		string rec = "";
		for (ull t3ci = p2ci; t3ci != 0;) {
			playerState temp3 = uncompressState(t3ci / e);
			stringstream info;
			info << stateInfoLine(temp3);
			info << t3ci << endl;
			rec = info.str() + rec;
			
			if (states[t3ci] == 0) break;
			
			t3ci = compressState2(uncompressRelative(states[t3ci] / e, temp3)) * e + states[t3ci] % e;
		}
		cout << rec;
		result << rec;
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
	
	uint pci = compressRelative(p, p2) * 4 + oldInp;
	
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
		cout << "Border size: " << border.size() << ", FPs: " << curFPs << endl;
		result << "Border size: " << border.size() << ", FPs: " << curFPs << endl;
		
		if (activeWrite) {
			ofstream file(filename);
			file << result.str();
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
	
	cout << "Elapsed: " << (std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime)).count() << " seconds\n";
	result << "Elapsed: " << (std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime)).count() << " seconds\n";
	cout << "State map size: " << states.size() << endl;
	result << "State map size: " << states.size() << endl;
	
	ofstream file(filename);
	file << result.str();
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
		cout << "Border size: " << border.size() << ", time: " << curtime << endl;
		result << "Border size: " << border.size() << ", time: " << curtime << endl;
		
		if (activeWrite) {
			ofstream file(filename);
			file << result.str();
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
	
	cout << "Elapsed: " << (std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime)).count() << " seconds\n";
	result << "Elapsed: " << (std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime)).count() << " seconds\n";
	cout << "State map size: " << states.size() << endl;
	result << "State map size: " << states.size() << endl;
	
	ofstream file(filename);
	file << result.str();
	file.close();
	
	states.clear();
}

int main() {
	initializeData();
	
	switch (bruteforceType) {
		case 0:
			bruteforceMinTime();
			break;
		case 1:
			bruteforceMinInputTime();
			break;
	}
	
	system("pause");
	return 0;
}
