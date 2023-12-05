#pragma once

#include <array>

namespace Globals {
	constexpr static char COMPETITORS = 3;
}

typedef std::array<bool, Globals::COMPETITORS> Decisions;