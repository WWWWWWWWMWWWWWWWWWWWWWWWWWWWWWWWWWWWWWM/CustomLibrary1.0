#ifndef DISPLAY
#define DISPLAY

#include <Windows.h>
#include <string>
#include <vector>
#include <wchar.h>

//Point Object
struct CellCoords
{
	short m_x;
	short m_y;

	bool operator==(const CellCoords &x) const { return x.m_x == m_x && x.m_y == m_y ? true : false; }

	CellCoords() {}

	CellCoords(const CellCoords &xy)
		: m_x{ xy.m_x }, m_y{ xy.m_y } {}

	CellCoords(CellCoords &&xy)
		: m_x{ std::move(xy.m_x) }, m_y{ std::move(xy.m_y) } {}

	CellCoords(const short &x, const short &y)
		: m_y{ y }, m_x{ x } {}

	CellCoords& operator=(const CellCoords &xy) { m_x = xy.m_x; m_y = xy.m_y; return *this; }
};

//Colours
enum Colour
{
	FG_BLACK = 0x0000,
	FG_DARK_BLUE = 0x0001,
	FG_DARK_GREEN = 0x0002,
	FG_DARK_CYAN = 0x0003,
	FG_DARK_RED = 0x0004,
	FG_DARK_MAGENTA = 0x0005,
	FG_DARK_YELLOW = 0x0006,
	FG_GREY = 0x0007,
	FG_DARK_GREY = 0x0008,
	FG_BLUE = 0x0009,
	FG_GREEN = 0x000A,
	FG_CYAN = 0x000B,
	FG_RED = 0x000C,
	FG_MAGENTA = 0x000D,
	FG_YELLOW = 0x000E,
	FG_WHITE = 0x000F,
	BG_BLACK = 0x0000,
	BG_DARK_BLUE = 0x0010,
	BG_DARK_GREEN = 0x0020,
	BG_DARK_CYAN = 0x0030,
	BG_DARK_RED = 0x0040,
	BG_DARK_MAGENTA = 0x0050,
	BG_DARK_YELLOW = 0x0060,
	BG_GREY = 0x0070,
	BG_DARK_GREY = 0x0080,
	BG_BLUE = 0x0090,
	BG_GREEN = 0x00A0,
	BG_CYAN = 0x00B0,
	BG_RED = 0x00C0,
	BG_MAGENTA = 0x00D0,
	BG_YELLOW = 0x00E0,
	BG_WHITE = 0x00F0
};

//Screen Object
class Display
{
private:

	const int m_xScreen;
	const int m_yScreen;
	const int m_sizeScreen = m_xScreen * m_yScreen;

	CHAR_INFO *m_screen;
	SMALL_RECT m_rectWindow;

	HANDLE m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);

	HANDLE m_hOriginalConsole;

public:

	//----------------------------------Constructor & Destructor----------------------------------

	Display(const char &xScreen = 30, const char &yScreen = 30, const char &fontW = 12, const char &fontH = 12)
		: m_xScreen{ xScreen }, m_yScreen{ yScreen }
	{
		m_rectWindow = { 0, 0, 1, 1 };
		SetConsoleWindowInfo(m_hConsole, TRUE, &m_rectWindow);

		COORD coord = { (short)m_xScreen, (short)m_yScreen };
		SetConsoleScreenBufferSize(m_hConsole, coord);

		SetConsoleActiveScreenBuffer(m_hConsole);

		CONSOLE_FONT_INFOEX cfi;
		cfi.cbSize = sizeof(cfi);
		cfi.nFont = 0;
		cfi.dwFontSize.X = fontW;
		cfi.dwFontSize.Y = fontH;
		cfi.FontFamily = FF_DONTCARE;
		cfi.FontWeight = FW_NORMAL;

		wcscpy_s(cfi.FaceName, L"Lucida Console");

		SetCurrentConsoleFontEx(m_hConsole, false, &cfi);

		m_rectWindow = { 0, 0, (short)m_xScreen - 1, (short)m_yScreen - 1 };
		SetConsoleWindowInfo(m_hConsole, TRUE, &m_rectWindow);

		SetConsoleMode(m_hConsoleIn, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
		m_screen = new CHAR_INFO[m_sizeScreen];
		std::memset(m_screen, 0, sizeof(CHAR_INFO) * m_sizeScreen);
	}

	~Display()
	{
		SetConsoleActiveScreenBuffer(m_hOriginalConsole);
		delete[] m_screen;
	}

	//---------------------------------------Getter & Setter--------------------------------------

	const int getFreeSpace()
	{
		int m_freeSpace = 0;
		for (int i = 0; i < m_sizeScreen; i++)
			m_freeSpace += m_screen[i].Char.UnicodeChar != 0 ? 0 : 1;
		return m_freeSpace;
	}
	const short &get_xScreen() const { return m_xScreen; }
	const int& getSizeScreen() const { return m_sizeScreen; }

	//-------------------------------------------Methods------------------------------------------

	//Display Buffer
	Display& bufferOutput()
	{
		WriteConsoleOutput(m_hConsole, m_screen, { (short)m_xScreen, (short)m_yScreen }, { 0, 0 }, &m_rectWindow);
		return *this;
	}

	//Change Screen Buffer
	Display& setConsoleBuffer() { SetConsoleActiveScreenBuffer(m_hConsole); return *this; }

	//Access Location
	wchar_t& accessLoc(const int &x, const int &y, const Colour &col = FG_WHITE) const
	{
		m_screen[y * m_xScreen + x].Attributes = col;
		return m_screen[y * m_xScreen + x].Char.UnicodeChar;
	}
	wchar_t& accessLoc(const int &xy, const Colour &col = FG_WHITE) const
	{
		m_screen[xy].Attributes = col;
		return m_screen[xy].Char.UnicodeChar;
	}
	wchar_t& accessLoc(const CellCoords &xy, const Colour &col = FG_WHITE) const
	{
		m_screen[xy.m_y * m_xScreen + xy.m_x].Attributes = col;
		return m_screen[xy.m_y * m_xScreen + xy.m_x].Char.UnicodeChar;
	}

	//Check Location
	const bool checkLoc(const short &x, const short &y, const std::vector<wchar_t> &character) const
	{
		for (const auto &val : character)
			if (m_screen[y * m_xScreen + x].Char.UnicodeChar == val)
				return true;
		return false;
	}
	const bool checkLoc(const CellCoords &xy, const std::vector<wchar_t> &character) const
	{
		for (const auto &val : character)
			if (m_screen[xy.m_y * m_xScreen + xy.m_x].Char.UnicodeChar == val)
				return true;
		return false;
	}
	const bool checkLoc(const int &xy, const std::vector<wchar_t> &character) const
	{
		for (const auto &val : character)
			if (m_screen[xy].Char.UnicodeChar == val)
				return true;
		return false;
	}

	//Check Location inc. Colour
	const bool checkLoc(const short &x, const short &y, const std::vector<wchar_t> &character, const Colour &col) const
	{
		for (const auto &val : character)
			if (m_screen[y * m_xScreen + x].Char.UnicodeChar == val && m_screen[y * m_xScreen + x].Attributes == col)
				return true;
		return false;
	}
	const bool checkLoc(const CellCoords &xy, const std::vector<wchar_t> &character, const Colour &col) const
	{
		for (const auto &val : character)
			if (m_screen[xy.m_y * m_xScreen + xy.m_x].Char.UnicodeChar == val && m_screen[xy.m_y * m_xScreen + xy.m_x].Attributes == col)
				return true;
		return false;
	}
	const bool checkLoc(const int &xy, const std::vector<wchar_t> &character, const Colour &col) const
	{
		for (const auto &val : character)
			if (m_screen[xy].Char.UnicodeChar == val && m_screen[xy].Attributes == col)
				return true;
		return false;
	}

	//Write String into Location (Needs multiple spaces)
	Display& writeInLoc(const CellCoords &xy, const std::wstring &words, const Colour &col = FG_WHITE)
	{
		for (short i = 0; i < words.size(); i++)
		{
			m_screen[xy.m_y * m_xScreen + xy.m_x + i].Attributes = col;
			m_screen[xy.m_y * m_xScreen + xy.m_x + i].Char.UnicodeChar = words[i];
		}
		return *this;
	}
	Display& writeInLoc(const short &x, const short &y, const std::wstring &words, const Colour &col = FG_WHITE)
	{
		for (short i = 0; i < words.size(); i++)
		{
			m_screen[y * m_xScreen + x + i].Attributes = col;
			m_screen[y * m_xScreen + x + i].Char.UnicodeChar = words[i];
		}
		return *this;
	}
	Display& writeInLoc(const int &xy, const std::wstring &words, const Colour &col = FG_WHITE)
	{
		for (short i = 0; i < words.size(); i++)
		{
			m_screen[xy].Attributes = col;
			m_screen[xy + i].Char.UnicodeChar = words[i];
		}
		return *this;
	}

	//Clear Screen
	Display& refresh(const wchar_t &character = 0)
	{
		for (int i = 0; i < m_sizeScreen; i++)
		{
			m_screen[i].Attributes = FG_BLACK;
			m_screen[i].Char.UnicodeChar = character;
		}
		return *this;
	}

	//Kill Console
	Display& destroy()
	{
		SetConsoleActiveScreenBuffer(m_hOriginalConsole);
		delete[] m_screen;
		m_screen = nullptr;
		return *this;
	}

};

#endif //DISPLAY2