#include "cachelab.h"
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

const int NUM = 16;

typedef struct {
	char identifier;
	unsigned address;
	int size;
}Instruct;

typedef struct {
	Instruct* instruct_p;
	int length;
}InstructPoint;

typedef struct {
	short hit;
	short miss;
	short eviction;
}State;

typedef struct {
	State* state_p;
	int length;
}StatePoint;

typedef struct {
	int valid;
	unsigned tag;
}Cache;

InstructPoint malloc_instruct(InstructPoint instruct_point, int length);
InstructPoint readInstruct(const char* filename);
StatePoint initState(int size);
Cache** initCache(int S, int E);
int** initLRU(int S, int E);
unsigned getBlock(unsigned address, int b);
unsigned getSet(unsigned address, int b, int s);
unsigned getTag(unsigned address, int b, int s);
void reorderLRU(int** LRU, unsigned set, int id);
//void printSummary(int hit_count, int miss_count, int eviction_count);
void evalState(InstructPoint instruct_point, Cache** cache_p, int** LRU, int s, int E, int b, StatePoint state_point);

int main(int argc, char** argv)
{
    int opt, s, E, b;
    char* t = NULL;
    while(-1 != (opt = getopt(argc, argv, "hvs:E:b:t:"))){
	switch (opt){
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
		t = optarg;
		break;
	    default:
		printf("wrong argument\n");
		break;
	}
    }
	int S = 1 << s;
	//int B = 1 << b;
	
	// instruct initiation 
	const char* filename = t;
	InstructPoint instruct_point;
	instruct_point = readInstruct(filename);

	// state initiation 
	StatePoint state_point;
	state_point = initState(instruct_point.length);

	// cache initiation 
	Cache** cache_p = NULL;
	cache_p = initCache(S, E);

	// LRU initiation
	int** LRU = NULL;
	LRU = initLRU(S, E);

	// process
	evalState(instruct_point, cache_p, LRU, s, E, b, state_point);

	// print
	int hit_count = 0;
	int miss_count = 0;
	int eviction_count = 0;
	for (int i = 0; i < state_point.length; i++) {
		hit_count += state_point.state_p[i].hit;
		miss_count += state_point.state_p[i].miss;
		eviction_count += state_point.state_p[i].eviction;
		//printf("%d: hit = %d, miss = %d, eviction = %d\n", i, state_point.state_p[i].hit, state_point.state_p[i].miss, state_point.state_p[i].eviction);
	}
	
	// free 
	free(instruct_point.instruct_p);
	free(state_point.state_p);
	for (int i = 0; i < S; i++) {
		free(cache_p[i]);
		free(LRU[i]);
	}
	free(cache_p);
	free(LRU);
	
	printSummary(hit_count, miss_count, eviction_count);
	return 0;
}

InstructPoint malloc_instruct(InstructPoint instruct_point, int length) {
	InstructPoint p;
	p.length = length << 2;
	p.instruct_p = (Instruct *)malloc(p.length * sizeof(Instruct));
	memcpy(p.instruct_p, instruct_point.instruct_p, length * sizeof(Instruct));
	free(instruct_point.instruct_p);
	instruct_point.instruct_p = NULL;
	return p;
}

InstructPoint readInstruct(const char* filename) {
	int count = 0;

	InstructPoint instruct_point;
	instruct_point.length = NUM;
	instruct_point.instruct_p = (Instruct *)malloc(instruct_point.length * sizeof(Instruct));

	FILE *pFile;	// pointer to FILE object

	pFile = fopen(filename, "r");	// open file for reading

	char identifier;
	unsigned address;
	int size;

	// Reading lines like " M 20£¬1" or "L 19,3"
	while (fscanf(pFile, " %c %x, %d", &identifier, &address, &size) > 0) {
		instruct_point.instruct_p[count].identifier = identifier;
		instruct_point.instruct_p[count].address = address;
		instruct_point.instruct_p[count].size = size;
		count++;
		if (count >= instruct_point.length) {
			instruct_point = malloc_instruct(instruct_point, instruct_point.length);
		}
		//printf("%c %x, %d\n", identifier, address, size);
	}
	instruct_point.length = count;

	fclose(pFile);

	return instruct_point;
}

StatePoint initState(int size) {
	StatePoint state_point;
	state_point.length = size;
	state_point.state_p = (State *)malloc(state_point.length * sizeof(State));
	for (int i = 0; i < state_point.length; i++) {
		state_point.state_p[i].hit = 0;
		state_point.state_p[i].miss = 0;
		state_point.state_p[i].eviction = 0;
	}
	return state_point;
}

Cache** initCache(int S, int E) {
	Cache** cache = (Cache **)malloc(S * sizeof(Cache*));
	for (int i = 0; i < S; i++) {
		cache[i] = (Cache *)malloc(E * sizeof(Cache));
	}
	for (int i = 0; i < S; i++) {
		for (int j = 0; j < E; j++) {
			cache[i][j].valid = 0;
			cache[i][j].tag = 0;
		}
	}
	return cache;
}

int** initLRU(int S, int E) {
	int** LRU = (int **)malloc(S * sizeof(int*));
	for (int i = 0; i < S; i++) {
		LRU[i] = (int *)malloc(E * sizeof(int));
	}
	for (int i = 0; i < S; i++) {
		for (int j = 0; j < E; j++) {
			LRU[i][j] = j;
		}
	}
	return LRU;
}

unsigned getBlock(unsigned address, int b) {
	unsigned mask_block = (1 << b) - 1;
	return (address & mask_block);
}
unsigned getSet(unsigned address, int b, int s) {
	unsigned mask_set = (1 << s) - 1;
	return ((address >> b) & mask_set);
}
unsigned getTag(unsigned address, int b, int s) {
	return (address >> (b + s));
}

void reorderLRU(int** LRU, unsigned set, int val) {
    int id;
    for (id = 0; ;id++){
	if (LRU[set][id] == val){
	    break;
	}
    }
    int tmp = LRU[set][id];
	for (int i = id; i > 0; i--) {
		LRU[set][i] = LRU[set][i - 1];
	}
	LRU[set][0] = tmp;
}

//void printSummary(int hit_count, int miss_count, int eviction_count) {
//	printf("hits:%d misses:%d evictions:%d\n", hit_count, miss_count, eviction_count);
//}

void evalState(InstructPoint instruct_point, Cache** cache_p, int** LRU, int s, int E, int b, StatePoint state_point) {
	char identifier;
	unsigned address;
	//int size;

	unsigned set;
	unsigned tag;

	int k;
	for (int i = 0; i < instruct_point.length; i++) {
		identifier = instruct_point.instruct_p[i].identifier;
		address = instruct_point.instruct_p[i].address;
		//size = instruct_point.instruct_p[i].size;
		switch (identifier) {
		case 'I':
			break;
		case 'L':
		case 'S':
		case 'M':
			set = getSet(address, b, s);
			tag = getTag(address, b, s);
			for (k = 0; k < E; k++) {
				if (cache_p[set][k].valid && (cache_p[set][k].tag == tag)) {
					state_point.state_p[i].hit = 1;
					if (identifier == 'M') {
						state_point.state_p[i].hit = 2;
					}
					reorderLRU(LRU, set, k);
					break;
				}
			}
			if (k == E) {
				state_point.state_p[i].miss = 1;
				if (identifier == 'M') {
					state_point.state_p[i].hit = 1;
				}
				cache_p[set][LRU[set][E - 1]].tag = tag;
				if (cache_p[set][LRU[set][E - 1]].valid) {
					state_point.state_p[i].eviction = 1;
				}
				else {
					cache_p[set][LRU[set][E - 1]].valid = 1;
				}
				reorderLRU(LRU, set,LRU[set][E - 1]);
			}
			break;
		default:
			printf("Illegal Instruction!!");
			break;
		}
	}
}
