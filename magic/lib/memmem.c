
#include <stdint.h>
#include <common.h>

void *
memmem (const void *haystack, size_t hs_len, const void *needle, size_t ne_len)
{
    const char *hs = haystack;
    const char *ne = needle;
    
    if (ne_len == 0)
        return (void *)hs;
    int i;
    int c = ne[0];
    const char *end = hs + hs_len - ne_len;
    
    for ( ; hs <= end; hs++)
    {
        if (hs[0] != c)
            continue;
        for (i = ne_len - 1; i != 0; i--)
            if (hs[i] != ne[i])
                break;
        if (i == 0)
            return (void *)hs;
    }
    
    return NULL;
}

