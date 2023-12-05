#pragma once

#include <vector>
#include <memory>
#include <map>
#include <stdexcept>

#include "Types.h"

#define TURN_TYPE long unsigned

class Strategy;

class GameManager {
public:
	constexpr static char COMPETITORS = Globals::COMPETITORS;

	GameManager();
	~GameManager();

	// adds a strategy to compete with, up to COMPETITORS at a time
	void AddStrategy(Strategy* strat);

	// replaces the "id"-th strategy
	void AddStrategy(Strategy* strat, size_t id);

	// returns a vector of currently added strategies
	const std::vector<Strategy*>* GetStrategies() const;

	// sets how many turns the game should last; can't be set in a started game
	void SetMaxTurns(TURN_TYPE turns) {
		if (started_) {
			throw std::logic_error("Can't set max turns of a started game!");
		}

		turnsMax_ = turns;
	}
	TURN_TYPE GetMaxTurns() const { return turnsMax_; }

	// returns the current turn number
	TURN_TYPE GetTurn() const { return turnsCur_; }

	// returns the score for the "id"-th strategy
	// the ID are stored in the strategies themselves
	// and can be obtained from there
	double GetScore(size_t id) const;

	// returns what strategy has the id "id"
	Strategy* GetStrategy(size_t id) const;

	// registers an outcome score, where `bitOutcome` is the bit number
	// representing every strategy's turn (for example: 110 = COOPERATE, COOPERATE, DEFECT)
	void AddOutcome(char bitOutcome, std::vector<double> scores);

	// returns this turn's outcomes (ie the scores every strategy got this turn)
	const std::vector<double>* GetOutcomes() const;

	// returns ALL turns' strategies' decisions last turn (ie whether a strategy DEFECTED or COOPERATED)
	const Decisions* GetDecisions() const;

	// returns whether the game has begun
	bool GetStarted() const { return started_; }

	// finishes the game, clears all scores, decisions and strategies
	// **DOES NOT CLEAR OUTCOMES!** This is to allow restarting the game
	// with same outcomes but different strategies easily.
	void Reset();

	// locks in data and allows the game manager to perform turns.
	// this function will throw if the prerequisites aren't met
	// (amount of strategies doesn't match COMPETITORS or there are missing possible outcomes)
	void StartGame();
	
	// performs a turn, querying every strategy for a decision and incrementing points afterwards
	bool DoTurn();

private:
	void incrementScores(Decisions& outs);
	std::vector<Strategy*> strats_;
	std::map<char, std::vector<double>> outcomes_;
	Decisions pastTurn_;

	std::vector<double> scores_;
	std::vector<double> scoreChanges_;

	bool started_ = false;

	TURN_TYPE turnsMax_ = 0;
	TURN_TYPE turnsCur_ = 0;
};