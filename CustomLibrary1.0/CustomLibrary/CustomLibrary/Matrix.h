#pragma once

#include <vector>
#include <functional>

#include "Error.h"
#include "Vector.h"
#include "RandomGenerator.h"

namespace ctl
{
	template<typename Type, typename Allocator = std::allocator<Type>, 
		typename = typename std::enable_if_t<std::is_arithmetic_v<Type>>>
	class Matrix
	{
	public:

		//-------------------------------------------------------------------------------
		//------------------------------Constructors-------------------------------------
		//-------------------------------------------------------------------------------

		Matrix() = default;
		Matrix(const Matrix &) = default;
		Matrix(Matrix &&) = default;

		Matrix(const NumVec<size_t, 2> &size)
			: m_dim(size)
		{
			m_data.reserve(m_dim.product());
		}
		Matrix(const NumVec<size_t, 2> &size, const Type &init)
			: m_data(size.product(), init)
			, m_dim(size)
		{
		}
		Matrix(const NumVec<size_t, 2> &size, const std::initializer_list<Type> &init)
			: m_data(init.begin(), init.end())
			, m_dim(size)
		{
		}
		template<typename Iter, typename = typename std::enable_if_t<!std::is_same_v<typename std::iterator_traits<Iter>::value_type, void>>>
		Matrix(const Iter &begin, const Iter &end)
			: m_data(begin, end)
			, m_dim({ 1, m_data.size() })
		{
		}
		Matrix(std::vector<Type> &&v)
			: m_data(std::move(v))
			, m_dim(m_data.size())
		{
		}

		//~Matrix()
		//{
		//	std::clog << "Destructor\n";
		//}

		//-------------------------------------------------------------------------------
		//-------------------------------Overloads---------------------------------------
		//-------------------------------------------------------------------------------

		Matrix& operator=(const Matrix &) = default;
		Matrix& operator=(Matrix &&) = default;

		auto& operator=(const Type &t) { return apply([&](Type &x) { x = t; }); }
		auto& operator=(const std::initializer_list<Type> &init) { m_data = init; m_dim = { init.size(), 1 }; return *this; }

		auto& operator()(const size_t &x, const size_t &y) { return m_data[x + m_dim[0] * y]; }
		const auto& operator()(const size_t &x, const size_t &y) const { return m_data[x + m_dim[0] * y]; }

		const auto& data() const { return m_data; }

		auto& loc(const size_t &xy) { return m_data[xy]; }
		const auto& loc(const size_t &xy) const { return m_data[xy]; }

		const auto& dim() const { return m_dim; }

		//Scalar

		auto operator+(const Type &x) const { Matrix temp(*this); return temp.apply([&x](Type &y) constexpr { y += x; }); }
		auto operator-(const Type &x) const { Matrix temp(*this); return temp.apply([&x](Type &y) constexpr { y -= x; }); }
		auto operator*(const Type &x) const { Matrix temp(*this); return temp.apply([&x](Type &y) constexpr { y *= x; }); }
		auto operator/(const Type &x) const { Matrix temp(*this); return temp.apply([&x](Type &y) constexpr { y /= x; }); }

		auto& operator+=(const Type &x) { return apply([&x](Type &y) constexpr { y += x; }); }
		auto& operator-=(const Type &x) { return apply([&x](Type &y) constexpr { y -= x; }); }
		auto& operator*=(const Type &x) { return apply([&x](Type &y) constexpr { y *= x; }); }
		auto& operator/=(const Type &x) { return apply([&x](Type &y) constexpr { y /= x; }); }

		//Elementwise

	private:
		template<typename Arith>
		auto& _elementwise_(const Matrix &from, Matrix &to, Arith &&arith) const
		{
			if (from.m_dim != to.m_dim)
				throw Log("Matrix: operator elementwise: size differs.", Log::Severity::ERR0R);

			auto iter = from.m_data.begin();
			return to.apply([&](Type &x) { x = arith(x, *iter++); });
		}

	public:
		auto operator+(const Matrix &m) const { Matrix temp(m); return _elementwise_(*this, temp, std::plus()); }
		auto operator-(const Matrix &m) const { Matrix temp(m); return _elementwise_(*this, temp, std::minus()); }
		auto operator*(const Matrix &m) const { Matrix temp(m); return _elementwise_(*this, temp, std::multiplies()); }
		auto operator/(const Matrix &m) const { Matrix temp(m); return _elementwise_(*this, temp, std::divides()); }

		auto& operator+(Matrix &&m) const { return _elementwise_(*this, m, std::plus()); }
		auto& operator-(Matrix &&m) const { return _elementwise_(*this, m, std::minus()); }
		auto& operator*(Matrix &&m) const { return _elementwise_(*this, m, std::multiplies()); }
		auto& operator/(Matrix &&m) const { return _elementwise_(*this, m, std::divides()); }

		auto& operator+=(const Matrix &m) { return _elementwise_(m, *this, std::plus()); }
		auto& operator-=(const Matrix &m) { return _elementwise_(m, *this, std::minus()); }
		auto& operator*=(const Matrix &m) { return _elementwise_(m, *this, std::multiplies()); }
		auto& operator/=(const Matrix &m) { return _elementwise_(m, *this, std::divides()); }

		//-------------------------------------------------------------------------------
		//--------------------------------Methods----------------------------------------
		//-------------------------------------------------------------------------------

		auto dotProduct(const Matrix &mat2) const
		{
			if (m_dim[0] != mat2.m_dim[1])
				throw Log("Matrix: dotProduct: width not same as height.", Log::Severity::ERR0R);

			Matrix<Type, Allocator> mat({ mat2.m_dim[0], m_dim[1] }, 0);

			for (size_t y1 = 0; y1 < m_dim[1]; ++y1)
				for (size_t x2 = 0; x2 < mat2.m_dim[0]; ++x2)
					for (size_t x1 = 0; x1 < m_dim[0]; ++x1)
						mat(x2, y1) += (*this)(x1, y1) * mat2(x2, x1);

			return mat;
		}

		auto transpose() const
		{
			Matrix<Type, Allocator> mat(NumVec<size_t, 2>{ m_dim[1], m_dim[0] });

			for (size_t x = 0; x < m_dim[0]; ++x)
				for (size_t y = 0; y < m_dim[1]; ++y)
					mat.emplace_back((*this)(x, y));

			return mat;
		}

		template<typename Gen>
		auto& randomize(ctl::RandomGen<Gen> &gen, const ctl::NumVec<Type, 2> &range)
		{
			for (auto& i : m_data)
				i = gen.randNumber<Type>(range[0], range[1]);

			return *this;
		}

		template<typename F>
		auto& apply(F &&func) { return apply(func); }
		template<typename F>
		auto& apply(F &func)
		{
			for (auto& i : m_data)
				func(i);

			return *this;
		}

		auto& emplace_back(const Type &ele)
		{
			if (m_data.size() == m_data.capacity())
				throw Log("Matrix: emplace_back: reserved size reached", Log::Severity::ERR0R);

			m_data.emplace_back(ele);

			return *this;
		}
		auto& emplace_back(Type&& ele)
		{
			if (m_data.size() == m_data.capacity())
				throw Log("Matrix: emplace_back: reserved size reached", Log::Severity::ERR0R);

			m_data.emplace_back(ele);

			return *this;
		}

#ifdef _IOSTREAM_
		friend auto& operator<<(std::ostream &out, const Matrix &m)
		{
			for (size_t i = 0, delim = m.m_dim[0]; i < m.m_data.size(); ++i)
			{
				if (i == delim)
					delim += m.m_dim[0],
					std::cout << '\n';

				std::cout << m.m_data[i] << '\t';
			}

			return out;
		}
#endif // _IOSTREAM_

	private:
		std::vector<Type, Allocator> m_data;
		ctl::NumVec<size_t, 2> m_dim;
	};

	//Scalar forwarding

	template<typename Type, typename Allocator>
	auto operator+(const Type &x, const Matrix<Type, Allocator> &m) { return m + x; }
	template<typename Type, typename Allocator>
	auto operator-(const Type &x, const Matrix<Type, Allocator> &m) { return m - x; }
	template<typename Type, typename Allocator>
	auto operator*(const Type &x, const Matrix<Type, Allocator> &m) { return m * x; }
	template<typename Type, typename Allocator>
	auto operator/(const Type &x, const Matrix<Type, Allocator> &m) { return m / x; }

	//Elementwise forwarding

	template<typename Type, typename Allocator>
	auto& operator+(Matrix<Type, Allocator> &&m1, Matrix<Type, Allocator> &m2) { return m2 + std::move(m1); }
	template<typename Type, typename Allocator>
	auto& operator-(Matrix<Type, Allocator> &&m1, Matrix<Type, Allocator> &m2) { return m2 - std::move(m1); }
	template<typename Type, typename Allocator>
	auto& operator*(Matrix<Type, Allocator> &&m1, Matrix<Type, Allocator> &m2) { return m2 * std::move(m1); }
	template<typename Type, typename Allocator>
	auto& operator/(Matrix<Type, Allocator> &&m1, Matrix<Type, Allocator> &m2) { return m2 / std::move(m1); }
}