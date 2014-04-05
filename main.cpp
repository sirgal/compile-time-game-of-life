#include <iostream>
#include <tuple>

using std::tuple;

struct O {
    static const bool alive = false;
    static void print() { std::cout << "O"; }
};
struct X {
    static const bool alive = true;
    static void print() { std::cout << "X"; }
};

// starting level
typedef tuple<
  O, O, O, O, O,
	O, O, X, O, O,
	O, O, O, X, O,
	O, X, X, X, O,
	O, O, O, O, O
> start;

// field dimensions
const int width  = 5;
const int height = 5;
// iterations of the game
const int iterations = 20;

const int point_count = width*height;
static_assert( point_count == std::tuple_size<start>::value, "Dimension mismatch!" );

// helper functions to determine borders of a gaming field
template <int N> struct is_top {
    static const bool value = N < width;
};
template <int N> struct is_bot {
    static const bool value = N + width >= point_count;
};
template <int N> struct is_left {
    static const bool value = N % width == 0;
};
template <int N> struct is_right {
    static const bool value = (N + 1) % width == 0;
};

// count alive elements in a tuple
template <typename tuple, int N>
struct tuple_counter
{
    static const int value = std::tuple_element<N, tuple>::type::alive
                                 + tuple_counter<tuple, N-1>::value;
};

template <typename tuple>
struct tuple_counter<tuple, 0>
{
    static const int value = std::tuple_element<0, tuple>::type::alive ? 1 : 0;
};

// print the game field nicely
template <typename tuple, int N>
struct Printer {
    static void print_tuple()
    {
        Printer<tuple, N-1>::print_tuple();
        if( N % width == 0 ) std::cout << std::endl;
        std::tuple_element<N, tuple>::type::print();
    }
};

template <typename tuple>
struct Printer<tuple, 0> {
    static void print_tuple()
    {
        std::tuple_element<0, tuple>::type::print();
    }
};

// game rules that determine next point state
template <typename point, typename neighbors>
struct calc_next_point_state
{
    static const int neighbor_cnt =
            tuple_counter<neighbors, std::tuple_size<neighbors>::value - 1>::value;

    typedef typename std::conditional <
        point::alive,
        typename std::conditional <
            (neighbor_cnt > 3) || (neighbor_cnt < 2),
            O,
            X
        >::type,
        typename std::conditional <
            (neighbor_cnt == 3),
            X,
            O
        >::type
    >::type type;
};

// the main level grid
template <typename initial_state, int N>
struct level
{
    typedef typename std::tuple_element<N, initial_state>::type point;

    typedef tuple
    <
    // maybe these aren't completely correct, needs checking
    // left
    typename level<initial_state, is_left<N>::value ? (N + width - 1) : (N - 1)>::point,
    // right
    typename level<initial_state, is_right<N>::value ? (N - width + 1) : (N + 1)>::point,
    // top
    typename level<initial_state, is_top<N>::value ? (point_count - width + N) : (N - width)>::point,
    // top-left
    typename level<initial_state, (N == 0) ? (point_count - 1) : (is_left<N>::value ? (N - 1) : (is_top<N>::value ? (point_count - width + N - 1) : (N - width - 1)))>::point,
    // top-right
    typename level<initial_state, (N == (width - 1)) ? (point_count - width) : (is_right<N>::value ? (N - width * 2 + 1) : (is_top<N>::value ? (point_count - width + N + 1) : (N - width + 1)))>::point,
    // bottom
    typename level<initial_state, (N + width >= point_count) ? (N + width - point_count) : (N + width)>::point,
    // bottom-left
    typename level<initial_state, (N == (point_count - width)) ? (width - 1) : (is_left<N>::value ? (N + width * 2 - 1) : (is_bot<N>::value ? (N + width - point_count - 1) : (N + width - 1)))>::point,
    // bottom-right
    typename level<initial_state, (N == (point_count - 1)) ? (0) : ((N + 1) % width == 0 ? (N + 1) : (is_bot<N>::value ? (N + width - point_count + 1) : (N + width + 1)))>::point
    > neighbors;

    typedef typename calc_next_point_state<
	    typename level<initial_state, N>::point,
	    typename level<initial_state, N>::neighbors
    >::type next_point_state;
};

// concatenate two tuples into one
template <typename tuple_1, typename tuple_2>
struct my_tuple_cat
{
    typedef decltype(tuple_cat(std::declval<tuple_1>(), std::declval<tuple_2>())) result;
};

// get the next gaming field tuple
template <typename field, int iter>
struct next_field_state
{
    typedef typename my_tuple_cat <
		tuple< typename level<field, point_count - iter>::next_point_state >,
               typename next_field_state<field, iter-1>::next_field
	>::result next_field;
};

template <typename field>
struct next_field_state<field, 1>
{
    typedef tuple< typename level<field, point_count - 1>::next_point_state> next_field;
};

// calculate the game and print it
template <typename field, int iters>
struct game_process
{
    static void print()
    {
        Printer< field, point_count - 1 >::print_tuple();
        std::cout << std::endl << std::endl;
        game_process< typename next_field_state<field, point_count>::next_field, iters-1 >::print();
    }
};

template <typename field>
struct game_process<field, 0>
{
    static void print()
    {
        Printer< field, point_count - 1 >::print_tuple();
        std::cout << std::endl;
    }
};

int main()
{
    game_process< start, iterations >::print();
    return 0;
}
