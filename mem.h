// allocation with error checking
void *xmalloc(size_t bytes);
void *xcalloc(size_t n, size_t size);
void *xrealloc(void *p, size_t bytes);

// monotonic allocation
void *mmalloc(int bytes);
