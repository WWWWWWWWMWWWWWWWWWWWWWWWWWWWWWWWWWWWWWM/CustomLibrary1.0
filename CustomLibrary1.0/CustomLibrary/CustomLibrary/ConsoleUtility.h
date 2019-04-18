#pragma once

#include <limits>
#include <iostream>
#include <string>

namespace ctl
{
	template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
	constexpr T numberInput()
	{
		T input;
		while (true)
		{
			std::cin >> input;
			if (std::cin.fail())
			{
				std::cin.clear();
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				std::cerr << "Invalid." << std::endl;
			}
			else
			{
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				return input;
			}
		}
	}

	template<typename Iter>
	const auto stringInput(const Iter &begin, const Iter &end)
	{
		using FilteredType = typename std::decay_t<decltype(**begin)>;
		using StringType = typename std::conditional_t<std::is_same_v<FilteredType, char> || std::is_same_v<FilteredType, wchar_t>, std::basic_string<FilteredType>, FilteredType>;
		StringType string;

		while (true)
			if constexpr (std::is_same<StringType, std::wstring>::value)
			{
				std::getline(std::wcin, string);
				if (string == L"help")
					for (auto b = begin; b != end; ++b)
						std::wcout << L"\"" << *b << L"\"\n";
				else if (std::find(begin, end, string) != end)
					return string;
				else
					std::wcerr << L"Invalid." << std::endl;
			}
			else
			{
				std::getline(std::cin, string);
				if (string == "help")
					for (auto b = begin; b != end; ++b)
						std::cout << "\"" << *b << "\"\n";
				else if (std::find(begin, end, string) != end)
					return string;
				else
					std::cerr << "Invalid." << std::endl;
			}

		return string;
	}
	template<typename StringType>
	const auto stringInput(const std::initializer_list<StringType> &list) { return stringInput(list.begin(), list.end()); }
}