#pragma once

#include <CustomLibrary/CustomSDL/C_EngineRef.h>
#include <CustomLibrary/CustomSDL/Renderer/C_Text.h>
#include <CustomLibrary/CustomSDL/Adaptors/C_Button.h>

namespace ctl
{
	template<typename FontType>
	class Option
	{
	public:
		Option(const ctl::Window::Ref &e)
			: m_text(e)
			, m_button(std::make_tuple(&m_pos[0], &m_pos[1], &m_text.texture().dim()[0], &m_text.texture().dim()[1]))
		{
		}

		void handleEvent(const SDL_Event &e)
		{
			switch (e.type)
			{
			case SDL_MOUSEMOTION:
				if (m_button.inside())
				{
					if (!m_wasInside)
					{
						m_text.font()->style(TTF_STYLE_BOLD);
						_dimUpdate_();
						m_text.font()->style(TTF_STYLE_NORMAL);

						m_wasInside = true;
					}
				}
				else if (m_wasInside)
				{
					_dimUpdate_();
					m_wasInside = false;
				}
				break;

			case SDL_WINDOWEVENT:
				if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
					_dimUpdate_();
				break;

			default:
				break;
			}

			event(e);
		}

		virtual void event(const SDL_Event &e) = 0;
		virtual void render() { m_text.texture().render(m_pos); }

		constexpr const ctl::Vector<int, 2>& pos() const { return m_pos; }
		constexpr ctl::Vector<int, 2>& pos() { return m_pos; }
		constexpr const Text<FontType>& text() const { return m_text; }
		constexpr Text<FontType>& text() { return m_text; }

	protected:
		ctl::Vector<int, 2> m_pos = { 0, 0 };
		ctl::Text<FontType> m_text;
		ctl::MouseCol<int*, int*, const int*, const int*> m_button;

		virtual void _dimUpdate_() = 0;

	private:
		bool m_wasInside = false;
	};
}