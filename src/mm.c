#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
/* The standard allocator interface from stdlib.h.  These are the
 * functions you must implement, more information on each function is
 * found below. They are declared here in case you want to use one
 * function in the implementation of another. */
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

/* When requesting memory from the OS using sbrk(), request it in
 * increments of CHUNK_SIZE. */
#define CHUNK_SIZE (1<<12)
typedef struct amul_butter{
    size_t metadata;
    struct amul_butter *nextb;
    
}
    amul_butter;

static amul_butter *BlockStore[13];
/*
 * This function, defined in bulk.c, allocates a contiguous memory
 * region of at least size bytes.  It MAY NOT BE USED as the allocator
 * for pool-allocated regions.  Memory allocated using bulk_alloc()
 * must be freed by bulk_free().
 *
 * This function will return NULL on failure.
 */
extern void *bulk_alloc(size_t size);

/*
 * This function is also defined in bulk.c, and it frees an allocation
 * created with bulk_alloc().  Note that the pointer passed to this
 * function MUST have been returned by bulk_alloc(), and the size MUST
 * be the same as the size passed to bulk_alloc() when that memory was
 * allocated.  Any other usage is likely to fail, and may crash your
 * program.
 *
 * Passing incorrect arguments to this function will result in an
 * error message notifying you of this mistake.
 */
extern void bulk_free(void *ptr, size_t size);

/*
 * This function computes the log base 2 of the allocation block size
 * for a given allocation.  To find the allocation block size from the
 * result of this function, use 1 << block_index(x).
 *
 * This function ALREADY ACCOUNTS FOR both padding and the size of the
 * header.
 *
 * Note that its results are NOT meaningful for any
 * size > 4088!
 *
 * You do NOT need to understand how this function works.  If you are
 * curious, see the gcc info page and search for __builtin_clz; it
 * basically counts the number of leading binary zeroes in the value
 * passed as its argument.
 */
static inline __attribute__((unused)) int block_index(size_t x) {
    if (x <= 8) {
        return 5;
    } else {
        return 32 - __builtin_clz((unsigned int)x + 7);
    }
}

void *malloc(size_t size) {

    if (size==0){
        return NULL;
    }
    
    if (size<=4088){
        size_t blockidx = block_index(size);
        size_t realsize = 1<< blockidx;
        if(BlockStore[blockidx]!=NULL){
            amul_butter *kohli=BlockStore[blockidx];
            BlockStore[blockidx]=kohli->nextb;

            return ((void*)kohli) +sizeof(size_t);
        }
    void *new_butter = sbrk(CHUNK_SIZE);
    
    size_t how_many_butter= CHUNK_SIZE / realsize;
    
    BlockStore[blockidx] = new_butter;

    for(size_t i=0;i<how_many_butter;i++){
        
        void *slice = (new_butter+(i*realsize));
        //slice ->nextb = BlockStore[blockidx];
        if (i==how_many_butter-1){
            ((amul_butter*)slice)->metadata = realsize;
            ((amul_butter*)slice)->nextb=NULL;
            break;
        }
        ((amul_butter*)slice) ->nextb = ((void*)slice) + realsize;
        ((amul_butter*)slice)->metadata = realsize;
        // BlockStore[blockidx] = slice;
        ((amul_butter*)new_butter) ->metadata = realsize;
        
    }
    BlockStore[blockidx]=BlockStore[blockidx]->nextb;
    return ((void*)new_butter) +sizeof(size_t);

    } else{
        //if (size%8!=0){
            //size = size + (8-(size%8));
            //}
        amul_butter *bada_pack = bulk_alloc(size+8);
        bada_pack ->metadata = (size + sizeof(size_t)) ;
        return ((void*)(bada_pack)) +sizeof(size_t);
    }

    
//    return bulk_alloc(size);
}


void *calloc(size_t nmemb, size_t size) {
    size_t total_mem = nmemb * size;
    if (total_mem == 0) {
        return NULL;
    }

    void *ptr = malloc(total_mem);
    if (ptr != NULL) {
        memset(ptr, 0, total_mem);
    }
    return ptr;
}


void *realloc(void *ptr, size_t size) {
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    if (ptr == NULL) {
        return malloc(size);
    }

    amul_butter *butter = (amul_butter *)(ptr - sizeof(size_t));
    size_t original_size = (butter->metadata) - sizeof(size_t);

    if (size <= original_size) {
        return ptr;
    }

    void *newptr = malloc(size);
    memcpy(newptr, ptr, original_size);
    free(ptr);
    return newptr;
}

void free(void *ptr) {
    if (ptr != NULL) {
        amul_butter *tukda = (amul_butter *)(ptr - sizeof(size_t));
        if ((tukda->metadata) > 4096) {
            bulk_free(ptr - sizeof(size_t), tukda->metadata);
        } else {
            size_t idex = block_index(tukda->metadata - 8);
            tukda->nextb = BlockStore[idex];
            BlockStore[idex] = tukda;
        }
    }
}
