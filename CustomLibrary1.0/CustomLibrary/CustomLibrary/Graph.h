#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <array>
#include <queue>

#include "Error.h"

namespace ctl
{
	class Graph1
	{
	public:
		using Node = unsigned int;
		using Edge = std::pair<double, std::array<Node, 2>>;

		Graph1() {}
		Graph1(const Node &nodes, std::vector<Edge> edges)
			: m_nodes{ std::move(nodes) }, m_vertercies{ std::vector<Vertex>(m_nodes) }, m_edges{ std::move(edges) }
		{
			for (const auto &val : m_edges)
			{
				m_vertercies[val.second[0]].push_back({ val.first, val.second[1] });
				m_vertercies[val.second[1]].push_back({ val.first, val.second[0] });
			}
		}
		Graph1(Node nodes)
			: m_nodes{ std::move(nodes) }, m_vertercies{ std::vector<Vertex>(m_nodes) } {}

		friend std::ostream& operator<<(std::ostream &out, const Graph1 &a)
		{
			out << "[( ";
			for (int it = 0; it < a.m_nodes; it++)
				out << it << ' ';
			out << ')';
			for (const auto &it : a.m_vertercies)
			{
				out << "{ ";
				for (const auto &it2 : it)
					out << it2.second << ' ';
				out << '}';
			}
			out << ']';
			return out;
		}

		void setNodes(const Node &nodeLim) { m_nodes = nodeLim; m_vertercies.resize(nodeLim); }
		void addEdge(Edge edge)
		{
			const Node m = std::max(edge.second[0], edge.second[1]);
			if (m > m_vertercies.size())
				m_vertercies.resize(m + 1);
			m_vertercies[edge.second[0]].push_back({ edge.first, edge.second[1] });
			m_vertercies[edge.second[1]].push_back({ edge.first, edge.second[0] });
			m_edges.push_back(std::move(edge));
		}

		//0 -> none, 1 -> path, 2 -> circle and path
		short eulerian() const
		{
			if (!isConnected())
				return 0;

			unsigned int odd = 0;
			for (const auto &link : m_vertercies)
				if (link.size() & 1)
					odd++;

			return odd == 0 ? 2 : odd == 2 ? 1 : 0;
		}

		bool isConnected() const
		{
			for (const auto &val : goThrough(std::vector<bool>(m_nodes)))
				if (!val)
					return false;
			return true;
		}

		double dijkstra(const Node &start, const Node &destination) const
		{
			std::priority_queue<Link, std::vector<Link>, std::greater<Link>> pq;
			std::vector<double> dist(m_nodes, HUGE_VAL);

			pq.push({ 0, start });
			dist[start] = 0;

			while (!pq.empty())
			{
				const int current = pq.top().second;
				pq.pop();
				for (const auto &x : m_vertercies[current])
					if (dist[x.second] > dist[current] + x.first)
					{
						dist[x.second] = dist[current] + x.first;
						pq.push({ dist[x.second], x.second });
					}
			}
			return dist[destination];
		}

		Graph1 minimumSpanningTree()
		{
			std::sort(m_edges.begin(), m_edges.end());
			std::vector<Node> id; id.reserve(m_nodes);
			for (size_t i = 0; i < id.capacity(); i++)
				id.push_back(i);

			Graph1 gph(m_nodes);
			static auto root = [&](Node x) { for (; id[x] != x; x = id[x]) id[x] = id[id[x]]; return x; };

			for (const auto &val : m_edges)
			{
				const Node x = root(val.second[0]), y = root(val.second[1]);
				if (x != y)
					gph.addEdge(val), id[x] = id[y];
			}

			return gph;
		}

	private:
		using Link = std::pair<double, Node>;
		using Vertex = std::vector<Link>;

		Node m_nodes;

		std::vector<Vertex> m_vertercies;
		std::vector<Edge> m_edges;

		std::vector<bool> goThrough(std::vector<bool> check, const Node &node = 0) const
		{
			check[node] = true;
			for (const auto &val : m_vertercies[node])
				if (!check[val.second])
					check = goThrough(std::move(check), val.second);
			return check;
		}
	};

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
			Graph path(m_adjList.size());

			pq.emplace(0., start);
			dist[start] = 0.;

			while (!pq.empty())
			{
				const size_t current = pq.top().second;
				pq.pop();
				visited[current] = true;

				for (const auto &x : m_adjList[current])
					if (!visited[x.second] && dist[x.second] > dist[current] + x.first)
					{
						dist[x.second] = dist[current] + x.first;
						pq.emplace(dist[x.second], x.second);
						path.m_adjList[x.second] = current;
					}
			}

			return dist;
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
	};
}