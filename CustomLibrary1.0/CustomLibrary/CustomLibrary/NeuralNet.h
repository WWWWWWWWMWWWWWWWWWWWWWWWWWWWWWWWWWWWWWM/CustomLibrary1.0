#pragma once

#include <fstream>

#include "Vector.h"
#include "RandomGenerator.h"
#include "utility.h"
#include "Matrix.h"

namespace ctl
{
	class NeuralNet
	{
	public:
		using Data = std::pair<std::vector<double>, std::vector<bool>>;

		//---------------------------------------------------------
		//--------------------Constructors-------------------------
		//---------------------------------------------------------

		NeuralNet() = default;
		NeuralNet(const NeuralNet &) = default;
		NeuralNet(NeuralNet &&) = default;

		template<typename Gen>
		NeuralNet(const std::initializer_list<size_t> &neurons, const ctl::NumVec<double, 2> &initRange, RandomGen<Gen> &rand)
			: m_neurons(neurons.begin(), neurons.end())
		{
			m_connections.reserve(m_neurons.size() - 1);

			for (auto i = m_neurons.begin(), length = m_neurons.end() - 1; i != length; ++i)
				m_connections.push_back({
					Matrix<double>({ *i, *(i + 1) }, 0.).randomize(rand, initRange),
					Matrix<double>({ 1, *(i + 1) }, 0.).randomize(rand, initRange) });
		}
		NeuralNet(const std::initializer_list<size_t> &neurons)
			: m_neurons(neurons.begin(), neurons.end())
		{
			m_connections.reserve(m_neurons.size() - 1);

			for (size_t i = 0, length = m_neurons.size() - 1; i < length; ++i)
				m_connections.push_back({ Matrix<double>(), Matrix<double>() });
		}

		//---------------------------------------------------------
		//----------------------Operators--------------------------
		//---------------------------------------------------------

		NeuralNet& operator=(const NeuralNet &) = default;
		NeuralNet& operator=(NeuralNet &&) = default;

		const auto& neurons() const { return m_neurons; }
		const auto& connections() const { return m_connections; }

		//---------------------------------------------------------
		//------------------------Methods--------------------------
		//---------------------------------------------------------

		auto& train(const Data &d, const double &learnRate)
		{
			if (d.first.size() != m_neurons.front() || d.second.size() != m_neurons.back())
				throw Log("Neural Network: train: Data is inconsistant with amount of neurons.", Log::Severity::ERR0R);

			//Calculate output and store it
			std::vector<Matrix<double>> neuronOutput;
			neuronOutput.reserve(m_neurons.size());

			neuronOutput.emplace_back(d.first.begin(), d.first.end());

			for (size_t i = 0, length = neuronOutput.capacity() - 1; i < length; ++i)
				neuronOutput.emplace_back((m_connections[i][0].dotProduct(neuronOutput[i]) + m_connections[i][1]).apply(sigmoid));


			//Calculate output error
			Matrix<double> o_Error(NumVec<size_t, 2>{ 1, m_neurons.back() });
			for (size_t i = 0; i < m_neurons.back(); ++i)
				o_Error.emplace_back(neuronOutput.back()(0, i) - d.second[i]);


			//Calculate output errors
			std::vector<Matrix<double>> output_errors;
			output_errors.reserve(m_connections.size());

			output_errors.emplace_back(std::move(o_Error));

			{
				auto iterCon = m_connections.rbegin();
				auto iterOut = output_errors.begin();

				for (; std::distance(output_errors.begin(), iterOut) + 1 < static_cast<ptrdiff_t>(output_errors.capacity()); ++iterOut, ++iterCon)
					output_errors.emplace_back((*iterCon)[0].transpose().dotProduct(*iterOut));
			}


			//Calculate delta
			{
				auto iterErrOut = output_errors.begin();
				auto iterNeuOut = neuronOutput.rbegin();
				auto iterCon	= m_connections.rbegin();

				for (; iterCon != m_connections.rend(); ++iterErrOut, ++iterNeuOut, ++iterCon)
				{
					Matrix<double> biasDelta(*iterErrOut * learnRate * *iterNeuOut * (1. - *iterNeuOut));
					Matrix<double> weightDelta(biasDelta.dotProduct((iterNeuOut + 1)->transpose()));

					//Sum delta
					(*iterCon)[0] += weightDelta;
					(*iterCon)[1] += biasDelta;
				}
			}

			return *this;
		}

		auto query(const std::vector<double> &d) const
		{
			//Check
			if (d.size() != m_neurons.front())
				throw Log("Neural Network: query: Data is inconsistant with amount of neurons.", Log::Severity::ERR0R);

			//Feedforward
			Matrix<double> pred(d.begin(), d.end());
			for (const auto& i : m_connections)
				pred = (i[1] + i[0].dotProduct(pred)).apply(sigmoid);

			return pred;
		}

		template<typename Iter, typename = typename std::enable_if_t<std::is_same_v<typename std::iterator_traits<Iter>::value_type, Data>>>
		auto cost(Iter begin, const Iter &end) const
		{
			double cost = 0.;
			for (; begin != end; ++begin)
			{
				const auto pred = query(begin->first);
				for (size_t i = 0; i < m_neurons.back(); ++i)
					cost += std::pow(pred(0, i) - begin->second[i], 2);
			}
			
			return cost;
		}

		auto& save(const std::string &fileName) const
		{
			std::ofstream file(fileName, std::ios::binary | std::ios::out);
			if (!file)
				throw Log("NeuralNet: save: file couldn't be created.", Log::Severity::ERR0R);

			for (const auto& iter : m_connections)
			{
				for (const auto& iterArr : iter)
				{
					//Store data
					file << iterArr.dim()[0] << ' ' << iterArr.dim()[1] << '\n';
					for (const auto& i : iterArr.data())
						file << i << ' ';
				}

				file << '\n';
			}

			return *this;
		}

		auto& open(const std::string &fileName)
		{
			std::ifstream file(fileName, std::ios::binary | std::ios::in);
			if (!file)
				throw Log("NeuralNet: open: file is empty or non existant.", Log::Severity::ERR0R);

			for (auto& conn : m_connections)
				for (auto& wei_bia : conn)
				{
					//Extract width and height
					NumVec<size_t, 2> dim;
					for (char i = 0; i < 2; ++i)
						file >> dim[i];

					wei_bia = Matrix<double>(dim);

					for (size_t i = 0, length = wei_bia.dim().product(); i < length; ++i)
					{
						double dat;
						file >> dat;

						wei_bia.emplace_back(std::move(dat));
					}
				}

			return *this;
		}

	private:
		using Weight_Bias = std::array<Matrix<double>, 2>;

		std::vector<Weight_Bias> m_connections;
		std::vector<size_t> m_neurons;
	};
}