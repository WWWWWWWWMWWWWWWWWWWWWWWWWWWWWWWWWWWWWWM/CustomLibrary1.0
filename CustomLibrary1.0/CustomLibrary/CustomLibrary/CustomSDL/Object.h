#pragma once


namespace ctl
{
	class Object
	{
	protected:
		ctl::Window::Ref m_window;

	public:
		Object(const ctl::Window::Ref &e)
			: m_window(e)
		{}

		virtual void render() = 0;
		virtual void event(const SDL_Event &e) = 0;
	};
}