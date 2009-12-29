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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


// Definitions {{{
#define TC_HASH_SIZE    100
#define TC_BUFFER_SIZE  (16 * 1024) 
#define TC_MAX_NGRAMS   400
#define Bool            char
#define uchar           unsigned char
#define TC_TRUE         1
#define TC_FALSE        0
#define TC_FREE         1
#define TC_BUSY         0
#define MIN_NGRAM_LEN   2
#define MAX_NGRAM_LEN   5

#define TC_OK               TC_TRUE
#define TC_ERR              -1
#define TC_ERR_MEM          -2
#define TC_NO_FILE          -3
#define TC_ERR_FILE_SIZE    -4
#define TC_NO_NGRAM         -5
// }}}

// Data types {{{
typedef struct {
    uchar * str;
    long freq;
    int len;
    struct ngram_t * next;
} ngram_t;

typedef struct {
    /* linked list */
    ngram_t * first;
    ngram_t * last;
    long total;
} ngram_set;

typedef struct {
    ngram_set * table;
    long size;
    long ngrams;
} ngram_hash;

typedef struct result_stack {
    struct NGrams * result;
    struct result_stack * next;
} result_stack;

typedef struct TextCat {
    /* pools of memory */
    void * temp;   /* temporary memory */
    void * memory; /* "return" structures and internal stack */

    /* callback */
    void * (*malloc)(size_t);
    void * (*free)(void *);
    Bool * (*parse_str)(struct TextCat *, uchar *, size_t , int * (*set_ngram)(struct TextCat *, const uchar *, size_t));
    Bool * (*save)(struct TextCat *, const uchar *, struct NGrams *);
    /* config issues */
    size_t allocate_size;
    int hash_size;
    int min_ngram_len;
    int max_ngram_len;
    int max_ngrams;
    /* internal stuff */
    ngram_hash hash;
    result_stack  * results;
    /* status */
    int error;
    int status;
} TextCat;


typedef struct {
    uchar * str;
    int size;
    long freq;
    long position;
} NGram;

typedef struct NGrams {
    NGram * ngram;
    long size;
} NGrams;
// }}}


Bool TextCat_Init(TextCat ** tc);
Bool TextCat_Destroy(TextCat * tc);
int TextCat_parse(TextCat * tc, const uchar * text, size_t length, NGrams ** ngram);
void TextCat_reset_handlers(TextCat * tc);

Bool mempool_init(void ** memory, void * (*xmalloc)(size_t), void * (*xfree)(void *), size_t block_size);
void mempool_done(void ** memory);
void * mempool_calloc(void * memory, size_t nmemb, size_t size);
void * mempool_malloc(void * memory, size_t size);
uchar * mempool_strndup(void * memory, uchar * key, size_t len);
void mempool_reset(void * memory);

/* Backward declarations {{{ */
long textcat_simple_hash(const uchar *p, size_t len, size_t max_number);
Bool textcat_ngram_find(const ngram_set * nset, const uchar * key, size_t len, ngram_t ** item);
Bool textcat_ngram_create(TextCat * tc, ngram_set * nset, const uchar * key, size_t len, ngram_t ** item);
Bool textcat_ngram_incr(TextCat * tc, const uchar * key, size_t len);
Bool textcat_copy_result(TextCat * tc, NGrams ** result);
void textcat_sort_result(NGrams * ngrams);
int textcat_qsort_fnc_freq(const void * a, const void * b);
int textcat_qsort_fnc_str(const void * a, const void * b);
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
