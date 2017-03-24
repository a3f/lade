#include "cli-utils.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

char *strpcat(char *restrict s1, const char *restrict s2)
{
    s1 = strchr(s1, '\0');
    while (*s2)
        *s1++ = *s2++;
    *s1 = '\0';
    return s1;
}

char *strdup(const char*str) {
    size_t size = strlen(str) + 1;
    char *buf = malloc(size);
    return memcpy(buf, str, size);
}

char *concat(const char *first, ...)
{
    if ( first == NULL) return NULL;
    if (*first == '\0') return strdup("");

    va_list args4len, args4cat;
    va_start(args4len, first);
    va_copy(args4cat, args4len);

    const char *arg = first;
    size_t len = 0;
    do len += strlen(arg); while((arg = va_arg(args4len, char*)));

    va_end(args4len);

    char *buf = malloc(len + 1);
    *buf = '\0';
    char *p = buf;

    arg = first;
    do strcat(p, arg); while((arg = va_arg(args4cat, char*)));

    va_end(args4cat);

    return buf;
}
