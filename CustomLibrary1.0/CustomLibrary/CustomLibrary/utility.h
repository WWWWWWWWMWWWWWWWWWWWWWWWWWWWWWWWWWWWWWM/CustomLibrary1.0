#pragma once

#include <iostream>
#include <type_traits>
#include <array>
#include <string_view>
#include <string>

namespace ctl
{
	//----------------------------------------Variadic condition------------------------------------------

	enum class enabler {};

	template <bool Condition>
	using EnableIf = typename std::enable_if<Condition, enabler>::type;

	template<typename... Conds>
	struct and_ : std::true_type {};

	template<typename Cond, typename... Conds>
	struct and_<Cond, Conds...> : std::conditional<Cond::value, and_<Conds...>, std::false_type>::type {};

	//---------------------------------------Constexpr Dereferenced Pointer----------------------------------------

	//Dereference ptr even if already dereferenced
	template<typename T>
	constexpr T& deref_ptr(T *ptr) { return *ptr; }
	//Dereference ptr even if already dereferenced
	template<typename T>
	constexpr T& deref_ptr(T &ptr) { return ptr; }

	//-------------------------------------Array Extension-------------------------------------

	class Array
	{
	public:
		//Constexpr array-wide cast
		template<typename T, typename U, size_t i>
		static constexpr std::array<T, i> cast(const std::array<U, i> &a) { return array_cast_helper<T>(a, build_indices<i>()); }

	private:
		Array(Array &&x) = delete;
		Array(const Array &x) = delete;

		template <std::size_t... Is>
		struct indices {};

		template <std::size_t N, std::size_t... Is>
		struct build_indices : build_indices<N - 1, N - 1, Is...> {};

		template <std::size_t... Is>
		struct build_indices<0, Is...> : indices<Is...> {};

		template<typename T, typename U, size_t i, size_t... Is>
		static constexpr std::array<T, i> array_cast_helper(const std::array<U, i> &a, indices<Is...>) { return { static_cast<T>(std::get<Is>(a))... }; }
	};

	//----------------------------------------Math Extensions---------------------------------------------

	constexpr double PI = 3.1415926535897932;

	template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
	constexpr T mapVal(const T &val, const T &oldMin, const T &oldMax, const T &newMin, const T &newMax) { return newMin + (newMax - newMin) * val / (oldMax - oldMin); }

	template<typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
	constexpr T to_degrees(const T &val) { return val / PI * 180; }
	template<typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
	constexpr T to_radians(const T &val) { return val / 180 * PI; }

	template<class CharT, class Traits, class Allocator>
	bool is_number(const std::basic_string<CharT, Traits, Allocator>& str) { return !str.empty() && str.find_first_not_of("0123456789.") == std::string::npos; }
	bool is_number(const char& ch) { constexpr std::string_view nums = "0123456789."; return std::find(nums.begin(), nums.end(), ch) != nums.end(); }

	//-----------------------------------------------------u8string----------------------------------------------------
	
	using char8_t = unsigned char;
	using u8string = std::basic_string<char8_t>;

	//-------------------------------------------------Parsing------------------------------------------------

	//std::getline, but with multiple delimiters
	template<class CharT, class Traits, class Allocator>
	auto& getline(std::basic_istream<CharT, Traits>& input, std::basic_string<CharT, Traits, Allocator>& str, const std::initializer_list<CharT> delim)
	{
		str.erase();
		for (CharT ch; input.get(ch);)
			if (std::any_of(delim.begin(), delim.end(), ch))
				return input;
			else if (str.size() == str.max_size())
			{
				input.setstate(std::ios_base::failbit);
				return input;
			}
			else
				str.push_back(ch);

		return input;
	}

	//ctl::getline, but string includes ending
	template<class CharT, class Traits, class Allocator>
	auto& getlineWEnd(std::basic_istream<CharT, Traits>& input, std::basic_string<CharT, Traits, Allocator>& str, const std::initializer_list<CharT> delim)
	{
		str.erase();
		for (char ch; input.get(ch);)
		{
			str.push_back(ch);
			if (std::any_of(delim.begin(), delim.end(), [&](const CharT &c) constexpr { return c == ch; }))
				break;
		}
		return input;
	};

	////Get string from getline
	//template<class CharT, class Traits, class Allocator, typename P>
	//std::string wordFGetline(std::basic_istream<CharT, Traits>& fin, 
	//	std::basic_istream<CharT, Traits>(*func)(std::basic_istream<CharT, Traits>&, std::basic_string<CharT, Traits, Allocator>&, const P),
	//	const P &arg)
	//{
	//	std::string temp;
	//	func(fin, temp, arg);
	//	return temp;
	//}

	//Gets strings inbetween delimiters
	template<class CharT, class Traits, class Allocator>
	auto parseString(const std::basic_string<CharT, Traits, Allocator>& s, const CharT &delim)
	{
		std::vector<std::basic_string<CharT, Traits, Allocator>> temp;

		size_t newPos = 0, oldPos = 0;
		oldPos = std::exchange(newPos, s.find(delim, newPos + 1));
		temp.emplace_back(s.substr(oldPos, newPos - oldPos));

		while (newPos != std::basic_string<CharT, Traits, Allocator>::npos)
		{
			oldPos = std::exchange(newPos, s.find(delim, newPos + 1));
			temp.emplace_back(s.substr(oldPos + 1, newPos - oldPos - 1));
		}

		temp.shrink_to_fit();
		return temp;
	};

	//---------------------------------------------Formating----------------------------------------------

	template<typename InputIt>
	std::string combine(InputIt first, const InputIt &last, const char &ch)
	{
		std::string temp;
		temp.append(*first++);
		for (; first != last; ++first)
			temp.append(ch + *first);

		return temp;
	}
}