#include "search.h"

#include <stdio.h>
#include <windows.h>

typedef struct {
    const char *text;
    const char *pat;
    size_t m;
    size_t positions;         /* сколько позиций проверять: n - m + 1 */
    size_t block;             /* позиций в одном блоке */
    LONG blocks;
    volatile LONG next;       /* номер следующего свободного блока */
    long long *count;         /* результат по каждому блоку */
    long long *first;
    int verbose;
} Job;

/* Наивный алгоритм: в каждой позиции сравниваем образец посимвольно */
static void search_block(Job *j, LONG b)
{
    size_t from = (size_t)b * j->block;
    size_t to = from + j->block;
    if (to > j->positions)
        to = j->positions;

    long long cnt = 0, first = -1;
    for (size_t i = from; i < to; i++) {
        size_t k = 0;
        while (k < j->m && j->text[i + k] == j->pat[k])
            k++;
        if (k == j->m) {
            if (first < 0)
                first = (long long)i;
            cnt++;
        }
    }
    j->count[b] = cnt;        /* у каждого блока своя ячейка */
    j->first[b] = first;
}

/* Поток берёт блоки, пока они не кончатся */
static DWORD WINAPI worker(LPVOID arg)
{
    Job *j = arg;
    int done = 0;
    if (j->verbose)
        printf("  [TID=%lu] start\n", GetCurrentThreadId());
    for (;;) {
        LONG b = InterlockedIncrement(&j->next) - 1;
        if (b >= j->blocks)
            break;
        search_block(j, b);
        done++;
    }
    if (j->verbose)
        printf("  [TID=%lu] done, %d blocks\n", GetCurrentThreadId(), done);
    return 0;
}

int parallel_search(const char *text, size_t n, const char *pat, size_t m,
                    int max_threads, int verbose, SearchResult *res)
{
    HANDLE heap = GetProcessHeap();
    Job j = {0};
    j.text = text;
    j.pat = pat;
    j.m = m;
    j.positions = n - m + 1;
    j.verbose = verbose;

    /* примерно 8 блоков на поток */
    j.block = j.positions / ((size_t)max_threads * 8);
    if (j.block < 65536)
        j.block = 65536;
    j.blocks = (LONG)((j.positions + j.block - 1) / j.block);

    j.count = HeapAlloc(heap, 0, j.blocks * sizeof(long long));
    j.first = HeapAlloc(heap, 0, j.blocks * sizeof(long long));
    HANDLE *th = HeapAlloc(heap, HEAP_ZERO_MEMORY, max_threads * sizeof(HANDLE));
    if (!j.count || !j.first || !th) {
        fprintf(stderr, "error: not enough memory\n");
        if (j.count) HeapFree(heap, 0, j.count);
        if (j.first) HeapFree(heap, 0, j.first);
        if (th)      HeapFree(heap, 0, th);
        return -1;
    }

    int created = 0;
    for (int i = 0; i < max_threads; i++) {
        th[created] = CreateThread(NULL, 0, worker, &j, 0, NULL);
        if (th[created] == NULL) {
            fprintf(stderr, "error: CreateThread failed (code %lu), continuing with %d threads\n",
                    GetLastError(), created);
            break;
        }
        created++;
    }
    if (created == 0)
        worker(&j);           /* ни одного потока - ищем в главном */

    int ret = 0;
    for (int i = 0; i < created; i++) {
        if (WaitForSingleObject(th[i], INFINITE) != WAIT_OBJECT_0) {
            fprintf(stderr, "error: WaitForSingleObject failed (code %lu)\n", GetLastError());
            ret = -1;
        }
        CloseHandle(th[i]);
    }

    res->count = 0;
    res->first = -1;
    res->threads = created;
    for (LONG b = 0; b < j.blocks; b++) {
        res->count += j.count[b];
        if (res->first < 0)
            res->first = j.first[b];   /* блоки по порядку, первый найденный и есть первый */
    }
    HeapFree(heap, 0, j.count);
    HeapFree(heap, 0, j.first);
    HeapFree(heap, 0, th);
    return ret;
}
