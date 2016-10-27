#pragma once
#include <algorithm>


namespace mart {

template<class C>
void sort(C& c)
{
	std::sort(c.begin(), c.end());
}

template<class C, class V>
auto find(C& c, const V& value) -> decltype(c.begin())
{
	return std::find(c.begin(), c.end(),value);
}

template<class C, class UnaryPredicate>
auto find_if(C& c, UnaryPredicate p) -> decltype(c.begin())
{
	return std::find_if(c.begin(), c.end(), p);
}

}
