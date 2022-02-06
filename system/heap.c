#include <heap.h>

void sbrk(int32 delta) {
    struct procent *currentProcess = &proctab[currpid];
    if (delta < 0 && (uint32)-delta > currentProcess->heapSize)
        delta = -(int32)currentProcess->heapSize;

    uint32 newHeapSize = currentProcess->heapSize + delta;
    uint32 newEndPageId = VM_HEAP_START_PAGE_ID + getRoundedUpPageCountOfMemorySize(newHeapSize);
    uint32 oldEndPageId = VM_HEAP_START_PAGE_ID + getRoundedUpPageCountOfMemorySize(currentProcess->heapSize);

    if (newEndPageId > oldEndPageId)
        allocateVirtualMemoryPages(currentProcess->pageDirectoryPhysicalAddress, oldEndPageId, newEndPageId - oldEndPageId);
    else if (newEndPageId < oldEndPageId)
        deallocateVirtualMemoryPages(currentProcess->pageDirectoryPhysicalAddress, newEndPageId, oldEndPageId - newEndPageId);

    currentProcess->heapSize = newHeapSize;
}
