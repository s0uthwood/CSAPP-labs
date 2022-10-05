#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "cachelab.h"

int s = 0, E = 0, b = 0, v = 0;
char *tracefile = NULL;
int S = 0, B = 0;
int hit_count, miss_count, eviction_count;
unsigned long long lru_counter = 1, set_index_mask;


FILE *fp = NULL;

typedef struct {
    char valid_bit;
    unsigned long long tag;
    unsigned long long lru;
} cache_line_t, *cache_set_t, **cache_t;

cache_t cache;

typedef struct {

} Node;

void printUsage(char *name){
    printf("./%s [-hv] -s <s> -E <E> -b <b> -t <tracefile>", name);
    puts("Options:");
    puts("  -h         Print this help message.");
    puts("  -v         Optional verbose flag.");
    puts("  -s <num>   Number of set index bits.");
    puts("  -E <num>   Number of lines per set.");
    puts("  -b <num>   Number of block offset bits.");
    puts("  -t <file>  Trace file.");
    puts("");
    puts("Examples:");
    puts("  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace");
    puts("  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace");
}

void initCache() {
    cache = (cache_t)malloc(sizeof(cache_set_t) * S);
    for (int i = 0; i < S; i++) {
        cache[i] = (cache_set_t)malloc(sizeof(cache_line_t) * E);
        for (int j = 0; j < E; j++) {
            cache[i][j].valid_bit = 0;
            cache[i][j].tag = 0;
            cache[i][j].lru = 0;
        }
    }
    set_index_mask = (1 << s) - 1;
    return;
}

void releaseCache() {
    for (int i = 0; i < S; i++) {
        free(cache[i]);
        cache[i] = NULL;
    }
    free(cache);
    cache = NULL;
    return;
}

void accessData(unsigned long long address){
    unsigned int eviction_lru = -1LL;
    unsigned long long eviction_line = 0;
    unsigned long long tag = address >> (s + b);
    cache_set_t cache_set = cache[(address >> b) & set_index_mask];
    int i;
    for (i = 0; ; ++i) {
        if (i >= E) {
            ++miss_count;
            if (v)
                printf("miss ");
            for (int j = 0; j < E; ++j) {
                if (cache_set[j].lru < eviction_lru) {
                    eviction_line = j;
                    eviction_lru = cache_set[j].lru;
                }
            }
            if (cache_set[eviction_line].valid_bit) {
                ++eviction_count;
                if (v) 
                    printf("eviction ");
            }
            cache_set[eviction_line].valid_bit = 1;
            cache_set[eviction_line].tag = tag;
            cache_set[eviction_line].lru = lru_counter++;
            return;
        }
        if (cache_set[i].valid_bit && cache_set[i].tag == tag) {
            break;
        }
    }
    ++hit_count;
    if (v) 
        printf("hit ");
    cache_set[i].lru = lru_counter++;
    return;
}

void replayTrace(FILE* f) {
    unsigned long long address;
    int size;
    char buf[1024];
    while (fgets(buf, 1023, f)) {
        if (buf[1] != 'M' && buf[1] != 'L' && buf[1] != 'S') {
            continue;
        }
        sscanf(buf + 3, "%llx,%u ", &address, &size);
        if (v)
            printf("%c %llx,%u ", buf[1], address, size);
        accessData(address);
        if (buf[1] == 'M') {
            accessData(address);
        }
        if (v) putchar('\n');
    }
    return;
}

int main(int argc, char* argv[]) {
    char o;
    while ((o = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (o) {
            case 'h':
                printUsage(argv[0]);
                return 0;
            case 'v':
                v = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                tracefile = optarg;
                fp = fopen(tracefile, "r");
                if (fp == NULL) {
                    printf("File %s open failed.", tracefile);
                    return -1;
                }
                break;
            default:
                printUsage(argv[0]);
                return 0;
        }
    }
    if (s <= 0 || E <= 0 || !tracefile || b <= 0) {
        printUsage(argv[0]);
        return -1;
    }
    S = 1 << s;
    B = 1 << b;

    initCache();
    replayTrace(fp);

    printSummary(hit_count, miss_count, eviction_count);

    releaseCache();
    fclose(fp);
    
    return 0;
}
