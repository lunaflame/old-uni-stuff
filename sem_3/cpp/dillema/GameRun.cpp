#include <vector>
#include <iostream>
#include <stdexcept>
#include <string>
#include <memory>
#include <utility>

#include "GameRun.h"
#include "GameManager.h"
#include "Strategy.h"

void showWinners(std::map<Strategy*, double> scores) {
    std::vector<bool> winners = std::vector<bool>();

    double mx = 0;
    for (auto const& [strat, score] : scores) {
        mx = std::max(mx, score);
    }

    for (auto const& [strat, score] : scores) {
        std::cout << "  " << strat->GetName() << ": " << score;

        if (score == mx) {
            std::cout << " (WINNER!)";
        }
        std::cout << std::endl;
    }
}

void doDetailed(GameManager& mgr, Strategies& strats) {
    for (int i = 0; i < GameManager::COMPETITORS; i++) {
        mgr.AddStrategy(strats[i].get());
    }

    mgr.StartGame();

    while (mgr.DoTurn()) {
        if (mgr.GetTurn() == mgr.GetMaxTurns()) {
            std::cout << "Turn #" << mgr.GetTurn() << " - ";
            continue; // don't spew out the last turn; the finisher will spew it anyways
        }
        std::cout << "Turn #" << mgr.GetTurn() << ": " << std::endl;

        const std::vector<double> scores = *mgr.GetOutcomes();
        const Decisions turns = *mgr.GetDecisions();

        for (int i = 0; i < GameManager::COMPETITORS; i++) {
            std::cout << "  " << mgr.GetStrategy(i)->GetName() << " #" << i + 1 << ": ";
            std::string decision;

            if (turns[i] == Strategy::DEFECT) {
                decision = "Defected";
            }
            else {
                decision = "Cooperated";
            }

            std::cout << mgr.GetScore(i) << " - " << decision << " (+" << scores[i] << ")" << std::endl;
        }

        std::cout << "Press any key to continue..." << std::endl;

        getchar();
    }

    std::cout << "Done! Scores:" << std::endl;
    std::map<Strategy*, double> scores;
    for (int i = 0; i < GameManager::COMPETITORS; i++) {
        Strategy* str = mgr.GetStrategy(i);
        scores[str] = mgr.GetScore(i);
    }

    showWinners(scores);
}

void doFast(GameManager& mgr, Strategies& strats) {
    for (int i = 0; i < GameManager::COMPETITORS; i++) {
        mgr.AddStrategy(strats[i].get());
    }

    std::cout << "Running " << mgr.GetMaxTurns() << " turns..." << std::endl;

    mgr.StartGame();

    while (mgr.DoTurn());

    std::cout << "Done! Scores:" << std::endl;
    std::map<Strategy*, double> scores;
    for (int i = 0; i < GameManager::COMPETITORS; i++) {
        Strategy* str = mgr.GetStrategy(i);
        scores[str] = mgr.GetScore(i);
    }

    showWinners(scores);
}


// use a fixed size array
void doTournamentMatch(GameManager& mgr, std::shared_ptr<Strategy> strats[GameManager::COMPETITORS]) {
    mgr.Reset();

    for (int i = 0; i < GameManager::COMPETITORS; i++) {
        mgr.AddStrategy(strats[i].get(), i);
    }

    mgr.StartGame();
    while (mgr.DoTurn());
}

void showOutcomes(GameManager& mgr,
    std::map<Strategy*, double>& scores,
    std::shared_ptr<Strategy> curStrats[GameManager::COMPETITORS]) {

    std::cout << "Round complete, scores:" << std::endl;
    
    std::map<Strategy*, char> participated;

    for (size_t i = 0; i < GameManager::COMPETITORS; i++) {
        Strategy* strat = curStrats[i].get();
        scores[strat] += mgr.GetScore(i);
        participated[strat] = i;
    }

    for (auto const& [strat, curScore] : scores) {
        std::cout << "  " << strat->GetName() << ": ";
        
        std::cout << curScore;

        // really? it took them 10 years to add a "contains" method?
        if (participated.contains(strat)) {
            std::cout << " (+" << mgr.GetScore(participated[strat]) << ")";
        }

        std::cout << std::endl;
    }

    getchar();
}

void doTournament(GameManager& mgr, Strategies& strats) {
    char sz = (char)strats.size();

    std::shared_ptr<Strategy> sArr[GameManager::COMPETITORS] = {};
    std::map<Strategy*, double> scores;

    for (auto const& value : strats) {
        scores[value.get()] = 0;
    }

    // this is not very pretty but the only other method i can think of
    // is not a very readable/obvious one

    for (int p1 = 0; p1 < sz; p1++) {
        sArr[0] = strats[p1];
        for (int p2 = p1 + 1; p2 < sz; p2++) {
            sArr[1] = strats[p2];
            for (int p3 = p2 + 1; p3 < sz; p3++) {
                sArr[2] = strats[p3];
                
                doTournamentMatch(mgr, sArr);
                showOutcomes(mgr, scores, sArr);
            }
        }
    }

    std::cout << "Tournament complete! Final scores:" << std::endl;
    showWinners(scores);
}