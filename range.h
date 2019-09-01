struct range : std::pair<int, int> {
  struct iterator {
    int i;

    iterator(int i) : i(i) {}

    int operator*() { return i; }

    iterator &operator++() {
      i++;
      return *this;
    }

    bool operator!=(iterator x) { return i != x.i; }
  };

  range() {}
  range(int first, int second) : pair(first, second) {}

  iterator begin() { return first; }
  iterator end() { return second; }
};
