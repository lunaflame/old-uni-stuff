#include "gol_widget.h"

#include <string>
#include <format>
#include <regex>
#include <iostream>
#include <QDebug>

#include "gol_game.h"

// throwing function
void validateSize(size_t w, size_t h) {
    if (w > GoL_Game::MaxWidth) {
        throw std::exception(
            std::format("Field width cannot be higher than {}! (tried to make a {}-wide field)", GoL_Game::MaxWidth, w).c_str()
        );
    }
    else if (h > GoL_Game::MaxHeight) {
        throw std::exception(
            std::format("Field height cannot be higher than {}! (tried to make a {}-high field)", GoL_Game::MaxHeight, h).c_str()
        );
    }
}

// returns false if it should stop reading completely
bool readCellsLine(GoL_Game& ret, std::string line, size_t& curPos) {
    std::string curNum;

    auto gSz = ret.GetSize();
    auto& [w, h] = gSz;

    qDebug() << "-- Reading RLE --";

    for (char c : line) {
        // read cell data
        if (std::isdigit(c)) {
            curNum.push_back(c);
        }
        else {
            int num = std::atoi(curNum.c_str());
            if (num == 0) {
                num = 1;
            }

            bool state = false;

            if (c == 'b') {
                state = false; // dead
            } else if (c == 'o') {
                state = true; // alive
            } else if (c == '$') {
                // special; just set cursor to the beginning of the next line

                size_t add = (w - (curPos % w)) + (w * (num - 1));
                if (curPos % w == 0) { add -= w; } // wtf?

                // qDebug() << "$: setting cell " << curPos << " -> " << curPos + add;
                curPos += add;
                curNum = "";
                continue;
            } else if (c == '!') {
                return false; // end of input; bail completely
            } else {
                std::string bad("Unrecognized character in cell data: '");
                bad += c;
                bad += "'";

                throw std::exception(bad.c_str());
            }

            // qDebug() << "Setting " << num << " of " << state << "(" << curPos << " - " << curPos + num << ")";

            if (curPos + num > (size_t)(w * h)) {
                throw std::exception("Out of range cells!");
            }

            for (int i = curPos; i < curPos + num; i++) {
                auto xy = ret.IndToXY(i);
                ret.SetState(std::get<0>(xy), std::get<1>(xy), state);
            }

            curPos += num;
            curNum = "";
        }
    }

    return true;
}

void matchRuleLine(GoL_Game& ret, std::string line) {
    std::regex coords("x = (\\d+), y = (\\d+)");
    std::smatch match;

    if (!std::regex_search(line, match, coords)) {
        throw std::exception("Could not parse field size.");
    }

    size_t w = std::stoll(match[1].str());
    size_t h = std::stoll(match[2].str());

    validateSize(w, h);

    ret.ResetSize(w, h);

    std::regex rules(", rule = B(\\d+)/S(\\d+)");

    if (!std::regex_search(line, match, rules)) {
        qDebug() << "rules not found; bailing";
        return; // rules are optional
    }

    qDebug() << "rules found";

    // set all rules to 0
    for (int i = 0; i <= 9; i++) {
        ret.SetRevivalRule(i, false);
        ret.SetSurvivalRule(i, false);
    }

    for (char rc : match[1].str()) {
        ret.SetRevivalRule(rc - '0', true);
    }

    for (char sc : match[2].str()) {
        ret.SetSurvivalRule(sc - '0', true);
    }

    for (int i = 0; i <= 9; i++) {
        qDebug() << "cell " << i <<
            ret.GetRevivalRule(i) << ", " <<
            ret.GetSurvivalRule(i);
    }
}

void RLEToGame(std::string rle, GoL_Game& ret) {
    std::istringstream str(rle);
    std::string line;

    // first line: 

    bool foundRules = false;

    size_t curPos = 0;

    while (std::getline(str, line)) {
        if (line[0] == '#' || line.size() == 0) {
            continue; // comment or empty line
        }

        if (!foundRules) {
            // first valid line must be ruleset
            matchRuleLine(ret, line);
            foundRules = true;
            continue;
        }

        if (!readCellsLine(ret, line, curPos)) {
            break;
        }
    }
}