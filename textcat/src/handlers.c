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
#include <dirent.h>


#ifndef DIR_NAME
    #define DIR_NAME "./knowledge/"
#endif

Bool knowledge_save(TextCat * tc, const uchar * id, NGrams * result)
{
    uchar * fname, * content;
    long i, len, offset;
    int fd;

    fname = mempool_malloc(tc->memory, strlen(id) + strlen(DIR_NAME) + 2);
    sprintf(fname, "%s/%s", DIR_NAME, id);
    mkdir(DIR_NAME, 0777);
    fd = open(fname, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd == -1) {
        tc->error = TC_NO_FILE;
        return TC_FALSE;
    }
    len     = tc->max_ngrams * (tc->max_ngram_len +1) + 1;
    content = mempool_malloc(tc->memory, len); 
    offset  = 0;

    /* sort by freq */
    textcat_ngram_sort_by_freq(result);

    for(i=0; i < result->size; i++) {
        strncpy(content+offset, result->ngram[i].str, result->ngram[i].size);
        offset += result->ngram[i].size+1;
        *(content+ offset-1) = '\n';
    }
    *(content+offset) = '\0';
    i = write(fd, content, offset);
    close(fd);
    return TC_TRUE;
}

Bool knowledge_list(TextCat * tc, uchar *** list, int * size) 
{
    DIR * fd;
    struct dirent * info;
    int len, i;

    fd = opendir(DIR_NAME);
    if (fd == NULL) {
        tc->error = TC_NO_FILE;
        return TC_FALSE;
    }
    len = -2; /* . and .. aren't files */
    i   = 0;
    while (readdir(fd))  len++;
    rewinddir(fd);
    *list = mempool_malloc(tc->memory, len * sizeof(char *));
    CHECK_MEM(*list);
    while (info = readdir(fd)) {
        if (strcmp(info->d_name, ".") == 0 || strcmp(info->d_name, "..") == 0) {
            continue;
        }
        *(*list+i) = mempool_strdup(tc->memory, info->d_name);
        CHECK_MEM(*(*list+i));
        i++;
    }
    *size = len;
    closedir(fd);
    return TC_TRUE;
}
