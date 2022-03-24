#include <mart-common/experimental/link.h>

#include <catch2/catch.hpp>

#include <iostream>
#include <utility>

using namespace mart;

struct Foo{
	int a=0;
	int b;
	virtual int foo() { return a; }
};

// make sure this even works with multiple inheritance
struct Node : Foo, LinkTarget<2> {
	Node() = default;
	Node( int i )
		: id{i}
	{
	}
	int        id = -11;
	Link<Node> left;
	Link<Node> right;
};


TEST_CASE("connected_link_can_be_dereferenced", "[link_tests]" )
{
	Node       target{5};
	Link<Node> l;
	l.connect( target );
	CHECK( l );
	CHECK( l->id == 5 );
}

TEST_CASE("connected_link_is_disconnected_on_target_destruction", "[link_tests]" )
{
	Link<Node> l;
	{
		Node target{5};
		l.connect( target );
	}
	CHECK( !l );
}

TEST_CASE("connected_link_follows_on_move_construction", "[link_tests]" )
{
	Node target{5};
	Link<Node> l;
	l.connect( target );
	{
		Node target2{std::move(target)};
		target2.id = 1;
		CHECK( l->id == 1 );
	}
	CHECK( !l );
}

TEST_CASE("connected_link_follows_on_move_assignment", "[link_tests]" )
{
	Node       target1{5};
	Link<Node> l;
	l.connect( target1 );
	{
		Node target2{6};
		target2    = std::move( target1 );

		CHECK( l );
		CHECK( l->id == 5 );
		target2.id = 7;
		CHECK( l->id == 7 );
	}
	CHECK( !l );
}

TEST_CASE("connected_link_follows_on_swap_construction", "[link_tests]" )
{
	Node       target1{1};
	Link<Node> l1;
	l1.connect( target1 );
	{
		Node target2{2};

		Link<Node> l2;
		l2.connect( target2 );

		std::swap( target1, target2 );

		CHECK( l1->id == 1 );
		CHECK( l2->id == 2 );
	}
	CHECK( !l1 );
}

TEST_CASE("connected_link_does_not_follow_on_copy_construction", "[link_tests]" )
{
	Node       target1{1};
	Link<Node> l;
	l.connect( target1 );
	{
		Node target2{ target1 };
		target2.id = 2;
		CHECK( l->id == 1 );
	}
	CHECK( l );
}

TEST_CASE( "connected_link_does_not_follow_on_copy_assignment", "[link_tests]" )
{
	Node       target1{1};
	Link<Node> l;
	l.connect( target1 );
	{
		Node target2{2};
		target2 = target1;

		CHECK( l );
		CHECK( l->id == 1 );
		target2.id = 2;
		CHECK( l->id == 1 );
	}
	CHECK( l );
}


void check_correct_links( const std::vector<Node>& nodes )
{
	for( auto&& n : nodes ) {
		if( n.left ) {
			CHECK( n.left->id == n.id - 1 );
		}
		if( n.right ) {
			CHECK( n.right->id == n.id + 1 );
		}
	}
}


TEST_CASE( "say_hello", "[link_tests]" )
{
	std::vector<Node> nodes;
	nodes.push_back( {0} );
	nodes.push_back( {1} );
	nodes.push_back( {2} );
	nodes[0].right.connect( nodes[1] );
	nodes[1].right.connect( nodes[2] );
	nodes[1].left.connect( nodes[0] );
	nodes[2].left.connect( nodes[1] );

	check_correct_links( nodes );

	nodes.resize( 5 );
	check_correct_links( nodes );

	nodes.resize( 100 );
	check_correct_links( nodes );

	std::sort( nodes.begin(), nodes.end(), []( const auto& l, const auto& r ) { return l.id > r.id; } );
	check_correct_links( nodes );

	nodes.resize( 2 );
	check_correct_links( nodes );
}
