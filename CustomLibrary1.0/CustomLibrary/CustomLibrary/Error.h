#ifndef MYERROR
#define MYERROR

#include <exception>
#include <string>
#include <fstream>
#include <queue>
#include <iostream>

namespace ctl
{
#ifdef LOGGING
	constexpr bool LOG = true;
#else
	constexpr bool LOG = false;
#endif // LOGGING

	class Log : public std::exception
	{
	public:
		enum class Severity { NOTE, WARNING, ERR0R };
		//Constructor logs automaticly
		Log(const std::string &msg, const Severity &typ = Severity::NOTE)
		{
			m_buf.emplace(msg, typ);
			std::clog << _typToString_(m_buf.back().second) << m_buf.back().first << '\n';
			if (m_buf.size() > 50) //50 is the message limit
				m_buf.pop();
		}

		//Output everything to file
		static void fstreamFlush(const std::string &fileName)
		{
			std::ofstream errorFile("Logs-" + fileName + ".txt", std::ios::out);
			while (!m_buf.empty())
			{
				errorFile << _typToString_(m_buf.front().second) << m_buf.front().first << '\n';
				m_buf.pop();
			}
		}

		const char* what() const noexcept { return (_typToString_(m_buf.back().second) + m_buf.back().first).c_str(); }

	private:
		static std::string _typToString_(const Severity &typ) noexcept
		{
			switch (typ)
			{
			case Log::Severity::NOTE:	 return "NOTE: ";
			case Log::Severity::ERR0R:	 return "ERROR: ";
			case Log::Severity::WARNING: return "WARNING: ";
			default:					 return "UNKNOWN: ";
			}
		}

		//Queue for logs and errors
		static std::queue<std::pair<std::string, Severity>> m_buf;
	};

	std::queue<std::pair<std::string, Log::Severity>> Log::m_buf;
}

#endif // !MYERROR