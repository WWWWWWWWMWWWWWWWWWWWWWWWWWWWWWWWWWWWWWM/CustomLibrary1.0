#pragma once

#include <tuple>

#include <CustomLibrary/CustomSDL/C_Engine.h>
#include <CustomLibrary/utility.h>

namespace ctl
{
	template<typename ...T>
	const bool mouseInside(const std::tuple<T...> &dim)
	{
		ctl::Vector<int, 2> mouse;
		SDL_GetMouseState(&mouse[0], &mouse[1]);

		if (mouse[0] < *std::get<0>(dim))
			return false;
		else if (mouse[0] > *std::get<0>(dim) + *std::get<2>(dim))
			return false;
		else if (mouse[1] < *std::get<1>(dim))
			return false;
		else if (mouse[1] > *std::get<1>(dim) + *std::get<3>(dim))
			return false;

		return true;
	}
	
	template<typename ...T>
	class MouseCol
	{
	public:
		constexpr MouseCol() = default;

		template<typename = typename std::enable_if<and_<std::is_pointer<T>...>::value>::type>
		constexpr MouseCol(std::tuple<T...> &&v)
			: m_dim(std::move(v))
		{}
		template<typename = typename std::enable_if<and_<std::is_pointer<T>...>::value>::type>
		constexpr void set(std::tuple<T...> &&v) { m_dim = std::move(v); }

		constexpr bool inside() const { return mouseInside(m_dim); }

	private:
		std::tuple<T...> m_dim;
	};
}