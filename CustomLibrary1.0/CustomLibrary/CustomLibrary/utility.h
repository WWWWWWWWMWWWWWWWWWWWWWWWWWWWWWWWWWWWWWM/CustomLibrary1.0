#pragma once

#include <iostream>
#include <type_traits>
#include <array>
#include <string_view>
#include <string>

namespace ctl
{
	//---------------------------------------Constexpr Dereferenced Pointer----------------------------------------

	//Dereference ptr even if already dereferenced
	template<typename T>
	constexpr T& deref_ptr(T *ptr) { return *ptr; }
	//Dereference ptr even if already dereferenced
	template<typename T>
	constexpr T& deref_ptr(T &ptr) { return ptr; }

	//-------------------------------------Array Extension-------------------------------------

	template <std::size_t... Is>
	struct _indices_ {};

	template <std::size_t N, std::size_t... Is>
	struct _build_indices_ : _build_indices_<N - 1, N - 1, Is...> {};

	template <std::size_t... Is>
	struct _build_indices_<0, Is...> : _indices_<Is...> {};

	template<typename T, typename U, size_t i, size_t... Is>
	static constexpr std::array<T, i> _array_cast_helper_(const std::array<U, i> &a, _indices_<Is...>) { return { static_cast<T>(std::get<Is>(a))... }; }

	//Constexpr array-wide cast
	template<typename T, typename U, size_t i>
	static constexpr std::array<T, i> arrayCast(const std::array<U, i> &a) { return _array_cast_helper_<T>(a, _build_indices_<i>()); }

	//----------------------------------------Math Extensions---------------------------------------------

	constexpr double PI = 3.1415926535897932;

	template<typename T, typename = typename std::enable_if_t<std::is_arithmetic_v<T>>>
	constexpr T mapVal(const T &val, const T &oldMin, const T &oldMax, const T &newMin, const T &newMax) { return newMin + (newMax - newMin) * val / (oldMax - oldMin); }

	template<typename T, typename = typename std::enable_if_t<std::is_floating_point_v<T>>>
	constexpr T to_degrees(const T &val) { return val / PI * 180; }
	template<typename T, typename = typename std::enable_if_t<std::is_floating_point_v<T>>>
	constexpr T to_radians(const T &val) { return val / 180 * PI; }

	template<class CharT, class Traits, class Allocator>
	bool is_number(const std::basic_string<CharT, Traits, Allocator>& str) { return !str.empty() && str.find_first_not_of("0123456789.") == std::string::npos; }
	bool is_number(const char& ch) { constexpr std::string_view nums = "0123456789."; return std::find(nums.begin(), nums.end(), ch) != nums.end(); }

	double& sigmoid(double &x) { x = 1 / (1 + std::exp(-x)); return x; }

	//-----------------------------------------------------u8string----------------------------------------------------
	
	using char8_t = unsigned char;
	using u8string = std::basic_string<char8_t>;
}