#ifndef CUSTOMINPUT
#define CUSTOMINPUT

#include <limits>
#include <iostream>
#include <string>
#include <type_traits>

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

	template<typename CharT, typename Traits, typename Allocator>
	const std::basic_string<CharT, Traits, Allocator> stringInput(const std::initializer_list<std::basic_string<CharT, Traits, Allocator>> &list)
	{
		std::basic_string<CharT, Traits, Allocator> type;

		while (true)
			if constexpr (std::is_same<std::basic_string<CharT, Traits, Allocator>, std::wstring>::value)
			{
				std::getline(std::wcin, type);
				if (type == L"help")
					for (const auto &i : list)
						std::wcout << L"\"" << i << L"\"\n";
				else if (std::find(list.begin(), list.end(), type) != list.end())
					return type;
				else
					std::wcerr << L"Invalid." << std::endl;
			}
			else constexpr
			{
				std::getline(std::cin, type);
				if (type == "help")
					for (const auto &i : list)
						std::cout << "\"" << i << "\"\n";
				else if (std::find(list.begin(), list.end(), type) != list.end())
					return type;
				else
					std::cerr << "Invalid." << std::endl;
			}

		return type;
	}
}

#endif // !CUSTOMINPUT