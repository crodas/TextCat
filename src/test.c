#include "textcat.h"

int main()
{
    char  * text[] = {"cesar cesar cesar cesar", "another foobar testing"};
    TextCat * tc;
    NGrams * t1, * t2;
    int i;
    TextCat_Init(&tc);
        TextCat_parse(tc, text[0], strlen(text[0]), &t1);
        for (i=0; i < t1->size; i++) {
            printf("Token=(%s), Freq=%d, Post=%d\n", t1->ngram[i].str, t1->ngram[i].freq, t1->ngram[i].position);
        }
        TextCat_parse(tc, text[1], strlen(text[1]), &t2);
    TextCat_Destroy(tc);
}
