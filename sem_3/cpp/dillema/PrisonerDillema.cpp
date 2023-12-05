#ifndef TESTING

#include <iostream>
#include <tuple>
#include <map>
#include <string>

#include "GameManager.h"
#include "arg_parser.h"
#include "MatrixParser.h"

#include "GameRun.h"
#include "strategies/Strategies.h"

int main(int argc, char** argv) {
    GameManager mgr;
    std::map<std::string, std::string> flags;
       
    std::vector<std::shared_ptr<Strategy>> strats;

    try {
        strats = ParseLaunchArgs(argc, argv, flags);
    }
    catch (std::exception ex) {
        std::cout << "Exception while parsing launch args: " << ex.what() << std::endl;
        return -1;
    }

    try {
        ConfigureMatrix(flags.at("matrix"), mgr);
    }
    catch (std::exception ex) {
        std::cout << "Exception while configuring the game with the matrix.\n C++ sez: \"" << ex.what()
            << "\"\n Thank u c++ veri helpful" << std::endl;

        return -2;
    }


    mgr.SetMaxTurns(std::stoi(flags.at("steps")));

    try {
        std::string mode = flags.at("mode");
        std::cout << "Running mode: " << mode << std::endl;
        if (mode == "detailed") {
            doDetailed(mgr, strats);
        }
        else if (mode == "fast") {
            doFast(mgr, strats);
        }
        else if (mode == "tournament") {
            doTournament(mgr, strats);
        }
        else {
            auto err = std::string("unrecognized mode (");
            err += mode + ")";
            throw std::runtime_error(err.c_str());
        }
    }
    catch (std::exception exc) {
        std::cout << "Exception thrown while playing the game: " << exc.what() << std::endl;
        return -3;
    }

    return 0;
}

#endif