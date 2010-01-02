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

int main(int argc, char * argv[])
{
    TextCat * tc;
    int i;

    TextCat_Init(&tc);
    for (i=1; i < argc; i++) {
        printf("Adding %s\n", basename(argv[i]));
        if (TextCat_parse_file(tc, argv[i] ,  NULL) == TC_FALSE) {
            printf("Error while reading %s\n", argv[i]);
            continue;
        }
        if (TextCat_save(tc, basename(argv[i])) == TC_FALSE) {
            printf("\tError while saving n-grams %s\n", basename(argv[i]));
        } else {
            printf("\tOK\n");
        }

    }
    TextCat_Destroy(tc);
    return 0;
}
