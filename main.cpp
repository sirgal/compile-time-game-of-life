#include <iostream>
#include <tuple>

using namespace std;

struct O { };   // dead
struct X { };   // alive

const int dimension  = 5; // height\length of the field
const int iterations = 5; // begin from 0

// starting level
using start = tuple<
                    O, O, O, O, O,
                    O, O, X, O, O,
                    O, O, O, X, O,
                    O, X, X, X, O,
                    O, O, O, O, O
                    >;

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
constexpr bool is_top( int dim, int N ) {
    return (N - dim) < 0;
}
constexpr bool is_bot( int dim, int N ) {
    return (N + dim) >= dim*dim;
}
constexpr bool is_left( int dim, int N ) {
    return N % dim == 0;
}
constexpr bool is_right( int dim, int N ) {
    return (N + 1) % dim == 0;
}

// count alive elements in a tuple
template <typename tuple, int N>
struct tuple_counter
{
    constexpr static int value = is_alive<typename tuple_element<N, tuple>::type>() + tuple_counter<tuple, N-1>::value;
};

template <typename tuple>
struct tuple_counter<tuple, 0>
{
    constexpr static int value = is_alive<typename tuple_element<0, tuple>::type>();
};

// print the game field nicely
template <typename tuple, int dim, int N>
struct Printer {
    static void print_tuple()
    {
        Printer<tuple, dim, N-1>::print_tuple();
        if( N % dim == 0 ) cout << endl;
        print<typename tuple_element<N, tuple>::type>();
    }
};

template <typename tuple, int dim>
struct Printer<tuple, dim, 0> {
    static void print_tuple()
    {
        print<typename tuple_element<0, tuple>::type>();
    }
};

// game rules that determine next point state
template <typename point, typename neighbors>
struct calc_next_point_state
{
    constexpr static int neighbor_count = 8 - 1;

    using type =
        typename conditional <
            is_alive<point>(),
            typename conditional <
                (tuple_counter<neighbors, neighbor_count>::value > 3)
                    || (tuple_counter<neighbors, neighbor_count>::value < 2),
                O,
                X
            >::type,
            typename conditional <
                (tuple_counter<neighbors, neighbor_count>::value == 3),
                X,
                O
            >::type
        >::type;
};

// the main level grid
template <int dim, typename initial_state>
struct level
{
    static_assert( dim*dim == tuple_size<initial_state>::value, "Dimension mismatch!" );

    constexpr static int point_count = dim*dim;

    template <int N>
    using point = typename tuple_element<N, initial_state>::type;

    template <int N>
    using neighbors = tuple
    <
    // left
    point< is_left(dim, N) ? (N + dim - 1) : (N - 1) >,
    // right
    point< is_right(dim, N) ? (N - dim + 1) : (N + 1) >,
    // top
    point< is_top(dim, N) ? (point_count - dim + N) : (N - dim) >,
    // top-left
    point< (N == 0) ? (point_count - 1) : (is_left(dim, N) ? (N - 1) : ( is_top(dim, N) ? (point_count - dim + N - 1) : (N - dim - 1)) ) >,
    // top-right
    point< (N == (dim-1)) ? (point_count - dim) : (is_right(dim, N) ? (N - dim*2 + 1) : ( is_top(dim, N) ? (point_count - dim + N + 1) : (N - dim + 1)) ) >,
    // bottom
    point< (N + dim >= point_count) ? (N + dim - point_count) : (N + dim) >,
    // bottom-left
    point< (N == (point_count - dim)) ? (dim - 1) : (is_left(dim, N) ? (N + dim*2 - 1) : (is_bot(dim, N) ? (N + dim - point_count - 1) : (N + dim - 1))) >,
    // bottom-right
    point< (N == (point_count - 1)) ? (0) : ((N+1) % dim == 0 ? (N+1) : (is_bot(dim, N) ? (N + dim - point_count + 1) : (N+dim+1)) )>
    >;

    template <int N>
    using next_point_state = typename calc_next_point_state<point<N>, neighbors<N>>::type;
};

// concatenate multiple tuples into one
template <typename... tuples>
struct my_tuple_cat
{
    typedef decltype( tuple_cat( declval<tuples>() ... ) ) type;
};

// get the next gaming field tuple
template<int dim, typename field, int iter>
struct next_field_state
{
    template<int N>
    using point = level<dim, field>::next_point_state<N>;

    using next_field = typename my_tuple_cat< tuple< point<dim*dim - iter> >, typename next_field_state<dim, field, iter-1>::next_field >::type;
};

template<int dim, typename field>
struct next_field_state<dim, field, 1>
{
    template<int N>
    using point = level<dim, field>::next_point_state<N>;

    using next_field = tuple< point<dim*dim - 1> >;
};

// calculate next game field 'iter' times based on 'field' with 'dim' height
template <int dim, typename field, int iters>
struct game_process
{
    using result = typename game_process<dim, typename next_field_state<dim, field, dim*dim>::next_field, iters-1>::result;
};

template <int dim, typename field>
struct game_process<dim, field, 0>
{
    using result = typename next_field_state<dim, field, dim*dim>::next_field;
};

// prints the resulting tuple
int main()
{
    Printer<game_process< dimension, start, iterations >::result, dimension, dimension*dimension - 1>::print_tuple();

    cout << endl;

    return 0;
}
