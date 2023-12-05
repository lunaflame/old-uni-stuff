#pragma once

#include <memory>
#include <vector>

class GameManager;
class Strategy;

typedef std::vector<std::shared_ptr<Strategy>> Strategies;

void doDetailed(GameManager& mgr, Strategies& strats);
void doFast(GameManager& mg, Strategies& strats);
void doTournament(GameManager& mgr, Strategies& strats);
