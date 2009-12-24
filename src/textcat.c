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

static long textcat_simple_hash(const uchar *p, int len);
static Bool textcat_ngram_find(const ngram_set * nset, const uchar * key, int len, ngram ** item);
static int textcat_ngram_incr(TextCat * tc, const uchar * key, int len);
static ngram * textcat_ngram_create(TextCat * tc, ngram_set * nset, const uchar * key, int len);

// simple_hash(const uchar *p, int len) {{{
/*
 * fast and furious little hash function
 *
 * (Note that we could use some kind of rolling checksum, and update it
 * during n-gram construction) 
 */
static long textcat_simple_hash(const uchar *p, int len)
{
	long h = len * 13;
	while (*p) {
		h = (h<<5)-h + *p++;
	}
	return (long)h;
}
// }}}

// textcat_find_ngram(const ngram_set * nset, const uchar * key, int len, ngram ** item) {{{
static Bool textcat_ngram_find(const ngram_set * nset, const uchar * key, int len, ngram ** item)
{
    ngram * entry;
    long i;
    for (i=0; i < nset->total; i++) 
    {
        entry = &nset->ngrams[i];
        if (entry->status == TC_FREE) {
            break;
        }
        if (entry->len == len && strncmp(nset->pool + entry->offset, key, len) == 0) {
            *item = entry;
            return TC_TRUE;
        }
    }
    return TC_FALSE;
}
// }}}

// textcat_ngram_incr(TextCat * tc, ngram_set * nset, const uchar * key, int len) {{{
static int textcat_ngram_incr(TextCat * tc, const uchar * key, int len)
{
    ngram * item;
    ngram_set * nset;
    int spot;
    spot = textcat_simple_hash(key, len) & tc->hash_size;
    nset = &(tc->hash.table[spot]);
    if (textcat_ngram_find(nset, key, len, &item) == TC_TRUE) {
        item->freq++;
    } else {
        item = textcat_ngram_create(tc, nset, key, len);
        item->freq++;
    }
}
// }}}

// textcat_ngram_create(TextCat * tc, ngram_set * nset, const uchar * key, int len) {{{
static ngram * textcat_ngram_create(TextCat * tc, ngram_set * nset, const uchar * key, int len)
{
    ngram * item;
    if (nset->size == 0) {
        nset->size   = tc->ngram_precreate;
        nset->ngrams = tc->calloc(nset->size, sizeof(ngram));
    } else if (nset->total+1 > nset->size) {
        nset->size  += tc->ngram_precreate;
        nset->ngrams = tc->realloc(nset->ngrams, nset->size * sizeof(ngram));
    }
    item = & nset->ngrams[nset->total];
    
    /* Copy the String using our buffer */
    if  (nset->pool_size == 0) {
        nset->pool_size = tc->pool_preallocate_size;
        nset->pool      = tc->malloc(nset->pool_size * sizeof(uchar));
    } else if (nset->pool_offset + len > nset->pool_size) {
        nset->pool_size += tc->pool_preallocate_size;
        nset->pool       = tc->realloc(nset->pool, nset->pool_size  * sizeof(uchar));
    }

    /* setup the new N-gram */
    item->status = TC_BUSY;
    item->freq   = 0;
    item->len    = len;
    item->offset = nset->pool_offset;

    strncpy(nset->pool + nset->pool_offset, key, len);
    nset->pool_offset += len;
    nset->pool_number++;

    nset->total++;

    return item;
}
// }}}

// textcat_init_hash(TextCat * tc) {{{
static Bool textcat_init_hash(TextCat * tc)
{
    ngram_set * table;
    int i;

    tc->hash.table = tc->calloc(tc->hash_size+1, sizeof(ngram_set));
    tc->hash.total = tc->hash_size;

    if (tc->hash.table == NULL) {
        tc->last_status = TC_ERR_MEM;
        return TC_FALSE;
    }

    table = tc->hash.table;
    for (i=0; i < tc->hash.total; i++) {
        table[i].total = 0;
        table[i].size  = 0;
        table[i].pool_size = 0;
        table[i].pool_number = 0;
        table[i].pool_offset = 0;
    }
    return TC_TRUE; 
}
// }}}

// textcat_destroy_hash(TextCat * tc)  {{{
static void textcat_destroy_hash(TextCat * tc) 
{
    int i;
    ngram_set * table;
    table = tc->hash.table;
    for (i=0; i < tc->hash.total; i++) {
        if (table[i].size > 0) {
            char xtable[50];
            int e;
            for (e=0; e < table[i].total; e++) {
                strncpy(xtable, table[i].pool + table[i].ngrams[e].offset, table[i].ngrams[e].len); 
                xtable[  table[i].ngrams[e].len ] = '\0';
                printf("(%s)  (%d)\n", xtable, table[i].ngrams[e].freq);
            }
            tc->free(table[i].ngrams);
        }
        if (table[i].pool_size > 0) {
            tc->free(table[i].pool);
        }
    }
}
// }}}

// TextCat_Init(TextCat ** tcc) {{{
Bool TextCat_Init(TextCat ** tcc)
{
    TextCat * tc;
    tc = (TextCat *) malloc(sizeof(TextCat));
    tc->calloc  = calloc;
    tc->malloc  = malloc;
    tc->realloc = realloc;
    tc->free    = free;
    tc->ngram_precreate = TC_NGRAM_PRECREATE;
    tc->pool_preallocate_size = TC_BUFFER_SIZE;
    tc->hash_size = TC_HASH_SIZE;
    tc->min_ngram_len = MIN_NGRAM_LEN;
    tc->max_ngram_len = MAX_NGRAM_LEN;
    tc->last_status = TC_OK;
    tc->status = TC_FREE;
    *tcc = tc;
    return TC_TRUE;
}
// }}}

int TextCat_parse(TextCat * tc, const uchar * text, long length)
{
    uchar *t1;
    int i;

    tc->status = TC_BUSY; /* Set this instance as busy */
    if (textcat_init_hash(tc) == TC_FALSE) {
        tc->status = TC_FREE;
        return TC_FALSE;
    }

    t1 = text;
    while (*t1) {
        for (i=tc->min_ngram_len; i <= tc->max_ngram_len; i++) {
            if (t1-text > length-i) {
                break;
            }
            if (textcat_ngram_incr(tc, t1, i) == TC_FALSE) {
            }
        }
        t1++;
    }

    textcat_destroy_hash(tc);
    tc->free(tc->hash.table);
    tc->last_status = TC_OK;
    tc->status = TC_FREE;
    return TC_TRUE;
}

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
