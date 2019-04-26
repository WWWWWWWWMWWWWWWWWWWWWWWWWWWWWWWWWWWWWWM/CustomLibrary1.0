#pragma once

#include <array>

#include <CustomLibrary/RandomGenerator.h>

namespace ctl
{
	class GenAlgo
	{
	public:
		double fitness = 0.;
	};

	template<typename Iter, typename Gen, typename Func, typename Type = typename std::iterator_traits<Iter>::value_type,
		typename = typename std::enable_if_t<!std::is_same_v<Type, void>>>
	auto select(const Iter &begin, const Iter &end, RandomGen<Gen> &rand, Func breed)
	{
		//Spawn new generation
		std::vector<Type> newGen;
		newGen.reserve(std::distance(begin, end));

		//Initilize score
		auto totalScore = 0.;
		for (auto iter = begin; iter != end; ++iter)
			totalScore += iter->fitness + .01; //avoid Null

		for (size_t i = 0; i < newGen.capacity(); ++i)
		{
			std::array<Iter, 2> parents;

			for (auto& parent : parents)
			{
				//Rolette style picking
				auto popIter = begin;
				for (double prog = 0., peak = rand.randNumber<double>(0., totalScore); true; ++popIter)
				{
					prog += popIter->fitness + .01;
					if (prog >= peak)
					{
						parent = popIter;
						break;
					}
				}
			}

			newGen.emplace_back(breed(*parents[0], *parents[1]));
		}

		return newGen;
	}
}