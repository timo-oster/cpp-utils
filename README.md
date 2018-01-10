# cpp_utils: A collection of helpful utilities for C++

This is a collection of largely independent small utilities for programming in
C++.

It includes, among others, the following tools:

## `range()` function

This allows for easier usage of `boost::irange` to iterate over a range of
integer numbers. It takes either one parameter (the end of the range), two
parameters (start and end), or three parameters (start, end, stepsize).
The range describes an interval including start but not including end.

The advantage over `boost::irange` is the automatic type deduction. When
specifying start and end of the range, a type that is large enough to hold
both numbers is automatically chosen. This allows to write code like this

```cpp
some_arcane_sequence_type c{};
for(auto i: range(1, c.size()))
{
	// do something with c[i]...
}
```

instead of

```cpp
some_arcane_sequence_type c{};
for(auto i = decltype(c.size()){1}; i < c.size(); ++i)
{
	// do something with c[i]...
}
```

and still get warning-free code without having to look up what type `c.size()`
returns.

Be careful though. This does not save you from the dangers of unsigned overflow.
In particular, do not try to use something like `range(c.size()-1, -1, -1)` to
iterate backwards over a sequence where the `size()` function returns a
`std::size_t`. Instead, use `boost::adaptors::reverse(range(c.size()))`.

## `make_string()` to generate a string using the interface for streams

This allows for generating strings from e.g. text and numbers inline
without declaring a temporary `stringstream` or using something like `sprintf`.

Example:

```cpp
// n_iterations is the current iteration number of some algorithm
std::fostream out(make_string() << "output_file_" << n_iterations << ".txt");
// write to out
```

This works because make_string is a class that holds a stringstream, which you
can stream into with `operator<<()`, and conversion operators to `std::string`
and `const char*`, which allow it to be used in place of those types.

## `remove_if` for containers with non-reorderable elements

This applies mainly to `std::(unordered_)set` and `std::(unordered_)map` and the
like. The standard `std::remove_if` works by reordering elements of a sequence,
as it only knows the `begin()` and `end()` iterators. This version does not work
on iterators, but calls the `erase()` function for every element that matches
the predicate. It is assumed that iterators are not invalidated when calling
`erase()` on the container. This guaranteed for `std::unordered_set` and
`std::unordered_map` since C++14.

Example:

```cpp
auto uset = std::unordered_set<int32_t>{};
boost::insert(uset, range(10));
// remove all odd numbers
remove_if(uset, [](int32_t i){ return i%2==1; });
```

## `negate()` for negating the effect of a predicate

A predicate is a functor whose `operator()` returns a boolean value.
`negate(predicate)` returns a functor that returns the negated version of the
predicate.

This is made obsolete in C++17 by `std::not_fn`, which has an uglier name.

## `as_signed()` and `as_unsigned()`

These transform an integer number into the corresponding signed (unsigned) type
of the same width. Use this in situations where you want to make sure you are
using (not using) unsigned arithmetic without knowing the exact type of the
number.

## `sgn()` function returning the sign of a number

Returns +1 if the number if positive, -1 if it is negative, 0 if it is zero.

## Interoperability between boost::tuple and std::tuple

Allows using `boost::tuple` like `std::tuple` in many situations. In particular,
it allows to use `std::get` with `boost::tuple`, and it allows to use
`boost::tuple` with structured bindings. This is useful for boost algorithms
that still return `boost::tuple`, such as `boost::range::combine`.