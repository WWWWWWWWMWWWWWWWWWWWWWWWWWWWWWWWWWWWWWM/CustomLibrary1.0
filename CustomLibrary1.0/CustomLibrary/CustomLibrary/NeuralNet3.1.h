#ifndef NEURALNET31
#define NEURALNET31

#include <vector>
#include <cmath>
#include <fstream>
#include <string>
#include <iostream>

#include "RandomGenerator.h"
#include "MatrixMath2.1.h"
#include "Error.h"

struct DataPackage
{
	std::vector<double> d_values;
	std::vector<bool> d_classification;
	DataPackage(const std::vector<double> &values, const std::vector<bool> &classification)
		: d_values{ values }, d_classification{ classification } {}
};

void sigmoid(double &x)
{
	x = 1 / (1 + std::exp(-x));
}

class Agent;

class NeuralNet
{
private:

	RandomGenerator<> *m_RandNum;

	const std::vector<int> m_Neurons;

	enum Element
	{
		WEIGHT,
		BIAS
	};
	struct Weight_Bias
	{
		Matrix<double> s_Weight_Bias[2];

		Weight_Bias(Matrix<double> &&w, Matrix<double> &&b)
			: s_Weight_Bias{ std::move(w), std::move(b) } {}
		Weight_Bias(const Matrix<double> &w, const Matrix<double> &b)
			: s_Weight_Bias{ w, b } {}
	};
	std::vector<Weight_Bias> m_Connections;

public:

	//----------------------------------Constructor & Destructor----------------------------------

	NeuralNet(const std::vector<int> &neurons, RandomGenerator<> *randNum, const char &minRand = 0, const char &maxRand = 1)
		: m_Neurons{ neurons }, m_RandNum{ randNum }
	{
		//Check for Zero
		for (const auto &val : neurons)
			if (val <= 0)
				throw Error("NeuralNet Constructor -> neurons have a zero value.");

		//Allocate memory to connections & randomize
		m_Connections.reserve(m_Neurons.size() - 1);
		for (short i = 0; i < m_Connections.capacity(); i++)
			m_Connections.push_back(std::move(Weight_Bias(std::move(Matrix<double>(m_Neurons[i], m_Neurons[i + 1]).randomizeAll(*m_RandNum, minRand, maxRand)), std::move(Matrix<double>(1, m_Neurons[i + 1]).randomizeAll(*m_RandNum, minRand, maxRand)))));
			
	}

	NeuralNet(NeuralNet &&net)
		: m_Neurons{ std::move(net.m_Neurons) }, m_Connections{ std::move(net.m_Connections) }, m_RandNum{ std::move(net.m_RandNum) } {}

	NeuralNet(const NeuralNet &net)
		: m_Neurons{ net.m_Neurons }, m_Connections{ net.m_Connections }, m_RandNum{ net.m_RandNum } {}

	//---------------------------------------Getter & Setter--------------------------------------

	const std::vector<int>& getNeuronsNum() const { return m_Neurons; }

	double& setConnectionWeight(const short &layer, const int &x, const int &y) { return m_Connections[layer].s_Weight_Bias[WEIGHT].accessLoc(x, y); }
	double& setConnectionBias(const short &layer, const int &y) { return m_Connections[layer].s_Weight_Bias[BIAS].accessLoc(y); }

	//--------------------------------------Function Methods--------------------------------------

	NeuralNet& backProp(const std::vector<DataPackage> &data, const bool &log = false, const double &learnlimit = 0.001, const double &learningRate = 0.2, const double &momentum = 0.001)
	{

		for (const auto &val : data)
			if (val.d_values.size() != m_Neurons.front() || val.d_classification.size() != m_Neurons.back())
				throw Error("NeuralNet BackProp -> Too many or not enough inputs / outputs.");

		std::vector<Weight_Bias> prev_DeltaConnections(m_Connections);
		std::reverse(std::begin(prev_DeltaConnections), std::end(prev_DeltaConnections));

		for (unsigned int i = 0; true; i++)
		{
			const int index = m_RandNum->randNumber<int>(0, data.size() - 1);

			//Calculate and store sigmoid output
			std::vector<Matrix<double>> neuronOutput;
			neuronOutput.reserve(m_Neurons.size());
			neuronOutput.push_back(data[index].d_values);
			for (short i = 0; i < neuronOutput.capacity() - 1; i++)
				neuronOutput.push_back(std::move((m_Connections[i].s_Weight_Bias[WEIGHT].matrixProd(neuronOutput[i]) + m_Connections[i].s_Weight_Bias[BIAS]).modifyEach(sigmoid)));

			//Output Error Calculation
			Matrix<double> o_Error(1, m_Neurons.back());
			for (char i = 0; i < m_Neurons.back(); i++)
				o_Error.accessLoc(i) = data[index].d_classification[i] - neuronOutput.back().accessLoc(i);

			//Delta Calculation
			std::vector<Weight_Bias> delta; delta.reserve(m_Connections.size());
			for (short i = 0; i < delta.capacity(); i++)
			{
				Matrix<double> error(o_Error);
				for (short c = 0; c < i; c++)
					error = m_Connections[m_Connections.size() - c - 1].s_Weight_Bias[WEIGHT].transpose().matrixProd(error);
				Matrix<double> deltaBias(std::move(error * learningRate * (neuronOutput[neuronOutput.size() - i - 1] * (-neuronOutput[neuronOutput.size() - i - 1] + 1))));
				Matrix<double> deltaWeight(std::move(deltaBias.matrixProd(neuronOutput[neuronOutput.size() - i - 2].transpose())));
				delta.push_back(std::move(Weight_Bias(std::move(deltaWeight), std::move(deltaBias))));
			}

			//Adding up
			for (short i = 0; i < m_Connections.size(); i++) //Improve the values
				for (char b = 0; b < 2; b++) //Weight and Bias
					m_Connections[i].s_Weight_Bias[b] += delta[delta.size() - i - 1].s_Weight_Bias[b] + (prev_DeltaConnections[delta.size() - i - 1].s_Weight_Bias[b] * momentum);

			//Store for next iteration
			prev_DeltaConnections = std::move(delta);
			
			//Progress Check
			if (i % 1000 == 0)
			{
				double cost = 0.0;
				for (short b = 0; b < data.size(); b++)
					for (char i = 0; i < m_Neurons.back(); i++)
						cost += std::pow(getPred(data[b]).accessLoc(i) - (double)data[b].d_classification[i], 2);
				i = 0;
				if (log)
					std::cout << cost << "\n";
				//Exit
				if (cost < learnlimit)
					return *this;
			}
		}
	}

	Matrix<double> getPred(const DataPackage &data)
	{
		//Check
		if (data.d_values.size() != m_Neurons.front()) 
			throw Error("NeuralNet getPred -> Too many / not enough inputs.");

		//Feedforward
		Matrix<double> pred(data.d_values);
		for (short i = 0; i < m_Connections.size(); i++)
			pred = std::move((m_Connections[i].s_Weight_Bias[WEIGHT].matrixProd(pred) + m_Connections[i].s_Weight_Bias[BIAS]).modifyEach(sigmoid));

		return pred;
	}

	NeuralNet& save(const std::string &fileName)
	{
		//Create File
		std::ofstream file(fileName + ".nn");
		if (!file)
			throw Error("NeuralNet save -> File can't be created.");

		//Write in File (Formated)
		for (const auto &val : m_Connections)
			file << val.s_Weight_Bias[WEIGHT] << "\n" << val.s_Weight_Bias[BIAS] << "\n";

		return *this;
	}

	NeuralNet& open(const std::string &fileName)
	{
		//Open File
		std::ifstream file(fileName + ".nn");
		if (!file) 
			throw Error("NeuralNet open -> File doesn't exist.");

		//Extract Contents for each Layer for each Pair
		std::string content[2];
		for (auto &val : m_Connections)
			for (auto &val2 : val.s_Weight_Bias)
			{
				for (char i = 0; i < 2; i++) //2 for Column and Row
					file >> content[i];
				val2 = Matrix<double>(std::stoi(content[0]), std::stoi(content[1]));

				for (int i = 0; i < val2.size(); i++)
				{
					file >> content[0];
					val2.accessLoc(i) = std::stod(content[0]);
				}
			}
		return *this;
	}

	//------------------------------------Operator Overloads-------------------------------------

	NeuralNet& operator=(const NeuralNet &val)
	{
		if (m_Neurons == val.m_Neurons)
			throw Error("NeuralNet copying -> Number of Neurons not the same.");

		m_Connections = val.m_Connections;

		return *this;
	}

	NeuralNet& operator=(NeuralNet &&val)
	{
		if (m_Neurons == val.m_Neurons)
			throw Error("NeuralNet moving -> Number of Neurons not the same.");

		m_Connections = std::move(val.m_Connections);

		return *this;
	}

	friend std::vector<Agent> geneticAlgorithm(RandomGenerator<> *randNum, std::vector<Agent> &, const char &);

}; //End of NeuralNet

//-------------------------------------------------------------------------------------------
//-------------------------------------Genetic Algorithm-------------------------------------
//-------------------------------------------------------------------------------------------

//Single Agent Object
class Agent
{

private:

	NeuralNet s_brain;
	double s_fitness = 0;

public:

	NeuralNet & getBrain() { return s_brain; }
	const double& getFitness() const { return s_fitness; }

	Agent(RandomGenerator<> *randNum, const std::vector<int> &neurons)
		: s_brain{ neurons, randNum, -1 } {}

	Agent(const Agent &val)
		: s_brain{ val.s_brain }, s_fitness{ val.s_fitness } {}

	Agent(Agent &&val)
		: s_brain{ std::move(val.s_brain) }, s_fitness{ std::move(val.s_fitness) } {}

	Agent& increaseFit(const double &size)
	{
		s_fitness += size;
		if (s_fitness < 0)
			s_fitness = 0;
		return *this;
	}

	Agent& operator=(const Agent &val)
	{
		s_brain = val.s_brain;
		s_fitness = val.s_fitness;
		return *this;
	}

	Agent& operator=(Agent &&val)
	{
		s_brain = std::move(val.s_brain);
		s_fitness = std::move(s_fitness);
		return *this;
	}
};

//Select & Breed & Mutate
std::vector<Agent> geneticAlgorithm(RandomGenerator<> *randNum, std::vector<Agent> &population, const char &mutation = 20)
{
	//New Generation
	std::vector<Agent> newPopulation;
	newPopulation.reserve(population.size());

	//Get totalFitness for roulette
	long double totalFitness = 0;
	for (const auto &val : population)
		totalFitness += val.getFitness();

	for (short d = 0; d < population.size(); d++)
	{
		//Select Parents
		std::vector<Agent> parents;
		parents.reserve(2);

		//2 for parents; c for index of population; b for Iterator of fitness by population
		for (char i = 0; i < 2; i++)
			for (double c = 0, b = 0, randIndex = randNum->randNumber<int>(0, totalFitness); true; c++)
			{
				b += population[c].getFitness();
				if (b >= randIndex)
				{
					parents.push_back(population[c]);
					break;
				}
			}

		//Child product
		Agent child(std::move(parents.front()));

		//mutate
		const int mutationChance = mutation;

		for (short c = 0; c < child.getBrain().m_Connections.size(); c++) //Each layer
			for (char b = 0; b < 2; b++) //Weight & Bias
				for (int i = 0; i < child.getBrain().m_Connections[c].s_Weight_Bias[b].size(); i++) //Each val in matrix
				{
					if ((bool)randNum->randNumber<char>(0, 1))
						child.getBrain().m_Connections[c].s_Weight_Bias[b].accessLoc(i) = std::move(parents.back().getBrain().m_Connections[c].s_Weight_Bias[b].accessLoc(i));
					if (randNum->randNumber<char>(0, 100) <= mutationChance)
						child.getBrain().m_Connections[c].s_Weight_Bias[b].accessLoc(i) += randNum->randNumber<double>(-1, 1);
				}

		newPopulation.push_back(std::move(child));
	}

	return newPopulation;
}

#endif // !NEURALNET31