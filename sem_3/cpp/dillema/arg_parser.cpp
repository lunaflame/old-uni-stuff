#include <string>
#include <regex>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "GameManager.h"
#include "StrategyFactory.h"
#include "Strategy.h"
#include "arg_parser.h"

#define DEFAULT_STEPS "10"

typedef std::vector<std::shared_ptr<Strategy>> Strategies;

static bool hasFlag(std::map<std::string, std::string>& map, const std::string& key) {
	return map.contains(key);
	/*
	try {
		map.at(key);
		return true;
	}
	catch (std::exception dontcare) {
		return false;
	}*/
}

static void doDefaultArgs(Strategies& strats, std::map<std::string, std::string>& flags) {
	if (!hasFlag(flags, "mode")) {
		if (strats.size() > 3) {
			flags.insert({ "mode", "tournament" });
		} else {
			flags.insert({ "mode", "detailed" });
		}
	}

	if (!hasFlag(flags, "steps")) {
		flags.insert({"steps", DEFAULT_STEPS });
	} else {
		try { std::stoi(flags.at("steps")); } // couldn't convert steps to an int; set to default
		catch (std::exception const&) { flags.insert({ "steps", DEFAULT_STEPS }); }
	}

	if (!hasFlag(flags, "matrix")) {
		flags.insert({ "matrix", "" });
	}
}

std::vector<std::shared_ptr<Strategy>> ParseLaunchArgs(
	int argc, char** argv,
	std::map<std::string, std::string>& args) {

	const std::regex flag_regex("\\-\\-?(.+)");
	std::smatch match;

	std::string curFlag;
	StrategyFactory fac = StrategyFactory::Get();

	Strategies strats;

	for (int i = 1; i < argc; i++) {
		// todo: this should be handled better
		std::string asStr(argv[i]);
		if (curFlag.empty() && std::regex_match(asStr, match, flag_regex)) {
			curFlag = match[1];
		} else if (!curFlag.empty()) {
			// flag set and this is the value
			args.insert({ curFlag, asStr });
			curFlag.clear();
		} else {
			try {
				Strategy* strat = fac.NewStrategyByName(argv[i]);
				if (strat != nullptr) {
					strats.push_back(std::shared_ptr<Strategy>(strat));
				}
			} catch (std::out_of_range const& ex) {
				std::string exc = "Strategy \"";
				exc += argv[i];
				exc += "\" did not exist! Exiting...";

				throw std::runtime_error(exc.c_str());
				return strats;
			}
			
		}
	}

	if (strats.size() < GameManager::COMPETITORS) {
		std::string exc = "Not enough strategies provided; need at least 3, but " +
			std::to_string(strats.size()) +
			" were given. Exiting...";

		throw std::runtime_error(exc.c_str());
		return strats;
	}

	doDefaultArgs(strats, args);

	std::string cfgPath;
	if (args.contains("config")) {
		cfgPath = args.at("config");
	}

	for (auto const& strat : strats) {
		strat->ReadConfig(cfgPath);
	}

	return strats;
}