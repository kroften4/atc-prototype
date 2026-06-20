#ifndef __UTILS_H__
#define __UTILS_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define _THROW_NOT_YET_IMPLEMENTED()                               \
    do {                                                           \
        (void)fprintf(stderr, "%s not yet implemented", __func__); \
        exit(42);                                                  \
    } while (0)

static inline bool is_alpha(char ch)
{
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}
static inline bool is_capital(char ch)
{
    return ('A' <= ch && ch <= 'Z');
}
static inline bool is_numeric(char ch)
{
    return '0' <= ch && ch <= '9';
}
static inline bool is_dir(char ch)
{
    return ch == 'q' || ch == 'w' || ch == 'e' || ch == 'd' || ch == 'c' ||
           ch == 'x' || ch == 'z' || ch == 'a';
}

static inline int char_digit_to_int(char digit)
{
	return digit - '0';
}

static inline char int_digit_to_char(int digit)
{
    assert(digit >= 0 && digit <= 9);
	return digit + '0';
}



#endif // __UTILS_H__
