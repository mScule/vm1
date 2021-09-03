#include "str.h"
#include <stdio.h>

int main() {
    char *str1 = str_new("hello");
    char *str2 = str_new("one");
    
    str1 = str_combine(str1, str1);
    str1 = str_combine(str1, str1);
    str2 = str_combine(str2, str1);

    printf("string %s length: %d\n", str1, str_length(str1));
    printf("string %s length: %d\n", str2, str_length(str2));

    getchar();
}
