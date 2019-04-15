#pragma once

#include <array>
#include <sstream>

#include <CustomLibrary/CustomSDL/C_EngineRef.h>
#include <CustomLibrary/CustomSDL/Renderer/C_SimpleTexture.h>

namespace ctl
{
	class Image
	{
	public:
		Image() = default;
		Image(const ctl::Window::Ref &e)
			: m_texture(e)
		{}
		Image(const Image &) = default;
		Image(Image &&) = default;
		Image& operator=(const Image &) = default;
		Image& operator=(Image &&) = default;

		void loadFromFile(const std::string &path, const std::array<int*, 2> &blit = { nullptr }, const ctl::Vector<Uint8, 3> &key = { 0xFF, 0xFF, 0xFF })
		{
			SDL_Surface* loadedSurface = IMG_Load(path.c_str());
			if (loadedSurface == nullptr)
				throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);

			m_texture.load(_resizeSurfaceY_(loadedSurface, blit), key);
		}
		void loadFromStream(std::stringstream &content, const std::array<int*, 2> &blit = { nullptr }, const ctl::Vector<Uint8, 3> &key = { 0xFF, 0xFF, 0xFF })
		{
			SDL_Surface* loadedSurface = IMG_Load_RW(SDL_RWFromMem(&content.str()[0], content.str().size()), 1);
			if (loadedSurface == nullptr)
				throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);

			m_texture.load(_resizeSurfaceY_(loadedSurface, blit), key);
		}

		constexpr ctl::SimpleTexture& texture() { return m_texture; }

	private:
		SDL_Surface* _resizeSurfaceY_(SDL_Surface *surface, const std::array<int*, 2> &blit)
		{
			if (blit[0] == nullptr || blit[1] == nullptr)
			{
				SDL_Surface *tempSurface;
				if (blit[0] == nullptr && blit[1] == nullptr)
					tempSurface = SDL_CreateRGBSurface(0, *blit[0], *blit[1], surface->format->BitsPerPixel,
						surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
				else
					tempSurface = SDL_CreateRGBSurface(0, blit[0] == nullptr ? static_cast<int>(static_cast<double>(*blit[1]) / surface->h * surface->w) : *blit[0],
						blit[1] == nullptr ? static_cast<int>(static_cast<double>(*blit[0]) / surface->w * surface->h) : *blit[1],
						surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);

				SDL_BlitScaled(surface, nullptr, tempSurface, nullptr);

				SDL_FreeSurface(surface);
				surface = tempSurface;
			}

			return surface;
		}

		ctl::SimpleTexture m_texture;
	};
}