#include "Strategy.h"

#include <iostream>
#include <stdexcept>
#include <array>

Decisions MemoryStrategy::GetOpponentTurns(const unsigned long turnNum) {
	if (turnNum > pastTurns_.size()) {
		throw std::out_of_range("GetOpponentTurns: getting turns of a round that didn't happen yet!");
	}
	
	// get the turn under that turn number
	Decisions thatTurn = pastTurns_.at(turnNum - 1);
	Decisions ret;

	// collect turns of every opponent who is not us into the array
	char amt = 0;

	for (size_t i = 0; i < GameManager::COMPETITORS; i++) {
		if (i == GetNumber()) { continue; }

		ret[amt] = thatTurn[i];
		amt++;
	}

	return ret;
}