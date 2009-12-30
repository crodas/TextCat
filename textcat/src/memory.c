/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author:  César Rodas <crodas@member.fsf.org>                         |
   +----------------------------------------------------------------------+
 */

#include "textcat.h"
#include "textcat_internal.h"

typedef struct memblock {
    void * pool;
    size_t size;
    size_t offset;
    struct memblock * next;
} memblock;

typedef struct mempool {
    memblock * first;
    memblock * last;
    size_t size;
    size_t offset;
    size_t blocks;
    /* callback */
    void * (*malloc)(size_t);
    void * (*free)(void *);
    size_t block_size;
} mempool;

static Bool mempool_add_memblock (mempool * pool, size_t rsize);

//mempool_init(void ** memory, void * (*xmalloc)(size_t), void * (*xfree)(void *), size_t block_size) {{{
extern Bool mempool_init(void ** memory, void * (*xmalloc)(size_t), void * (*xfree)(void *), size_t block_size)
{
    mempool * mem;
    mem = xmalloc(sizeof(mempool));
    if (mem == NULL) {
        *memory = NULL;
        return TC_FALSE;
    }
    mem->first      = NULL;
    mem->last       = NULL;
    mem->blocks     = 0;
    mem->offset     = 0;
    mem->size       = 0;
    mem->free       = xfree;
    mem->malloc     = xmalloc;
    mem->block_size = block_size;
    *memory  = (void *)mem;
    return TC_TRUE;
}
// }}}

// mempool_done(void * memory) {{{
void mempool_done(void ** memory)
{
    mempool * mem;
    mem = *memory;
    void * (*xfree)(size_t);
    xfree = mem->free;
    if (mem->blocks > 0) {
        memblock * mem1, * mem2;
        mem1 = mem->first;
        while (mem1) {
            mem2 = mem1->next;
            if (mem1->size > 0) {
                xfree(mem1->pool);
            }
            xfree(mem1);
            mem1 = mem2;
        }
    }
    xfree(*memory);
    *memory = 0;
}
// }}}

// mempool_reset(void * memory) {{{
void mempool_reset(void * memory)
{
    mempool * pool;
    memblock * block, * next;
    pool  = (mempool *) memory;
    if (pool->first == NULL) {
        return;
    }
    for (block = pool->first->next; block; block = next) {
        pool->blocks--;
        pool->size -= block->size;
        if (block->size > 0) {
            pool->free(block->pool);
        }
        next = block->next;
        pool->free(block);
    }
    block = pool->first;
    block->offset = 0;
    block->next   = NULL; 
    pool->last  = pool->first;
    pool->offset = 0;

}
// }}}

// mempool_malloc(void * memory, size_t size) {{{
void * mempool_malloc(void * memory, size_t size)
{
    mempool * pool;
    void  * mmem;
    size_t free; 
    pool  = (mempool *) memory;
    free  = pool->size - pool->offset;
    if (free < size && mempool_add_memblock(pool, size) == TC_FALSE) {
        return NULL;
    } 
    mmem = pool->last->pool +  pool->last->offset;
    pool->last->offset += size;
    pool->offset       += size;

    return mmem;
}
// }}}

// mempool_calloc(void * memory, size_t nmemb, size_t size) {{{
void * mempool_calloc(void * memory, size_t nmemb, size_t size)
{
    void * mem;
    mem = mempool_malloc(memory, nmemb * size);
    if (mem != NULL) {
        memset(mem, 0, nmemb * size);
    }
    return mem;
}
// }}}

// mempool_strndup(void * memory, uchar * key, size_t len) {{{
uchar * mempool_strndup(void * memory, uchar * key, size_t len)
{
    uchar * mem;
    mem = mempool_malloc(memory, len + 1);
    if (mem == NULL) {
        return NULL;
    }
    strncpy(mem, key, len);
    *(mem+len) = '\0';
    return mem;
}
// }}}

//mempool_add_memblock (mempool * pool, size_t rsize) {{{
static Bool mempool_add_memblock (mempool * pool, size_t rsize)
{
    size_t size;
    memblock * mem;
    size = pool->block_size > rsize ? pool->block_size : rsize;
    mem  = (memblock *) pool->malloc( sizeof(memblock) );
    if (mem == NULL) {
        return TC_FALSE;
    }
    mem->size   = size;
    mem->offset = 0; 
    mem->next   = NULL;
    mem->pool   = (void *) pool->malloc( size );
    if (mem->pool == NULL) {
        pool->free(mem);
        return TC_FALSE;
    }
    if (pool->first == NULL) {
        pool->first = mem;
    }
    if (pool->last != NULL) {
        pool->last->next = mem;
    }
    pool->last   = mem;
    pool->offset = pool->size; /* Just the last block is free */
    pool->size  += size;
    pool->blocks++;
    return TC_TRUE;
}
// }}}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
