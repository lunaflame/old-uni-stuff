#pragma once
#include <map>
#include <string>

#include "GameManager.h"

// smart ptr to a vector of smart ptrs to strategies, EEK
std::vector<std::shared_ptr<Strategy>> ParseLaunchArgs(
	int argc, char** argv,
	std::map<std::string, std::string>& args);