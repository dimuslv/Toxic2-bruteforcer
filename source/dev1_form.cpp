#include "dev1_basics.cpp"

constexpr int collision[]{dud};
constexpr int totalSize = (arraySize(collision) / 2) * 2;
constexpr int level_width = collision[totalSize] * 32;
constexpr int level_height = totalSize * 32 / level_width;
constexpr int leftSideType = 0;

constexpr bool advancedMinInput = true;
constexpr bool valueDescent = true;
constexpr int maxValue = USHRT_MAX;

const string filename = "result.txt";

constexpr int deviationFromPerfection = -1;
constexpr bool terminateOnWin = true;

constexpr int customSize = 1;

constexpr int rememberPeriod = 1;

constexpr bool animAccuracy = false;

#include "dev1_collision.cpp"
#include "dev1_simulator.cpp"
#include "dev1_extra.cpp"
#include "dev1_visualization.cpp"
#include "dev1_bruteforcer.cpp"

bool hasLostCustom(const playerState &p) {
	return false;
}

bool hasWon(const playerState &p) {
	return dud;
}

void doSpecial(playerState &p) {
	/*if (checkDamage(p) && !p.hit) {
		p.startHit();
	}*/
	
	//doConveyors(p, {x, x2, y, LEFT});
}

string solutionLine(string inputs, const playerState &startP, const playerState &endP) {
	stringstream answer;
	answer << inputs << " " << endP._x << '\n';
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
	file.open(filename);
	initializeData();
	jthread windowThread{windowUpdate};
	
	//bruteforce<MinTimeBF>();
	bruteforce<MinTimeHollowBF>();
	//bruteforce<MinTimeOptimizerBF()>();
	//bruteforce<SpectralBF>();
	
	system("pause");
	return 0;
}
