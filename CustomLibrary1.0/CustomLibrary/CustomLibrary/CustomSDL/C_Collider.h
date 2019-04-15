#pragma once

#include <cmath>

#include <CustomLibrary/CustomSDL/C_Engine.h>
#include <CustomLibrary/array.h>

namespace ctl
{
	
	template<typename T>
	constexpr T _power2_(T x)
	{
		return x * x;
	}

	template<typename T, typename A>
	constexpr bool coll(const ctl::Vector<T, 4> &r, const ctl::Vector<A, 3> &c)
	{
		//Closest point on collision box
		int cX = 0, cY = 0;
		
		//Find closest x offset
		if (c[0] + c[2] / 2 < r[0])
			cX = r[0];
		else if (c[0] + c[2] / 2 > r[0] + r[2])
			cX = r[0] + r[2];
		else
			cX = c[0] + c[2] / 2;

		//Find closest y offset
		if (c[1] + c[2] / 2 < r[1])
			cY = r[1];
		else if (c[1] + c[2] / 2 > r[1] + r[3])
			cY = r[1] + r[3];
		else
			cY = c[1] + c[2] / 2;

		//If the closest point is inside the circle
		return _power2_(cX - c[0] - c[2] / 2) + _power2_(cY - c[1] - c[2] / 2) < _power2_(c[2]) / 4;
	}
	template<typename T, typename A>
	constexpr bool coll(const ctl::Vector<T, 4> &r1, const ctl::Vector<A, 4> &r2)
	{
		//If any of the sides from A are outside of B
		return r1[1] + r1[3] <= r2[1] || r1[1] >= r2[1] + r2[3] || r1[0] + r1[2] <= r2[0] || r1[0] >= r2[0] + r2[2];
	}
	template<typename T, typename A>
	constexpr bool coll(const ctl::Vector<T, 3> &c1, const ctl::Vector<A, 3> &c2)
	{
		//If the distance between the centres of the circles is less than the sum of their rad
		return _power2_(c1[0] + c1[2] / 2 - c2[0] - c2[2] / 2) + _power2_(c1[1] + c1[2] / 2 - c2[1] - c2[2] / 2) < _power2_(c1[2] + c2[2]) / 4;
	}

	class BoxCollider
	{
	public:
		BoxCollider(const ctl::Vector<int, 4> *const r)
			: m_col(r) {}

		//Box/Box collision detection
		constexpr bool checkCollision(const ctl::Vector<int, 4> &r) const
		{
			return coll(*m_col, r);
		}
		//Box/Circle collision detection
		constexpr bool checkCollision(const ctl::Vector<int, 3> &c) const
		{
			return coll(*m_col, c);
		}

		constexpr const ctl::Vector<int, 4>* getCollider() const { return m_col; }

	private:
		//Pointer
		const ctl::Vector<int, 4> *const m_col;
	};

	class CircleCollider
	{
	public:
		CircleCollider(ctl::Vector<int, 3> *const c)
			: m_col(c) {}

		//Circle/Circle collision detection
		constexpr bool checkCollision(const ctl::Vector<int, 3> &c) const
		{
			return coll(*m_col, c);
		}
		//Box/Circle collision detection
		constexpr bool checkCollision(const ctl::Vector<int, 4> &r) const
		{
			return coll(r, *m_col);
		}

		constexpr const ctl::Vector<int, 3>* getCollider() const { return m_col; }

	private:
		ctl::Vector<int, 3> *const m_col;
	};
}