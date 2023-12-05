#include "Spiteful.h"

#include "../StrategyFactory.h"
#include "../GameManager.h"

void Spiteful::Reset() {
	MemoryStrategy::Reset();
	mad_ = false;
}

bool Spiteful::DoTurn(const unsigned long turnNum) {
	if (turnNum == 0) {
		return COOPERATE;
	}

	Decisions turns = GetOpponentTurns(turnNum);

	if (turns[0] == DEFECT || turns[1] == DEFECT || mad_) {
		mad_ = true;
		return DEFECT;
	}

	return COOPERATE;
}

bool Spiteful::registered = StrategyFactory::Get().RegisterStrategy("Vengeful", &Spiteful::Create)
 && StrategyFactory::Get().RegisterStrategy("Spiteful", &Spiteful::Create);