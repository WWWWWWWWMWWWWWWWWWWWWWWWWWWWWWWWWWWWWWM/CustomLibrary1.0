#pragma once

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

			for (size_t i = 0; i < m_connections.capacity(); ++i)
				m_connections.push_back({
					Matrix<double>({ m_neurons[i], m_neurons[i + 1] }, 0.).randomize(rand, initRange),
					Matrix<double>({ 1, m_neurons[i + 1] }, 0.).randomize(rand, initRange) });
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

		auto& train(const Data &d)
		{


			return *this;
		}

	private:
		using Weight_Bias = std::array<Matrix<double>, 2>;

		std::vector<Weight_Bias> m_connections;
		const std::vector<size_t> m_neurons;
	};
}