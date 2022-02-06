/* xsh_sort.c - xsh_sort */

#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>

void sort(int32 *array, uint32 n) {
	for (uint32 i = 0; i < n; i++)
		for (uint32 j = i + 1; j < n; j++)
			if (array[i] > array[j]) {
				int32 tmp = array[i];
				array[i] = array[j];
				array[j] = tmp;
			}
}

/*------------------------------------------------------------------------
 * xsh_sort - sort user's input
 *------------------------------------------------------------------------
 */
shellcmd xsh_sort(int nargs, char *args[]) {
	int32 *array = malloc(0);
	uint32 n = 0;
	
	for (int32 i = 1; i < nargs; i++) {
		int32 x = atoi(args[i]);
		kprintf("xsh_sort: x = %d\n", x);

		array = realloc(array, sizeof(int32) * (n + 1));
		array[n++] = x;

		sort(array, n);
		for (uint32 j = 0; j < n; j++)
			printf("%d%c", array[j], " \n"[j == n - 1]);
	}

	return 0;
}
