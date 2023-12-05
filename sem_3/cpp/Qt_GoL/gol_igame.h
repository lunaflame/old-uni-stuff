#pragma once

#include <tuple>

class GoL_IGame {
public:
	virtual ~GoL_IGame() {};

	// Create an empty field. Cannot be used until ResetSize is called.
	GoL_IGame() {};

	// Create an empty field of size (w, h). Can be used immediately.
	GoL_IGame(size_t width, size_t height) {};

	// Clear out all cells from the field and set its' size to (w, h).
	virtual void ResetSize(size_t w, size_t h) = 0;
	
	// Set a survival rule for `n` neighboring cells.
	// (should alive cells stay alive when there are `n` neighbors?)
	virtual void SetSurvivalRule(unsigned int n, bool state) = 0;

	// Set a revival rule for `n` neighboring cells.
	// (should dead cells become alive when there are `n` neighbors?)
	virtual void SetRevivalRule(unsigned int n, bool state) = 0;

	// Get the current survival/revival rules for `n` neighbors.
	virtual bool GetSurvivalRule(unsigned int n) = 0;
	virtual bool GetRevivalRule(unsigned int n) = 0;

	// Perform a turn.
	virtual void NextGeneration() = 0;
	
	// Get the state of a cell at (x, y). Coordinates wrap around
	// (i.e. GetCellState(-1, -1) is the same as GetCellState(width - 1, height - 1)
	virtual bool GetCellState(long long x, long long y) = 0;

	// Set the state of a cell at (x, y). Coordinates _do not_ wrap around.
	virtual void SetState(size_t x, size_t y, bool state) = 0;

	// Get the current width and height of the field.
	virtual std::tuple<size_t, size_t> GetSize() = 0;

	// Get the maximum supported width and height of the field.
	// You should not attempt to create fields bigger than what this
	// function returns in either direction.
	virtual std::tuple<size_t, size_t> GetMaxSize() = 0;

	// Converts a number into an (x, y) coordinate.
	// (i.e. for a field of size (5, 5) `0` is (0, 0), `1` is (1, 0), `6` is (1, 1))
	virtual std::tuple<size_t, size_t> IndToXY(size_t ind) = 0;

	GoL_IGame& operator= (GoL_IGame&& other) = delete;
	GoL_IGame(const GoL_IGame& other) = delete;
};