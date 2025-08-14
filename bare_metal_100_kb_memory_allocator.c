#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define MEMORY_SIZE (100 * 1024)
#define ALIGNMENT sizeof(int)
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

typedef struct Block {
    size_t size;
    int free;
    struct Block *next;
} Block;

static uint8_t memory_pool[MEMORY_SIZE];
static Block *free_list = NULL;

static void init_memory() {
    free_list = (Block*)memory_pool;
    free_list->size = MEMORY_SIZE - sizeof(Block);
    free_list->free = 1;
    free_list->next = NULL;
}

int* allocate(int size) {
    if (size <= 0) return NULL;
    if (!free_list) init_memory();

    size_t aligned_size = ALIGN(size);
    Block *current = free_list;
    Block *prev = NULL;

    while (current) {
        if (current->free && current->size >= aligned_size) {
            if (current->size > aligned_size + sizeof(Block)) {
                Block *new_block = (Block*)((uint8_t*)current + sizeof(Block) + aligned_size);
                new_block->size = current->size - aligned_size - sizeof(Block);
                new_block->free = 1;
                new_block->next = current->next;
                current->next = new_block;
                current->size = aligned_size;
            }
            current->free = 0;
            return (int*)((uint8_t*)current + sizeof(Block));
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}

void deallocate(int *ptr) {
    if (!ptr) return;

    Block *block = (Block*)((uint8_t*)ptr - sizeof(Block));
    block->free = 1;

    Block *current = free_list;
    while (current) {
        if (current->free && current->next && current->next->free) {
            current->size += sizeof(Block) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

int main() {
    int *mem1 = allocate(128);
    int *mem2 = allocate(1024);
    int *mem3 = allocate(4096);
    deallocate(mem2);
    mem2 = allocate(512);

    printf("Allocation results: %p %p %p\n", (void*)mem1, (void*)mem2, (void*)mem3);
    return 0;
}
