#pragma once

#include <vector>
#include <variant>
#include <cmath>

#include "Error.h"
#include "utility.h"

namespace ctl
{
	double negate(double x) { return -x; }
	double factorial(double x)
	{
		if (std::round(x) != x || x < 0)
			throw Log("Number has a point or it's negative.", Log::Type::ERROR);
		double y = x;
		for (short i = 1; i < x; i++)
			y *= i;
		return x > 0 ? y : 1;
	}
	double dsin(double x) { return std::sin(ctl::to_radians(x)); } double dasin(double x) { return std::asin(ctl::to_radians(x)); }
	double dcos(double x) { return std::cos(ctl::to_radians(x)); } double dacos(double x) { return std::acos(ctl::to_radians(x)); }
	double dtan(double x) { return std::tan(ctl::to_radians(x)); } double datan(double x) { return std::atan(ctl::to_radians(x)); }

	double modulus(double x, double y) { return static_cast<double>(static_cast<int>(x) % static_cast<int>(y)); }
	double root(double x, double y) { return std::pow(x, 1. / y); }

	class Math
	{
	public:
		Math() = default;
		Math(const Math &) = default;
		Math(Math &&) = default;
		constexpr Math& operator=(const Math &) = default;
		constexpr Math& operator=(Math &&) = default;

		Math(const double &d) : m_dat(d) {}
		Math(double(*func)(double)) : m_dat(func) {}
		Math(double(*func)(double, double)) : m_dat(func) {}
		Math(const char &c) : m_dat(c) {}
		Math(const bool &b) : m_dis(b) {}

		Math& operator=(double &&d) { m_dat = d; return *this; }

		template<typename T>
		constexpr bool operator==(const T &x) const
		{
			if (auto val = std::get_if<T>(&m_dat))
				return *val == x;
			return false;
		}
		template<>
		constexpr bool operator==<>(const Math &x) const { return m_dat == x.m_dat && m_dis == x.m_dis; }

		template<typename T>
		constexpr bool is_Type() const { return std::get_if<T>(&m_dat) ? true : false; }

		template<typename T>
		constexpr const T& get() const { return *std::get_if<T>(&m_dat); }
		constexpr const auto& varant() const { return m_dat; }
			
	private:
		std::variant<double, char, double(*)(double), double(*)(double, double)> m_dat;
		bool m_dis = false;
	};

	class Equation_Base
	{
	public:
		virtual bool parse(std::vector<Math> &, const std::string &) = 0;
		virtual double evaluate(std::vector<Math>) = 0;
	};

	class Equation : public Equation_Base
	{
	public:
		template<typename T>
		bool _parseSample_(std::vector<Math> &list, const std::string &buf, const T &sample)
		{
			if constexpr (LOG)
				Log("Equation2.0: Parse: parseSample called.");

			const auto iter_sample = std::find_if(sample.begin(), sample.end(), [&buf](const T::value_type &x) constexpr { return x.first == buf; });
			if (iter_sample != sample.end())
			{
				list.emplace_back(iter_sample->second);
				return true;
			}
			return false;
		};

		bool parseModifer(std::vector<Math> &list, const std::string &buf)
		{
			if constexpr (ctl::LOG)
				ctl::Log("Equation2.0: Parse: parseModifer called.");

			constexpr std::array<std::pair<std::string_view, double(*)(double)>, 9> modif1
			{ std::pair<std::string_view, double(*)(double)>
				{ "sin", dsin }, { "cos", dcos }, { "tan", dtan },
				{ "arcsin", dasin }, { "arccos", dacos }, { "arctan", datan },
				{ "!", factorial }, { "log", std::log }, { "sqrt", std::sqrt }
			};
			if (_parseSample_(list, buf, modif1))
				return true;
			return false;
		}

		bool parseJoiner(std::vector<Math> &list, const std::string &buf)
		{
			if constexpr (ctl::LOG)
				ctl::Log("Equation2.0: Parse: parseJoiner called.");

			constexpr std::array<std::pair<std::string_view, double(*)(double, double)>, 3> modif2
			{ std::pair<std::string_view, double(*)(double, double)>
				{ "mod", modulus }, { "root", root }, { "pow", std::pow }
			};
			if (_parseSample_(list, buf, modif2))
				return true;
			return false;
		}

		bool parseConstant(std::vector<Math> &list, const std::string &buf)
		{
			if constexpr (ctl::LOG)
				ctl::Log("Equation2.0: Parse: parseConstant called.");

			constexpr std::array<std::pair<std::string_view, double>, 2> constant
			{ std::pair<std::string_view, double>
				{ "e", 2.7182818 }, { "pi", 3.1415926 }
			};
			if (_parseSample_(list, buf, constant))
				return true;
			return false;
		}

		bool parse(std::vector<Math> &list, const std::string &buf) override
		{
			if constexpr (LOG)
				Log("Equation2.0: parse called.");

			if (parseModifer(list, buf))
				return true;

			if (parseJoiner(list, buf))
				return true;

			if (parseConstant(list, buf))
				return true;

			return false;
		}

		//Handles double(*)(double)
		void evaluateModifer(std::vector<Math> &subEqu)
		{
			if constexpr (LOG)
				Log("Equation2.0: Evaluate: modifer started.");

			for (auto iter = subEqu.begin(), end = subEqu.end(); iter != subEqu.end(); ++iter)
				if (iter->is_Type<double(*)(double)>())
				{
					*iter = iter->get<double(*)(double)>()((iter + 1)->get<double>());
					iter = subEqu.erase(iter + 1, iter + 2) - 1;
					end = subEqu.end();
				}
		}

		//Handles double(*)(double, double)
		void evaluatejoiner(std::vector<Math> &subEqu)
		{
			if constexpr (LOG)
				Log("Equation2.0: Evaluate: joiner started.");

			for (auto iter = subEqu.begin(), end = subEqu.end(); iter != end; ++iter)
			{
				const auto isFunc = iter->is_Type<double(*)(double, double)>();
				const auto isMult = *iter == '*';
				const auto isDivi = *iter == '/';

				if (isFunc || isMult || isDivi)
				{
					if (isFunc)
						*(iter - 1) = iter->get<double(*)(double, double)>()((iter - 1)->get<double>(), (iter + 1)->get<double>());
					else if (isMult)
						*(iter - 1) = (iter - 1)->get<double>() * (iter + 1)->get<double>();
					else
					{
						const auto numerus = (iter + 1)->get<double>();
						if (numerus == 0.)
							throw Log("Equation2.0: Evaluate: numerus is 0.", Log::Type::ERROR);
						*(iter - 1) = (iter - 1)->get<double>() / (iter + 1)->get<double>();
					}

					iter = subEqu.erase(iter, iter + 2) - 1;
					end = subEqu.end();
				}
			}
		}

		double evaluate(std::vector<Math> subEqu) override
		{
			if constexpr (LOG)
				Log("Equation2.0: evaluation started.");

			//Erase disabled spots
			subEqu.erase(std::remove(subEqu.begin(), subEqu.end(), Math(true)), subEqu.end());

			//Step 1: Modifer
			evaluateModifer(subEqu);

			//Step 2: Joiner
			evaluatejoiner(subEqu);

			//Step 3: Sum up
			auto sum = 0.;
			for (auto& i : subEqu)
				sum += i.get<double>();

			return sum;
		}
	};

	template<typename Equ, typename = typename std::enable_if<std::is_base_of<Equation_Base, Equ>::value>::type>
	double solve(std::string equString)
	{
		//Remove whitespace
		equString.erase(std::remove(equString.begin(), equString.end(), ' '), equString.end());

		if constexpr (LOG)
			Log("Equation2.0: parsing started.");

		//Parse
		//Known limitations:
		//- Will filter out functions with same beginning. Example: Custom function esin will be filtered out because of e.
		//- Multiplication needs to be explicit
		std::vector<Math> equMain;
		//Flag for flushing when next char is not a number
		bool isNum = false;
		for (auto iter = equString.begin(), end = equString.end(), nearEnd = --equString.end(); iter != end; ++iter)
		{
			static std::string buf;
			constexpr auto reset = [&]()
			{
				if (!buf.empty())
				{
					equMain.emplace_back(std::stod(buf));
					buf.erase();
					isNum = false;
				}
			};
			switch (*iter)
			{
			case '-':
				reset();
				//Use double(*)(double) function to act as -1
				equMain.emplace_back(negate);
				break;

			case '+':
				//all Math objects will be added at the final stage
				reset();
				break;

			case '(': case ')': case '*': case '/':
				reset();
				equMain.emplace_back(*iter);
				break;

			default:
				if (ctl::is_number(*iter))
				{
					buf.push_back(*iter);
					isNum = true;
					//drop if end is reached
					if (iter == nearEnd)
						reset();
					continue;
				}
				else if (isNum)
					reset();

				buf.push_back(*iter);
				//User stuff
				if (Equ().parse(equMain, buf))
					buf.erase();

				break;
			}
		}
		equMain.shrink_to_fit();

		if constexpr (LOG)
			Log("Equation2.0: bracket search started.");

		//Find brackets
		using EquIter = std::array<std::vector<Math>::iterator, 2>;
		std::vector<EquIter> equBrackets;
		for (auto iter = equMain.begin(), end = equMain.end(); iter != end; ++iter)
			if (*iter == '(')
				equBrackets.push_back({ iter, end });
			else if (*iter == ')')
			{
				auto temp = std::find_if(equBrackets.rbegin(), equBrackets.rend(), [&](const EquIter &x) { return x[1] == end; });
				if (temp == equBrackets.rend())
					throw Log("Equation2.0: Synthax Error: missing closing bracket.");

				(*temp)[1] = iter;
			}

		if constexpr (LOG)
			Log("Equation2.0: bracket sort started.");

		//Sort order of calculation
		std::sort(equBrackets.begin(), equBrackets.end(), [](const EquIter &x, const EquIter &y) constexpr { return std::distance(x[0], x[1]) < std::distance(y[0], y[1]); });
		if (!equBrackets.empty() && equBrackets.front()[1] == equMain.end())
			throw Log("Back brackets missing.");

		//Calculate
		for (auto iter = equBrackets.begin(), end = equBrackets.end(); iter != end; ++iter)
		{
			*((*iter)[0]) = Equ().evaluate({ (*iter)[0] + 1, (*iter)[1] });
			//Use disabled functionality for placeholders
			std::fill((*iter)[0] + 1, (*iter)[1] + 1, Math(true));
		}

		return Equ().evaluate(equMain);
	}
}