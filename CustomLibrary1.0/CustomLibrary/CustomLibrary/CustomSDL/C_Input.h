#pragma once

#include <vector>

#include "C_Text.h"
#include "C_GeometricShape.h"

namespace ctl
{
	//FT::
	template<typename FontType>
	class Input
	{
		static constexpr char m_character = '_';

		bool m_renderText = true;
		bool m_active = false;
		bool m_lock = false;

		std::vector<std::string> m_ut8Letters;
		ctl::Text<FontType> m_text;

		template<typename F, typename ...P>
		void _renderFunc_(const Vector<int, 2> &pos, F func, const SDL_Colour &c, P... para)
		{
			for (; m_renderText; m_renderText = false)
				if (m_ut8Letters.empty())
					m_text.loadSolid(m_active ? "_" : " ", c);
				else
				{
					std::string temp(textString());
					if (m_active)
						temp.push_back(m_character);

					(m_text.*func)(temp, para..., c);
				}

			m_text.texture().render(pos);
		}

		void _renderWrapped_(const Vector<int, 2> &pos, const SDL_Colour &c, const Uint16 &wrapper)
		{
			for (; m_renderText; m_renderText = false)
				if (m_ut8Letters.empty())
					m_text.loadSolid(m_active ? "_" : " ", c);
				else
				{
					std::string temp(textString());

					int linePos = temp.find_last_of('\n');
					std::string fromNewLine(linePos != temp.npos ? temp.substr(++linePos) : temp);
					if (m_text.hypoSize(fromNewLine)[0] > wrapper)
						m_ut8Letters.insert((++m_ut8Letters.rbegin()).base(), "\n"),
						temp.insert((++temp.rbegin()).base(), '\n');
					
					if (m_active)
						temp.push_back(m_character);

					m_text.loadWrapped(temp, wrapper, c);
				}

			m_text.texture().render(pos);
		}

	public:
		Input(const ctl::Window::Ref &engine)
			: m_text(engine) { g_geoRender.engine() = engine; }
		Input(const ctl::Window::Ref &engine, ctl::Font<FontType> &f)
			: m_text(engine, f) { g_geoRender.engine() = engine; }

		void event(const SDL_Event &e)
		{
			switch (e.type)
			{
			case SDL_KEYDOWN:
				if (m_active)
					//Handle deleting
					if (e.key.keysym.sym == SDLK_BACKSPACE && m_ut8Letters.size() != 0)
						m_ut8Letters.pop_back(),
						m_renderText = true;

					//Handle newline
					else if (e.key.keysym.sym == SDLK_RETURN && space)
						m_ut8Letters.push_back("\n"),
						m_renderText = true;
				break;

			case SDL_TEXTINPUT:
				if (m_active)
					//Handle Text input
					m_ut8Letters.push_back(e.text.text),
					m_renderText = true;
				break;

			case SDL_WINDOWEVENT:
				//Reload on resize
				if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
					m_renderText = true;
				break;

			default:
				break;
			}
		}

		//Exception: If rendering fails
		void renderWrapped(const ctl::Vector<int, 2> &xy, const Uint16 &wrapper, const SDL_Colour &c = { 0, 0, 0, 0xFF })
		{ _renderFunc_(xy, &ctl::Text<FontType>::loadWrapped, c, wrapper); }

		//Exception: If rendering fails
		void renderBlended(const ctl::Vector<int, 2> &xy, const SDL_Colour &c = { 0, 0, 0, 0xFF })
		{ _renderFunc_(xy, &ctl::Text<FontType>::loadBlended, c); }

		//Exception: If rendering fails
		void renderShaded(const ctl::Vector<int, 2> &xy, const SDL_Colour &bg = { 0xFF, 0xFF, 0xFF, 0xFF }, const SDL_Colour &c = { 0, 0, 0, 0xFF })
		{ _renderFunc_(xy, &ctl::Text<FontType>::loadShaded, c, bg); }

		//Exception: If rendering fails
		void renderSolid(const ctl::Vector<int, 2> &xy, const SDL_Colour &c = { 0, 0, 0, 0xFF })
		{ _renderFunc_(xy, &ctl::Text<FontType>::loadSolid, c); }

		std::string textString() const
		{
			std::string out;
			for (auto& i : m_ut8Letters)
				out += i;
			return out;
		}

		constexpr void lock() { m_lock = !m_lock; }
		void startInput() { if (!m_lock) m_active = true, m_renderText = true; SDL_StartTextInput(); }
		void stopInput() { if (!m_lock) m_active = false, m_renderText = true; SDL_StopTextInput(); }

		void clear()
		{
			m_ut8Letters.clear();
			m_renderText = true;
		}

		bool space = true;

		constexpr const ctl::Text<FontType>& text() const { return m_text; }
		constexpr ctl::Text<FontType>& text() { return m_text; }

		constexpr const std::vector<std::string>& textRaw() const { return m_ut8Letters; }
		constexpr std::vector<std::string>& textRaw() { m_renderText = true; return m_ut8Letters; }
	};
}