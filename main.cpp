#include <iostream>
#include <tuple>

using namespace std;

struct O { };   // dead
struct X { };   // alive

// starting level
using start = tuple<
                    O, O, O, O, O,
                    O, O, X, O, O,
                    O, O, O, X, O,
                    O, X, X, X, O,
                    O, O, O, O, O
                    >;
// field dimensions
const int width  = 5;
const int height = 5;
// iterations of the game
const int iterations = 20;

const int point_count = width*height;
static_assert( point_count == tuple_size<start>(), "Dimension mismatch!" );

// helper functions to determine whether the point is alive or dead
template <typename T>
constexpr bool is_alive();

template<>
constexpr bool is_alive<O>()
{ return false; }

template<>
constexpr bool is_alive<X>()
{ return true; }

// print O or X, based on element type
template <typename Type>
void print();

template<>
void print<O>()
{ cout << "O"; }

template<>
void print<X>()
{ cout << "X"; }

// helper functions to determine borders of a gaming field
constexpr bool is_top( int N )
{ return N < width; }
constexpr bool is_bot( int N )
{ return (N + width) >= point_count; }
constexpr bool is_left( int N )
{ return N % width == 0; }
constexpr bool is_right( int N )
{ return (N + 1) % width == 0; }

// count alive elements in a tuple
template <typename tuple, int N>
struct tuple_counter
{
    constexpr static int value = is_alive<typename tuple_element<N, tuple>::type>()
                                 + tuple_counter<tuple, N-1>::value;
};

template <typename tuple>
struct tuple_counter<tuple, 0>
{
    constexpr static int value = is_alive<typename tuple_element<0, tuple>::type>();
};

// print the game field nicely
template <typename tuple, int N>
struct Printer {
    static void print_tuple()
    {
        Printer<tuple, N-1>::print_tuple();
        if( N % width == 0 ) cout << endl;
        print<typename tuple_element<N, tuple>::type>();
    }
};

template <typename tuple>
struct Printer<tuple, 0> {
    static void print_tuple()
    {
        print<typename tuple_element<0, tuple>::type>();
    }
};

// game rules that determine next point state
template <typename point, typename neighbors>
struct calc_next_point_state
{
    constexpr static int neighbor_cnt =
            tuple_counter<neighbors, tuple_size<neighbors>() - 1>::value;

    using type =
        typename conditional <
            is_alive<point>(),
            typename conditional <
                (neighbor_cnt > 3) || (neighbor_cnt < 2),
                O,
                X
            >::type,
            typename conditional <
                (neighbor_cnt == 3),
                X,
                O
            >::type
        >::type;
};

// the main level grid
template <typename initial_state>
struct level
{
    template <int N>
    using point = typename tuple_element<N, initial_state>::type;

    template <int N>
    using neighbors = tuple
    <
    // maybe these aren't completely correct, needs checking
    // left
    point< is_left(N) ? (N + width - 1) : (N - 1) >,
    // right
    point< is_right(N) ? (N - width + 1) : (N + 1) >,
    // top
    point< is_top(N) ? (point_count - width + N) : (N - width) >,
    // top-left
    point< (N == 0) ? (point_count - 1) : (is_left(N) ? (N - 1) : ( is_top(N) ? (point_count - width + N - 1) : (N - width - 1)) ) >,
    // top-right
    point< (N == (width-1)) ? (point_count - width) : (is_right(N) ? (N - width*2 + 1) : ( is_top(N) ? (point_count - width + N + 1) : (N - width + 1)) ) >,
    // bottom
    point< (N + width >= point_count) ? (N + width - point_count) : (N + width) >,
    // bottom-left
    point< (N == (point_count - width)) ? (width - 1) : (is_left(N) ? (N + width*2 - 1) : (is_bot(N) ? (N + width - point_count - 1) : (N + width - 1))) >,
    // bottom-right
    point< (N == (point_count - 1)) ? (0) : ((N+1) % width == 0 ? (N+1) : (is_bot(N) ? (N + width - point_count + 1) : (N+width+1)) )>
    >;

    template <int N>
    using next_point_state = typename calc_next_point_state<point<N>, neighbors<N>>::type;
};

// concatenate two tuples into one
template <typename tuple_1, typename tuple_2>
struct my_tuple_cat
{
    using result = decltype( tuple_cat( declval<tuple_1>(), declval<tuple_2>()  ) );
};

// get the next gaming field tuple
template <typename field, int iter>
struct next_field_state
{
    template<int N>
    using point = typename level<field>::template next_point_state<N>;

    using next_field = typename my_tuple_cat <
                                    tuple< point<point_count - iter> >,
                                    typename next_field_state<field, iter-1>::next_field
                                >::result;
};

template <typename field>
struct next_field_state<field, 1>
{
    template<int N>
    using point = typename level<field>::template next_point_state<N>;

    using next_field = tuple< point<point_count - 1> >;
};

// calculate the game and print it
template <typename field, int iters>
struct game_process
{
    static void print()
    {
        Printer< field, point_count - 1 >::print_tuple();
        cout << endl << endl;
        game_process< typename next_field_state<field, point_count>::next_field, iters-1 >::print();
    }
};

template <typename field>
struct game_process<field, 0>
{
    static void print()
    {
        Printer< field, point_count - 1 >::print_tuple();
        cout << endl;
    }
};

int main()
{
    game_process< start, iterations >::print();

    return 0;
}
