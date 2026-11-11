/* ЛР3, вариант 18: поиск образца в строке наивным алгоритмом в несколько потоков.
   Запуск: lab3.exe -t <макс. потоков> [-n <длина текста>] [-p <образец>] [-a <размер алфавита>]
                    [-s <seed>] [-r <повторов>] [-v] */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "search.h"
#include "util.h"

static void usage(void)
{
    fprintf(stderr,
            "usage: lab3.exe -t <max threads> [-n <text length>] [-p <pattern>] [-a <alphabet>]\n"
            "                [-s <seed>] [-r <repeats>] [-v]\n"
            "  -t  maximum number of threads working at the same time (required, 1..256)\n"
            "  -n  text length (default 100000000)\n"
            "  -p  pattern to search (default abcabd)\n"
            "  -a  text consists of the first <alphabet> letters a, b, c... (default 4)\n"
            "  -s  seed for random text (default 1)\n"
            "  -r  search several times, print the best time (default 1)\n"
            "  -v  print TID of every thread\n");
}

static int parse_long(const char *s, long long min, long long max, long long *out)
{
    char *end;
    long long v = strtoll(s, &end, 10);
    if (*s == '\0' || *end != '\0' || v < min || v > max)
        return -1;
    *out = v;
    return 0;
}

int main(int argc, char **argv)
{
    long long threads = 0, n = 100000000, alphabet = 4, seed = 1, repeats = 1;
    const char *pat = "abcabd";
    int verbose = 0;

    for (int i = 1; i < argc; i++) {
        long long *dst = NULL, min = 1, max = 0;
        if (strcmp(argv[i], "-t") == 0)      { dst = &threads;  max = 256; }
        else if (strcmp(argv[i], "-n") == 0) { dst = &n;        max = 1000000000LL; }
        else if (strcmp(argv[i], "-a") == 0) { dst = &alphabet; max = 26; }
        else if (strcmp(argv[i], "-s") == 0) { dst = &seed;     min = 0; max = 0xFFFFFFFFLL; }
        else if (strcmp(argv[i], "-r") == 0) { dst = &repeats;  max = 1000; }
        else if (strcmp(argv[i], "-v") == 0) { verbose = 1; continue; }
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc && argv[i + 1][0] != '\0') {
            pat = argv[++i];
            continue;
        } else { usage(); return 1; }

        const char *opt = argv[i];
        if (i + 1 >= argc || parse_long(argv[++i], min, max, dst) != 0) {
            fprintf(stderr, "error: bad value for %s\n", opt);
            usage();
            return 1;
        }
    }
    if (threads == 0) {
        usage();
        return 1;
    }
    size_t m = strlen(pat);
    if ((long long)m > n) {
        fprintf(stderr, "error: pattern is longer than the text\n");
        return 1;
    }

    printf("PID=%lu main TID=%lu, n=%lld, pattern=\"%s\" (m=%zu), max threads=%lld\n",
           GetCurrentProcessId(), GetCurrentThreadId(), n, pat, m, threads);

    HANDLE heap = GetProcessHeap();
    char *text = HeapAlloc(heap, 0, (size_t)n);
    if (text == NULL) {
        fprintf(stderr, "error: not enough memory for a text of %lld bytes\n", n);
        return 1;
    }
    fill_text(text, (size_t)n, (int)alphabet, (unsigned)seed);

    SearchResult res;
    double best = 0;
    int ret = 0;
    for (long long r = 0; r < repeats; r++) {
        double t0 = now_ms();
        if (parallel_search(text, (size_t)n, pat, m, (int)threads, verbose && r == 0, &res) != 0) {
            ret = 1;
            break;
        }
        double t = now_ms() - t0;
        if (r == 0 || t < best)
            best = t;
    }
    if (ret == 0)
        printf("found %lld, first at %lld, threads=%d, time=%.2f ms\n",
               res.count, res.first, res.threads, best);
    HeapFree(heap, 0, text);
    return ret;
}
