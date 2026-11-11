#include "util.h"

#include <windows.h>

void fill_text(char *s, size_t n, int alphabet, unsigned seed)
{
    unsigned x = seed ? seed : 1;
    for (size_t i = 0; i < n; i++) {
        x ^= x << 13;          /* xorshift32 */
        x ^= x >> 17;
        x ^= x << 5;
        s[i] = (char)('a' + (x >> 8) % (unsigned)alphabet);
    }
}

double now_ms(void)
{
    static LARGE_INTEGER freq;
    LARGE_INTEGER t;
    if (freq.QuadPart == 0)
        QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t);
    return (double)t.QuadPart * 1000.0 / (double)freq.QuadPart;
}
