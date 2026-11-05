#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_POOL 1000000

typedef struct {
    size_t start;
    size_t size;
    int alloc_id;
    int is_free;
} Block;

static unsigned char *pool = NULL;
static size_t pool_size = 0;
static Block *blocks = NULL;
static size_t block_count = 0;
static size_t block_capacity = 0;
static int next_alloc_id = 1;
static int strategy = 1;

int read_number(const char *prompt, long *result){
    char line[128];
    char *end_ptr;
 
    printf("%s", prompt);
    if(!fgets(line, sizeof line, stdin)){
        return -1;
    }
    *result = strtol(line, &end_ptr, 10);
    if(end_ptr == line){
        return 0;
    }
    while(*end_ptr == ' ' || *end_ptr == '\t' || *end_ptr == '\n'){
        end_ptr++;
    }
    if(*end_ptr == '\0'){
        return 1;
    }
    return 0;
}

int ensure_capacity(size_t needed){
    size_t new_capacity;
    Block *resized;

    if (needed <= block_capacity){
        return 1;
    }
    new_capacity = block_capacity ? block_capacity * 2 : 8;
    while (new_capacity < needed){
        new_capacity *= 2;
    }
    resized = realloc(blocks, new_capacity * sizeof *blocks);
    if(!resized){
        return 0;
    }
    blocks = resized;
    block_capacity = new_capacity;
    return 1;
}

 void insert_block(size_t position, Block new_block) {
    memmove(&blocks[position + 1], &blocks[position],
            (block_count - position) * sizeof *blocks);
    blocks[position] = new_block;
    block_count++;
}

 void remove_block(size_t position) {
    memmove(&blocks[position], &blocks[position + 1],
            (block_count - position - 1) * sizeof *blocks);
    block_count--;
}

 void merge_free_blocks(void) {
    size_t index = 0;

    while (index + 1 < block_count) {
        if (blocks[index].is_free && blocks[index + 1].is_free) {
            blocks[index].size += blocks[index + 1].size;
            remove_block(index + 1);
        } else {
            index++;
        }
    }
}

 int find_block_by_id(int alloc_id) {
    if (alloc_id <= 0){
        return -1;
    }
    for (size_t index = 0; index < block_count; index++){
        if (!blocks[index].is_free && blocks[index].alloc_id == alloc_id){
            return (int)index;
        }
    }
    return -1;
}

 int first_fit(size_t size) {
    for(size_t index = 0; index < block_count; index++){
        if(blocks[index].is_free && blocks[index].size >= size) {
            return (int)index;
        }
    }
    return -1;
}

 int best_fit(size_t size) {
    int best = -1;

    for(size_t index = 0; index < block_count; index++) {
        if(blocks[index].is_free && blocks[index].size >= size) {
            if(best < 0 || blocks[index].size < blocks[best].size) {
                best = (int )index;
            }
        }
    }
    return best;
}

 int worst_fit(size_t size ) {
    int worst = -1;

    for (size_t index = 0; index < block_count; index++) {
        if(blocks[index].is_free && blocks[index].size >= size) {
            if(worst < 0 || blocks[index].size > blocks[worst].size) {
                worst = (int)index;
            }
        }
    }
    return worst;
}

 int find_fit(size_t size) {
    switch (strategy) {
        case 2:
            return best_fit(size);
        case 3:
            return worst_fit(size);
        default:
            return first_fit(size);
    }
}

 size_t total_free(void) {
    size_t total = 0;

    for (size_t index = 0; index < block_count; index++) {
        if (blocks[index].is_free) {
            total += blocks[index].size;
        }
    }
    return total;
}

 size_t largest_free(void) {
    size_t largest = 0;

    for (size_t index = 0; index < block_count; index++) {
        if (blocks[index].is_free && blocks[index].size > largest) {
            largest = blocks[index].size;
        }
    }
    return largest;
}

 void report_fragmentation(size_t needed) {
    size_t free_total = total_free();

    if (free_total >= needed) {
        printf("External fragmentation: need %zu bytes, %zu bytes free in total,\n"
               "but no single free block is large enough. Free blocks:",
               needed, free_total);
        for (size_t index = 0; index < block_count; index++) {
            if (blocks[index].is_free) {
                printf(" [%zu @ %zu]", blocks[index].size, blocks[index].start);
            }
        }
        printf("\n");
    }
}

 void display_stats(void) {
    size_t allocated_bytes = 0;
    size_t free_bytes = 0;
    size_t allocated_count = 0;
    size_t free_count = 0;

    for (size_t index = 0; index < block_count; index++) {
        if (blocks[index].is_free) {
            free_bytes += blocks[index].size;
            free_count++;
        } else {
            allocated_bytes += blocks[index].size;
            allocated_count++;
        }
    }
    printf("Total memory: %zu bytes\n", pool_size);
    printf("Allocated: %zu bytes\n", allocated_bytes);
    printf("Free: %zu bytes\n", free_bytes);
    printf("Allocated blocks: %zu\n", allocated_count);
    printf("Free blocks: %zu\n", free_count);
    printf("Largest free block: %zu bytes\n", largest_free());
    if(free_count > 1) {
        printf("Note: free memory is split across %zu blocks.\n", free_count);
    }
}

 void display_map(void) {
    printf("Memory Map (%zu bytes):\n", pool_size);
    for(size_t index = 0; index < block_count; index++) {
        if(blocks[index].is_free) {
            printf("[Free: %zu bytes, Start: %zu]\n",
                   blocks[index].size, blocks[index].start);
        } 
        else{
            printf("[Allocated #%d: %zu bytes, Start: %zu]\n",
                   blocks[index].alloc_id, blocks[index].size, blocks[index].start);
        }
    }
}

 void allocate_memory(size_t size) {
    int block_index = find_fit(size);
    int needs_split;
    int new_id;
    size_t start;
    size_t remainder;

    if (block_index < 0) {
        printf("Allocation failed: no contiguous free block is large enough.\n");
        report_fragmentation(size);
        return;
    }
    needs_split = blocks[block_index].size > size;
    if (needs_split && !ensure_capacity(block_count + 1)) {
        printf("Allocation failed: could not grow metadata array.\n");
        return;
    }
    start = blocks[block_index].start;
    remainder = blocks[block_index].size - size;
    blocks[block_index].size = size;
    blocks[block_index].is_free = 0;
    blocks[block_index].alloc_id = next_alloc_id++;
    new_id = blocks[block_index].alloc_id;
    if (needs_split) {
        Block free_block = { start + size, remainder, 0, 1 };
        insert_block((size_t)block_index + 1, free_block);
    }
    printf("Allocation successful. ID: %d, Start: %zu, Size: %zu\n",
           new_id, start, size);
}

 void release_memory(int alloc_id) {
    int block_index;

    if (alloc_id <= 0) {
        printf("Invalid ID.\n");
        return;
    }
    block_index = find_block_by_id(alloc_id);
    if (block_index < 0) {
        printf("Release failed: unknown or already-free allocation ID %d.\n", alloc_id);
        return;
    }
    blocks[block_index].is_free = 1;
    blocks[block_index].alloc_id = 0;
    merge_free_blocks();
    printf("Allocation released.\n");
}

 void resize_allocation(int alloc_id, size_t new_size) {
    int block_index = find_block_by_id(alloc_id);
    int target_index;
    size_t old_size;
    size_t extra_bytes;
    size_t next_index;
    size_t old_start;
    size_t new_start;
    size_t leftover;

    if (block_index < 0) {
        printf("Resize failed: unknown allocation ID %d.\n", alloc_id);
        return;
    }
    old_size = blocks[block_index].size;

    if (new_size == old_size) {
        printf("Size unchanged.\n");
        return;
    }

    if (new_size < old_size) {
        Block free_block;

        if (!ensure_capacity(block_count + 1)) {
            printf("Resize failed: could not grow metadata array.\n");
            return;
        }
        free_block.start = blocks[block_index].start + new_size;
        free_block.size = old_size - new_size;
        free_block.alloc_id = 0;
        free_block.is_free = 1;
        blocks[block_index].size = new_size;
        insert_block((size_t)block_index + 1, free_block);
        merge_free_blocks();
        printf("Resize successful. ID: %d, Start: %zu, Size: %zu\n",
               alloc_id, blocks[block_index].start, new_size);
        return;
    }

    extra_bytes = new_size - old_size;
    next_index = (size_t)block_index + 1;
    if (next_index < block_count && blocks[next_index].is_free &&
        blocks[next_index].size >= extra_bytes) {
        if (blocks[next_index].size == extra_bytes) {
            remove_block(next_index);
        } 
        else {
            blocks[next_index].start += extra_bytes;
            blocks[next_index].size -= extra_bytes;
        }
        blocks[block_index].size = new_size;
        printf("Resize successful (in place). ID: %d, Start: %zu, Size: %zu\n",
               alloc_id, blocks[block_index].start, new_size);
        return;
    }

    target_index = find_fit(new_size);
    if (target_index < 0) {
        printf("Resize failed: no suitable free region. Original allocation unchanged.\n");
        report_fragmentation(new_size);
        return;
    }
    if (!ensure_capacity(block_count + 1)) {
        printf("Resize failed: could not grow metadata array. Original unchanged.\n");
        return;
    }
    old_start = blocks[block_index].start;
    new_start = blocks[target_index].start;
    leftover = blocks[target_index].size - new_size;
    memcpy(pool + new_start, pool + old_start, old_size);

    blocks[target_index].is_free = 0;
    blocks[target_index].alloc_id = alloc_id;
    blocks[target_index].size = new_size;
    if (leftover > 0) {
        Block free_block = { new_start + new_size, leftover, 0, 1 };
        insert_block((size_t)target_index + 1, free_block);
        if (block_index > target_index) {
            block_index++;
        }
    }
    blocks[block_index].is_free = 1;
    blocks[block_index].alloc_id = 0;
    merge_free_blocks();
    printf("Resize successful (moved). ID: %d, Start: %zu, Size: %zu\n",
           alloc_id, new_start, new_size);
}

 void choose_strategy(void) {
    long choice;
    int status;

    while(1) {
        status = read_number(
            "Select allocation strategy (1. First Fit, 2. Best Fit, 3. Worst Fit): ",
            &choice);
        if (status < 0) {
            exit(0);
        }
        if (status == 1 && choice >= 1 && choice <= 3) {
            break;
        }
        printf("Invalid strategy.\n");
    }
    strategy = (int)choice;
    if (strategy == 1) {
        printf("Strategy set to First Fit.\n");
    } 
    else if (strategy == 2) {
        printf("Strategy set to Best Fit.\n");
    } 
    else {
        printf("Strategy set to Worst Fit.\n");
    }
}

int main(){
    long input_value;
    int status;

    while(1){
        status = read_number("Enter memory pool size: ", &input_value);
        if(status < 0){
            return 0;
        }
        if(status == 1 && input_value >= 1 && input_value <= MAX_POOL) {
            break;
        }
        printf("Invalid pool size (1 - %d).\n", MAX_POOL);
    }
    pool_size = (size_t)input_value;
    pool = malloc(pool_size);
    if (!pool) {
        printf("Failed to allocate memory pool.\n");
        return 1;
    }
    memset(pool, 0, pool_size);

    if (!ensure_capacity(8)) {
        printf("Failed to allocate metadata.\n");
        free(pool);
        return 1;
    }
    blocks[0].start = 0;
    blocks[0].size = pool_size;
    blocks[0].alloc_id = 0;
    blocks[0].is_free = 1;
    block_count = 1;

    choose_strategy();

    while(1) {
        printf("\nMenu:\n1. Allocate memory\n2. Release memory\n3. Resize allocation\n"
               "4. Display memory map\n5. Display memory statistics\n"
               "6. Change allocation strategy\n7. Exit\n");
        status = read_number("Choice: ", &input_value);
        if (status < 0) {
            break;
        }
        if (status != 1) {
            printf("Invalid choice.\n");
            continue;
        }

        if (input_value == 7) {
            break;
        } 
        else if (input_value == 1) {
            long requested_size;

            status = read_number("Requested size: ", &requested_size);
            if (status < 0) {
                break;
            }
            if (status != 1 || requested_size <= 0 || (size_t)requested_size > pool_size) {
                printf("Invalid size (1 - %zu).\n", pool_size);
            } 
            else {
                allocate_memory((size_t)requested_size);
            }
        } 
        else if (input_value == 2) {
            long alloc_id;

            status = read_number("Allocation ID: ", &alloc_id);
            if (status < 0) {
                break;
            }
            if (status != 1 || alloc_id <= 0 || alloc_id > 2147483647L) {
                printf("Invalid ID.\n");
            } 
            else {
                release_memory((int)alloc_id);
            }
        } 
        else if (input_value == 3) {
            long alloc_id;
            long requested_size;

            status = read_number("Allocation ID: ", &alloc_id);
            if (status < 0) {
                break;
            }
            if (status != 1 || alloc_id <= 0 || alloc_id > 2147483647L) {
                printf("Invalid ID.\n");
                continue;
            }
            if (find_block_by_id((int)alloc_id) < 0) {
                printf("Unknown allocation ID.\n");
                continue;
            }
            status = read_number("New size: ", &requested_size);
            if (status < 0) {
                break;
            }
            if (status != 1 || requested_size <= 0 || (size_t)requested_size > pool_size) {
                printf("Invalid size (1 - %zu).\n", pool_size);
            } 
            else {
                resize_allocation((int)alloc_id, (size_t)requested_size);
            }
        } 
        else if (input_value == 4) {
            display_map();
        } 
        else if (input_value == 5) {
            display_stats();
        } 
        else if (input_value == 6) {
            choose_strategy();
        } 
        else {
            printf("Invalid choice.\n");
        }
    }

    free(blocks);
    free(pool);
    return 0;
}