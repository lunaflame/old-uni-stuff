#include "Trivials.h"
#include <exception>

#include "../GameManager.h"
#include "../StrategyFactory.h"

bool AlwaysCoop::DoTurn(const unsigned long turnNum) {
	return COOPERATE;
}

bool AlwaysBetray::DoTurn(const unsigned long turnNum) {
	return DEFECT;
}

bool Random::DoTurn(const unsigned long turnNum) {
	std::random_device generator;
	std::mt19937 gen(generator()); // my god, how hard is it to just make a sane random number generator API?
	std::uniform_int_distribution<int> distribution(1, 2);

	int outNum = distribution(generator);
	bool outcome = outNum == 1;

	if (outcome) {
		return DEFECT;
	}
	else {
		return COOPERATE;
	}
}

bool Erroring::DoTurn(const unsigned long turnNum) {
	throw std::runtime_error("All I do is error");
}


bool AlwaysCoop::registered = StrategyFactory::Get().RegisterStrategy("AlwaysCoop", &AlwaysCoop::Create);
bool AlwaysBetray::registered = StrategyFactory::Get().RegisterStrategy("AlwaysBetray", &AlwaysBetray::Create);
bool Random::registered = StrategyFactory::Get().RegisterStrategy("Random", &Random::Create);
bool Erroring::registered = StrategyFactory::Get().RegisterStrategy("Erroring", &Erroring::Create);