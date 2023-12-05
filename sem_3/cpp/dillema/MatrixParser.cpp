#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <tuple>
#include <regex>
#include <fstream>

#include "MatrixParser.h"
#include "GameManager.h"
#include "Strategy.h"


/*
Format:
[C]ooperate/[D]efect,[C/D],... = Score,Score,...
...

Example:
C,C,C = 5,5,5
C,C,D = 0,0,10
C,D,D = 0,7,7
D,D,D = 0,0,0
*/

const std::string defaultMtrx[] = {
	"C, C, C = 7, 7, 7",

	"C, C, D = 3, 3, 9",
	"C, D, C = 3, 9, 3",
	"D, C, C = 9, 3, 3",

	"C, D, D = 0, 5, 5",
	"D, C, D = 5, 0, 5",
	"D, D, C = 5, 5, 0",

	"D, D, D = 1, 1, 1"
};

std::tuple<char, std::vector<double>> ParseMatrixRow(const GameManager& gm, std::string row) {
	// super naive parsing
	// #1. read every character until a '=' is encountered
	//		#1.1. if it's a 'C' or a 'D', add to the bit "array" accordingly
	// #2. after the '=', check if the amount of letters read matches the amount of simultaneous competitors
	// #3. if matches, read every score (number) after the '=' and add it to the array
	// #4. return {bit, scores} where `bit` identifies what decision combination it represents

	size_t amtStrats = GameManager::COMPETITORS;

	char bit = 0;
	size_t count = 0;
	size_t endWhere = 0;

	std::vector<double> scores;
	
	// bits: C is 1, D is 0

	for (size_t i = 0; i < row.size(); i++) {
		char act = row[i];
		switch (act) {
			case '=': {
				if (count != amtStrats) {
					throw std::length_error("Matrix parsing: actions count doesn't match strategy count.");
				}

				endWhere = i;
				break;
			}

			// cooperate is 0, defect is 1
			case 'C':
				count++;
				continue;

			case 'D':
				bit += 1 << (amtStrats - count - 1);
				count++;
				continue;
		}
	}

	if (count != amtStrats) {
		throw std::length_error("Matrix parsing: actions count doesn't match strategy count.");
	}

	count = 0;

	const std::regex digitMatch("([0-9]+)");
	std::smatch base_match;
	row = row.substr(endWhere);

	while (std::regex_search(row, base_match, digitMatch)) {
		std::string base = base_match[0].str();
		scores.push_back(atof(base.c_str()));
		count++;
		row = row.substr(base_match.prefix().length() + base.size());
	}
	
	if (count != amtStrats) {
		throw std::length_error("Matrix parsing: scores count doesn't match strategy count.");
	}

	return std::make_tuple(bit, scores);
}


void addOutcome(GameManager& mgr, std::string line) {
	std::tuple<char, std::vector<double>> tp = ParseMatrixRow(mgr, line);
	char bit;

	auto scoresPtr = new std::vector<double>;
	auto scores = *scoresPtr;

	std::tie(bit, scores) = tp;

	mgr.AddOutcome(bit, scores);
}

void ConfigureMatrix(const std::string& filePath, GameManager& mgr) {
	std::vector<std::string> lines;

	if (filePath != "") {
		std::ifstream mxFile(filePath);
		mxFile.exceptions( /*std::ifstream::failbit |*/ std::ifstream::badbit);
		// I think that you misunderstand what failbit means. It does not mean that the file cannot be read.
		// It is rather used as a flag that the last operation succeeded. 

		// i am going to lose my MIND

		if (mxFile.fail()) {
			std::string err = "Failed to open file: ";
			err += filePath;
			throw std::runtime_error(err.c_str());
		}

		std::string curLine;

		while (std::getline(mxFile, curLine)) {
			addOutcome(mgr, curLine);
		}
	}
	else {
		// parse the default matrix
		for (const std::string& text : defaultMtrx) {
			addOutcome(mgr, text);
		}
	}
}