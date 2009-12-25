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
    uchar * pool;
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

// Bool mempool_init(TextCat * tc) {{{
extern Bool mempool_init(TextCat * tc)
{
    mempool * mem;
    mem = tc->malloc(sizeof(mempool));
    if (mem == NULL) {
        tc->error = TC_NGRAM_PRECREATE;
        return TC_FALSE;
    }
    mem->first  = NULL;
    mem->last   = NULL;
    mem->blocks = 0;
    mem->usage  = 0;
    mem->size   = 0;
    tc->memory = mem;
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
            tc->free(mem1);
            mem1 = mem2;
        }
    }
    tc->free(mem);
}
// }}}

// mempool_malloc(TextCat * tc, size_t size) {{
void * mempool_malloc(TextCat * tc, size_t size)
{
    mempool * mem;
    size_t free; 
    mem  = (mempool *) tc->memory;
    free = mem->size - mem->usage;
    if (free < size) {
        mem->blocks++;
    }

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
