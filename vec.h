template <class T, int small = 4> struct vec {
  typedef T *iterator;
  typedef const T *const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  int cap = small;
  int n;
  T *p = v;
  T v[small];

  vec(int n) : n(n) {
    if (n <= cap)
      return;
    cap = n;
    p = (T *)xmalloc(cap * sizeof(T));
  }

  vec() : n(0) {}

  // move
  vec(vec &b) : n(b.n) {
    if (b.p == b.v) {
      memcpy(v, b.v, n * sizeof(T));
      return;
    }
    cap = b.cap;
    p = b.p;
    b.p = b.v;
  }

  ~vec() {
    if (p != v)
      free(p);
  }

  // capacity
  void reserve(int m) {
    if (m <= cap)
      return;
    cap = std::max(m, cap * 2);
    if (p == v) {
      p = (T *)xmalloc(cap * sizeof(T));
      memcpy(p, v, n * sizeof(T));
      return;
    }
    p = (T *)xrealloc(p, cap * sizeof(T));
  }

  void resize(int m) {
    reserve(m);
    n = m;
  }

  void resize(int m, T a) {
    reserve(m);
    for (auto i = n; i < m; ++i)
      p[i] = a;
    n = m;
  }

  // element access
  T &operator[](int i) {
    assert(i >= 0);
    assert(i < n);
    return p[i];
  }

  const T &operator[](int i) const {
    assert(i >= 0);
    assert(i < n);
    return p[i];
  }

  T &back() {
    assert(n > 0);
    return p[n - 1];
  }

  const T &back() const {
    assert(n > 0);
    return p[n - 1];
  }

  // iterators
  iterator begin() { return p; }
  const_iterator begin() const { return p; }

  iterator end() { return p + n; }
  const_iterator end() const { return p + n; }

  reverse_iterator rbegin() { return reverse_iterator(p + n); }
  const_reverse_iterator rbegin() const { return reverse_iterator(p + n); }

  reverse_iterator rend() { return reverse_iterator(p); }
  const_reverse_iterator rend() const { return const_reverse_iterator(p); }

  // modifiers
  void push(T a) {
    reserve(n + 1);
    p[n++] = a;
  }

  void pop() {
    assert(n > 0);
    --n;
  }

  void insert(const_iterator position, T *first, T *last) {
    assert(p <= position);
    assert(position <= end());
    auto i = position - p;

    assert(first <= last);
    auto m = last - first;

    reserve(n + m);
    memmove(p + i + m, p + i, (n - i) * sizeof(T));
    memcpy(p + i, first, m * sizeof(T));
    n += m;
  }

  void erase(iterator position) { erase(position, position + 1); }

  void erase(iterator first, iterator last) {
    assert(p <= first);
    assert(first <= end());

    assert(p <= last);
    assert(last <= end());

    assert(first <= last);

    memmove(first, last, (end() - last) * sizeof(T));
    n -= last - first;
  }

  bool operator==(const vec &b) {
    if (n != b.n)
      return false;
    return !memcmp(p, b.p, n * sizeof(T));
  }
};
