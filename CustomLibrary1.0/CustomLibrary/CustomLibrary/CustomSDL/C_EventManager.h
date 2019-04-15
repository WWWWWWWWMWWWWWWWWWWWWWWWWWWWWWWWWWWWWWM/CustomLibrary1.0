#pragma once

#include "C_Engine.h"

namespace ctl
{
	class EventManager
	{
	public:
		//Exception: If something goes wrong with the event system
		static void pushEvent(SDL_Event &e)
		{
			if (SDL_PushEvent(&e) < 0)
				ctl::Log(SDL_GetError(), ctl::Log::Type::WARNING);
		}
		//Exception: If something goes wrong with the event system
		static void pushFlag(const Uint32 &t, const Uint32 &winID, void *data1 = nullptr, void *data2 = nullptr)
		{
			SDL_Event temp;
			SDL_zero(temp);
			temp.user.type = t;
			temp.window.windowID = winID;
			temp.user.data1 = data1;
			temp.user.data2 = data2;

			if (SDL_PushEvent(&temp) < 0)
				ctl::Log(SDL_GetError(), ctl::Log::Type::WARNING);
		}
	};
}