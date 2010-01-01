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

#define FILE_BUFFER 1024

Bool knowledge_save(void * memory, const uchar * id, NGrams * result)
{
    uchar * fname, * content;
    long i, ret, offset;
    int fd;

    fname = mempool_malloc(memory, strlen(id) + strlen(DIR_NAME) + 2);
    sprintf(fname, "%s/%s", DIR_NAME, id);
    mkdir(DIR_NAME, 0777);

    fd = open(fname, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd == -1) {
        return TC_FALSE;
    }
    content = mempool_malloc(memory, FILE_BUFFER); 
    if (content == NULL) {
        return TC_FALSE;
    }

    /* sort by freq */
    textcat_ngram_sort_by_freq(result);

    for(i=0,offset=0; i < result->size; i++) {
        if (offset + result->ngram[i].size >= FILE_BUFFER - 1) {
            ret    = write(fd, content, offset);
            offset = 0;
        }
        strncpy(content+offset, result->ngram[i].str, result->ngram[i].size);
        offset += result->ngram[i].size+1;
        *(content+ offset-1) = '\n';
    }
    ret = write(fd, content, offset);
    close(fd);
    return TC_TRUE;
}

Bool knowledge_list(void * memory, uchar *** list, int * size) 
{
    DIR * fd;
    struct dirent * info;
    int len, i;

    fd = opendir(DIR_NAME);
    if (fd == NULL) {
        return TC_FALSE;
    }
    len = -2; /* . and .. aren't files */
    i   = 0;
    while (readdir(fd))  len++;
    rewinddir(fd);
    *list = mempool_malloc(memory, len * sizeof(char *));
    if (*list == NULL) {
        return TC_FALSE;
    }
    while (info = readdir(fd)) {
        if (strcmp(info->d_name, ".") == 0 || strcmp(info->d_name, "..") == 0) {
            continue;
        }
        *(*list+i) = mempool_strdup(memory, info->d_name);
        if (*(*list+i) == NULL) {
            return TC_FALSE;
        }
        i++;
    }
    *size = len;
    closedir(fd);
    return TC_TRUE;
}

Bool knowledge_load(void * memory, const uchar * id, NGrams * result, int max)
{
    int i, fd;
    int bytes, offset;
    uchar * fname,  * content;

    fname = mempool_malloc(memory, strlen(id) + strlen(DIR_NAME) + 2);
    sprintf(fname, "%s/%s", DIR_NAME, id);

    fd = open(fname, O_RDONLY);
    printf("%s %d\n", fname, fd);
    if (fd == -1) {
        return TC_FALSE;
    }

    
    content = mempool_malloc(memory, FILE_BUFFER);
    do {
        offset = 0;
        bytes  = read(fd, content, FILE_BUFFER);

    } while (bytes > 0);
    

    close(fd);
    return TC_TRUE;
}
