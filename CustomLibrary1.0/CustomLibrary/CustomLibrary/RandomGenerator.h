#ifndef RANDOMGEN
#define RANDOMGEN

#include <random>
#include <type_traits>

#include "Error.h"

namespace ctl
{
	//Seperated
	namespace Gen
	{
		using Mersenne = std::mt19937;
		using Linear = std::minstd_rand;
		using SubWCar = std::ranlux24_base;
	}

	//Gen::
	template<class G, typename = typename std::enable_if<std::is_same<Gen::Mersenne, G>::value || std::is_same<Gen::Linear, G>::value || std::is_same<Gen::SubWCar, G>::value>::type>
	class RandomGen
	{
	public:
		RandomGen() : m_gen{ rd() } {}

		template<typename Type, typename = typename std::enable_if<std::is_arithmetic<Type>::value, Type>::type>
		constexpr Type randNumber(const Type &min, const Type &max)
		{
			if (min >= max)
				throw ctl::Log("RandomGenerator: min is larger or equal to max.", ctl::Log::Severity::ERR0R);

			if constexpr (std::is_floating_point<Type>::value)
				return std::uniform_real_distribution<>(min, max)(m_gen);
			else
				return std::uniform_int_distribution<>(min, max)(m_gen);
		}

		template<typename Iter, typename = typename std::enable_if_t<!std::is_same_v<typename std::iterator_traits<Iter>::type_value, void>>>
		constexpr Iter randIter(Iter first, const Iter &last)
		{
			std::advance(first, randNumber<size_t>(0, std::distance(first, last) - 1));
			return first;
		}
	private:
		std::random_device rd;
		G m_gen;
	};
}
#endif // !RANDOMGEN