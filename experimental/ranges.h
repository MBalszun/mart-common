#pragma once
#include "../ranges.h"



namespace mart {
namespace experimental {

template<class It>
struct range {
	It _begin;
	It _end;
	constexpr It begin() const { return _begin; }
	constexpr It end() const { return _end; }
};

template<class R>
auto view_reversed(R&& r) -> range<std::reverse_iterator<decltype(r.begin())>> {
	using Rit = std::reverse_iterator<decltype(r.begin())>;
	return{ Rit{ r.end() },Rit{ r.begin() } };
}

}
}