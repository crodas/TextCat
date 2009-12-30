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

#define CHECK_MEM(x)   if (x == NULL) { tc->error = TC_ERR_MEM; return TC_FALSE;  }
#define CHECK_MEM_EX(x,Y)   if (x == NULL) { tc->error = TC_ERR_MEM; Y; return TC_FALSE;  }

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
/* */
Bool textcat_result_merge(TextCat *tc, result_stack * stack, NGrams ** result);
Bool knowledge_save(TextCat * tc, const uchar * id, NGrams * ngrams);
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
