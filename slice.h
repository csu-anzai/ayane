template <class T> struct slice {
  T *first;
  T *last;

  slice(T *first, T *last) : first(first), last(last) {}

  T *begin() { return first; }
  T *end() { return last; }
};
