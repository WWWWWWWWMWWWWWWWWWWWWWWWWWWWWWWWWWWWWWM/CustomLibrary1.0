#pragma once

#include <memory>
#include <string>

#include <CustomLibrary/utility.h>
#include "C_SimpleTexture.h"

namespace ctl
{
	struct Unique_Deleter { void operator()(TTF_Font *f) const { TTF_CloseFont(f); } };
	namespace FT
	{
		using shared = std::shared_ptr<TTF_Font>;
		using unique = std::unique_ptr<TTF_Font, Unique_Deleter>;
	}

	//FT::
	template<typename T>
	class Font
	{
		T m_ptr;
		unsigned int m_pt;

	public:
		Font() = default;
		Font(const Font &) = default;
		Font& operator=(const Font &) = default;
		Font(Font &&) = default;
		Font& operator=(Font &&) = default;
		Font(const std::string &path, const int &pt) { loadFont(path, pt); }

		//Exception: If file doesn't exit
		void loadFont(const std::string &path, const int &pt)
		{
			if constexpr (std::is_same<T, FT::shared>::value)
				m_ptr.reset(TTF_OpenFont(path.c_str(), pt), TTF_CloseFont);
			else
				m_ptr.reset(TTF_OpenFont(path.c_str(), pt));

			if (!m_ptr)
				throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
			m_pt = pt;
		}

		//Exception: If font not loaded
		//Font Styles: https://www.libsdl.org/projects/SDL_ttf/docs/SDL_ttf_22.html
		void style(const int &style)
		{
			if (!m_ptr)
				throw ctl::Log("Font: Font not loaded.", ctl::Log::Type::ERROR);

			TTF_SetFontStyle(m_ptr.get(), style);
		}
		//Exception: If font not loaded
		int style()
		{
			if (!m_ptr)
				throw ctl::Log("Font: Font not loaded.", ctl::Log::Type::ERROR);

			return TTF_GetFontStyle(m_ptr.get());
		}

		operator TTF_Font*() { return m_ptr.get(); }
		constexpr const T& ptr() const { return m_ptr; }
		constexpr const unsigned int& pt() const { return m_pt; }
	};

	//FT::
	template<typename T>
	class Text
	{
		using _trueType_ = typename std::conditional<std::is_same<T, FT::unique>::value, Font<FT::unique>*, Font<FT::shared>>::type;

		std::string m_text;
		_trueType_ m_font;
		SimpleTexture m_texture;

		void _load_(SDL_Surface *s, const std::string &text)
		{
			if (s == nullptr)
				throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);

			m_texture.load(s);
			m_text = text;
		}

	public:
		Text() = default;
		Text(const WindowRef &e) : m_texture(e) {}
		Text(const WindowRef &e, typename std::remove_pointer<_trueType_>::type &f) : m_font(&f), m_texture(e) {}
		Text(Text &&x) = default;
		Text(const Text &) = default;
		Text& operator=(Text &&x) = default;
		Text& operator=(const Text &x) = default;

		//Exception: If font has a problem
		void loadSolid(const std::string &text, const SDL_Color &colour = { 0, 0, 0, 0xFF }) 
		{ _load_(TTF_RenderUTF8_Solid(ctl::deref_ptr(m_font), text.c_str(), colour), text); }
		//Exception: If font has a problem
		void loadShaded(const std::string &text, const SDL_Color &bg, const SDL_Color &colour = { 0, 0, 0, 0xFF }) 
		{ _load_(TTF_RenderUTF8_Shaded(ctl::deref_ptr(m_font), text.c_str(), colour, bg), text); }
		//Exception: If font has a problem
		void loadBlended(const std::string &text, const SDL_Color &colour = { 0, 0, 0, 0xFF }) 
		{ _load_(TTF_RenderUTF8_Blended(ctl::deref_ptr(m_font), text.c_str(), colour), text); }
		//Exception: If font has a problem
		void loadWrapped(const std::string &text, const Uint16 &wrapper, const SDL_Color &colour = { 0, 0, 0, 0xFF }) 
		{ _load_(TTF_RenderUTF8_Blended_Wrapped(ctl::deref_ptr(m_font), text.c_str(), colour, wrapper), text); }

		//Exception: If font has a problem
		ctl::Vector<int, 2> hypoSize(const std::string &text) 
		{
			ctl::Vector<int, 2> temp; 
			if (TTF_SizeUTF8(ctl::deref_ptr(m_font), text.c_str(), &temp[0], &temp[1]) != 0)
				throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
			return temp;
		}

		constexpr const std::string& textString() const { return m_text; }

		constexpr _trueType_& font() { return m_font; }
		constexpr const _trueType_& font() const { return m_font; }

		constexpr SimpleTexture& texture() { return m_texture; }
		constexpr const SimpleTexture& texture() const { return m_texture; }
	};
}