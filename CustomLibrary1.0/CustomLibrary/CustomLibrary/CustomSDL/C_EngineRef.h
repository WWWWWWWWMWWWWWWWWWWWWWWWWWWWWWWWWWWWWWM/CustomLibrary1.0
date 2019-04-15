#pragma once

#include "C_Engine.h"

namespace ctl
{
	/*class EngineRef
	{
	public:
		constexpr EngineRef() = default;
		constexpr EngineRef(ctl::Window *engine) : m_engine(engine) {}
		constexpr EngineRef(EngineRef &&x) = default;
		constexpr EngineRef(const EngineRef &x) = default;
		constexpr EngineRef& operator=(EngineRef &&x) = default;
		constexpr EngineRef& operator=(const EngineRef &x) = default;

		constexpr EngineRef& set(ctl::Window *engine) { m_engine = engine; return *this; }

		operator ctl::Window*() { return m_engine; }
		constexpr ctl::Window* operator->() const { return m_engine; }

	private:
		ctl::Window *m_engine = nullptr;
	};
*/
	using WindowRef = ctl::Window*;
}