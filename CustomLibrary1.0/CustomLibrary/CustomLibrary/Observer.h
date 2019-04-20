#pragma once

#include <vector>
#include <algorithm>

namespace ctl
{
	template<typename ...T>
	struct ObSu
	{
		class Observer
		{
		public:
			virtual void update(const T&...) = 0;
		};

		class Subject
		{
		public:
			Subject& attach(Observer &o) { m_list.emplace_back(&o); return *this; }
			Subject& detach(Observer &o) { m_list.erase(std::remove(m_list.begin(), m_list.end(), &o), m_list.end()); return *this; }

			Subject& notify(const T&... para)
			{
				for (auto& i : m_list)
					i->update(para...);

				return *this;
			}

		private:
			std::vector<Observer*> m_list;
		};
	};
}