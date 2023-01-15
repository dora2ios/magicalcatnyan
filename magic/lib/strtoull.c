
#include <common.h>

#ifndef ULLONG_MAX
#define ULLONG_MAX (~(unsigned long long)0) /* 0xFFFFFFFFFFFFFFFF */
#endif

#ifndef LLONG_MAX
#define LLONG_MAX ((unsigned long long)(ULLONG_MAX >> 1)) /* 0x7FFFFFFFFFFFFFFF */
#endif

#ifndef LLONG_MIN
#define LLONG_MIN (~LLONG_MAX) /* 0x8000000000000000 */
#endif

int
isspace(c)
int c;
{
    return (c == '\t' || c == '\n' ||
            c == '\v' || c == '\f' || c == '\r' || c == ' ' ? 1 : 0);
}

static void fake(void)
{
    
}

unsigned long long
strtoull(const char * __restrict nptr, char ** __restrict endptr, int base)
{
    const char *s;
    unsigned long long acc;
    char c;
    unsigned long long cutoff;
    int neg, any, cutlim;
    
    /*
     * See strtoq for comments as to the logic used.
     */
    s = nptr;
    do {
        c = *s++;
    } while (isspace((unsigned char)c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else {
        neg = 0;
        if (c == '+')
            c = *s++;
    }
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;
    acc = any = 0;
    if (base < 2 || base > 36)
        goto noconv;
    
    cutoff = ULLONG_MAX / base;
    cutlim = ULLONG_MAX % base;
    for ( ; ; c = *s++) {
        if (c >= '0' && c <= '9')
            c -= '0';
        else if (c >= 'A' && c <= 'Z')
            c -= 'A' - 10;
        else if (c >= 'a' && c <= 'z')
            c -= 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = ULLONG_MAX;
        //errno = ERANGE;
    } else if (!any) {
    noconv:
        fake();
        //errno = EINVAL;
    } else if (neg)
        acc = -acc;
    
    if (endptr != NULL)
        *endptr = (char *)(any ? s - 1 : nptr);
    
    return (acc);
}
