#include <mart-common/experimental/tmp.h>
#include <tuple>

namespace {

constexpr auto v_list1 = mart::tmp::value_list<int, 1, 2, 3, 4, 5>{};
constexpr auto v_list2 = mart::tmp::value_list<char, 'H', 'e', 'l', 'l', 'o'>{};

static_assert( mart::tmp::get_Nth_element( 2, v_list1 ) == 3 );

template<int First, char Second>
struct Type {
	static constexpr int  first  = First;
	static constexpr char second = Second;
};

template<class... T>
struct type_list
{};

auto mtype = mart::tmp::cartesian_value_product<std::tuple,int,char, Type>( v_list1, v_list2 );

} // namespace
