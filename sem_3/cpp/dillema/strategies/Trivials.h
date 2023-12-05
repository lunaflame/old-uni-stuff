// #pragma once // you should fix yourself... NOW!

#ifndef Trivial_IG
#define Trivial_IG

#include <iostream>
#include <random>

#include "../Strategy.h"
#include "../StrategyFactory.h"

class AlwaysCoop : Strategy {
public:
	static Strategy* Create() { return new AlwaysCoop; }
	bool DoTurn(const unsigned long turnNum);
private:
	static bool registered;

};

class AlwaysBetray : Strategy {
public:
	static Strategy* Create() { return new AlwaysBetray; }
	bool DoTurn(const unsigned long turnNum);
private:
	static bool registered;
};

class Random : Strategy {
public:
	static Strategy* Create() { return new Random; }
	bool DoTurn(const unsigned long turnNum);
private:
	static bool registered;
};

class Erroring : Strategy {
public:
	static Strategy* Create() { return new Erroring; }
	bool DoTurn(const unsigned long turnNum);
private:
	static bool registered;
};

#endif