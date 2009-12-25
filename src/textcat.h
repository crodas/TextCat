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

#include <stdlib.h>
#include <stdio.h>

typedef struct {
    long freq;
    unsigned char * str;
    int len;
    int status;
} ngram_t;

typedef struct {
    /* linked list */
    ngram_t * ngrams;
    long total;
    long size;
} ngram_set;

typedef struct {
    ngram_set * table;
    long size;
    long ngrams;
} ngram_hash;

typedef struct {
    /* pool of memory */
    void * memory;
    /* callback */
    void * (*malloc)(long);
    void * (*calloc)(long, long);
    void * (*realloc)(long, void *);
    void * (*free)(void *);
    /* config issues */
    int ngram_precreate;
    long allocate_size;
    int hash_size;
    int min_ngram_len;
    int max_ngram_len;
    /* internal stuff */
    ngram_hash hash;
    /* status */
    int error;
    int status;
} TextCat;


#define TC_HASH_SIZE        100
#define TC_BUFFER_SIZE      (16 * 1024) 
#define TC_NGRAM_PRECREATE  1
#define Bool            int
#define uchar           unsigned char
#define TC_TRUE         1
#define TC_FALSE        0
#define TC_OK           TC_TRUE
#define TC_ERR          -1
#define TC_ERR_MEM      -2
#define TC_FREE         1
#define TC_BUSY         0
#define MIN_NGRAM_LEN   2
#define MAX_NGRAM_LEN   5

Bool TextCat_Init(TextCat ** tc);
Bool TextCat_Destroy(TextCat * tc);
int TextCat_parse(TextCat * tc, const uchar * text, long length);

extern Bool mempool_init(TextCat * tc);
extern void mempool_done(TextCat * tc);
void * mempool_malloc(TextCat * tc, size_t size);
void * mempool_calloc(TextCat * tc, size_t nmemb, size_t size);
uchar * mempool_strndup(TextCat * tc, uchar * key, size_t len);
