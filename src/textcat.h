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
    size_t offset;
    int len;
    int status;
} ngram;

typedef struct {
    /* memory */
    unsigned char * pool; 
    int pool_number;
    long pool_offset;
    long pool_size;
    /* linked list */
    ngram * ngrams;
    long total;
    long size;
} ngram_set;

typedef struct {
    ngram_set * table;
    long total;
} ngram_hash;

typedef struct {
    void * (*malloc)(long);
    void * (*calloc)(long, long);
    void * (*realloc)(long, void *);
    void * (*free)(void *);
    int ngram_precreate;
    long pool_preallocate_size;
    int hash_size;
    ngram_hash hash;
    int min_ngram_len;
    int max_ngram_len;
    char * errstr;
    int last_status;
    int status;
} TextCat;


#define TC_HASH_SIZE        100
#define TC_BUFFER_SIZE      (16 * 1024) 
#define TC_NGRAM_PRECREATE  50 
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
