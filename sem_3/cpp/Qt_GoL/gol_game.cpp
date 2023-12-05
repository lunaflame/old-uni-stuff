#include "gol_game.h"
#include <memory>
#include <QDebug>
#include <assert.h>

GoL_Game::GoL_Game(size_t w, size_t h) {
	ResetSize(w, h);
	SetDefaultRules();
}

GoL_Game::GoL_Game() {
	SetDefaultRules();
}

void GoL_Game::SetDefaultRules() {
	for (int i = 0; i < 10; i++) {
		// reset all rules to false
		SetSurvivalRule(i, false);
		SetRevivalRule(i, false);
	}

	// S23
	SetSurvivalRule(2, true);
	SetSurvivalRule(3, true);

	// B3
	SetRevivalRule(3, true);
}

GoL_Game::~GoL_Game() {
	if (cells != nullptr) {
		delete[] cells;
		delete[] cellsAlt;
	}
}

void GoL_Game::ResetSize(size_t w, size_t h) {
	cells = new bool[w * h];
	cellsAlt = new bool[w * h];

	std::memset(cells, 0, w * h);
	std::memset(cellsAlt, 0, w * h);

	width = w;
	height = h;
}

void GoL_Game::SetSurvivalRule(unsigned int n, bool state) {
	assert(n < 10);
	surviveRule[n] = state;
}

void GoL_Game::SetRevivalRule(unsigned int n, bool state) {
	assert(n < 10);
	reviveRule[n] = state;
}

bool GoL_Game::GetSurvivalRule(unsigned int n) {
	assert(n < 10);
	return surviveRule[n];
}

bool GoL_Game::GetRevivalRule(unsigned int n) {
	assert(n < 10);
	return reviveRule[n];
}

void GoL_Game::NextGeneration() {
	std::memset(cellsAlt, 0, width * height);
	TickCount++;

	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			nextCellState(x, y);
		}
	}

	// swap fields
	bool* temp = cells;
	cells = cellsAlt;
	cellsAlt = temp;
}

void GoL_Game::SetState(size_t x, size_t y, bool state) {
	cells[y * width + x] = state;
}

void GoL_Game::SetStateAlt(size_t x, size_t y, bool state) {
	cellsAlt[y * width + x] = state;
}

std::tuple<size_t, size_t> GoL_Game::IndToXY(size_t ind) {
	return std::tuple<size_t, size_t>(ind % width, std::floor(ind / width));
}
 
bool GoL_Game::GetCellState(long long x, long long y) {
	if (x < 0) {
			// x = -225% of width -> x = -25% of width
		// qDebug() << "X wrap " << x << " -> " << (x % width) + width;
		x = (x % width)
			// x = -25% -> x = 75%
			+ width;
	}

	if (y < 0) {
		// qDebug() << "Y wrap " << y << " -> " << (y % height) + height;
		y = (y % height)
			+ height;
	}

	x %= width;
	y %= height;

	return cells[y * width + x];
}

void GoL_Game::nextCellState(size_t x, size_t y) {
	long long lx = x;
	long long ly = y;

	char aliveCells =
		  GetCellState(lx - 1, ly - 1) + GetCellState(lx, ly - 1) + GetCellState(lx + 1, ly - 1)
		+ GetCellState(lx - 1, ly) + 0 + GetCellState(lx + 1, ly)
		+ GetCellState(lx - 1, ly + 1) + GetCellState(lx, ly + 1) + GetCellState(lx + 1, ly + 1);


	if (GetCellState(lx, ly)) {
		SetStateAlt(lx, ly, surviveRule[aliveCells]);
	} else if (reviveRule[aliveCells]) {
		SetStateAlt(lx, ly, true);
	}
}

std::tuple<size_t, size_t> GoL_Game::GetSize() {
	return std::tuple<size_t, size_t>({ width, height });
}

std::tuple<size_t, size_t> GoL_Game::GetMaxSize() {
	return std::tuple<size_t, size_t>({ MaxWidth, MaxHeight });
}