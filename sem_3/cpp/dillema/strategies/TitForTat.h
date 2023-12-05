#ifndef TTT_IG
#define TTT_IG

#include <iostream>
#include <random>
#include <array>

#include "../Strategy.h"
#include "../Types.h"

class TitForTat : Strategy {
public:
	static Strategy* Create() { return new TitForTat; }
	bool DoTurn(const unsigned long turnNum);
	void PostTurn(Decisions othersTurns);
	void Reset();
private:
	bool lastDefect = false;
	static bool registered;
};

#endif