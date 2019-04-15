#pragma once

#include <chrono>

namespace ctl
{
	class Timer
	{
	public:
		void start()
		{
			m_started = true;
			m_paused = false;

			m_start = std::chrono::steady_clock::now();
		}

		void stop()
		{
			m_started = false;
			m_paused = false;
		}

		void pause()
		{
			if (m_started && !m_paused)
				m_paused = true,
				m_pause = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start);
		}

		void unpause()
		{
			if (m_started && m_paused)
				m_paused = false,
				m_start = std::chrono::steady_clock::now() - m_pause;
		}

		template<typename Unit = std::chrono::milliseconds>
		Unit getTicks()
		{
			if (m_started)
			{
				if (m_paused)
					return std::chrono::duration_cast<Unit>(m_pause);
				else
					return std::chrono::duration_cast<Unit>(std::chrono::steady_clock::now() - m_start);
			}
			else
				return Unit(0);
		}

		constexpr const bool& isStarted() const { return m_started; }
		constexpr const bool& isPaused() const { return m_paused; }

	private:
		std::chrono::time_point<std::chrono::steady_clock> m_start;
		std::chrono::milliseconds m_pause;

		bool m_paused = false;
		bool m_started = false;
	};
}