template <class Key, class T> struct set {
  typedef T *value_type;

private:
  int cap = 0x10;
  int n = 0;
  value_type *p = (value_type *)xcalloc(cap, sizeof(value_type));

  int slot(value_type *p, int cap, const Key &k) {
    auto mask = cap - 1;
    auto i = k.hash() & mask;
    while (p[i] && !(k == p[i]))
      i = (i + 1) & mask;
    return i;
  }

  void expand() {
    auto cap1 = cap * 2;
    auto p1 = (value_type *)xcalloc(cap1, sizeof(value_type));
    for (int i = 0; i != cap; ++i) {
      auto a = p[i];
      if (a) {
        Key k(a);
        p1[slot(p1, cap1, k)] = a;
      }
    }
    free(p);
    cap = cap1;
    p = p1;
  }

public:
  void add(const Key &k, value_type a) {
    if (++n > cap * 3 / 4)
      expand();
    auto i = slot(p, cap, k);
    assert(!p[i]);
    p[i] = a;
  }

  T *operator[](const Key &k) {
    auto i = slot(p, cap, k);
    if (p[i])
      return p[i];
    if (++n > cap * 3 / 4) {
      expand();
      i = slot(p, cap, k);
    }
    return p[i] = k.store();
  }
};
