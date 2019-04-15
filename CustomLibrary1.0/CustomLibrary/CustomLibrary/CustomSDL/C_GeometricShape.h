#pragma once

#include <vector>

#include <CustomLibrary/CustomSDL/C_Engine.h>
#include <CustomLibrary/CustomSDL/C_EngineRef.h>
#include <CustomLibrary/utility.h>

namespace ctl
{
	enum class Shapes { LINE, SQUARE, SQUARE_FILLED, POINT, CIRCLE, P_CIRCLE };

	//-------------------Single Render------------------

	//parameter template
	template<Shapes S, size_t x>
	void _geoRender_(const SDL_Color &rgba, const ctl::Vector<int, x> &xy, ctl::Window::Ref &e) { ctl::Log("GeometricRenderer: parameter error.", ctl::Log::Type::WARNING); }

	template<>
	void _geoRender_<Shapes::SQUARE>(const SDL_Color &rgba, const ctl::Vector<int, 4> &xy, ctl::Window::Ref &e)
	{
		SDL_SetRenderDrawColor(e->renderer(), rgba.r, rgba.g, rgba.b, rgba.a);

		//Modify for camera
		SDL_Rect temp = xy.to_SDL_Rect();
		temp.x -= e->camera[0];
		temp.y -= e->camera[1];

		if (SDL_RenderDrawRect(e->renderer(), &temp) != 0)
			throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
	}
	template<>
	void _geoRender_<Shapes::LINE>(const SDL_Color &rgba, const ctl::Vector<int, 4> &xy, ctl::Window::Ref &e)
	{
		SDL_SetRenderDrawColor(e->renderer(), rgba.r, rgba.g, rgba.b, rgba.a);

		SDL_Rect temp = xy.to_SDL_Rect();
		temp.x -= e->camera[0];
		temp.y -= e->camera[1];

		if (SDL_RenderDrawLine(e->renderer(), xy[0] - e->camera[0], xy[1] - e->camera[1], xy[2], xy[3]) != 0)
			throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
	}
	template<>
	void _geoRender_<Shapes::SQUARE_FILLED>(const SDL_Color &rgba, const ctl::Vector<int, 4> &xy, ctl::Window::Ref &e)
	{
		SDL_SetRenderDrawColor(e->renderer(), rgba.r, rgba.g, rgba.b, rgba.a);

		SDL_Rect temp = xy.to_SDL_Rect();
		temp.x -= e->camera[0];
		temp.y -= e->camera[1];

		if (SDL_RenderFillRect(e->renderer(), &temp) != 0)
			throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
	}
	template<>
	void _geoRender_<Shapes::POINT>(const SDL_Color &rgba, const ctl::Vector<int, 2> &xy, ctl::Window::Ref &e)
	{
		SDL_SetRenderDrawColor(e->renderer(), rgba.r, rgba.g, rgba.b, rgba.a);

		if (SDL_RenderDrawPoint(e->renderer(), xy[0] - e->camera[0], xy[1] - e->camera[1]) != 0)
			throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
	}
	template<>
	void _geoRender_<Shapes::CIRCLE>(const SDL_Color &rgba, const ctl::Vector<int, 3> &xy, ctl::Window::Ref &e)
	{
		SDL_SetRenderDrawColor(e->renderer(), rgba.r, rgba.g, rgba.b, rgba.a);

		ctl::Vector<int, 2> Pxy = { xy[2] - 1, 0 };
		ctl::Vector<int, 2> Dxy = { 1, 1 };
		int err = Dxy[0] - (xy[2] << 1);

		while (Pxy[0] >= Pxy[1])
		{
			const SDL_Point p[] =
			{
				{ xy[0] + Pxy[0], xy[1] + Pxy[1] }, { xy[0] + Pxy[1], xy[1] + Pxy[0] },
				{ xy[0] - Pxy[1], xy[1] + Pxy[0] }, { xy[0] - Pxy[0], xy[1] + Pxy[1] },
				{ xy[0] - Pxy[0], xy[1] - Pxy[1] }, { xy[0] - Pxy[1], xy[1] - Pxy[0] },
				{ xy[0] + Pxy[1], xy[1] - Pxy[0] }, { xy[0] + Pxy[0], xy[1] - Pxy[1] }
			};
			if (SDL_RenderDrawPoints(e->renderer(), p, 8) != 0)
				throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);

			if (err <= 0)
				++Pxy[1],
				err += Dxy[1],
				Dxy[1] += 2;

			else
				--Pxy[0],
				Dxy[0] += 2,
				err += Dxy[0] - (xy[2] << 1);
		}
	}

	//-------------------------END--------------------------

	class GeometricShape
	{
	public:
		GeometricShape() {}
		GeometricShape(const ctl::Window::Ref &e)
			: m_ref(e) {}

		//parameter template
		template<Shapes S, size_t x>
		void render(const SDL_Color &rgba, const ctl::Vector<int, x> &xy) { _geoRender_<S, x>(rgba, xy, m_ref); }

		template<typename Container>
		Container& prepData(Container &c)
		{
			for (auto iter = std::begin(c), end = std::end(c); iter != end; ++iter)
				(*iter)[0] -= m_ref->camera[0],
				(*iter)[1] -= m_ref->camera[1];

			return c;
		}
		ctl::Vector<int, 4>& prepData(ctl::Vector<int, 4> &v)
		{
			v[0] -= m_ref->camera[0];
			v[1] -= m_ref->camera[1];

			return v;
		}

		//Render circle (FAST)
		template<Shapes S, typename = typename std::enable_if<S == Shapes::P_CIRCLE>::type>
		void render(const SDL_Color &rgba, const ctl::Vector<int, 3> &xy, const int &pres)
		{
			SDL_SetRenderDrawColor(m_ref->renderer(), rgba.r, rgba.g, rgba.b, rgba.a);

			std::vector<SDL_Point> points;
			points.reserve(pres + 1);

			for (auto i = 0; i < pres; ++i)
			{
				const auto x = to_radians(360. / pres * (i + 1));
				points.push_back({ static_cast<int>(xy[2] * std::cos(x) + xy[0] - m_ref->camera[0]), static_cast<int>(xy[2] * std::sin(x) + xy[1] - m_ref->camera[1]) });
			}
			points.push_back(points.front());

			SDL_RenderDrawLines(m_ref->renderer(), points.data(), pres + 1);
		}

		//parameter template
		template<Shapes S, typename T>
		constexpr void render(const SDL_Color &rgba, const T &arr) { renderMass_impl<S, T>::_(rgba, arr, m_ref); }
		template<Shapes S, typename T, typename VecAcc>
		constexpr void render(const SDL_Color &rgba, const T &arr, VecAcc f) { renderMass_impl<S, T, VecAcc>::_(rgba, arr, m_ref, f); }

		constexpr const auto& engine() const { return m_ref; }
		constexpr auto& engine() { return m_ref; }

	private:
		//----------------------Mass Render---------------------

		template<Shapes S, typename T, typename VecAcc = void>
		struct renderMass_impl { static void _() { ctl::Log("GeometricRenderer: parameter error.", ctl::Log::Type::WARNING); } };

		template<typename Container>
		struct renderMass_impl<Shapes::SQUARE, Container>
		{
			static void _(const SDL_Color &rgba, const Container &arr, const ctl::Window::Ref &e)
			{
				SDL_SetRenderDrawColor(e->renderer(), rgba.r, rgba.g, rgba.b, rgba.a);

				//Modify for camera and transform to SDL_Rect
				std::vector<SDL_Rect> temp;
				temp.reserve(std::distance(std::begin(arr), std::end(arr)));
				for (auto& i : arr)
					temp.push_back({ i[0] - e->camera[0], i[1] - e->camera[1], i[2], i[3] });

				if (SDL_RenderDrawRects(e->renderer(), temp.data(), temp.size()) != 0)
					throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
			}
		};
		template<typename Container, typename VecAcc>
		struct renderMass_impl<Shapes::SQUARE, Container, VecAcc>
		{
			static void _(const SDL_Color &rgba, const Container &arr, const ctl::Window::Ref &e, VecAcc f)
			{
				SDL_SetRenderDrawColor(e->renderer(), rgba.r, rgba.g, rgba.b, rgba.a);

				std::vector<SDL_Rect> temp;
				temp.reserve(std::distance(std::begin(arr), std::end(arr)));
				for (auto& i : arr)
					temp.push_back({ f(i)[0] - e->camera[0], f(i)[1] - e->camera[1], f(i)[2], f(i)[3] });

				if (SDL_RenderDrawRects(e->renderer(), temp.data(), temp.size()) != 0)
					throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
			}
		};

		template<typename Container>
		struct renderMass_impl<Shapes::SQUARE_FILLED, Container>
		{
			static void _(const SDL_Color &rgba, const Container &arr, const ctl::Window::Ref &e)
			{
				SDL_SetRenderDrawColor(e->renderer(), rgba.r, rgba.g, rgba.b, rgba.a);

				//Modify for camera and transform to SDL_Rect
				std::vector<SDL_Rect> temp;
				temp.reserve(std::distance(std::begin(arr), std::end(arr)));
				for (auto& i : arr)
					temp.push_back({ i[0] - e->camera[0], i[1] - e->camera[1], i[2], i[3] });

				if (SDL_RenderFillRects(e->renderer(), temp.data(), temp.size()) != 0)
					throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
			}
		};
		template<typename Container, typename VecAcc>
		struct renderMass_impl<Shapes::SQUARE_FILLED, Container, VecAcc>
		{
			static void _(const SDL_Color &rgba, const Container &arr, const ctl::Window::Ref &e, VecAcc f)
			{
				SDL_SetRenderDrawColor(e->renderer(), rgba.r, rgba.g, rgba.b, rgba.a);

				std::vector<SDL_Rect> temp;
				temp.reserve(std::distance(std::begin(arr), std::end(arr)));
				for (auto& i : arr)
					temp.push_back({ f(i)[0] - e->camera[0], f(i)[1] - e->camera[1], f(i)[2], f(i)[3] });

				if (SDL_RenderFillRects(e->renderer(), temp.data(), temp.size()) != 0)
					throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
			}
		};

		template<typename Container>
		struct renderMass_impl<Shapes::POINT, Container>
		{
			static void _(const SDL_Color &rgba, const Container &arr, const ctl::Window::Ref &e)
			{
				SDL_SetRenderDrawColor(e->renderer(), rgba.r, rgba.g, rgba.b, rgba.a);

				std::vector<SDL_Point> temp;
				temp.reserve(std::distance(std::begin(arr), std::end(arr)));
				for (auto& i : arr)
					temp.push_back({ i[0] - e->camera[0], i[1] - e->camera[1] });

				if (SDL_RenderDrawPoints(e->renderer(), temp.data(), temp.size()) != 0)
					throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
			}
		};
		template<typename Container, typename VecAcc>
		struct renderMass_impl<Shapes::POINT, Container, VecAcc>
		{
			static void _(const SDL_Color &rgba, const Container &arr, const ctl::Window::Ref &e, VecAcc f)
			{
				SDL_SetRenderDrawColor(e->renderer(), rgba.r, rgba.g, rgba.b, rgba.a);

				std::vector<SDL_Point> temp;
				temp.reserve(std::distance(std::begin(arr), std::end(arr)));
				for (auto& i : arr)
					temp.push_back({ f(i)[0] - e->camera[0], f(i)[1] - e->camera[1] });

				if (SDL_RenderDrawPoints(e->renderer(), temp.data(), temp.size()) != 0)
					throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
			}
		};

		template<typename Container>
		struct renderMass_impl<Shapes::LINE, Container>
		{
			static void _(const SDL_Color &rgba, const Container &arr, const ctl::Window::Ref &e)
			{
				SDL_SetRenderDrawColor(e->renderer(), rgba.r, rgba.g, rgba.b, rgba.a);

				std::vector<SDL_Point> temp;
				temp.reserve(std::distance(std::begin(arr), std::end(arr)));
				for (auto& i : arr)
					temp.push_back({ i[0] - e->camera[0], i[1] - e->camera[1] });

				if (SDL_RenderDrawLines(e->renderer(), temp.data(), temp.size()) != 0)
					throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
			}
		};
		template<typename Container, typename VecAcc>
		struct renderMass_impl<Shapes::LINE, Container, VecAcc>
		{
			static void _(const SDL_Color &rgba, const Container &arr, const ctl::Window::Ref &e, VecAcc f)
			{
				SDL_SetRenderDrawColor(e->renderer(), rgba.r, rgba.g, rgba.b, rgba.a);

				std::vector<SDL_Point> temp;
				temp.reserve(std::distance(std::begin(arr), std::end(arr)));
				for (auto& i : arr)
					temp.push_back({ f(i)[0] - e->camera[0], f(i)[1] - e->camera[1] });

				if (SDL_RenderDrawLines(e->renderer(), temp.data(), temp.size()) != 0)
					throw ctl::Log(SDL_GetError(), ctl::Log::Type::ERROR);
			}
		};

		//-------------------------END--------------------------

		ctl::Window::Ref m_ref;
	} g_geoRender;
}