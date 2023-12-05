#include "pch.h"

#include <vector>
#include <iostream>

#include "GameManager.h"
#include "GameManager.cpp"

#include "StrategyFactory.h"

#include "Strategy.h"
#include "Strategy.cpp"

#include "strategies/Strategies.h"
#include "strategies/Prober.cpp"
#include "strategies/TitForTat.cpp"
#include "strategies/Trivials.cpp"
#include "strategies/Spiteful.cpp"

// Try constructing everything: the game manager, strategy factory and
// every registered strategy

TEST(Core, Construction) {
	GameManager mgr;
	StrategyFactory fac = StrategyFactory::Get();

	std::map<std::string, stratCtor> all_strats = fac.GetAll();
	std::vector<Strategy*> strats;

	for (auto const& [name, ctor] : all_strats) {
		strats.push_back(ctor());
	}

	fac.NewStrategyByName("Nonexistent strategy");

	// no crashes = good
}

#define DEFAULT_STRATEGY "AlwaysCoop"
#define DEFAULT_TURNS 10

GameManager* makeSampleGame() {
	GameManager* mgrPtr = new GameManager;

	StrategyFactory fac = StrategyFactory::Get();

	for (int i = 0; i < 1 << GameManager::COMPETITORS; i++) {
		std::vector<double> scores({ 5, 5, 5 });
		mgrPtr->AddOutcome(i, scores);
	}

	for (int i = 0; i < GameManager::COMPETITORS; i++) {
		Strategy* strat = fac.NewStrategyByName(DEFAULT_STRATEGY);
		mgrPtr->AddStrategy(strat);
	}

	mgrPtr->SetMaxTurns(DEFAULT_TURNS);

	return mgrPtr;
}

TEST(Core, PreGame) {
	GameManager mgr;
	StrategyFactory fac = StrategyFactory::Get();

	// starting a game without outcomes nor strategies: bad!
	EXPECT_ANY_THROW(mgr.StartGame());
	EXPECT_ANY_THROW(mgr.DoTurn());
	EXPECT_ANY_THROW(mgr.GetStrategy(1)); // no strategies loaded yet; should throw
	EXPECT_ANY_THROW(mgr.GetScore(3));		// getting score for strategy #3 (0-ind so 4th); should throw

	for (int i = 1; i < (1 << GameManager::COMPETITORS); i++) {
		std::vector<double> scores({ 5, 5, 5 });
		mgr.AddOutcome(i, scores);
	}

	std::vector<double> scores({ 5, 5, 5 });
	EXPECT_ANY_THROW(mgr.AddOutcome((1 << GameManager::COMPETITORS) + 1, scores));

	std::vector<std::shared_ptr<Strategy>> stratsToDel;

	for (int i = 1; i < GameManager::COMPETITORS; i++) {
		Strategy* strat = fac.NewStrategyByName(DEFAULT_STRATEGY);
		EXPECT_ANY_THROW(mgr.AddStrategy(strat, i)); // pushing above max
		mgr.AddStrategy(strat);
		
		stratsToDel.push_back(std::shared_ptr<Strategy>(strat));
	}

	// intentionally missing outcome `0`

	EXPECT_ANY_THROW(mgr.StartGame()); // shouldn't work due to missing strategy

	Strategy* strat = fac.NewStrategyByName(DEFAULT_STRATEGY);
	
	mgr.AddStrategy(strat, 2);

	EXPECT_ANY_THROW(mgr.StartGame()); // shouldn't work due to missing outcome

	mgr.AddOutcome(0, std::vector<double>({ 5, 5, 5 }));
	
	// test being able to replace strategies
	Strategy* replaceStrat = fac.NewStrategyByName(DEFAULT_STRATEGY);
	auto oldStrat = mgr.GetStrategy(0);

	mgr.AddStrategy(replaceStrat, 0);

	EXPECT_ANY_THROW(mgr.AddStrategy(replaceStrat)); // max already pushed
	EXPECT_ANY_THROW(mgr.AddStrategy(replaceStrat, 3)); // ID above max

	EXPECT_NO_THROW(mgr.StartGame()); // should work; all present

	EXPECT_ANY_THROW(mgr.AddStrategy(replaceStrat, 0)); // game already started

	EXPECT_NE(replaceStrat, oldStrat);
	EXPECT_EQ(mgr.GetStrategy(0), replaceStrat);

	// test clearing
	mgr.Reset();

	EXPECT_TRUE(mgr.GetStrategies()->empty());	// clearing must empty out the strategies
	EXPECT_ANY_THROW(mgr.GetStrategy(0));		// trying to get a non-existent strategy should throw
	EXPECT_FALSE(mgr.GetOutcomes()->size());		// clearing must not reset outcomes
}

TEST(Core, Game) {

	std::shared_ptr<GameManager> mgrPtr(makeSampleGame());

	EXPECT_EQ(mgrPtr->GetMaxTurns(), DEFAULT_TURNS);

	mgrPtr->StartGame();
	size_t turnCounter = 0;

	while (mgrPtr->DoTurn()) {
		turnCounter++;
		EXPECT_EQ(mgrPtr->GetTurn(), turnCounter);
	}

	// check the last turn's score changes
	const std::vector<double>* ocs = mgrPtr->GetOutcomes();
	for (const double& score : *ocs) {
		// the scoring matrix is all 5's
		EXPECT_EQ(score, 5); // not a very dynamic check, huh
	}

	const Decisions* decs = mgrPtr->GetDecisions();
	EXPECT_EQ(decs->size(), 3); // [strategy_id] = turn


	// check the last turn's strategies' decisions
	// (default is they all cooperate)
	for (const bool turn : *decs) {
		EXPECT_EQ(turn, Strategy::COOPERATE);
	}

	// check the scores are correct (DEFAULT_TURNS * 5pts)
	for (char i = 0; i < GameManager::COMPETITORS; i++) {
		EXPECT_EQ(mgrPtr->GetScore(i), 5 * mgrPtr->GetMaxTurns());
	}
}

TEST(Core, History) {
	GameManager mgr;
	StrategyFactory fac = StrategyFactory::Get();

	// todo: play a game of coop, coop, betray
	// then try reading the history and total score
}