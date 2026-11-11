#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

/* Случайный текст из первых alphabet букв латиницы (a, b, c, ...) */
void   fill_text(char *s, size_t n, int alphabet, unsigned seed);
double now_ms(void);

#endif
