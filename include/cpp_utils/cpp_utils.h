#ifndef CPP_UTILS_HH
#define CPP_UTILS_HH

#include <boost/range/irange.hpp>
#include <boost/tuple/tuple.hpp>

#include <sstream>
#include <iterator>
#include <type_traits>
#include <utility>

namespace std
{
    // Interoperability between boost::tuple and std::tuple.
    // Enables using boost::tuple like std::tuple in many situations,
    // like using std::get<N>() or structured bindings.
    // Unfortunately, you can not use it with std::tie.

    template<typename T, typename Tail>
    struct tuple_size<boost::tuples::cons<T, Tail>>
        : std::integral_constant<
            decltype(boost::tuples::length<boost::tuples::cons<T, Tail>>::value),
            boost::tuples::length<boost::tuples::cons<T, Tail>>::value
          >
    {};

    template<std::size_t N, typename T, typename Tail>
    struct tuple_element<N, boost::tuples::cons<T, Tail>>
    {
        using type = typename boost::tuples::
                element<int(N), boost::tuples::cons<T, Tail>>::type;
    };

    template <std::size_t N, typename T, typename Tail>
    constexpr
    typename std::tuple_element<N, boost::tuples::cons<T, Tail>>::type&&
    get(boost::tuples::cons<T, Tail>&& t)
    {
        return boost::get<N>(std::forward(t));
    }

    template <std::size_t N, typename T, typename Tail>
    constexpr
    typename std::tuple_element<N, boost::tuples::cons<T, Tail>>::type&
    get(boost::tuples::cons<T, Tail>& t)
    {
        return boost::get<N>(t);
    }

    template<std::size_t N, typename T, typename Tail>
    constexpr
    const typename std::tuple_element<N, boost::tuples::cons<T, Tail>>::type&&
    get(const boost::tuples::cons<T, Tail>&& t)
    {
        return boost::get<N>(std::forward(t));
    }

    template<std::size_t N, typename T, typename Tail>
    constexpr
    const typename std::tuple_element<N, boost::tuples::cons<T, Tail>>::type&
    get(const boost::tuples::cons<T, Tail>& t)
    {
        return boost::get<N>(t);
    }
} // namespace std

namespace cpp_utils
{

/**
 * Return the sign of a number (-1 if negative, 1 if positive, 0 if 0)
 */
template <typename T, typename = std::enable_if_t<std::is_signed<T>::value>>
int sgn(T val)
{
    return (T{0} < val) - (val < T{0});
}


template<typename... Ts> struct make_void { typedef void type;};

/// replacement for void_t from C++17
template<typename... Ts> using void_t = typename make_void<Ts...>::type;


/**
 * @brief An adaptor class to negate a unary predicate functor
 * @details We could use std::not1 instead, but it requires the Predicate to
 *          have a member typedef @c argument_type. This is unnecessarily
 *          restrictive as it prevents the predicate from overloading its
 *          operator() for multiple types.
 *
 * @tparam Predicate A predicate functor returning a type with boolean semantics
 */
template <class Predicate>
struct negator
{
    Predicate pred;

    template <typename... Args>
    decltype(auto) operator()(Args&&... args) const
    {
        return !pred(std::forward<Args>(args)...);
    }
};

/**
 * Negate a predicate functor
 */
template <class Predicate>
negator<Predicate> negate(Predicate predicate)
{
    return negator<Predicate>{predicate};
}


/**
 * Convert an unsigned number to a signed number with the same width
 */
template <class T>
inline std::make_signed_t<T> as_signed(T t)
{
    return std::make_signed_t<T>(t);
}


/**
 * Convert a signed number to an unsigned number with the same width
 */
template <class T>
inline std::make_unsigned_t<T> as_unsigned(T t)
{
    return std::make_unsigned_t<T>(t);
}


/**
 * Compile-time boolean checking if a type is equality comparable (has
 * operator==())
 */
template <typename T>
constexpr bool is_equality_comparable =
        std::is_convertible<decltype(std::declval<T>() == std::declval<T>()),
                            bool>::value &&
                std::is_convertible<decltype(std::declval<T>()
                                             != std::declval<T>()),
                                    bool>::value;

/**
 * Compile-time boolean checking if a type is comparable (has operator< and
 * operator>)
 */
template <typename T>
constexpr bool is_comparable =
        std::is_convertible<decltype(std::declval<T>() < std::declval<T>()),
                            bool>::value &&
                std::is_convertible<decltype(std::declval<T>()
                                             > std::declval<T>()),
                                    bool>::value;


/**
 * @brief Define a range of integral values between @a start and @a end with
 *     optional stepsize.
 * @details Automatically picks a type that is large enough to hold both
 *          @a start and @a end.
 */
template <typename T, typename U, typename V = int>
inline auto range(T start, U end, V step = 1)
{
    using D = std::decay_t<decltype(true ? start : end)>;
    return boost::irange<D>(start, end, step);
}

/**
 * Define a range of integral values between 0 and @a end
 */
template <typename T>
inline auto range(T end)
{
    return boost::irange(T{0}, end);
}


/**
 * @brief remove_if for (non-sequence) containers (such as map or set).
 * @details Works only for containers where iterators are not invalidated when
 *     erasing elements. This is the case for std::unordered_map and
 *     std::unordered_set since C++14.
 *
 *     std::remove_if works only for containers where elements can be reordered,
 *     because it only works on iterators. This algorithm also requires that
 *     the container has an erase function taking an iterator to the element to
 *     erase.
 *
 * @param container Container implementing begin(), end() and erase() functions
 * @param pred Unary predicate taking an argument of the @c value_type of the
 *      container (what is returned by `*(c.begin())`) and returning @c true if
 *      the element should be removed.
 */
template <typename Container,
          typename Predicate>
void remove_if(Container& container, Predicate pred)
{
    using std::begin;
    using std::end;
    for(auto it = begin(container); it != end(container);)
    {
        if(pred(*it))
        {
            it = container.erase(it);
        }
        else
        {
            ++it;
        }
    }
}


/**
 * @brief Create a construct that can be piped into like a stream and that
 *     automatically converts to a string.
 * @details Use in cases where you want to easily construct a string without
 *      explicitly creating a stringstream. Like so:
 *
 * ```cpp
 * for(auto i: range(100))
 * {
 *     std::string filename = make_string() << "output_" << i << ".txt";
 *     // write to filename ...
 * }
 * ```
 */
struct make_string
{
    std::stringstream ss;
    template <typename T>
    make_string& operator<<(const T& data)
    {
        ss << data;
        return *this;
    }
    operator std::string()
    {
        return ss.str();
    }
    operator const char*()
    {
        return ss.str().c_str();
    }
};
} // namespace cpp_utils

#endif // CPP_UTILS_HH
