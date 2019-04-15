#pragma once

#include "Error.h"
#include <SDL_mixer.h>

class SoundChunk
{
public:
	~SoundChunk()
	{
		Mix_FreeChunk(m_chunk);
	}

	//Load sound from path
	void loadFromWAV(const std::string &path)
	{
		m_chunk = Mix_LoadWAV(path.c_str());
		if (m_chunk == nullptr)
			throw Error(Mix_GetError());
	}

	//Play sound
	void playSound(const int &ch, const int &loop = 0)
	{
		Mix_PlayChannel(ch, m_chunk, loop);
	}

private:
	Mix_Chunk *m_chunk;
};
