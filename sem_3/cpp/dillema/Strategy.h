#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <array>
#include <iostream>

#include "GameManager.h"

class Strategy {
public:
	static constexpr bool DEFECT = true;
	static constexpr bool COOPERATE = false;

	Strategy() {};
	virtual ~Strategy() {};
	
	// set before doing a turn

	void SetNumber(char num) { stratNum = num; }
	void SetName(std::string nm) { name = nm; }

	size_t GetNumber() { return stratNum; }
	std::string GetName() { return name; }

	// ReadConfig is called when the strategy is initialized to allow it
	// to read a custom config file, if one is present
	virtual void ReadConfig(std::string path) {};

	// DoTurn is called when the strategy is required to make a turn,
	// either defecting or cooperating (by returning DEFECT or COOPERATE)
	// mandatory implementation
	virtual bool DoTurn(const unsigned long turnNum) = 0;
	
	// PostTurn is called after every strategy has made their move,
	// and provides a vector of the others' turns
	// optional implementation
	virtual void PostTurn(Decisions othersTurns) { };

	// Reset is called every time the strategy is added to the competitors list
	// to allow you to reset the state of the strategy before the next game.
	// optional implementation
	virtual void Reset() { };
private:
	size_t stratNum;
	std::string name;
};

class MemoryStrategy : public Strategy {
public:
	MemoryStrategy() { };
	virtual ~MemoryStrategy() { };

	void Reset() {
		pastTurns_.clear();
	};

	void PostTurn(Decisions* othersTurns) {

		Decisions cpy;
		for (int i = 0; i < GameManager::COMPETITORS; i++) {
			cpy[i] = (*othersTurns)[i];
		}

		pastTurns_.push_back(cpy);
	};

	Decisions GetOpponentTurns(const unsigned long turnNum);
private:
	std::vector<Decisions> pastTurns_;
};