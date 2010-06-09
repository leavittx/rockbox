int add_pool(void *start, size_t size);
void remove_block(struct BlockInfo *block);
void print_lists();

void *bmalloc(size_t size);
void bfree(void *ptr);

