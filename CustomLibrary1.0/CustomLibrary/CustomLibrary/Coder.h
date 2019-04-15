#pragma once

#include <sstream>
#include <functional>

namespace ctl
{
	//s -> to encrypt, i -> where to start
	template<typename CharT, typename Traits, typename Allocator>
	auto encode(std::basic_stringstream<CharT, Traits, Allocator> &s, long long i = 0)
	{
		std::basic_stringstream<CharT, Traits, Allocator> temp;

		for (char iter; s.get(iter); ++i)
			temp << (iter + 3) * (iter + i) << ' ';

		return temp;
	}
	//s -> to encrypt, i -> where to start
	template<typename CharT, typename Traits, typename Allocator>
	auto encode(const std::basic_string<CharT, Traits, Allocator> &s, long long i = 0)
	{
		std::basic_string<CharT, Traits, Allocator> temp;

		for (auto iter = s.begin(), end = s.end(); iter != end; ++iter, ++i)
			temp += std::to_string((*iter + 3) * (*iter + i)) + ' ';

		return temp;
	}
	//s -> to decrypt, i -> where to start
	template<typename CharT, typename Traits, typename Allocator>
	auto decode(std::basic_stringstream<CharT, Traits, Allocator> &s, long long i = 0)
	{
		std::basic_stringstream<CharT, Traits, Allocator> temp;

		for (std::basic_stringstream<CharT, Traits, Allocator> iter; std::getline(s, iter, ' '); ++i)
			temp << static_cast<char>(-(i + 3) / 2 + std::sqrt(((i + 3) / 2) * ((i + 3) / 2) - (i * 3 - std::stoll(iter))));

		return temp;
	}

	//s -> to hash
	auto lock(std::stringstream &s)
	{
		return std::to_string(std::hash<std::string>()(s.str())) + '\n' + s.str();
	}
	//Check if hashed string is correct, s -> to check
	auto unlock(std::stringstream &s)
	{
		std::string temp = s.str();

		auto loc = temp.find_first_of('\n');
		std::string hashCode = temp.substr(0, loc);
		temp.erase(0, loc + 1);

		//Put cleared contents back
		s.str(temp);

		return std::hash<std::string>()(temp) == std::stoull(hashCode);
	}
}