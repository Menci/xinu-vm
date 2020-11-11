/*  main.c  - main */

#include <xinu.h>

uint32 tickOfProcess[3], sleepMsOfProcess[3];

#define EMPTY_LOOP_COUNT 10000000
#define SLEEP_MS 1

void ticker(uint32 id) {
	while (1) {
		for (register uint32 i = 0; i < EMPTY_LOOP_COUNT; i++)
			;
		tickOfProcess[id]++;

		// 以下为睡眠
		sleepMsOfProcess[id] += SLEEP_MS;
		sleepms(SLEEP_MS);
	}
}

#undef kprintf

process	main(void)
{
	pid32 pids[3];
	pids[0] = create(ticker, 8192, 1, 10, "1", 1, 0);
	pids[1] = create(ticker, 8192, 2, 20, "2", 1, 1);
	pids[2] = create(ticker, 8192, 3, 30, "3", 1, 2);

	for (uint32 i = 0; i < 3; i++)
		resume(pids[i]);

	uint32 count = 0;
	while (1) {
		sleepms(2000);

		intmask mask = disable();

		// Print statistics
		kprintf("------------------\n");
		for (uint32 i = 0; i < 3; i++) {
			kprintf(
				"Process %u: %u\t(time slice reassigned %d times (%dms), total CPU time %dms, slept %dms)\n",
				i,
				tickOfProcess[i],
				processTable[pids[i]].timeSliceReassignCount,
				processTable[pids[i]].timeSliceReassignCount * processTable[pids[i]].timeSlice,
				processTable[pids[i]].totalCpuTime,
				sleepMsOfProcess[i]
			);
		}
		kprintf("------------------\n");

		restore(mask);

		yield();
	}

	return OK;
}
