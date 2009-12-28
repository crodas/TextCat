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

/* Backward declarations {{{ */
long textcat_simple_hash(const uchar *p, size_t len);
Bool textcat_ngram_find(const ngram_set * nset, const uchar * key, size_t len, ngram_t ** item);
Bool textcat_ngram_create(TextCat * tc, ngram_set * nset, const uchar * key, size_t len, ngram_t ** item);
int textcat_ngram_incr(TextCat * tc, const uchar * key, size_t len);
Bool textcat_copy_result(TextCat * tc, NGrams ** result);
void textcat_sort_result(NGrams * ngrams);
int textcat_qsort_fnc_freq(const void * a, const void * b);
int textcat_qsort_fnc_str(const void * a, const void * b);
static int textcat_default_text_parser(TextCat *tc, const uchar * text, size_t length, int * (*set_ngram)(TextCat *, const uchar *, size_t));
/* }}} */

// TextCat_Init(TextCat ** tcc) {{{
Bool TextCat_Init(TextCat ** tcc)
{
    TextCat * tc;
    tc = (TextCat *) malloc(sizeof(TextCat));
    if (tc == NULL) {
        return TC_FALSE;
    }
    tc->malloc        = malloc;
    tc->free          = free;
    tc->allocate_size = TC_BUFFER_SIZE;
    tc->hash_size     = TC_HASH_SIZE;
    tc->min_ngram_len = MIN_NGRAM_LEN;
    tc->max_ngram_len = MAX_NGRAM_LEN;
    tc->max_ngrams    = TC_MAX_NGRAMS;
    tc->error         = TC_OK;
    tc->status        = TC_FREE;
    tc->result        = NULL;
    *tcc              = tc;
    TextCat_reset_handlers(tc);
    return TC_TRUE;
}
// }}}

// TextCat_parse(TextCat * tc, const uchar * text, size_t length,  NGrams ** ngrams) {{{
int TextCat_parse(TextCat * tc, const uchar * text, size_t length,  NGrams ** ngrams)
{
    tc->status = TC_BUSY; /* Set this instance as busy */
    if (textcat_init_hash(tc) == TC_FALSE) {
        tc->status = TC_FREE;
        return TC_FALSE;
    }

    if (tc->result == NULL) {
        mempool_init(&tc->result, tc->malloc, tc->free, tc->allocate_size);
        CHECK_MEM_EX(tc->result, textcat_destroy_hash(tc); tc->status=TC_FREE; )
    }
    
    if (tc->parse_str(tc, text, length, &textcat_ngram_incr) == TC_FALSE) {
        textcat_destroy_hash(tc);
        tc->status = TC_FREE;
        return TC_FALSE;
    }
    if (textcat_copy_result(tc, ngrams) == TC_FALSE) {
        textcat_destroy_hash(tc);
        tc->status = TC_FREE;
        return TC_FALSE;
    }
    textcat_sort_result(*ngrams);

    textcat_destroy_hash(tc);
    tc->error  = TC_OK;
    tc->status = TC_FREE;
    return TC_TRUE;
}
// }}}

// TextCat_parse_file(TextCat * tc, const uchar * filename, NGrams ** ngrams) {{{
int TextCat_parse_file(TextCat * tc, const uchar * filename, NGrams ** ngrams)
{
    int fd;
    size_t bytes;
    uchar * buffer;
    struct stat info;

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        tc->error = TC_NO_FILE;
        return TC_FALSE;
    }
    if (fstat(fd, &info) == -1) {
        tc->error = TC_NO_FILE;
        return TC_FALSE;
    }
    buffer = malloc(info.st_size + 1);
    bytes  = read(fd, buffer, info.st_size);
    if (bytes != info.st_size) {
        free(buffer);
        tc->error = TC_ERR_FILE_SIZE;
        return TC_FALSE;
    }
    close(fd);
    *(buffer+bytes) = '\0';
    fd = TextCat_parse(tc, buffer, bytes, ngrams);
    free(buffer);
    return fd;
}
// }}}

// Default Parsing text callback {{{
static int textcat_default_text_parser(TextCat *tc, const uchar * text, size_t length, int * (*set_ngram)(TextCat *, const uchar *, size_t))
{
    int i,e;
    uchar *ntext;
    /* create a copy of the text in order to do a best-effort
     * to clean it, setting everything to lower-case, removing
     * non-alpha and whitespaces.
     */
    ntext = mempool_strndup(tc->memory, text, length);
    for (i=0, e=0; i < length; i++) {
        if (isalpha(ntext[i])) {
            ntext[e++] = tolower(ntext[i]);
        }
        if (isblank(text[i])) {
            while (isblank(ntext[++i]));
            ntext[e++] = ' ';
            --i;
        }
    }
    length = e;
    /* extract the ngrams, and pass-it to the library (with the callback */
    for (e=0; e < length; e++) {
        for (i=tc->min_ngram_len; i <= tc->max_ngram_len; i++) {
            if (length-e < tc->min_ngram_len) {
                break;
            }
            if (set_ngram(tc, ntext+e, i) == TC_FALSE) {
                return TC_FALSE;
            }
        }
    }
    return TC_TRUE;
}
// }}}

// TextCat_reset_handler(TextCat * tc) {{{
void TextCat_reset_handlers(TextCat * tc)
{
    tc->parse_str = &textcat_default_text_parser;
}
// }}}

// TextCat_Destroy(TextCat * tc) {{{
Bool TextCat_Destroy(TextCat * tc) 
{
    if (tc->result != NULL) {
        mempool_done(tc->result);
    }
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
