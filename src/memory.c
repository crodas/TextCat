/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2008 The PHP Group                                |
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
   | Based on the LibTextCat project,                                     |
   | Copyright (c) 2003, WiseGuys Internet B.V.                           |
   +----------------------------------------------------------------------+
 */

#include "textcat.h"

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
    size_t usage;
    size_t blocks;
} mempool;

static Bool mempool_add_memblock (TextCat * tc, size_t rsize);

// Bool mempool_init(TextCat * tc) {{{
extern Bool mempool_init(TextCat * tc)
{
    mempool * mem;
    mem = tc->malloc(sizeof(mempool));
    if (mem == NULL) {
        tc->error = TC_ERR_MEM;
        return TC_FALSE;
    }
    mem->first  = NULL;
    mem->last   = NULL;
    mem->blocks = 0;
    mem->usage  = 0;
    mem->size   = 0;
    tc->memory  = (void *)mem;
    return TC_TRUE;
}
// }}}

// mempool_done(TextCat * tc) {{{
extern void mempool_done(TextCat * tc)
{
    mempool * mem;
    mem = tc->memory;
    if (mem->blocks > 0) {
        memblock * mem1, * mem2;
        mem1 = mem->first;
        while (mem1) {
            mem2 = mem1->next;
            if (mem1->size > 0) {
                tc->free(mem1->pool);
            }
            tc->free(mem1);
            mem1 = mem2;
        }
    }
    tc->free(mem);
}
// }}}

// mempool_malloc(TextCat * tc, size_t size) {{{
void * mempool_malloc(TextCat * tc, size_t size)
{
    mempool * pool;
    void  * mmem;
    size_t free; 
    pool  = (mempool *) tc->memory;
    free  = pool->size - pool->usage;
    if (free < size && mempool_add_memblock(tc, size) == TC_FALSE) {
        return NULL;
    }
    mmem = pool->last->pool +  pool->last->offset;
    pool->last->offset += size;
    pool->usage += size;


    return mmem;
}
// }}}

// mempool_calloc(TextCat * tc, size_t nmemb, size_t size) {{{
void * mempool_calloc(TextCat * tc, size_t nmemb, size_t size)
{
    void * mem;
    mem = mempool_malloc(tc, nmemb * size);
    if (mem != NULL) {
        memset(mem, 0, nmemb * size);
    }
    return mem;
}
// }}}

// mempool_strndup(TextCat * tc, uchar * key, size_t len) {{{
uchar * mempool_strndup(TextCat * tc, uchar * key, size_t len)
{
    uchar * mem;
    mem = mempool_malloc(tc, len + 1);
    if (mem == NULL) {
        return NULL;
    }
    strncpy(mem, key, len);
    *(mem+len) = '\0';
    return mem;
}
// }}}

// mempool_add_memblock (TextCat * tc, size_t rsize) {{{
static Bool mempool_add_memblock (TextCat * tc, size_t rsize)
{
    size_t size;
    memblock * mem;
    mempool  * pool;
    size = tc->allocate_size > rsize ? tc->allocate_size : rsize;
    pool = (mempool *) tc->memory;
    mem  = (memblock *) tc->malloc( sizeof(memblock) );
    if (mem == NULL) {
        tc->error = TC_ERR_MEM;
        return TC_FALSE;
    }
    mem->size   = size;
    mem->offset = 0;
    mem->next   = NULL;
    mem->pool   = (void *) tc->malloc( size );
    if (mem->pool == NULL) {
        tc->error = TC_ERR_MEM;
        free(mem);
        return TC_FALSE;
    }
    if (pool->first == NULL) {
        pool->first = mem;
    }
    if (pool->last != NULL) {
        pool->last->next = mem;
    }
    pool->last  = mem;
    pool->usage = pool->size; /* Just the last block is free */
    pool->size += size;
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
