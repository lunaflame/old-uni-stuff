#pragma once

#include <tuple>
#include "gol_igame.h"

class GoL_Game : GoL_IGame {
public:
	static const size_t MaxWidth = 2000;
	static const size_t MaxHeight = 2000;

	GoL_Game(size_t w, size_t h);
	GoL_Game();
	~GoL_Game();
	
	void ResetSize(size_t w, size_t h);
	void NextGeneration();
	bool GetCellState(long long x, long long y);
	void SetState(size_t x, size_t y, bool state);
	void SetDefaultRules();

	void SetSurvivalRule(unsigned int n, bool state);
	void SetRevivalRule(unsigned int n, bool state);

	bool GetSurvivalRule(unsigned int n);
	bool GetRevivalRule(unsigned int n);

	std::tuple<size_t, size_t> GetSize();
	std::tuple<size_t, size_t> GetMaxSize();
	std::tuple<size_t, size_t> IndToXY(size_t ind);
private:
	bool* cells = nullptr;
	bool* cellsAlt = nullptr;

	void SetStateAlt(size_t x, size_t y, bool state);
	void nextCellState(size_t x, size_t y);
	
	bool reviveRule[10] = { 0 }; // default is S23/B3 (set in initialize)
	bool surviveRule[10] = { 0 }; // [surrounding_cells] = should_live

	int width = 0;
	int height = 0;

	size_t TickCount = 0;
};