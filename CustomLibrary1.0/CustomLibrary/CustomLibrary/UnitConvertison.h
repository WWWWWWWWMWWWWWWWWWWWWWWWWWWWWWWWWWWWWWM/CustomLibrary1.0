#ifndef UNITCONVERSION
#define UNITCONVERSION

#include <vector>
#include <algorithm>
#include <string>
#include <array>

#include "Error.h"

using PrefixPair = std::pair<std::string, double>;
const std::array<PrefixPair, 20> prefixes =
{ { 
	{ "Y", 1e24 },{ "Z", 1e21 },{ "E", 1e18 },{ "P", 1e15 },{ "T", 1e12 },{ "G", 1e9 },
	{ "M", 1e6 },{ "k", 1e3 },{ "h", 1e2 },{ "da", 1e1 },{ "d", 1e-1 },
	{ "c", 1e-2 },{ "m", 1e-3 },{ "u", 1e-6 },{ "n", 1e-9 },{ "p", 1e-12 },
	{ "f", 1e-15 },{ "a", 1e-18 },{ "z", 1e-21 },{ "y", 1e-24 }
} };

PrefixPair findPrefix(const std::string &from, std::string to)
{
	auto find = [&](const std::string &t) constexpr
	{
		for (const auto &prefix : prefixes)
			if (t.compare(0, prefix.first.size(), prefix.first) == 0)
				return prefix.second;
		Error("false prefix");
	};
	return PrefixPair(std::move(to), find(from) / find(to));
}

#endif // !UNITCONVERSION
