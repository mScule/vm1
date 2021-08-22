#include "str.h"

#include <stdlib.h>

int str_length(char *str)
{
    int i = 0;

    while(str[i] != '\0')
        i++;
    return i;
}

int str_is_equal(char *str1, char *str2)
{
    int
        str1_len = str_length(str1),
        str2_len = str_length(str2);

    if(str1_len == str2_len)
    {
        for(int i = 0; i < str1_len; i++)
            if(str1[i] != str2[i])
                return 0;
        return 1;
    }

    else
        return 0;
}

char *str_new(char *str)
{
    int str_len = str_length(str);

    char *new = (char*) malloc(sizeof(char) * str_len);

    for(int i = 0; i < str_len; i++)
        new[i] = str[i];

    new[str_len] = '\0';

    return new;
}

char *str_append(char *str, char *append)
{
    int
        str_len    = str_length(str),
        append_len = str_length(append),
        new_len    = str_len + append_len;

    char *new = (char*) malloc(sizeof(char) * new_len);

    int i1 = 0, i2;

    for(i2 = 0; i2 < str_len; i2++)
    {
        new[i1] = str[i2];
        i1++;
    }

    for(i2 = 0; i2 < append_len; i2++)
    {
        new[i1] = append[i2];
        i1++;
    }

    new[new_len] = '\0';

    free(str);

    return new;
}
