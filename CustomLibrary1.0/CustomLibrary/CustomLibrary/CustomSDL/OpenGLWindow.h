#pragma once

#include <CustomLibrary/CustomSDL/Engine.h>
#include <gl/glew.h>
#include <SDL_opengl.h>
#include <gl/GLU.h>

class OpenGLWindow
{
public:
	OpenGLWindow(const Vector<int, 2> &dim, const std::string &name, const bool &resizeable)
		: m_dim(dim), m_focus("10011")
	{
		//Use selected OpenGL version (3.1)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		//Set options
		Uint32 options = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
		if (resizeable)
			options |= SDL_WINDOW_RESIZABLE;
		//Create window
		if ((m_window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_dim[0], m_dim[1], options)) == nullptr)
			throw Error(SDL_GetError());

		//Create context
		if ((m_context = SDL_GL_CreateContext(m_window)) == nullptr)
			throw Error(SDL_GetError());

		//Initialize GLEW
		GLenum err = glewInit();
		if (err != GLEW_OK)
			throw Error(reinterpret_cast<char const*>(glewGetErrorString(err)));

		//Use Vsync
		if (SDL_GL_SetSwapInterval(1) < 0)
			throw Error(SDL_GetError());

		//Enable depth testing
		glEnable(GL_DEPTH_TEST);

		//Get identifier
		m_id = SDL_GetWindowID(m_window);
	}

	OpenGLWindow(OpenGLWindow &&x) = delete;
	OpenGLWindow(const OpenGLWindow &x) = delete;

	virtual ~OpenGLWindow()
	{
		if (m_window != nullptr && m_renderer != nullptr)
			destroy();
	}

	void destroy() const
	{
		//Destroy window
		SDL_DestroyWindow(m_window);
	}

	void pollEvents(const SDL_Event &e)
	{
		if (e.type == SDL_WINDOWEVENT && e.window.windowID == m_id && m_focus[SHOWN])
		{
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
				destroy();
			}

			//User stuff
			event(e);
		}
	}

	void pollRender()
	{
		if (!m_focus[MINIMIZED] && m_focus[SHOWN])
		{
			//Clear screen
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			//User stuff
			render();

			//Update screen
			SDL_GL_SwapWindow(m_window);
		}
	}

	virtual void event(const SDL_Event &e) {}
	virtual void render() {}

	//Interface for foci
	enum Focus
	{
		MOUSE,
		KEYBOARD,
		FULLSCREEN,
		MINIMIZED,
		SHOWN
	};

	//Getters
	constexpr Vector<int, 2> getDim() const { return m_dim; }
	constexpr bool getFocus(const Focus &f) const { return m_focus[f]; }
	constexpr SDL_Window* getWindow() { return m_window; }
	constexpr SDL_Renderer* getRenderer() { return m_renderer; }

	//BG colour
	SDL_Color m_bg = { 0xFF, 0xFF, 0xFF, 0xFF };

	//Camera
	Vector<int, 4> m_camera = { 0, 0, m_dim[0], m_dim[1] };

private:
	//Window data
	SDL_Window *m_window = nullptr;
	SDL_Renderer *m_renderer = nullptr;
	int m_id;

	//OpenGL context
	SDL_GLContext m_context;

	//Window dimensions
	Vector<int, 2> m_dim;

	//Window focus
	std::bitset<5> m_focus;
};