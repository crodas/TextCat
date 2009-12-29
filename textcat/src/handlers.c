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

#ifndef DIR_NAME
    #define DIR_NAME "./ngrams/"
#endif

Bool knowledge_save(TextCat * tc, const uchar * id, NGrams * result)
{
    uchar * fname, * content;
    long i, len, offset;
    int fd;

    fname = mempool_malloc(tc->memory, strlen(id) + strlen(DIR_NAME) + 2);
    sprintf(fname, "%s/%s", DIR_NAME, id);
    printf("filename: %s (%d ngrams)\n", fname, result->size);
    mkdir(DIR_NAME, 0777);
    fd = open(fname, O_CREAT | O_TRUNC | O_WRONLY);
    if (fd == -1) {
        tc->error = TC_NO_FILE;
        return TC_FALSE;
    }
    len     = tc->max_ngrams * (tc->max_ngram_len +1) + 1;
    content = mempool_malloc(tc->memory, len); 
    offset  = 0;

    /* sort by freq */
    textcat_ngram_sort_by_freq(result);

    for(i=0; i < tc->max_ngrams; i++) {
        strncpy(content+offset, result->ngram[i].str, result->ngram[i].size);
        offset += result->ngram[i].size+1;
        *(content+ offset-1) = '\n';
    }
    *(content+offset) = '\0';
    i = write(fd, content, offset);
    close(fd);
}
