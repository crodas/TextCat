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

#define CHECK_MEM(x)   if (x == NULL) { tc->error = TC_ERR_MEM; return TC_FALSE;  }
#define CHECK_MEM_EX(x,Y)   if (x == NULL) { tc->error = TC_ERR_MEM; Y; return TC_FALSE;  }

// simple_hash(const uchar *, int) {{{
/*
 * fast and furious little hash function
 *
 * (Note that we could use some kind of rolling checksum, and update it
 * during n-gram construction) 
 */
long textcat_simple_hash(const uchar *p, size_t len)
{
	long h = len * 13;
	while (*p && len-- > 0) {
		h = (h<<5)-h + *p++;
	}
	return (long)h;
}
// }}}

// textcat_find_ngram(const ngram_set *, const uchar *, int, ngram **) {{{
Bool textcat_ngram_find(const ngram_set * nset, const uchar * key, size_t len, ngram_t ** item)
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

// textcat_ngram_incr(TextCat *, ngram_set *, const uchar *, size_t) {{{
Bool textcat_ngram_incr(TextCat * tc, const uchar * key, size_t len)
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
Bool textcat_ngram_create(TextCat * tc, ngram_set * nset, const uchar * key, size_t len, ngram_t ** ritem)
{
    ngram_t * item;
    item = mempool_malloc(tc->memory, sizeof(ngram_t));
    CHECK_MEM(item)

    /* setup the new N-gram */
    item->str  = mempool_strndup(tc->memory, key, len);
    item->freq = 0;
    item->len  = len;
    item->next = NULL;

    CHECK_MEM(item->str);
    
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
Bool textcat_init_hash(TextCat * tc)
{
    ngram_set * table;
    int i;


    mempool_init(&tc->memory, tc->malloc, tc->free, tc->allocate_size);
    CHECK_MEM(tc->memory)

    table = mempool_calloc(tc->memory, tc->hash_size, sizeof(ngram_set));

    CHECK_MEM(table)

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
void textcat_destroy_hash(TextCat * tc) 
{
    mempool_done(tc->memory);
}
// }}}

//textcat_copy_result(TextCat * tc, NGrams ** result) {{{
Bool textcat_copy_result(TextCat * tc, NGrams ** result)
{
    NGrams * ngrams;
    ngram_t * entry;
    int i, e;

    ngrams = (NGrams *) mempool_malloc(tc->result, sizeof(NGrams));
    CHECK_MEM(ngrams);
    ngrams->ngram = (NGram *) mempool_calloc(tc->result, tc->hash.ngrams, sizeof(NGram));
    CHECK_MEM(ngrams->ngram);
    ngrams->size = tc->hash.ngrams;

    for (i=0, e=0; i < tc->hash.size; i++) {
        for (entry = tc->hash.table[i].first; entry ; entry = entry->next) {
            ngrams->ngram[e].str      = mempool_strndup(tc->result, entry->str, entry->len);
            ngrams->ngram[e].freq     = entry->freq;
            ngrams->ngram[e].position = 0;
            CHECK_MEM(ngrams->ngram[e].str);
            e++;
        }
    }
    *result = ngrams;
    return TC_TRUE;
}
// }}}

// Sorting {{{
int textcat_qsort_fnc_freq(const void * a, const void * b)
{
    NGram *aa, *bb;
    aa = (NGram *) a;
    bb = (NGram *) b;
    return bb->freq - aa->freq;
}

int textcat_qsort_fnc_str(const void * a, const void * b)
{
    NGram *aa, *bb;
    aa = (NGram *) a;
    bb = (NGram *) b;
    return strcmp(aa->str, bb->str);
}

void textcat_sort_result(NGrams * ngrams)
{
    int i;
    /* sorting by freq */
    qsort(ngrams->ngram, ngrams->size, sizeof(NGram), textcat_qsort_fnc_freq);
    /* set their ranking */
    for (i=0; i < ngrams->size; i++) {
        ngrams->ngram[i].position = i;
    }
    /* sort by string, for fast comparition */
    qsort(ngrams->ngram, ngrams->size, sizeof(NGram), textcat_qsort_fnc_str);
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
