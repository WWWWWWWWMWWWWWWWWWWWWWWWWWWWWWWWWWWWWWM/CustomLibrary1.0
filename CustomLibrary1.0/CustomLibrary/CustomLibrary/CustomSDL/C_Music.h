#pragma once

#include <CustomLibrary/Error.h>

namespace ctl
{
	class Music
	{
	public:
		~Music()
		{
			if (m_music != nullptr)
				Mix_FreeMusic(m_music),
				m_music = nullptr;
		}

		void loadFromWAV(const std::string &path)
		{
			if ((m_music = Mix_LoadMUS(path.c_str())) == nullptr)
				throw Error(Mix_GetError());
		}

		//(-1 forever)
		void play(const int &loop)
		{
			if (!Mix_PlayingMusic())
				Mix_PlayMusic(m_music, loop);
			else if (Mix_PausedMusic())
				Mix_ResumeMusic();
		}
		void playFade(const int &loop, const int &ms)
		{
			if (!Mix_PlayingMusic())
				Mix_FadeInMusic(m_music, loop, ms);
			else if (Mix_PausedMusic())
				Mix_ResumeMusic();
		}

		static void pause()
		{
			if (!Mix_PausedMusic())
				Mix_PauseMusic();
		}

		static bool ipaused() { return Mix_PausedMusic(); }
		static bool playing() { return Mix_PlayingMusic(); }
		static void stop() { Mix_HaltMusic(); }
		static void stopFade(const int &ms) { Mix_FadeOutMusic(ms); }
		static void volume(const unsigned char &vol) { Mix_VolumeMusic(vol); }

	private:
		Mix_Music *m_music = nullptr;
	};
}