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

/* Backward declarations {{{ */
static long textcat_simple_hash(const uchar *p, int len);
static Bool textcat_ngram_find(const ngram_set * nset, const uchar * key, int len, ngram_t ** item);
static Bool textcat_ngram_create(TextCat * tc, ngram_set * nset, const uchar * key, int len, ngram_t ** item);
static int textcat_ngram_incr(TextCat * tc, const uchar * key, int len);
/* }}} */

// simple_hash(const uchar *, int) {{{
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

// textcat_find_ngram(const ngram_set *, const uchar *, int, ngram **) {{{
static Bool textcat_ngram_find(const ngram_set * nset, const uchar * key, int len, ngram_t ** item)
{
    ngram_t * entry;
    
    for (entry = nset->first; entry!=NULL; entry = entry->next) 
    {
        if (entry->len == len && strncmp(entry->str, key, len) == 0) {
            *item = entry;
            return TC_TRUE;
        }
    }
    return TC_FALSE;
}
// }}}

// textcat_ngram_incr(TextCat *, ngram_set *, const uchar *, int) {{{
static Bool textcat_ngram_incr(TextCat * tc, const uchar * key, int len)
{
    ngram_t * item;
    ngram_set * nset;
    int spot;
    spot = textcat_simple_hash(key, len) & (tc->hash_size - 1);
    nset = &(tc->hash.table[spot]);
    if (textcat_ngram_find(nset, key, len, &item) == TC_TRUE) {
        item->freq++;
    } else {
        if (textcat_ngram_create(tc, nset, key, len, &item)  == TC_FALSE) {
            return TC_FALSE;
        }
        item->freq++;
    }
    return TC_TRUE;
}
// }}}

// textcat_ngram_create(TextCat *, ngram_set *, const uchar *, int, ngram **) {{{
static Bool textcat_ngram_create(TextCat * tc, ngram_set * nset, const uchar * key, int len, ngram_t ** ritem)
{
    ngram_t * item;
    item = mempool_malloc(tc, sizeof(ngram_t));
    if (item == NULL) {
        return TC_FALSE;
    }
    /* setup the new N-gram */
    item->str  = mempool_strndup(tc, key, len);
    item->freq = 0;
    item->len  = len;
    item->next = NULL;

    if (item->str == NULL) {
        return TC_FALSE;
    }
    if (nset->first == NULL) {
        nset->first = item;
    }
    if (nset->last != NULL) {
        nset->last->next = item;
    }
    *ritem = item;
    nset->last = item;
    nset->total++;
    tc->hash.ngrams++;

    return TC_TRUE;
} 
// }}}

// textcat_init_hash(TextCat * tc) {{{
static Bool textcat_init_hash(TextCat * tc)
{
    ngram_set * table;
    int i;

    mempool_reset(tc);

    table = mempool_calloc(tc, tc->hash_size, sizeof(ngram_set));
    if (table == NULL) {
        return TC_FALSE;
    }
    for (i=0; i < tc->hash_size; i++) {
        table[i].first = NULL;
        table[i].last  = NULL;
        table[i].total = 0;
    }

    tc->hash.table  = table;
    tc->hash.ngrams = 0;
    tc->hash.size   = tc->hash_size;
    return TC_TRUE; 
}
// }}}

// textcat_destroy_hash(TextCat * tc)  {{{
static void textcat_destroy_hash(TextCat * tc) 
{
    int i;
    ngram_t * entry;
    for (i=0; i < tc->hash.size; i++) {
        for (entry = tc->hash.table[i].first; entry ; entry = entry->next) {
            printf("(%s)  (%d)\n", entry->str, entry->freq);
        }
    }
    printf("Ngrams: %d\n", tc->hash.ngrams);
}
// }}}

// TextCat_Init(TextCat ** tcc) {{{
Bool TextCat_Init(TextCat ** tcc)
{
    TextCat * tc;
    tc = (TextCat *) malloc(sizeof(TextCat));
    tc->malloc          = malloc;
    tc->free            = free;
    tc->allocate_size   = TC_BUFFER_SIZE;
    tc->hash_size       = TC_HASH_SIZE;
    tc->min_ngram_len   = MIN_NGRAM_LEN;
    tc->max_ngram_len   = MAX_NGRAM_LEN;
    tc->error   = TC_OK;
    tc->status  = TC_FREE;
    if (mempool_init(tc) == TC_FALSE) {
        free(tc);
        return TC_FALSE;
    }
    *tcc = tc;
    return TC_TRUE;
}
// }}}

int TextCat_parse(TextCat * tc, const uchar * text, long length,  NGram ** ngrams)
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
                textcat_destroy_hash(tc);
                tc->status = TC_FREE;
                return TC_FALSE;
            }
        }
        t1++;
    }

    textcat_destroy_hash(tc);
    tc->error  = TC_OK;
    tc->status = TC_FREE;
    return TC_TRUE;
}

// TextCat_Destroy(TextCat * tc) {{{
Bool TextCat_Destroy(TextCat * tc) 
{
    mempool_done(tc);
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
