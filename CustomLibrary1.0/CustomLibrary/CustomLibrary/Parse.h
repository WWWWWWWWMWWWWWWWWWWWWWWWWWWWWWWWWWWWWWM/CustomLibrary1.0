#pragma once

#include <iostream>
#include <vector>

namespace ctl
{
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
	auto& getlineWEnd(std::basic_istream<CharT, Traits>& input, std::basic_string<CharT, Traits, Allocator>& str, const std::initializer_list<CharT> &delim)
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