#pragma once

#include <CustomLibrary/CustomSDL/C_Engine.h>

template<size_t size>
class Morph
{
public:
	Morph(const ctl::Window::Ref &w, const ctl::Vector<int, size> &val, ctl::Vector<int, 2> *screen = nullptr)
		: m_window(w)
		, m_valOld(val)
	{
		if (screen != nullptr)
			m_screenOld = *screen;
		else
			m_screenOld = m_window->dim();
	}

	void morph()
	{
		for (size_t i = 0; i < size; ++i)
			*m_val[i] = { ctl::mapVal(*m_val[i], 0, m_screenOld[i & 1], 0, m_window->dim()[i & 1]) };
	}

	constexpr const ctl::Vector<int, size>& val() const { return m_val; }

	constexpr const ctl::Vector<int, size>& source() const { return m_valOld; }
	constexpr ctl::Vector<int, size>& source() { return m_valOld; }

private:
	ctl::Window::Ref m_window;

	ctl::Vector<int, size> m_val;

	ctl::Vector<int, size> m_valOld;
	ctl::Vector<int, 2> m_screenOld;
};