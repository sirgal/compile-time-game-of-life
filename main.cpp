#include <iostream>
#include <tuple>

using namespace std;

struct O { };   // dead
struct X { };   // alive

const int dimension  = 5;
const int iterations = 5; //zero-indexed, actually

using start = tuple<
                    O, O, O, O, O,
                    O, O, X, O, O,
                    O, O, O, X, O,
                    O, X, X, X, O,
                    O, O, O, O, O
                    >;

template <typename T>
constexpr bool is_alive();

template<>
constexpr bool is_alive<O>()
{ return false; }

template<>
constexpr bool is_alive<X>()
{ return true; }

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

template <typename Type>
void print();

template<>
void print<O>()
{ cout << "O"; }

template<>
void print<X>()
{ cout << "X"; }

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

template <typename Point, typename neighbors>
struct NextPointState
{
    constexpr static int neighbor_count = 8 - 1;

    using type =
        typename conditional <
            is_alive<Point>(),
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

template <int dim, typename initial_state>
struct Field
{
    static_assert( dim*dim == tuple_size<initial_state>::value, "Dimension mismatch!" );

    constexpr static int point_count = dim*dim;

    using points = initial_state;

    template <int N>
    using point = typename tuple_element<N, points>::type;

    template <int N>
    constexpr bool alive()
    { return is_alive<point<N>>(); }

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
    using next_point_state = typename NextPointState<point<N>, neighbors<N>>::type;
};

template <typename... tuples>
struct my_tuple_cat
{
    typedef decltype( tuple_cat( declval<tuples>() ... ) ) type;
};

template<int dim, typename field, int iter>
struct next_field_state
{
    template<int N>
    using point = Field<dim, field>::next_point_state<N>;

    using next_state = typename my_tuple_cat< tuple< point<dim*dim - iter> >, typename next_field_state<dim, field, iter-1>::next_state >::type;
};

template<int dim, typename field>
struct next_field_state<dim, field, 1>
{
    template<int N>
    using point = Field<dim, field>::next_point_state<N>;

    using next_state = tuple< point<dim*dim - 1> >;
};

template <int dim, typename field, int iters>
struct game_process
{
    using next_state = typename game_process<dim, typename next_field_state<dim, field, dim*dim>::next_state, iters-1>::next_state;
};

template <int dim, typename field>
struct game_process<dim, field, 0>
{
    using next_state = typename next_field_state<dim, field, dim*dim>::next_state;
};

int main()
{
    Printer<game_process< dimension, start, iterations >::next_state, dimension, dimension*dimension - 1>::print_tuple();

    cout << endl;

    return 0;
}
