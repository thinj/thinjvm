/*
 * main.c
 *
 *  Created on: Aug 17, 2012
 *      Author: hammer
 */

#include <stdio.h>
#include "heaplist.h"

#define MEMSIZE 200
static align_t heapmem[MEMSIZE];

static void verify(const char* file, const int line, int success) {
	if (!success) {
		printf("%s:%d Verification failed\n", file, line);
	}
}

#define VERIFY(X) verify(__FILE__, __LINE__, X)

int ia0[] = { 2, 8, 6, 7, 4, 0, 5, 9, 3, 1, };
int ia1[] = { 8, 3, 0, 4, 2, 6, 1, 7, 5, 9, };
int ia2[] = { 7, 8, 1, 3, 4, 5, 2, 6, 0, 9, };
int ia3[] = { 1, 3, 2, 4, 6, 7, 9, 0, 5, 8, };
int ia4[] = { 0, 9, 6, 8, 1, 2, 3, 7, 5, 4, };
int ia5[] = { 1, 7, 5, 2, 6, 4, 3, 8, 0, 9, };
int ia6[] = { 3, 5, 8, 1, 9, 7, 0, 6, 2, 4, };
int ia7[] = { 4, 2, 7, 5, 8, 3, 6, 0, 9, 1, };
int ia8[] = { 5, 0, 8, 7, 3, 9, 6, 4, 2, 1, };
int ia9[] = { 0, 8, 9, 1, 7, 2, 6, 3, 4, 5, };

typedef void* voidp;

void testRandom(int* ia) {
	voidp p[10];
	int size = sizeof(ia0) / sizeof(ia0[0]);

	int i;
	for (i = 0; i < size; i++) {
		p[i] = heap_alloc(ia[i] + 5);
		if (p[i] == NULL) {
			printf("out of mem\n");
			exit(1);
		}
	}

	for (i = 0; i < size; i++) {
		heap_free(p[ia[i]]);
	}
}

int heap_test() {
	heap_init(&heapmem[0], MEMSIZE);

	heapstat_t hused, hfree;

	void* p1 = heap_alloc(1);

	heap_stat(&hused, &hfree);
	VERIFY(hused.count == 1 && hused.size == 1 + 1 * HEAP_HEADER_SIZE);
	VERIFY(hfree.count == 1 && hfree.size == MEMSIZE - 1 - 1 * HEAP_HEADER_SIZE);

	void* p2 = heap_alloc(1);
	heap_stat(&hused, &hfree);
	VERIFY(hused.count == 2 && hused.size == 2 + 2 * HEAP_HEADER_SIZE);
	VERIFY(hfree.count == 1 && hfree.size == MEMSIZE - 2 - 2 * HEAP_HEADER_SIZE);

	void* p3 = heap_alloc(20);
	heap_stat(&hused, &hfree);
	VERIFY(hused.count == 3 && hused.size == 22 + 3 * HEAP_HEADER_SIZE);
	VERIFY(hfree.count == 1 && hfree.size == MEMSIZE - 22 - 3 * HEAP_HEADER_SIZE);

	// Test freeing heap:
	heap_free(p1);
	heap_stat(&hused, &hfree);
	VERIFY(hused.count == 2 && hused.size == 21 + 2 * HEAP_HEADER_SIZE);
	VERIFY(hfree.count == 2 && hfree.size == MEMSIZE - 21 - 2 * HEAP_HEADER_SIZE);

	heap_free(p2);
	heap_stat(&hused, &hfree);
	VERIFY(hused.count == 1 && hused.size == 20 + 1 * HEAP_HEADER_SIZE);
	VERIFY(hfree.count == 2 && hfree.size == MEMSIZE - 20 - 1 * HEAP_HEADER_SIZE);

	heap_free(p3);
	heap_stat(&hused, &hfree);
	VERIFY(hused.count == 0 && hused.size == 0);
	VERIFY(hfree.count == 1 && hfree.size == 200);

	p1 = heap_alloc(1);
	p2 = heap_alloc(20);
	p3 = heap_alloc(1);
	heap_stat(&hused, &hfree);
	VERIFY(hused.count == 3 && hused.size == 22 + 3 * HEAP_HEADER_SIZE);
	VERIFY(hfree.count == 1 && hfree.size == MEMSIZE - 22 - 3 * HEAP_HEADER_SIZE);

	heap_free(p1);
	heap_free(p2);
	heap_free(p3);
	heap_stat(&hused, &hfree);
	VERIFY(hused.count == 0 && hused.size == 0);
	VERIFY(hfree.count == 1 && hfree.size == 200);

	p1 = heap_alloc(95);
	p2 = heap_alloc(45);
	p3 = heap_alloc(MEMSIZE - 95 - 45 - 3 * HEAP_HEADER_SIZE);
	heap_stat(&hused, &hfree);
	VERIFY(hused.count == 3 && hused.size == 200);
	VERIFY(hfree.count == 0 && hfree.size == 0);

	// Out of mem test:
	void* p4 = heap_alloc(1);
	VERIFY(p4 == NULL);

	// Test reversed freeing order:
	heap_free(p1);
	heap_free(p3);
	heap_free(p2);
	heap_stat(&hused, &hfree);
	VERIFY(hused.count == 0 && hused.size == 0);
	VERIFY(hfree.count == 1 && hfree.size == 200);

	testRandom(ia0);
	testRandom(ia1);
	testRandom(ia2);
	testRandom(ia3);
	testRandom(ia4);
	testRandom(ia5);
	testRandom(ia6);
	testRandom(ia7);
	testRandom(ia8);
	testRandom(ia9);

	printf("End of Test\n");

	return 0;
}
