#pragma once
#include <string>
#include <map>
#include <memory>
#include <iostream>

// because i do Strategy->name = ... , forward decl means copypasting std::string name
// might as well just include strategy then, lole
// class Strategy;

#include "Strategy.h"

typedef Strategy* (*stratCtor)(void);
typedef Strategy* retType;

class StrategyFactory {
public:
	retType NewStrategyByName(std::string name) {
		if (!nameToStrat_.contains(name)) {
			return nullptr;
		}

		stratCtor ctor = nameToStrat_.at(name);

		auto ptr = retType(ctor());
		ptr->SetName(name);

		return ptr;
	};

	static StrategyFactory& Get() {
		static StrategyFactory ret;
		return ret;
	}
	
	const std::map<std::string, stratCtor> GetAll() const {
		return nameToStrat_;
	}

	bool RegisterStrategy(std::string name, stratCtor ctor) {
		std::cout << "Registered strat: " << name << std::endl;

		nameToStrat_.insert({ name, ctor });
		return true;
	}

private:
	std::map<std::string, stratCtor> nameToStrat_;
};