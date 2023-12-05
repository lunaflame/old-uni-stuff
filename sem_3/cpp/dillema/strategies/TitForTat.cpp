#include "TitForTat.h"

#include <array>

#include "../StrategyFactory.h"

void TitForTat::Reset() {
	lastDefect = false;
}

bool TitForTat::DoTurn(const unsigned long turnNum) {
	if (lastDefect)
		return DEFECT;

	return COOPERATE;
}

void TitForTat::PostTurn(Decisions othersTurns) {
	for (int i = 0; i < GameManager::COMPETITORS; i++) {
		if (othersTurns[i] == DEFECT) {
			lastDefect = true;
			return;
		}
	}

	lastDefect = false;
}

bool TitForTat::registered = StrategyFactory::Get().RegisterStrategy("TitForTat", &TitForTat::Create);