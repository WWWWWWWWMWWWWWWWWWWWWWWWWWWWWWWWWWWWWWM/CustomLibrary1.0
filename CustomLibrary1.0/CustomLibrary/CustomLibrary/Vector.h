#pragma once

#include <array>
#include <numeric>
#include "utility.h"

namespace ctl
{
	//Numeric vector for equations
	template<class TypeNum, size_t sizeArray>
	class NumVec
	{
	public:
		constexpr NumVec() = default;

		template<typename... S, typename = typename std::enable_if<and_<std::is_arithmetic<S>...>::value>::type>
		constexpr NumVec(S... ts) : m_m{ static_cast<TypeNum>(ts)... } {}

		template<typename S, typename = typename std::enable_if<std::is_arithmetic<S>::value>::type>
		constexpr NumVec(const ctl::NumVec<S, sizeArray> &e) : m_m(Array::cast<TypeNum, S, sizeArray>(e.data())) {}

		constexpr NumVec(const ctl::NumVec<TypeNum, sizeArray> &e) : m_m(e.m_m) {}
		constexpr void operator=(const ctl::NumVec<TypeNum, sizeArray> &e) { m_m = e.m_m; }

		constexpr NumVec(ctl::NumVec<TypeNum, sizeArray> &&e) : m_m(std::move(e.m_m)) {}
		constexpr void operator=(NumVec<TypeNum, sizeArray> &&e) { m_m = std::move(e.m_m); }

		constexpr operator std::array<TypeNum, sizeArray>() const { return m_m; }
		constexpr std::array<TypeNum, sizeArray>& data() { return m_m; }
		constexpr const std::array<TypeNum, sizeArray>& data() const { return m_m; }

#ifdef SDL_h_
		//WARNING: This method doesn't check if NumVec has 4 elements
		constexpr SDL_Rect to_SDL_Rect() const { return SDL_Rect{ m_m[0], m_m[1], m_m[2], m_m[3] }; }
		//WARNING: This method doesn't check if NumVec has 2 elements
		constexpr SDL_Point to_SDL_Point() const { return SDL_Point{ m_m[0], m_m[1] }; }
#endif // SDL_h_

		constexpr TypeNum& operator[](const int &x) { return m_m.at(x); }
		constexpr const TypeNum& operator[](const int &x) const { return m_m.at(x); }

		constexpr NumVec& operator+=(const NumVec<TypeNum, sizeArray> &v)
		{
			auto a = v.m_m.begin();
			for (auto b = m_m.begin(); a != v.m_m.end(); ++a, ++b)
				*b += *a;
			return *this;
		}

		constexpr bool operator==(const NumVec<TypeNum, sizeArray> &v) const { return m_m == v.m_m; }
		constexpr bool operator!=(const NumVec<TypeNum, sizeArray> &v) const { return m_m != v.m_m; }

		template<int y>
		constexpr NumVec<TypeNum, sizeArray + y> append(const NumVec<TypeNum, y> &v) const
		{
			NumVec<TypeNum, sizeArray + y> result;
			std::copy(m_m.begin(), m_m.end(), result.data().begin());
			std::copy(v.data().begin(), v.data().end(), result.data().begin() + sizeArray);
			return result;
		}

		constexpr auto sum() const { return std::accumulate(m_m.begin(), m_m.end(), 0); }
		constexpr auto product() const { return std::accumulate(m_m.begin(), m_m.end(), 1, std::multiplies<>()); }

	private:
		std::array<TypeNum, sizeArray> m_m;
	};
}