#ifndef Prober_IG
#define Prober_IG

#include <vector>
#include <array>

#include "../Strategy.h"
#include "../StrategyFactory.h"
#include "../Types.h"

#include "TitForTat.h"

class Prober : Strategy {
public:
	std::vector<bool> turnSequence;

	static Strategy* Create() { return new Prober; }
	bool DoTurn(const unsigned long turnNum);
	void PostTurn(Decisions othersTurns);
	void Reset();
	void ReadConfig(std::string path);

	const std::string configFn = "prober.cfg";
private:
	bool probeDefected = false;
	bool probing = false;

	TitForTat titStr;
	static bool registered;
};

#endif