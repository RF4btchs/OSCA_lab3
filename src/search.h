#ifndef SEARCH_H
#define SEARCH_H

#include <stddef.h>

typedef struct {
    long long count;          /* сколько раз образец встретился */
    long long first;          /* позиция первого вхождения, -1 - не найден */
    int threads;              /* сколько рабочих потоков реально запущено */
} SearchResult;

/* Наивный поиск образца pat (длина m) в тексте text (длина n).
   Все позиции 0..n-m делятся на блоки, блоки разбирают max_threads рабочих потоков.
   verbose - печатать TID каждого потока. 0 - успех, -1 - ошибка (нет памяти). */
int parallel_search(const char *text, size_t n, const char *pat, size_t m,
                    int max_threads, int verbose, SearchResult *res);

#endif
