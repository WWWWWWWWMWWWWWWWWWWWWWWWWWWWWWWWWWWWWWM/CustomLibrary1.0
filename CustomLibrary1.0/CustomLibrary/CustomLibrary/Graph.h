#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <array>
#include <queue>

#include "Error.h"

namespace ctl
{
	struct Directed {};
	struct Undirected {};

	template<typename T = void>
	class Graph
	{
	public:
		//source, destination, weight
		using Edge = std::tuple<size_t, size_t, double>;
		//node
		using Vertex = size_t;

		//--------------------------------------------------------------------------
		//--------------------------------Methods-----------------------------------
		//--------------------------------------------------------------------------

	private:
		//Mark all unchecked nodes
		std::vector<bool>& _goThrough_(std::vector<bool> &check, const Vertex &nextNode = 0) const
		{
			check[nextNode] = true;
			for (auto& i : m_adjList[nextNode])
				if (!check[i.second])
					_goThrough_(check, i.second);
			return check;
		}

	public:
		bool connected() const
		{
			std::vector<bool> table(m_adjList.size(), false);
			for (const auto& i : _goThrough_(table))
				if (!i)
					return false;
			return true;
		}

		auto dijkstra(const Vertex &start) const
		{
			std::priority_queue<DestEdge, std::vector<DestEdge>, std::greater<DestEdge>> pq;
			std::vector<double> dist(m_adjList.size(), std::numeric_limits<double>::max());
			std::vector<bool> visited(m_adjList.size(), false);

			pq.emplace(0., start);
			dist[start] = 0.;

			while (!pq.empty())
			{
				const auto current = pq.top().second;
				pq.pop();
				visited[current] = true;

				for (const auto &x : m_adjList[current])
					if (!visited[x.second] && dist[x.second] > dist[current] + x.first)
					{
						dist[x.second] = dist[current] + x.first;
						pq.emplace(dist[x.second], x.second);
					}
			}

			return dist;
		}

		auto dijkstraWPath(const Vertex &start) const
		{
			std::priority_queue<DestEdge, std::vector<DestEdge>, std::greater<DestEdge>> pq;
			std::vector<double> dist(m_adjList.size(), std::numeric_limits<double>::max());
			std::vector<bool> visited(m_adjList.size(), false);

			std::vector<size_t> path(m_adjList.size());

			pq.emplace(0., start);
			dist[start] = 0.;
			path.front() = -1;

			while (!pq.empty())
			{
				const auto current = pq.top().second;
				pq.pop();
				visited[current] = true;

				for (const auto &x : m_adjList[current])
					if (!visited[x.second] && dist[x.second] > dist[current] + x.first)
					{
						dist[x.second] = dist[current] + x.first;
						pq.emplace(dist[x.second], x.second);
						path[x.second] = current;
					}
			}

			return make_pair(dist, path);
		}

#ifdef _IOSTREAM_
		friend std::ostream& operator<<(std::ostream &out, const Graph &g)
		{
			for (size_t i = 0; i < g.m_adjList.size(); ++i)
			{
				std::cout << i << " ->   ";
				for (auto& iter : g.m_adjList[i])
					std::cout << iter.second << '(' << iter.first << ")\t";
				std::cout << '\n';
			}

			return out;
		}
#endif // _IOSTREAM_

	protected:
		//--------------------------------------------------------------------------
		//-----------------------------Constructors---------------------------------
		//--------------------------------------------------------------------------

		Graph() = default;
		Graph(const Graph &) = default;
		Graph(Graph &&) = default;

		Graph(const size_t &size)
			: m_adjList(size) {}

		//destination, weight
		using DestEdge = std::pair<double, size_t>;
		std::vector<std::vector<DestEdge>> m_adjList;
	};

	template<>
	class Graph<ctl::Directed> : public Graph<>
	{
	public:
		//--------------------------------------------------------------------------
		//-----------------------------Constructors---------------------------------
		//--------------------------------------------------------------------------

		Graph() = default;
		Graph(const Graph &) = default;
		Graph(Graph &&) = default;

		Graph(const std::initializer_list<Edge> &init, const size_t &size)
			: Graph<>(size)
		{
			for (auto& i : init)
				m_adjList[std::get<0>(i)].emplace_back(std::get<2>(i), std::get<1>(i));
		}

		//--------------------------------------------------------------------------
		//--------------------------------Methods-----------------------------------
		//--------------------------------------------------------------------------

		auto& pushEdge(const Edge &e)
		{
			if (m_adjList.size() < std::get<0>(e) + 1)
				m_adjList.resize(std::get<0>(e) + 1);

			m_adjList[std::get<0>(e)].emplace_back(std::get<2>(e), std::get<1>(e));

			return *this;
		}

		Graph transpose() const
		{
			Graph out;
			out.m_adjList.resize(m_adjList.size());

			for (size_t i = 0; i < m_adjList.size(); ++i)
				for (auto& iter : m_adjList[i])
					out.m_adjList[iter.first].emplace_back(i, iter.second);

			return out;
		}
	};

	template<>
	class Graph<ctl::Undirected> : public Graph<>
	{
	public:
		//--------------------------------------------------------------------------
		//-----------------------------Constructors---------------------------------
		//--------------------------------------------------------------------------

		Graph() = default;
		Graph(const Graph &) = default;
		Graph(Graph &&) = default;

		Graph(const std::initializer_list<Edge> &init, const size_t &size)
			: Graph<>(size)
		{
			for (auto& i : init)
				m_adjList[std::get<0>(i)].emplace_back(std::get<2>(i), std::get<1>(i)),
				m_adjList[std::get<1>(i)].emplace_back(std::get<2>(i), std::get<0>(i));
		}

		//--------------------------------------------------------------------------
		//--------------------------------Methods-----------------------------------
		//--------------------------------------------------------------------------

		enum Eulerian { NONE, PATH, CIRCLE };
		Eulerian eulerian() const
		{
			if (!connected())
				return NONE;

			uint8_t odds = 0;
			for (auto& i : m_adjList)
				if (i.size() & 1)
				{
					++odds;
					if (odds > 2)
						return NONE;
				}

			switch (odds)
			{
			case 0:  return CIRCLE;
			case 2:  return PATH;
			default: return NONE;
			}
		}

		auto& pushEdge(const Edge &e)
		{
			const auto max = std::max(std::get<0>(e) + 1, std::get<1>(e) + 1);
			if (m_adjList.size() < max)
				m_adjList.resize(max);

			m_adjList[std::get<0>(e)].emplace_back(std::get<2>(e), std::get<1>(e));
			m_adjList[std::get<1>(e)].emplace_back(std::get<2>(e), std::get<0>(e));

			return *this;
		}

		auto minimumSpanningTree() const
		{
			std::priority_queue<DestEdge, std::vector<DestEdge>, std::greater<DestEdge>> pq;
			std::vector<double> key(m_adjList.size(), std::numeric_limits<double>::max());
			std::vector<bool> inMST(m_adjList.size(), false);

			std::vector<DestEdge> parent(m_adjList.size());

			pq.emplace(0., 0);
			key[0] = 0.;

			while (!pq.empty())
			{
				const auto current = pq.top().second;
				pq.pop();

				inMST[current] = true;

				for (auto& i : m_adjList[current])
					if (!inMST[i.second] && key[i.second] > i.first)
					{
						key[i.second] = i.first;
						pq.emplace(key[i.second], i.second);
						parent[i.second] = { i.first, current };
					}
			}

			Graph mst;
			for (size_t i = 1; i < parent.size(); ++i)
				mst.pushEdge({ i, parent[i].second, parent[i].first });

			return mst;
		}
	};
}