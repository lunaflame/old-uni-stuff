#ifndef Spiteful_IG
#define Spiteful_IG

#include <iostream>
#include <random>

#include "../Strategy.h"
#include "../StrategyFactory.h"
#include "../Types.h"

class Spiteful : MemoryStrategy {
public:
	static Strategy* Create() { return new Spiteful; }
	bool DoTurn(const unsigned long turnNum);
	void Reset();
private:
	bool mad_ = false;
	static bool registered;
};

#endif