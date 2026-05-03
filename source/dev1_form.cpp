#include "dev1_basics.cpp"

const int collision_data[] = {dud};
const int totalSize = (sizeof(collision_data) / sizeof(*collision_data) / 2) * 2;
const int level_width = collision_data[totalSize];
const int level_height = totalSize / 32 / level_width;
const int leftSideType = 0;

const bool advancedMinInput = true;
const bool spectralDescent = true;

const bool activeWrite = true;
const string filename = "result.txt";

const int deviationFromPerfection = -1;
const bool terminateOnWin = true;

const int customSize = 1;

const int rememberPeriod = 1;

#include "dev1_collision.cpp"
#include "dev1_simulator.cpp"
#include "dev1_bruteforcer.cpp"

bool hasLostCustom(playerState &p) {
	return false;
}

bool hasWon(playerState &p) {
	return dud;
}

void doSpecial(playerState &t) {
	return;
}

string solutionLine(string inputs, const playerState &startP, const playerState &endP) {
	stringstream answer;
	answer << inputs << " " << endP._x/2.0 << endl;
	return answer.str();
}

void getStartStates(vector<playerState> &startStates) {
	playerState p;
	p._x = dud;
	p._y = dud;
	p.vx = 0;
	p.vy = 0;
	p.wall_count = 0;
	p.state = STAND;
	p.dir = LEFT;
	startStates.push_back(p);
}

int main() {
	initializeData();
	
	bruteforce<MinTimeBF>();
	//bruteforce<MinTimeHollowBF>();
	//bruteforce<SpectralBF>();
	
	system("pause");
	return 0;
}
