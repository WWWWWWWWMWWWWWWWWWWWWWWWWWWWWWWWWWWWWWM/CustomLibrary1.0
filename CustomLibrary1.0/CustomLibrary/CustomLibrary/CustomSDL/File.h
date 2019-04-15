#pragma once

#include "Engine.h"
#include <vector>

class File
{
public:
	void open(const std::string &name, const std::string &mode)
	{
		m_mode = mode;
		m_name = name;
		if (m_file != nullptr)
			close();

		m_file = SDL_RWFromFile(m_name.c_str(), m_mode.c_str());
	}

	bool exits()
	{
		return m_file != nullptr;
	}

	template<typename T>
	void save(const std::vector<T> &dat)
	{
		for (const auto &val : dat)
			SDL_RWwrite(m_file, &val, sizeof(T), 1);
	}

	template<typename T>
	std::vector<T> read(const unsigned int &reserve)
	{
		std::vector<T> buf(reserve);

		//Load data
		for (auto &val : buf)
			SDL_RWread(m_file, &val, sizeof(T), 1);

		return buf;
	}

	void close()
	{
		SDL_RWclose(m_file);
	}

	~File()
	{
		close();
	}

private:
	SDL_RWops *m_file = nullptr;
	std::string m_mode;
	std::string m_name;
};