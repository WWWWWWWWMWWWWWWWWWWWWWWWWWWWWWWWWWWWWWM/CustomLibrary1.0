#ifndef UNITCONVERSION
#define UNITCONVERSION

#include <vector>
#include <algorithm>
#include <string>
#include <array>

#include "Error.h"

namespace ctl
{
	enum PrefixIndex
	{
		YOCTO,
		ZEPTO,
		ATTO,
		FEMTO,
		PICO,
		NANO,
		MICRO,
		MILLI,
		CENTI,
		DECI,

		BASE,

		DECA,
		HECTO,
		KILO,
		MEGA,
		GIGA,
		TERA,
		PETA,
		EXA,
		ZETTA,
		YOTTA
	};

	constexpr std::array<double, 21> prefixes =
	{
		1e-24, 1e-21, 1e-18, 1e-15, 1e-12, 1e-9, 1e-6, 
		1e-3,  1e-2,  1e-1,  1.,	1e1,   1e2,  1e3,   
		1e6,   1e9,   1e12,  1e15,  1e18,  1e21, 1e24,
	};

	template<typename Type>
	Type convert(const Type &num, const PrefixIndex &from, const PrefixIndex &to)
	{
		return num * prefixes[from] / prefixes[to];
	}
}
#endif // !UNITCONVERSION