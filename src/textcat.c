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
   | Author:  Cesar Rodas <saddor@gmail.com>                              |
   +----------------------------------------------------------------------+
   | Base on the LibTextCat project,                                      |
   | Copyright (c) 2003, WiseGuys Internet B.V.                           |
   +----------------------------------------------------------------------+
 */

#include "textcat.h"

static long textcat_simple_hash(const char *p, int len);
static Bool textcat_ngram_find(const ngram_set * nset, const char * key, int len, ngram ** item);
static int textcat_ngram_incr(TextCat * tc, ngram_set * nset, const char * key, int len);
static ngram * textcat_ngram_create(TextCat * tc, ngram_set * nset, const char * key, int len);

// simple_hash(const char *p, int len) {{{
/*
 * fast and furious little hash function
 *
 * (Note that we could use some kind of rolling checksum, and update it
 * during n-gram construction) 
 */
static long textcat_simple_hash(const char *p, int len)
{
	long h = len * 13;
	while (*p) {
		h = (h<<5)-h + *p++;
	}
	return (long)h;
}
// }}}

// textcat_find_ngram(const ngram_set * nset, const char * key, int len, ngram ** item) {{{
static Bool textcat_ngram_find(const ngram_set * nset, const char * key, int len, ngram ** item)
{
    ngram * entry;
    long i;
    for (i=0; i < nset->total; i++) 
    {
        entry = &nset->ngrams[i];
        if (entry->status == TC_FREE) {
            break;
        }
        if (entry->len == len && strncmp(entry->str, key, len) == 0) {
            *item = entry;
            return TC_TRUE;
        }
    }
    return TC_FALSE;
}
// }}}

// textcat_ngram_incr(TextCat * tc, ngram_set * nset, const char * key, int len) {{{
static int textcat_ngram_incr(TextCat * tc, ngram_set * nset, const char * key, int len)
{
    ngram * item;
    if (textcat_ngram_find(nset, key, len, &item) == TC_TRUE) {
        item->freq++;
    } else {
        item = textcat_ngram_create(tc, nset, key, len);
        item->freq++;
    }
}
// }}}

// textcat_ngram_create(TextCat * tc, ngram_set * nset, const char * key, int len) {{{
static ngram * textcat_ngram_create(TextCat * tc, ngram_set * nset, const char * key, int len)
{
    ngram * item;
    if (nset->total+1 == nset->size) {
        nset->size  += tc->ngram_precreate;
        nset->ngrams = tc->realloc(nset->size, nset->ngrams);
    }
    item = & nset->ngrams[nset->total];
    
    /* setup the new N-gram */
    item->status = TC_BUSY;
    item->freq   = 1;
    item->len    = len;
    item->str    = nset->pool + nset->pool_offset;

    /* Copy the String using our buffer */
    if (nset->pool_offset + len > nset->pool_size) {
        nset->pool_size += tc->pool_preallocate_size;
        nset->pool       = tc->realloc(nset->pool_size, nset->pool);
    }
    strncpy(nset->pool + nset->pool_offset, key, len);

    nset->total++;

    return item;
}
// }}}

// TextCat_Init(TextCat ** tcc) {{{
Bool TextCat_Init(TextCat ** tcc)
{
    TextCat * tc;
    tc = (TextCat *) malloc(sizeof(TextCat));
    tc->malloc  = malloc;
    tc->realloc = realloc;
    tc->ngram_precreate = TC_NGRAM_PRECREATE;
    tc->pool_preallocate_size = TC_BUFFER_SIZE;
    tc->hash_size = TC_HASH_SIZE;
    *tcc = tc;
    return TC_TRUE;
}
// }}}

// TextCat_Destroy(TextCat * tc) {{{
Bool TextCat_Destroy(TextCat * tc) 
{
    free(tc);
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
