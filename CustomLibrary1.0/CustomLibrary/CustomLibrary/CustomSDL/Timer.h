#pragma once

#include <SDL.h>

//Get TICK time
class C_Timer
{
public:
	//Get time since start
	void start()
	{
		//Start the timer
		m_started = true;
		//Unpause the timer
		m_paused = false;

		//Get the current clock time
		m_startTicks = SDL_GetTicks();
		m_pausedTicks = 0;
	}

	void stop()
	{
		//Stop the timer
		m_started = false;
		//Unpause the timer
		m_paused = false;

		//Clear tick variables
		m_startTicks = 0;
		m_pausedTicks = 0;
	}

	void pause()
	{
		if (m_started && !m_paused)
		{
			//Pause the timer
			m_paused = true;

			//Calculate the paused ticks
			m_pausedTicks = SDL_GetTicks() - m_startTicks;
			m_startTicks = 0;
		}
	}

	void unpause()
	{
		//If the timer is running and paused
		if (m_started && m_paused)
		{
			//Unpause the timer
			m_paused = false;

			//Reset the starting ticks
			m_startTicks = SDL_GetTicks() - m_pausedTicks;

			//Reset the paused ticks
			m_pausedTicks = 0;
		}
	}

	//Get time
	Uint32 getTicks()
	{
		//The actual timer time
		Uint32 time = 0;

		//If the timer is running
		if (m_started)
			//If the timer is paused
			if (m_paused)
				//Return the number of ticks when the timer was paused
				time = m_pausedTicks;
			else
				//Return the current time minus the start time
				time = SDL_GetTicks() - m_startTicks;

		return time;
	}

	constexpr const bool& isStarted() const { return m_started; }
	constexpr const bool& isPaused() const { return m_paused; }

private:
	//The clock time when the timer started
	Uint32 m_startTicks = 0;
	//The ticks stored when the timer was paused
	Uint32 m_pausedTicks = 0;

	//The timer status
	bool m_paused = false;
	bool m_started = false;
};