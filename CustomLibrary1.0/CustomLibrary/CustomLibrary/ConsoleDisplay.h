#pragma once

#include <Windows.h>

#include "Vector.h"
#include "Error.h"

namespace ctl
{
	class ConsoleDisplay
	{
	public:
		enum Colour
		{
			FG_BLACK = 0x0000, FG_DARK_BLUE = 0x0001, FG_DARK_GREEN = 0x0002, FG_DARK_CYAN = 0x0003,
			FG_DARK_RED = 0x0004, FG_DARK_MAGENTA = 0x0005, FG_DARK_YELLOW = 0x0006, FG_GREY = 0x0007,
			FG_DARK_GREY = 0x0008, FG_BLUE = 0x0009, FG_GREEN = 0x000A, FG_CYAN = 0x000B,
			FG_RED = 0x000C, FG_MAGENTA = 0x000D, FG_YELLOW = 0x000E, FG_WHITE = 0x000F,

			BG_BLACK = 0x0000, BG_DARK_BLUE = 0x0010, BG_DARK_GREEN = 0x0020, BG_DARK_CYAN = 0x0030,
			BG_DARK_RED = 0x0040, BG_DARK_MAGENTA = 0x0050, BG_DARK_YELLOW = 0x0060, BG_GREY = 0x0070,
			BG_DARK_GREY = 0x0080, BG_BLUE = 0x0090, BG_GREEN = 0x00A0, BG_CYAN = 0x00B0,
			BG_RED = 0x00C0, BG_MAGENTA = 0x00D0, BG_YELLOW = 0x00E0, BG_WHITE = 0x00F0
		};

		//---------------------------------------------------------------------
		//---------------------Constructors & Destructors----------------------
		//---------------------------------------------------------------------

		ConsoleDisplay(const ctl::NumVec<short, 2> &consoleSize, const ctl::NumVec<short, 2> &fontSize, const std::wstring &fontName = L"Lucida Console")
			: m_screenSize(consoleSize)
			, m_fontSize(fontSize)
			, m_buff(consoleSize.product())
		{
			m_rectWindow = { 0, 0, 1, 1 };
			if (SetConsoleWindowInfo(m_hConsoleOut, TRUE, &m_rectWindow) == 0)
				throw Log("ConsoleDisplay: " + GetLastError(), ctl::Log::Severity::ERR0R);

			COORD coord = { m_screenSize[0], m_screenSize[1] };
			if (SetConsoleScreenBufferSize(m_hConsoleOut, coord) == 0)
				throw Log("ConsoleDisplay: " + GetLastError(), ctl::Log::Severity::ERR0R);

			if (SetConsoleActiveScreenBuffer(m_hConsoleOut) == 0)
				throw Log("ConsoleDisplay: " + GetLastError(), ctl::Log::Severity::ERR0R);

			CONSOLE_FONT_INFOEX cfi =
			{
				cfi.cbSize = sizeof(cfi),
				cfi.nFont = 0,
				cfi.dwFontSize.X = m_fontSize[0],
				cfi.dwFontSize.Y = m_fontSize[1],
				cfi.FontFamily = FF_DONTCARE,
				cfi.FontWeight = FW_NORMAL
			};
			wcscpy_s(cfi.FaceName, fontName.data());
			if (SetCurrentConsoleFontEx(m_hConsoleOut, false, &cfi) == 0)
				throw Log("ConsoleDisplay: " + GetLastError(), ctl::Log::Severity::ERR0R);

			m_rectWindow = { 0, 0, m_screenSize[0] - 1, m_screenSize[1] - 1 };
			if (SetConsoleWindowInfo(m_hConsoleOut, TRUE, &m_rectWindow) == 0)
				throw Log("ConsoleDisplay: " + GetLastError(), ctl::Log::Severity::ERR0R);

			if (SetConsoleMode(m_hConsoleIn, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT) == 0)
				throw Log("ConsoleDisplay: " + GetLastError(), ctl::Log::Severity::ERR0R);

			////Check screen size
			//CONSOLE_SCREEN_BUFFER_INFO csbi;
			//if (!GetConsoleScreenBufferInfo(m_hConsoleIn, &csbi))
			//	throw Log("ConsoleDisplay: " + GetLastError(), ctl::Log::Severity::ERR0R);
			//if (m_screenSize[0] > csbi.dwMaximumWindowSize.X)
			//	throw Log("ConsoleDisplay: console width too large.", ctl::Log::Severity::ERR0R);
			//if (m_screenSize[1] > csbi.dwMaximumWindowSize.Y)
			//	throw Log("ConsoleDisplay: console width too large.", ctl::Log::Severity::ERR0R);
		}

		~ConsoleDisplay()
		{
			FreeConsole();
			AllocConsole();

			freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
			freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
			
			std::cout.clear();
			std::cin.clear();
		}

		//---------------------------------------------------------------------
		//----------------------------Getters----------------------------------
		//---------------------------------------------------------------------

		constexpr const auto& dim() { return m_screenSize; }
		constexpr const auto& fontDim() { return m_fontSize; }

	private:
		constexpr void _locCheck_(const size_t &xy) const
		{
			if (xy > static_cast<size_t>(m_screenSize.product()) || xy < 0)
				throw Log("ConsoleDisplay: loc: out of range.", ctl::Log::Severity::ERR0R);
		}
	public:
		auto& loc(const size_t &xy) { _locCheck_(xy); return m_buff[xy]; }
		const auto& loc(const size_t &xy) const { _locCheck_(xy); return m_buff[xy]; }

		auto& loc(const ctl::NumVec<size_t, 2> &xy) { return loc(xy[0] + m_screenSize[0] * xy[1]); }
		const auto& loc(const ctl::NumVec<size_t, 2> &xy) const { return loc(xy[0] + m_screenSize[0] * xy[1]); }

		//---------------------------------------------------------------------
		//----------------------------Methods----------------------------------
		//---------------------------------------------------------------------

		auto& outputBuff()
		{
			if (WriteConsoleOutput(m_hConsoleOut, m_buff.data(), { m_screenSize[0], m_screenSize[1] }, { 0, 0 }, &m_rectWindow) == 0)
				throw Log("ConsoleDisplay: outputBuff: " + GetLastError(), ctl::Log::Severity::ERR0R);
			
			return *this;
		}

		auto& clear()
		{
			FillConsoleOutputCharacter(m_hConsoleOut, L' ', m_screenSize.product(), { 0, 0 }, nullptr);
			FillConsoleOutputAttribute(m_hConsoleOut, 0, m_screenSize.product(), { 0, 0 }, nullptr);

			return *this;
		}

		auto emptyCells() const
		{
			return std::count_if(m_buff.begin(), m_buff.end(), [](const CHAR_INFO &x) { return x.Char.AsciiChar == 0 && x.Attributes == 0; });
		}

		auto& write(const size_t &xy, const std::wstring &s, const Colour &col = FG_WHITE)
		{
			for (size_t i = 0; i < s.size(); ++i)
			{
				const auto newLoc = xy + i;
				loc(newLoc).Attributes = col;
				loc(newLoc).Char.UnicodeChar = s[i];
			}

			return *this;
		}
		auto& write(const ctl::NumVec<size_t, 2> &xy, const std::wstring &s, const Colour &col = FG_WHITE) { return write(xy[0] + m_screenSize[0] * xy[1], s, col); }

	private:
		ctl::NumVec<short, 2> m_screenSize;
		ctl::NumVec<short, 2> m_fontSize;

		std::vector<CHAR_INFO> m_buff;
		SMALL_RECT m_rectWindow;

		HANDLE m_hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
		HANDLE m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
	};
}