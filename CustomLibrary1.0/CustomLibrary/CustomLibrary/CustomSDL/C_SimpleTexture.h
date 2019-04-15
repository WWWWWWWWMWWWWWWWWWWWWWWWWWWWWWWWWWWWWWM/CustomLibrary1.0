#pragma once

#include <memory>

#include <CustomLibrary/CustomSDL/C_Engine.h>

namespace ctl
{
	struct Safe_Texture
	{
		Safe_Texture(SDL_Texture *t)
			: raw(t) {}

		~Safe_Texture()
		{
			if (raw != nullptr)
				SDL_DestroyTexture(raw),
				raw = nullptr;
		}

		operator SDL_Texture*() { return raw; }

		SDL_Texture *raw;
	};

	class Texture
	{
	public:
		Texture() = default;
		Texture(const ctl::Window::Ref &engine) : m_ref(engine) {}
		Texture(Texture &&x) = default;
		Texture(const Texture &x) = default;
		Texture& operator=(Texture &&x) = default;
		Texture& operator=(const Texture &x) = default;

		//Exception: If rendering fails
		void render(const ctl::Vector<int, 2> xy, const double &angle, const SDL_RendererFlip &flip, SDL_Point *const center = nullptr, const SDL_Rect *clip = nullptr) const
		{
			SDL_Rect renderQuad = { xy[0] - m_ref->camera[0], xy[1] - m_ref->camera[1], m_dim[0], m_dim[1] };

			if (clip != nullptr)
				renderQuad.w = clip->w,
				renderQuad.h = clip->h;

			if (SDL_RenderCopyEx(m_ref->renderer(), m_texture.get(), clip, &renderQuad, angle, center, flip) < 0)
				throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
		}
		//Exception: If rendering fails
		void render(const ctl::Vector<int, 2> xy, const SDL_Rect *clip = nullptr) const
		{
			SDL_Rect renderQuad = { xy[0] - m_ref->camera[0], xy[1] - m_ref->camera[1], m_dim[0], m_dim[1] };

			if (clip != nullptr)
				renderQuad.w = clip->w,
				renderQuad.h = clip->h;

			if (SDL_RenderCopy(m_ref->renderer(), m_texture.get(), clip, &renderQuad) < 0)
				throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
		}

		void colour(const ctl::Vector<Uint8, 3> &c) { SDL_SetTextureColorMod(m_texture.get(), c[0], c[1], c[2]); }
		ctl::Vector<Uint8, 3> colour() { ctl::Vector<Uint8, 3> c; SDL_GetTextureColorMod(m_texture.get(), &c[0], &c[1], &c[2]); return c; }

		void blendMode(const SDL_BlendMode &b) { SDL_SetTextureBlendMode(m_texture.get(), b); }
		SDL_BlendMode blendMode() { SDL_BlendMode b;  SDL_GetTextureBlendMode(m_texture.get(), &b); return b; }

		void alpha(const Uint8 &a) { SDL_SetTextureAlphaMod(m_texture.get(), a); }
		Uint8 alpha() { Uint8 a; SDL_GetTextureAlphaMod(m_texture.get(), &a); return a; }

		constexpr const ctl::Vector<int, 2>& dim() const { return m_dim; }

		constexpr ctl::Window::Ref& engine() { return m_ref; }
		constexpr const ctl::Window::Ref& engine() const { return m_ref; }

	protected:
		std::shared_ptr<SDL_Texture> m_texture;
		ctl::Vector<int, 2> m_dim = { 0 };

		ctl::Window::Ref m_ref;
	};

	class SimpleTexture : public Texture
	{
	public:
		SimpleTexture() = default;
		SimpleTexture(const ctl::Window::Ref &engine) : Texture(engine) {}
		SimpleTexture(SimpleTexture &&x) = default;
		SimpleTexture(const SimpleTexture &x) = default;
		SimpleTexture& operator=(SimpleTexture &&x) = default;
		SimpleTexture& operator=(const SimpleTexture &x) = default;

		//Exception: If creating texture fails. Most of the time it's because of nullptr surface.
		void load(SDL_Surface *surface, const ctl::Vector<Uint8, 3> &key = { 0xFF, 0xFF, 0xFF })
		{
			SDL_SetSurfaceRLE(surface, SDL_TRUE);
			SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, key[0], key[1], key[2]));

			m_dim[0] = std::move(surface->w);
			m_dim[1] = std::move(surface->h);

			m_texture.reset(SDL_CreateTextureFromSurface(m_ref->renderer(), surface), SDL_DestroyTexture);
			if (!m_texture)
				throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);

			SDL_FreeSurface(surface);
		}
	};

	class U_PixelFormat
	{
	public:
		U_PixelFormat() 
			: m_format(nullptr, SDL_FreeFormat) {}
		U_PixelFormat(const Uint32 &f)
			: m_format(SDL_AllocFormat(f), SDL_FreeFormat) {}

		void alloc(const Uint32 &f) { m_format = { SDL_AllocFormat(f), SDL_FreeFormat }; }

		Uint32 mapRGB(const ctl::Vector<Uint8, 3> &c) { return SDL_MapRGB(m_format.get(), c[0], c[1], c[2]); }
		Uint32 mapRGBA(const ctl::Vector<Uint8, 4> &c) { return SDL_MapRGBA(m_format.get(), c[0], c[1], c[2], c[3]); }

	private:
		std::unique_ptr<SDL_PixelFormat, void(*)(SDL_PixelFormat *)> m_format;
	};

	class ModdedTexture : public Texture
	{
	public:
		ModdedTexture(const ctl::Window::Ref &engine)
			: Texture(engine) {}
		ModdedTexture(ModdedTexture &&x)
			: Texture(std::move(x)) {}
		ModdedTexture(const ModdedTexture &x) = default;
		ModdedTexture& operator=(ModdedTexture &&x) { Texture::operator=(std::move(x)); return *this; }
		ModdedTexture& operator=(const ModdedTexture &x) = default;

		//Exception: Fail on surface convert or texture creation
		void load(SDL_Surface *s)
		{
			SDL_Surface *f_s = SDL_ConvertSurfaceFormat(s, SDL_GetWindowPixelFormat(m_ref->window()), 0);
			if (f_s == nullptr)
				throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);

			m_texture.reset(SDL_CreateTexture(m_ref->renderer(), SDL_GetWindowPixelFormat(m_ref->window()), SDL_TEXTUREACCESS_STREAMING, f_s->w, f_s->h), SDL_DestroyTexture);
			if (!m_texture)
				throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);

			lock();

			memcpy(m_pixels, f_s->pixels, f_s->pitch * f_s->h);
			unlock();

			m_pixels = nullptr;

			m_dim[0] = std::move(f_s->w);
			m_dim[1] = std::move(f_s->h);

			SDL_FreeSurface(f_s);
			SDL_FreeSurface(s);
		}

		void load(const ctl::Vector<int, 2> &wh, const SDL_TextureAccess &a)
		{
			m_texture.reset(SDL_CreateTexture(m_ref->renderer(), SDL_PIXELFORMAT_RGBA8888, a, wh[0], wh[1]), SDL_DestroyTexture);
			if (!m_texture)
				throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);

			m_dim = wh;
		}

		void lock()
		{
			if (m_pixels != nullptr)
				ctl::Log("Texture is already locked.", ctl::Log::Type::WARNING);

			if (SDL_LockTexture(m_texture.get(), nullptr, &m_pixels, &m_pitch) != 0)
				throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
		}

		void unlock()
		{
			if (m_pixels == nullptr)
				ctl::Log("Texture is already unlocked.", ctl::Log::Type::WARNING);

			SDL_UnlockTexture(m_texture.get());
			m_pixels = nullptr;
			m_pitch = 0;
		}

		void target() { if (SDL_SetRenderTarget(m_ref->renderer(), m_texture.get()) != 0) throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR); }

		void cpyPixel(void *p)
		{
			if (m_pixels != nullptr)
				memcpy(m_pixels, p, m_pitch * m_dim[1]);
		}

		template<typename T = void*>
		constexpr T pixels() { return static_cast<T>(m_pixels); }
		template<typename T = void*>
		constexpr T pixel(const ctl::Vector<Uint32, 2> &xy) { T p = static_cast<T>(m_pixels); return p[(xy[0] * m_pitch / 4) + xy[1]]; }

		constexpr const int& pitch() const { return m_pitch; }
		constexpr int& pitch() { return m_pitch; }
		constexpr int pixelCount() const { return m_pitch / 4 * m_dim[1]; }

	private:
		void *m_pixels;
		int m_pitch;
	};
}