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
   | Author:  César Rodas <crodas@member.fsf.org>                         |
   +----------------------------------------------------------------------+
 */

#include "textcat.h"
#include "textcat_internal.h"

/* Backward declarations {{{ */
static Bool textcat_default_text_parser(TextCat *tc, const uchar * text, size_t length, int * (*set_ngram)(TextCat *, const uchar *, size_t));
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
    *tcc              = tc;
    tc->memory        = NULL;
    tc->temp          = NULL;
    tc->results       = NULL;
    tc->knowledges    = NULL;
    tc->nknowledges   = 0;
    TextCat_reset_handlers(tc);
    return TC_TRUE;
}
// }}}

// TextCat_reset(Textcat *) {{{
Bool TextCat_reset(TextCat * tc)
{
    LOCK_INSTANCE(tc);
    if (tc->memory != NULL) {
        mempool_reset(tc->memory);
    }
    if (tc->temp != NULL) {
        mempool_reset(tc->temp);
    }
    UNLOCK_INSTANCE(tc);
    tc->results     = NULL;
    tc->knowledges  = NULL;
    tc->nknowledges = 0;
    return TC_TRUE;
}
// }}}

// TextCat_reset_handler(TextCat * tc) {{{
Bool TextCat_reset_handlers(TextCat * tc)
{
    LOCK_INSTANCE(tc);
    tc->parse_str = &textcat_default_text_parser;
    tc->save      = &knowledge_save;
    tc->list      = &knowledge_list;
    tc->load      = &knowledge_load;
    UNLOCK_INSTANCE(tc);
    return TC_TRUE;
}
// }}}

// TextCat_Destroy(TextCat * tc) {{{
Bool TextCat_Destroy(TextCat * tc) 
{
    LOCK_INSTANCE(tc);
    if (tc->memory != NULL) {
        mempool_done(&tc->memory);
    }
    if (tc->temp != NULL) {
        mempool_done(&tc->temp);
    }
    free(tc);
}
// }}}

// TextCat_parse(TextCat * tc, const uchar * text, size_t length,  NGrams ** ngrams) {{{
int TextCat_parse(TextCat * tc, const uchar * text, size_t length,  NGrams ** ngrams)
{
    NGrams * result;
    result_stack * stack, *stack_temp;

    LOCK_INSTANCE(tc);

    if (textcat_init_hash(tc) == TC_FALSE) {
        UNLOCK_INSTANCE(tc);
        return TC_FALSE;
    }
    
    if (tc->parse_str(tc, text, length, &textcat_ngram_incr) == TC_FALSE) {
        textcat_destroy_hash(tc);
        UNLOCK_INSTANCE(tc);
        return TC_FALSE;
    }
    if (textcat_copy_result(tc, &result) == TC_FALSE) {
        textcat_destroy_hash(tc);
        UNLOCK_INSTANCE(tc);
        return TC_FALSE;
    }

    /* add the result to our Result Stack {{{ */
    stack = mempool_malloc(tc->memory, sizeof(result_stack));
    CHECK_MEM_EX(stack, textcat_destroy_hash(tc); tc->status=TC_FREE; )
    stack->result = result;
    stack->next   = NULL;
    if (tc->results == NULL) {
        tc->results = stack;
    } else {
        stack_temp = tc->results;
        while (stack_temp->next != NULL) {
            stack_temp = stack_temp->next;
        }
        stack_temp->next = stack;
    }
    /* }}} */

    if (ngrams != NULL) {
        *ngrams = result;
    }

    textcat_destroy_hash(tc);
    tc->error  = TC_OK;
    UNLOCK_INSTANCE(tc);
    return TC_TRUE;
}
// }}}

// TextCat_parse_file(TextCat * tc, const uchar * filename, NGrams ** ngrams) {{{
int TextCat_parse_file(TextCat * tc, const uchar * filename, NGrams ** ngrams)
{
    int fd;
    size_t bytes;
    uchar * buffer;
    NGrams * result;
    result_stack * stack, *stack_temp;

    LOCK_INSTANCE(tc);
    if (textcat_init_hash(tc) == TC_FALSE) {
        UNLOCK_INSTANCE(tc);
        return TC_FALSE;
    }
    

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        UNLOCK_INSTANCE(tc);
        tc->error = TC_NO_FILE;
        return TC_FALSE;
    }

    buffer = mempool_malloc(tc->temp, 1024);
    CHECK_MEM(buffer);

    do {
        bytes = read(fd, buffer, 1024);
        if (bytes && tc->parse_str(tc, buffer, bytes, &textcat_ngram_incr) == TC_FALSE) {
            textcat_destroy_hash(tc);
            UNLOCK_INSTANCE(tc);
            return TC_FALSE;
        }
    } while (bytes > 0);
    close(fd);

    if (textcat_copy_result(tc, &result) == TC_FALSE) {
        textcat_destroy_hash(tc);
        UNLOCK_INSTANCE(tc);
        return TC_FALSE;
    }

    /* add the result to our Result Stack {{{ */
    stack = mempool_malloc(tc->memory, sizeof(result_stack));
    CHECK_MEM_EX(stack, textcat_destroy_hash(tc); tc->status=TC_FREE; )
    stack->result = result;
    stack->next   = NULL;
    if (tc->results == NULL) {
        tc->results = stack;
    } else {
        stack_temp = tc->results;
        while (stack_temp->next != NULL) {
            stack_temp = stack_temp->next;
        }
        stack_temp->next = stack;
    }
    /* }}} */

    if (ngrams != NULL) {
        *ngrams = result;
    }

    textcat_destroy_hash(tc);
    tc->error  = TC_OK;

    UNLOCK_INSTANCE(tc);
    return TC_TRUE;
}
// }}}

// TextCat_save(TextCat *, unsigned uchar *) {{{
Bool TextCat_save(TextCat * tc, const uchar * id)
{
    NGrams * results;

    LOCK_INSTANCE(tc);

    if (tc->results == NULL) {
        tc->error = TC_NO_NGRAM;
        return TC_FALSE;
    }
    if (textcat_result_merge(tc, tc->results, &results) == TC_FALSE) {
        return TC_FALSE;
    }
    if (tc->save(tc->temp, id, results) == TC_FALSE) {
        tc->error = TC_ERR_CALLBACK;
        return TC_FALSE;
    }
    TextCat_reset(tc);
    UNLOCK_INSTANCE(tc);
    return TC_TRUE;
}
// }}}

// TextCat_list(TextCat * tc, uchar *** list, int * len) {{{
Bool TextCat_list(TextCat * tc, uchar *** list, int * len)
{
    Bool ret;
    if (tc->knowledges ==  NULL) {
        if (tc->list(tc->memory, &tc->knowledges, &tc->nknowledges) == TC_FALSE) {
            tc->error = TC_ERR_CALLBACK;
            return TC_FALSE;
        }
    }
    if (list != NULL) {
        *list = tc->knowledges;
        *len  = tc->nknowledges;
    }
    return TC_TRUE;
}
// }}}

Bool TextCat_load(TextCat *tc) 
{
    uchar ** list;
    int len;
    int i;
    LOCK_INSTANCE(tc);
    if (TextCat_list(tc, NULL, NULL) == TC_FALSE) {
        UNLOCK_INSTANCE(tc);
        tc->error = TC_ERR_CALLBACK;
        return TC_FALSE;
    }
    for (i=0; i < tc->nknowledges; i++) {
        tc->load(tc->memory, tc->knowledges[i], NULL, tc->max_ngrams);
    }
    UNLOCK_INSTANCE(tc);
}

// Default Parsing text callback {{{
static Bool textcat_default_text_parser(TextCat *tc, const uchar * text, size_t length, int * (*set_ngram)(TextCat *, const uchar *, size_t))
{
    int i,e,x, valid;
    uchar *ntext;
    /* create a copy of the text in order to do a best-effort
     * to clean it, setting everything to lower-case, removing
     * non-alpha and whitespaces.
     */
    ntext = mempool_malloc(tc->temp,length+1);
    for (i=0, e=0; i < length; i++) {
        if (isalpha(text[i])) {
            ntext[e++] = tolower(text[i]);
        } else {
            while (++i < length && !isalpha(text[i]));
            ntext[e++] = ' ';
            i--;
        }
    }
    ntext[e++] = '\0';
    length     = e - 1;
    /* extract the ngrams, and pass-it to the library (with the callback) */
    for (e=0; e < length; e++) {
        for (i=tc->min_ngram_len; i <= tc->max_ngram_len; i++) {
            if (e+i > length) {
                break;
            }

            /* allow spaces only at the beging and end (in order to reduce n-grams quantities) {{{ */
            valid = 1;
            for (x=1; x < i-1; x++) {
                if (isblank(*(ntext+e+x))) {
                    valid = 0;
                    break;
                }
            }
            if (valid==0) {
                continue;
            }
            /* }}} */

            if (set_ngram(tc, ntext+e, i) == TC_FALSE) {
                return TC_FALSE;
            }
        }
    }
    return TC_TRUE;
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
