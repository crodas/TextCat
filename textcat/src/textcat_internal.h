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
   | Author:  Cesar Rodas <saddor@gmail.com>                              |
   +----------------------------------------------------------------------+
 */

#include <string.h>
#include <limits.h>
#ifdef HAVE_CONFIG
#   include "config.h"
#endif


#define CHECK_MEM(x)        if (x == NULL) { tc->error = TC_ERR_MEM; return TC_FALSE;  }
#define CHECK_MEM_EX(x,Y)   if (x == NULL) { tc->error = TC_ERR_MEM; Y; return TC_FALSE;  }

                                
#define INIT_MEMORY(x)      if (tc->x == NULL) { \
                                mempool_init(&tc->x, tc->malloc, tc->free, tc->allocate_size); \
                                CHECK_MEM_EX(tc->x, textcat_destroy_hash(tc); tc->status=TC_FREE;)  \
                            }

#define LOCK_INSTANCE(tc)   if (tc->status != TC_FREE) {\
                                tc->error = TC_BUSY; \
                                return TC_FALSE; \
                            } \
                            tc->status = TC_BUSY; /* lock it for our thread */ \
                            INIT_MEMORY(memory);  /* initialize tc->memory if it wasn't before */

#define UNLOCK_INSTANCE(tc)   tc->status = TC_FREE;

typedef struct {
    long dist;
    uchar * name;
} _cands;

/* Backward declarations {{{ */
Bool mempool_init(void ** memory, void * (*xmalloc)(size_t), void * (*xfree)(void *), size_t block_size);
void mempool_done(void ** memory);
void * mempool_calloc(void * memory, size_t nmemb, size_t size);
void * mempool_malloc(void * memory, size_t size);
uchar * mempool_strndup(void * memory, uchar * key, size_t len);
void mempool_reset(void * memory);


long textcat_simple_hash(const uchar *p, size_t len, size_t max_number);
Bool textcat_ngram_find(const ngram_set * nset, const uchar * key, size_t len, ngram_t ** item);
Bool textcat_ngram_create(TextCat * tc, ngram_set * nset, const uchar * key, size_t len, ngram_t ** item);
Bool textcat_ngram_incr(TextCat * tc, const uchar * key, size_t len);
Bool textcat_copy_result(TextCat * tc, NGrams ** result);
void textcat_ngram_sort_by_str(NGrams * ngrams);
void textcat_ngram_sort_by_freq(NGrams * ngrams);
Bool textcat_init_hash(TextCat * tc);
void textcat_destroy_hash(TextCat * tc);
/* */
Bool textcat_result_merge(TextCat *tc, result_stack * stack, NGrams ** result);
Bool knowledge_save(void *, const uchar * id, NGrams * ngrams);
Bool knowledge_list(void *, uchar *** list, int * size);
Bool knowledge_load(void * memory, const uchar * id, NGrams * result, int max);
long knowledge_dist(NGrams *a, NGrams *b);
Bool textcat_default_text_parser(TextCat *tc, const uchar * text, size_t length, int * (*set_ngram)(TextCat *, const uchar *, size_t));
/* }}} */

#define mempool_strdup(x,y) mempool_strndup(x, y, strlen(y))

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
