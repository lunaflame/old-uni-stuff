#include "Prober.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <array>

#include "../StrategyFactory.h"

void Prober::ReadConfig(std::string path) {
	std::filesystem::path cfgPath;
	cfgPath /= path;
	cfgPath /= configFn;

	std::ifstream mxFile(cfgPath);
	mxFile.exceptions(std::ifstream::badbit);

	// didnt read anything but already failed = empty file?
	if (mxFile.fail()) {
		std::cout << "Prober: config file (" << cfgPath << ") not found, using defaults." << std::endl;

		turnSequence.reserve(3);

		turnSequence.push_back(DEFECT);
		turnSequence.push_back(COOPERATE);
		turnSequence.push_back(COOPERATE);
		return;
	}

	std::cout << "ReadConfig: " << cfgPath << std::endl;
	std::string curLine;

	turnSequence.reserve(8);

	while (std::getline(mxFile, curLine)) {
		for (char& c : curLine) {
			if (c != 'C' && c != 'D') continue;

			if (c == 'C') {
				turnSequence.push_back(COOPERATE);
			} else {
				turnSequence.push_back(DEFECT);
			}
		}
	}
}

void Prober::Reset() {
	probeDefected = false;
	titStr.Reset();
}

bool Prober::DoTurn(const unsigned long turnNum) {
	// still probing; just do the probe sequence

	if (turnNum + 1 <= turnSequence.size()) {
		std::cout << "Prober still probing, doing turn " << turnSequence[turnNum] << std::endl;
		probing = true;
		return turnSequence[turnNum];
	}

	// not probing; if they defected during probe phase, always defect
	// otherwise just play titfortat
	probing = false;

	if (probeDefected) {
		return DEFECT;
	}
	return titStr.DoTurn(turnNum);

}

void Prober::PostTurn(Decisions othersTurns) {
	if (!probing) {
		// behave like titfortat
		titStr.PostTurn(othersTurns);
		return;
	}
	
	// probing...
	for (size_t i = 0; i < GameManager::COMPETITORS; i++) {
		// opponent defected during probe period: now we always defect
		if (othersTurns[i] == DEFECT && i != GetNumber()) {
			probeDefected = true;
			return;
		}
	}
}

bool Prober::registered = StrategyFactory::Get().RegisterStrategy("Prober", &Prober::Create);