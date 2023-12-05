#include <memory>
#include <stdexcept>
#include <cmath>
#include <bitset>
#include <iostream>
#include <cassert>

#include "GameManager.h"
#include "Strategy.h"

GameManager::GameManager() {
	pastTurn_ = { Strategy::DEFECT };

	for (int i = 0; i < GameManager::COMPETITORS; i++) {
		scores_.push_back(0); // initialize first scores
	}
}

GameManager::~GameManager() {
	// ?? need anything?
}

void GameManager::Reset() {
	started_ = false;
	turnsCur_ = 0;
	scores_.clear();

	for (int i = 0; i < GameManager::COMPETITORS; i++) {
		scores_.push_back(0);
	}

	// outcomes_->clear();
	strats_.clear();
	scoreChanges_.clear();
}

void GameManager::AddStrategy(Strategy* strat) {
	assert(strat != nullptr);

	size_t sz = strats_.size();
	if (sz >= GameManager::COMPETITORS) {
		throw std::runtime_error("AddStrategy: max strategies already pushed!");
	}

	strat->Reset();
	strats_.push_back(strat);
}

void GameManager::AddStrategy(Strategy* strat, size_t id) {
	if (started_) {
		throw std::runtime_error("Adding strategy to a started game!");
	}

	if (id >= GameManager::COMPETITORS) {
		throw std::out_of_range("AddStrategy: ID above max!");
	}

	if (id == strats_.size()) {
		// they tried to insert at the top; instead of replacing, just push back
		AddStrategy(strat);
		return;
	}
	else if (id > strats_.size()) {
		throw std::out_of_range("AddStrategy: tried to replace a strategy above currently inserted!");
	}

	strat->Reset();
	strats_.at(id) = strat;
}

void GameManager::AddOutcome(char bitOutcome, std::vector<double> scores) {
	if (bitOutcome > std::pow(2, GameManager::COMPETITORS)) {
		throw std::out_of_range("AddOutcome: ID above max!");
	}

	outcomes_.insert({ bitOutcome, scores });
}

const std::vector<Strategy*>* GameManager::GetStrategies() const {
	return &strats_;
}

const Decisions* GameManager::GetDecisions() const {
	return &pastTurn_;
}

double GameManager::GetScore(size_t who) const {
	if (who >= scores_.size()) {
		throw std::out_of_range("GetScore: ID above size!");
	}

	return scores_.at(who);
}

Strategy* GameManager::GetStrategy(size_t who) const {
	if (who >= strats_.size()) {
		throw std::out_of_range("GetStrategy: ID above max!");
	}

	return strats_.at(who);
}

const std::vector<double>* GameManager::GetOutcomes() const {
	return &scoreChanges_;
}

void GameManager::StartGame() {
	if (strats_.size() < GameManager::COMPETITORS) {
		throw std::length_error("Not enough strategies!");
	}
	
	if (outcomes_.size() < std::pow(2, GameManager::COMPETITORS)) {
		throw std::length_error("Not enough outcomes!");
	}

	started_ = true;
}

void GameManager::incrementScores(Decisions& outs) {
	char bit = 0;

	for (int i = 0; i < GameManager::COMPETITORS; i++) {
		// turn the decisions into a bit combination then use that as the index
		bit += (char)(outs[GameManager::COMPETITORS - 1 - i]) << i;
	}

	std::vector<double> outVals = outcomes_.at(bit);
	scoreChanges_ = outVals;

	for (int i = 0; i < GameManager::COMPETITORS; i++) {
		double toAdd = outVals.at(i);
		scores_.at(i) += toAdd;
	}
}

bool GameManager::DoTurn() {
	if (!started_) {
		throw std::logic_error("Can't do a turn of a game that didn't start!");
	}
	
	if (turnsCur_ >= turnsMax_) {
		return false;
	}

	//std::cout << "Turn #" << turnsCur_ << ", " << strats_->size() << std::endl;

	Decisions thisTurn;


	// not keeping history: reuse the entry allocated at start
	thisTurn = pastTurn_;

	size_t idx = 0;

	for (auto& strat : strats_) {
		strat->SetNumber(idx);

		try {
			bool decision = strat->DoTurn(turnsCur_);
			thisTurn[idx] = decision;
			idx++;
		} catch(std::exception const& ex) {
			std::string niceExc = "Strategy \"" + strat->GetName() + "\" threw an exception!\n"
				"\"" + ex.what() + "\"";
			throw std::runtime_error(niceExc.c_str());
		}
	}

	for (auto& stratPtr : strats_) {
		stratPtr->PostTurn(thisTurn);
	}

	incrementScores(thisTurn);
	pastTurn_ = thisTurn;
	turnsCur_++;

	return true;
}