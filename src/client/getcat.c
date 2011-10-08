/*
   +----------------------------------------------------------------------+
   | Copyright (c) 2011 The PHP Group                                     |
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

int main(int argc, char * argv[])
{
    TextCat * tc;
    int i;
    uchar ** langs;
    int total;


    TextCat_Init(&tc);
    for(i=1; i < argc;i++) {
        printf("Text: %s\n", argv[i]);
        TextCat_getCategory(tc, argv[i], strlen(argv[i]), &langs, &total);
        printf("\tLanguage: %s (outof %d)\n", langs[0], total);
    }
    TextCat_Destroy(tc);
    return 0;
}
