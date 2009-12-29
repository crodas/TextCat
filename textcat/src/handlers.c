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

Bool knowledge_save(TextCat * tc, const uchar * id, NGrams * ngrams)
{
    uchar * fname;
    fname = mempool_malloc(tc->memory, strlen(id) + strlen(DIR_NAME) + 2);
    sprintf(fname, "%s/%s", DIR_NAME, id);
    printf("filename: %s\n", fname);
    mkdir(DIR_NAME, 0777);
}
