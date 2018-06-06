#include <functional>
#include <vector>

namespace {

const std::vector<std::vector<int>> test_ranges{
	{},
	{1},
	{-1, 2, 3, 4, 5, 6, 7, 8},
	{-1, 1},
	{1, 1},
	{1, 1, 0, 1, 1},
	{2, 1, 1, 0, 1, 1},
	{2, 1, 1, 0, 3, 1, 1, 2, 4},
	{},
	{0, 1, 0},
	{5, 5, 5},
	{3, 5, 6, 3, 5, 7, 5, 7, 5, 5},
	{1, 1, 1, 1},
	{0, 1, 2, 3, 4},
	{0, -1, -2, -3, -4},
	{0, 3998, 345, -233, -244, 55, 611, 2445, -233, 0, 0},
};

using unary_pred_func_t = bool( int );

const std::vector<unary_pred_func_t*> unary_preds = {[]( int ) { return true; },
													 []( int ) { return false; },
													 []( int l ) { return l == 0; },
													 []( int l ) { return l != 0; },
													 []( int l ) { return l % 2 == 0; }};

using binary_pred_func_t = bool( int, int );

const std::vector<binary_pred_func_t*> binary_preds = {[]( int, int ) { return true; },
													   []( int, int ) { return false; },
													   []( int l, int r ) { return l == r; },
													   []( int l, int r ) { return l != r; },
													   []( int l, int r ) { return l % 2 == r % 2; }};

using comp_func_t					  = bool( int, int );
const std::vector<comp_func_t*> comps = {[]( int l, int r ) { return l < r; },
										 []( int l, int r ) { return l > r; },
										 []( int l, int r ) { return l / 4 < r / 4; }};

using siso_trafo_func_t = int( int );
const std::vector<siso_trafo_func_t*> siso_trafos
	= {[]( int ) { return 1; }, []( int i ) { return i * 2; }, []( int i ) { return i - 2; }};

using miso_trafo_func_t = int( int, int );
const std::vector<miso_trafo_func_t*> miso_trafos
	= {[]( int, int ) { return 1; }, []( int i, int l ) { return i * l; }, []( int l, int r ) { return l - r; }};

const std::vector<std::function<int()>> generators = {[]() { return 5; },
													  [i = 0]() mutable { return i++; },
													  [i = 1]() mutable {
														  i *= 2;
														  return i;
													  }};

} // namespace
