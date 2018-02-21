#ifndef __UTILITIES_HPP__
#define __UTILITIES_HPP__

//----- BoolType functions as a boolean on the Type level -----
template <bool flag>
struct BoolType {};

template<>
struct BoolType<false> {};

//----- swap_pair -----
template<typename T, typename U>
std::pair<U, T> swap_pair(const std::pair<T, U>& p) {
	return std::make_pair(p.second, p.first);
}

//----- random -----
void seed_rand() {
  srand(time(0));
}

// returns a random number in the interval [0,t)
template<typename T>
inline T random(T t)
{
  return T(t * ((long double)(rand()) / ((long double)(RAND_MAX) + 1.0)));
}


//----- iota fills an iterator range with subsequent values -----
template<typename Iterator, typename T>
void iota(Iterator first, Iterator last, T value) {
  for ( ; first != last; ++first, ++value) 
    *first = value;
}

#endif // __UTILITIES_HPP__
