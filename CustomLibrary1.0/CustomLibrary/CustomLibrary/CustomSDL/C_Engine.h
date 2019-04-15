#pragma once

#include <SDL.h>

#include <array>
#include <bitset>
#include <type_traits>
#include <string>

#include <CustomLibrary/Timer.h>
#include <CustomLibrary/Error.h>
#include <CustomLibrary/utility.h>

namespace ctl
{
	//Inherit this with your window
	class Window
	{
	public:
		//When dim is default the screen size is used.
		//For windowFlags check https://wiki.libsdl.org/SDL_WindowFlags#Values
		//For rendererFlags check https://wiki.libsdl.org/SDL_CreateRenderer#Remarks
		//Exception: Window or Renderer creation fails
		Window(const std::string &name, 
			const ctl::Vector<int, 2> &dim = { 0 }, 
			const Uint32 &windowFlags = SDL_WINDOW_SHOWN, 
			const Uint32 &rendererFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)
			: m_dim(dim)
			, m_focus("010011")
			, saveLoc(SDL_GetPrefPath("Michael & Co.", name.c_str()))
		{
			if (dim[0] == 0)
			{
				SDL_DisplayMode displayMode;
				if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0)
					m_dim[0] = displayMode.w,
					m_dim[1] = displayMode.h;
			}

			if ((m_window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_dim[0], m_dim[1], windowFlags)) == nullptr)
				throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);

			if ((m_renderer = SDL_CreateRenderer(m_window, -1, rendererFlags)) == nullptr)
				throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);

			SDL_SetRenderDrawColor(m_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			m_id = SDL_GetWindowID(m_window);
		}

		Window(Window &&x) = delete;
		Window(const Window &x) = delete;

		virtual ~Window() { destroy(); }

		void destroy() const
		{
			if (m_window != nullptr)
				SDL_DestroyRenderer(m_renderer);
			if (m_renderer != nullptr)
				SDL_DestroyWindow(m_window);
		}

		//For m check https://wiki.libsdl.org/SDL_BlendMode#Values
		void setBlend(const SDL_BlendMode &m) { SDL_SetRenderDrawBlendMode(m_renderer, m); }

		//This goes inside SDL handleEvent()
		void pollEvents(const SDL_Event &e)
		{
			if (e.window.windowID == m_id)
			{
				if (e.type == SDL_WINDOWEVENT)
					switch (e.window.event)
					{
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						m_dim[0] = e.window.data1;
						m_dim[1] = e.window.data2;
						break;

					case SDL_WINDOWEVENT_ENTER:
						m_focus[MOUSE] = true;
						break;

					case SDL_WINDOWEVENT_LEAVE:
						m_focus[MOUSE] = false;
						break;

					case SDL_WINDOWEVENT_FOCUS_GAINED:
						m_focus[KEYBOARD] = true;
						break;

					case SDL_WINDOWEVENT_FOCUS_LOST:
						m_focus[KEYBOARD] = false;
						break;

					case SDL_WINDOWEVENT_MINIMIZED:
						m_focus[MINIMIZED] = true;
						break;

					case SDL_WINDOWEVENT_MAXIMIZED:
						m_focus[MINIMIZED] = false;
						break;

					case SDL_WINDOWEVENT_RESTORED:
						m_focus[MINIMIZED] = false;
						break;

					case SDL_WINDOWEVENT_CLOSE:
						m_focus[SHOWN] = false;
						m_focus[CLOSE] = true;
						destroy();
						break;
					}

				//User stuff
				event(e);
			}
		}

		//This goes inside SDL renderOrder()
		void pollRender()
		{
			if (!m_focus[MINIMIZED] && m_focus[SHOWN])
			{
				//Clear screen
				SDL_SetRenderDrawColor(m_renderer, bg.r, bg.g, bg.b, bg.a);
				SDL_RenderClear(m_renderer);

				//User stuff
				render();

				//Update screen
				SDL_RenderPresent(m_renderer);
			}
		}

		virtual void event(const SDL_Event &e) = 0;
		virtual void render() = 0;

		enum Focus
		{
			MOUSE,
			KEYBOARD,
			FULLSCREEN,
			MINIMIZED,
			SHOWN,
			CLOSE
		};

		constexpr const auto& dim() const { return m_dim; }
		//Access with Focus::
		constexpr const auto focus(const Focus &f) const { return m_focus[f]; }

		constexpr auto* window() { return m_window; }
		constexpr auto* renderer() { return m_renderer; }

		constexpr const auto& ID() const { return m_id; }

		//Public variable for changing the background of the renderer
		SDL_Colour bg = { 0xFF, 0xFF, 0xFF, 0xFF };

		//Public variable for changing the cameras position
		ctl::Vector<int, 2> camera = { 0, 0 };

		//Const public variable for data save folder
		const std::string saveLoc;

		using Ref = ctl::Window*;

	private:
		SDL_Window *m_window = nullptr;
		SDL_Renderer *m_renderer = nullptr;

		Uint32 m_id;
		ctl::Vector<int, 2> m_dim;
		std::bitset<6> m_focus;
	};

	class SDL
	{
	public:
		//Exception: SDL or one of the core components won't initilize
		SDL(const int &fps, const Uint32 &SDL_Flags = SDL_INIT_EVERYTHING)
			: m_desFPS{ 1000 / fps }
		{
			if (SDL_Init(SDL_Flags) < 0)
				throw ctl::Log(SDL_GetError());

			//Set texture filtering to linear
			if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
				ctl::Log("Warning: Linear texture filtering not enabled!\n");

#ifdef SDL_IMAGE_H_
			//Initialize PNG loading
			if ((IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG)) != (IMG_INIT_PNG | IMG_INIT_JPG))
				throw ctl::Log(IMG_GetError());
#endif //SDL_IMAGE_H_

#ifdef SDL_MIXER_H_
			//Initialize SDL_mixer
			if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
				throw Error(Mix_GetError());
#endif //SDL_MIXER_H_

#ifdef SDL_TTF_H_
			//Initialize SDL_ttf
			if (TTF_Init() == -1)
				throw ctl::Log(TTF_GetError(), ctl::Log::Type::ERROR);
#endif //_SDL_TTF_H
		}

		SDL(SDL &&x) = delete;
		SDL(const SDL &x) = delete;

		virtual ~SDL()
		{
			//Quit SDL subsystems
#ifdef SDL_MIXER_H_
			Mix_Quit();
#endif //SDL_MIXER_H_

#ifdef SDL_TTF_H_
			TTF_Quit();
#endif //_SDL_TTF_H

#ifdef SDL_IMAGE_H_
			IMG_Quit();
#endif //SDL_IMAGE_H_

			SDL_Quit();
		}

		//Required in main()
		void run()
		{
			//Start counting frames per second
			Timer fpsTimer;
			fpsTimer.start();
			unsigned long long countedFrames = 0;

			//For delta calculation
			Uint64 now = SDL_GetPerformanceCounter();
			Uint64 last = 0;

			for (bool quit = false; !quit; ++countedFrames)
			{
				//Get start time of frame
				Uint32 fStart = SDL_GetTicks();

				//Calculate and correct fps
				m_fps = countedFrames / static_cast<double>(fpsTimer.getTicks<std::chrono::seconds>().count());

				//Calculate delta
				last = std::move(now);
				now = SDL_GetPerformanceCounter();
				m_delta = (now - last) / static_cast<double>(SDL_GetPerformanceFrequency());

				static SDL_Event e;
				while (SDL_PollEvent(&e) != 0)
					if (e.type != SDL_QUIT)
						handleEvent(e);
					else
						quit = true;

				//Render stuff
				renderOrder();

				int fTime = SDL_GetTicks() - fStart;
				if (m_desFPS > fTime)
					SDL_Delay(m_desFPS - fTime);
			}
		}

		//Frames per second
		constexpr const double& hz() const { return m_fps; }
		//Movement delta
		constexpr const double& movDelta() const { return m_delta; }

		//Pure virtual function for handling events.
		virtual void handleEvent(const SDL_Event &e) = 0;

		//Pure virtual function for rendering textures.
		virtual void renderOrder() = 0;

	private:
		double m_fps;
		double m_delta = 0;
		int m_desFPS;
	};
}