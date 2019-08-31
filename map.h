template <class Key, class T> struct map {
  typedef std::pair<Key, T> value_type;

  map *parent;
  int cap = 0x4;
  int n = 0;
  value_type *p = (value_type *)xcalloc(cap, sizeof(value_type));

  int slot(value_type *p, int cap, const Key &k) {
    auto mask = cap - 1;
    auto i = fnv(&k, sizeof k) & mask;
    while (p[i].first && p[i].first != k)
      i = (i + 1) & mask;
    return i;
  }

  void expand() {
    auto cap1 = cap * 2;
    auto p1 = (value_type *)xcalloc(cap1, sizeof(value_type));
    for (int i = 0; i != cap; ++i) {
      auto a = p[i];
      if (a.first)
        p1[slot(p1, cap1, a.first)] = a;
    }
    free(p);
    cap = cap1;
    p = p1;
  }

  map(map *parent) : parent(parent) {}
  ~map() { free(p); }

  bool put(const Key &k, T a) {
    auto i = slot(p, cap, k);
    bool existing = p[i].first;
    if (!existing) {
      if (++n > cap * 3 / 4) {
        expand();
        i = slot(p, cap, k);
      }
      p[i].first = k;
    }
    p[i].second = a;
    return existing;
  }

  T operator[](const Key &k) {
    for (auto m = this; m; m = m->parent) {
      auto i = slot(m->p, m->cap, k);
      if (m->p[i].first)
        return m->p[i].second;
    }
    return 0;
  }

private:
  map(map &);
};

template <class Key, class T> void fpr(FILE *F, map<Key, T> &m0) {
  for (auto m = &m0; m; m = m->parent) {
    fprintf(F, "\n%p: %d/%d", m, m->n, m->cap);
    for (int i = 0; i != m->cap; ++i) {
      fpr(F, "\n  ");
      if (!m->p[i].first) {
        fpr(F, '-');
        continue;
      }
      fpr(F, m->p[i].first);
      fpr(F, ": ");
      fpr(F, m->p[i].second);
    }
  }
}
