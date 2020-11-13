#pragma once
#include "..\TKernel\TKernel.h"
#include "resource.h"

namespace Tool
{
	BOOL FormatDataUnit(LPTSTR lpText, size_t size, int64_t qwTotalByBytes);
	BOOL FormatSpeedUnit(LPTSTR lpText, size_t size, int64_t qwSpeedByBytes);
	BOOL FormatTimeUnit(LPTSTR lpText, size_t size, int64_t qwTimeByS);

	template <typename Item, INT max_size>
	class MonotonousQueue //无需下标的单调队列
	{
		std::deque<Item> origin;
		std::deque<Item> maxDeq;
		std::deque<Item> minDeq;

	public:
		VOID push(const Item x)
		{
			origin.push_back(x);
			while (!maxDeq.empty() && maxDeq.back() < x) maxDeq.pop_back();
			while (!minDeq.empty() && minDeq.back() > x) minDeq.pop_back();
			maxDeq.push_back(x);
			minDeq.push_back(x);
		}
		VOID pop()
		{
			if (maxDeq.front() == origin.front()) maxDeq.pop_front();
			if (minDeq.front() == origin.front()) minDeq.pop_front();
			origin.pop_front();
		}
		VOID pop_until()
		{
			while (size() >= max_size) pop();
		}

		size_t size()
		{
			return origin.size();
		}
		Item max_element()
		{
			pop_until();
			return maxDeq.front();
		}
		Item min_element()
		{
			pop_until();
			while (size() >= max_size) pop();
			return minDeq.front();
		}

		Item& operator[] (size_t index)
		{
			return origin[index];
		}
	};

	BOOL IsForegroundFullscreen();
}