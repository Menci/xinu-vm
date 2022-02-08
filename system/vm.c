#include <xinu.h>

void *freePages[VM_MAX_PAGE_COUNT];
uint32 freePageCount;

void appendNewAvailableMemoryToFreePages(void *start, uint32 size) {
    byte *p = alignAddressUpToPage(start);
    uint32 newPageCount = getRoundedDownPageCountOfMemorySize(size - (p - (byte *)start));
    for (uint32 i = 0; i < newPageCount; i++) {
        if (freePageCount == VM_MAX_PAGE_COUNT) break;
        freePages[freePageCount++] = p + (i * VM_PAGE_SIZE);
    }
}

void *allocatePhysicalPage() {
    if (freePageCount == 0) return NULL;
    return freePages[--freePageCount];
}

void deallocatePhysicalPage(void *page) {
    for (uint32 i = 0; i < freePageCount; i++)
        if (freePages[i] == page) {
            kprintf("deallocatePhysicalPage: double free or corruption (%x)\n", page);
            return;
        }
    freePages[freePageCount++] = page;
}

PageDirectoryEntry *initializePageDirectoryEntry(PageDirectoryEntry *entry, bool8 present, uint32 physicalPageIdForPageTable) {
    entry->present                    = present;
    entry->readWrite                  = 1;
    entry->userSupervisor             = 1;
    entry->writeThrough               = 1;
    entry->disableCache               = 0;
    entry->accessed                   = 0;
    entry->dirty                      = 0;
    entry->global                     = 0;
    entry->zero1                      = 0;
    entry->zero2                      = 0;
    entry->physicalPageIdForPageTable = physicalPageIdForPageTable;
    return entry;
}

PageTableEntry *initializePageTableEntry(PageTableEntry *entry, bool8 present, uint32 physicalPageIdForPage) {
    entry->present               = present;
    entry->readWrite             = 1;
    entry->userSupervisor        = 1;
    entry->writeThrough          = 1;
    entry->disableCache          = 0;
    entry->accessed              = 0;
    entry->zero1                 = 0;
    entry->isLargePage           = 0;
    entry->zero2                 = 0;
    entry->zero3                 = 0;
    entry->physicalPageIdForPage = physicalPageIdForPage;
    return entry;
}

void switchPageDirectory(PageDirectory pageDirectoryPhysicalAddress) {
    asm volatile (
        "mov %0, %%cr3\n\t"
        :
        :"r"(pageDirectoryPhysicalAddress)
        :
    );
}

PageDirectory getCurrentPageDirectory() {
    PageDirectory result;
    asm volatile (
        "mov %%cr3, %0\n\t"
        :"=r"(result)
        :
        :
    );
    return result;
}

bool8 getPagingEnabled() {
    uint32 cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        :"=r"(cr0)
        :
        :
    );
    return (cr0 & 0x80000000) != 0;
}

void flushTlbForSinglePage(void *page) {
    asm volatile(
        "invlpg (%0)\n\t"
        :
        :"r"(page)
        :"memory"
    );
}

uint32 saveCurrentVirtualMemorySpaceTemporaryPageMappedPhysicalPageId() {
    PageTable pageTable = VM_PAGE_TABLE_FOR_TEMPORARY_PAGE_PAGE_ADDRESS;
    PageTableEntry *pageTableEntry = &pageTable[pageIdToPageTableEntryIndex(VM_TEMPORARY_PAGE_ID)];
    return pageTableEntry->physicalPageIdForPage;
}

void mapToCurrentVirtualMemorySpaceTemporaryPage(uint32 physicalPageId) {
    PageTable pageTable = VM_PAGE_TABLE_FOR_TEMPORARY_PAGE_PAGE_ADDRESS;
    PageTableEntry *pageTableEntry = &pageTable[pageIdToPageTableEntryIndex(VM_TEMPORARY_PAGE_ID)];
    initializePageTableEntry(pageTableEntry, physicalPageId != 0, physicalPageId);
    flushTlbForSinglePage(VM_TEMPORARY_PAGE_ADDRESS);
}

#define SAVE_TEMPORARY_PAGE \
    uint32 temporaryPageBackup = !getPagingEnabled() ? 0 : saveCurrentVirtualMemorySpaceTemporaryPageMappedPhysicalPageId();
#define ACCESS_PHYSICAL_PAGE_WITH_TEMPORARY_PAGE(Type, accessAddress, physicalAddress) \
    Type accessAddress; \
    if (getPagingEnabled()) { \
        mapToCurrentVirtualMemorySpaceTemporaryPage(pageAddressToPageId(physicalAddress)); \
        accessAddress = VM_TEMPORARY_PAGE_ADDRESS; \
    } else \
        accessAddress = physicalAddress;
#define RESTORE_TEMPORARY_PAGE \
    if (temporaryPageBackup) \
        mapToCurrentVirtualMemorySpaceTemporaryPage(temporaryPageBackup); \

PageDirectoryEntry pageDirectoryEntriesForKernelPageTables[VM_KERNEL_PAGE_TABLE_COUNT];

void initializeKernelPageTables() {
    uint32 currentPhysicalPageId = 0;
    for (uint32 i = 0; i < VM_KERNEL_PAGE_TABLE_COUNT; i++) {
        PageTable pageTable = allocatePhysicalPage();
        initializePageDirectoryEntry(&pageDirectoryEntriesForKernelPageTables[i], TRUE, pageAddressToPageId(pageTable));
        for (uint32 j = 0; j < VM_PAGES_PER_PAGE_TABLE; j++) {
            initializePageTableEntry(&pageTable[j], TRUE, currentPhysicalPageId++);
        }
    }
}

PageTable allocateEmptyPageTable(PageDirectoryEntry *directoryEntry) {
    PageTable pageTablePhysicalAddress = allocatePhysicalPage();
    initializePageDirectoryEntry(directoryEntry, TRUE, pageAddressToPageId(pageTablePhysicalAddress));

    SAVE_TEMPORARY_PAGE
    ACCESS_PHYSICAL_PAGE_WITH_TEMPORARY_PAGE(PageTable, pageTable, pageTablePhysicalAddress)

    memset(pageTable, 0, VM_PAGE_SIZE);

    for (uint32 i = 0; i < VM_PAGES_PER_PAGE_TABLE; i++)
        initializePageTableEntry(&pageTable[i], FALSE, 0);

    RESTORE_TEMPORARY_PAGE
    return pageTablePhysicalAddress;
}

PageDirectory allocateVirtualMemorySpace() {
    PageDirectory pageDirectoryPhysicalAddress = allocatePhysicalPage();

    SAVE_TEMPORARY_PAGE
    ACCESS_PHYSICAL_PAGE_WITH_TEMPORARY_PAGE(PageDirectory, pageDirectory, pageDirectoryPhysicalAddress)

    memset(pageDirectory, 0, VM_PAGE_SIZE);

    uint32 i = 0;
    for (; i < VM_KERNEL_PAGE_TABLE_COUNT; i++)
        pageDirectory[i] = pageDirectoryEntriesForKernelPageTables[i];

    for (; i < VM_PAGE_TABLES_PER_DIRECTORY; i++)
        initializePageDirectoryEntry(&pageDirectory[i], FALSE, 0);

    // Initialize the page table for the temporary page
    PageTable newPageTableForTemporaryPagePhysicalAddress = allocateEmptyPageTable(&pageDirectory[pageIdToPageDirectoryEntryIndex(VM_TEMPORARY_PAGE_ID)]);
    // Map the page table for temporary page to the virtual memory space
    ACCESS_PHYSICAL_PAGE_WITH_TEMPORARY_PAGE(PageTable, newPageTableForTemporaryPage, newPageTableForTemporaryPagePhysicalAddress)
    initializePageTableEntry(&newPageTableForTemporaryPage[pageIdToPageTableEntryIndex(VM_PAGE_TABLE_FOR_TEMPORARY_PAGE_PAGE_ID)], TRUE, pageAddressToPageId(newPageTableForTemporaryPagePhysicalAddress));

    RESTORE_TEMPORARY_PAGE
    return pageDirectoryPhysicalAddress;
}

void deallocateVirtualMemorySpace(PageDirectory pageDirectoryPhysicalAddress) {
    kprintf("deallocateVirtualMemorySpace: deallocating vm directory %x from process [%s]\n", pageDirectoryPhysicalAddress, proctab[currpid].prname);

    SAVE_TEMPORARY_PAGE

    for (uint32 i = VM_KERNEL_PAGE_TABLE_COUNT; i < VM_PAGE_TABLES_PER_DIRECTORY; i++) {
        ACCESS_PHYSICAL_PAGE_WITH_TEMPORARY_PAGE(PageDirectory, pageDirectory, pageDirectoryPhysicalAddress)
        PageDirectoryEntry *pageDirectoryEntry = &pageDirectory[i];
        if (!pageDirectoryEntry->present) continue;

        PageTable pageTablePhysicalAddress = pageIdToPageAddress(pageDirectoryEntry->physicalPageIdForPageTable);
        ACCESS_PHYSICAL_PAGE_WITH_TEMPORARY_PAGE(PageTable, pageTable, pageTablePhysicalAddress);
        for (uint32 j = 0; j < VM_PAGES_PER_PAGE_TABLE; j++) {
            PageTableEntry *pageTableEntry = &pageTable[j];
            if (!pageTableEntry->present) continue;

            // Don't touch the temporary page since it's owned by another virtual memory space
            if (
                i == pageIdToPageDirectoryEntryIndex(VM_TEMPORARY_PAGE_ID) &&
                j == pageIdToPageTableEntryIndex(VM_TEMPORARY_PAGE_ID)
            )
                continue;
            // Don't touch the page table for temporary page since it will be deallocated when deallocating page tables
            if (
                i == pageIdToPageDirectoryEntryIndex(VM_PAGE_TABLE_FOR_TEMPORARY_PAGE_PAGE_ID) &&
                j == pageIdToPageTableEntryIndex(VM_PAGE_TABLE_FOR_TEMPORARY_PAGE_PAGE_ID)
            )
                continue;

            deallocatePhysicalPage(pageIdToPageAddress(pageTableEntry->physicalPageIdForPage));
        }

        deallocatePhysicalPage(pageTablePhysicalAddress);
    }

    deallocatePhysicalPage(pageDirectoryPhysicalAddress);

    RESTORE_TEMPORARY_PAGE
}

PageDirectory initializeVirtualMemory() {
    initializeKernelPageTables();
    PageDirectory initializationPageDirectoryPhysicalAddress = allocateVirtualMemorySpace();
    switchPageDirectory(initializationPageDirectoryPhysicalAddress);

    // Enable paging
    asm volatile (
        "mov %%cr0, %%eax\n\t"
        "or $0x80000000, %%eax\n\t"
        "mov %%eax, %%cr0\n\t"
        :
        :
        :"eax"
    );

    return initializationPageDirectoryPhysicalAddress;
}

void writeToPhysicalMemoryPage(uint32 physicalPageId, void *src) {
    SAVE_TEMPORARY_PAGE
    ACCESS_PHYSICAL_PAGE_WITH_TEMPORARY_PAGE(void *, dst, pageIdToPageAddress(physicalPageId))
    if (src)
        moveMemory(dst, src, VM_PAGE_SIZE);
    else
        memset(dst, 0, VM_PAGE_SIZE);
    RESTORE_TEMPORARY_PAGE
}

void writeToAnotherVirtualMemorySpacePage(PageDirectory pageDirectoryPhysicalAddress, uint32 virtualPageId, void *src) {
    SAVE_TEMPORARY_PAGE

    ACCESS_PHYSICAL_PAGE_WITH_TEMPORARY_PAGE(PageDirectory, pageDirectory, pageDirectoryPhysicalAddress)

    PageDirectoryEntry *pageDirectoryEntry = &pageDirectory[pageIdToPageDirectoryEntryIndex(virtualPageId)];
    if (!pageDirectoryEntry->present) {
        kprintf("writeToAnotherVirtualMemorySpacePage: page directory entry not present\n");
        return;
    }
    PageTable pageTablePhysicalAddress = pageIdToPageAddress(pageDirectoryEntry->physicalPageIdForPageTable);

    ACCESS_PHYSICAL_PAGE_WITH_TEMPORARY_PAGE(PageTable, pageTable, pageTablePhysicalAddress)

    PageTableEntry *pageTableEntry = &pageTable[pageIdToPageTableEntryIndex(virtualPageId)];
    if (!pageTableEntry->present) {
        kprintf("writeToAnotherVirtualMemorySpacePage: page table entry not present\n");
        return;
    }

    writeToPhysicalMemoryPage(pageTableEntry->physicalPageIdForPage, src);

    RESTORE_TEMPORARY_PAGE
}

void allocateVirtualMemoryPage(PageDirectory pageDirectoryPhysicalAddress, uint32 virtualPageId, void *initialPageContent) {
    SAVE_TEMPORARY_PAGE

    ACCESS_PHYSICAL_PAGE_WITH_TEMPORARY_PAGE(PageDirectory, pageDirectory, pageDirectoryPhysicalAddress)

    PageDirectoryEntry *pageDirectoryEntry = &pageDirectory[pageIdToPageDirectoryEntryIndex(virtualPageId)];
    PageTable pageTablePhysicalAddress;
    if (!pageDirectoryEntry->present)
        pageTablePhysicalAddress = allocateEmptyPageTable(pageDirectoryEntry);
    else
        pageTablePhysicalAddress = pageIdToPageAddress(pageDirectoryEntry->physicalPageIdForPageTable);

    ACCESS_PHYSICAL_PAGE_WITH_TEMPORARY_PAGE(PageTable, pageTable, pageTablePhysicalAddress)

    PageTableEntry *pageTableEntry = &pageTable[pageIdToPageTableEntryIndex(virtualPageId)];
    if (pageTableEntry->present) {
        kprintf("allocateVirtualMemoryPage: page %u already mapped\n", virtualPageId);
        RESTORE_TEMPORARY_PAGE
        return;
    }

    void *page = allocatePhysicalPage();
    uint32 pageId = pageAddressToPageId(page);
    initializePageTableEntry(pageTableEntry, TRUE, pageId);
    writeToPhysicalMemoryPage(pageId, initialPageContent);

    if (pageDirectoryPhysicalAddress == getCurrentPageDirectory())
        flushTlbForSinglePage(pageIdToPageAddress(virtualPageId));

    RESTORE_TEMPORARY_PAGE
}

void deallocateVirtualMemoryPage(PageDirectory pageDirectoryPhysicalAddress, uint32 virtualPageId) {
    SAVE_TEMPORARY_PAGE

    ACCESS_PHYSICAL_PAGE_WITH_TEMPORARY_PAGE(PageDirectory, pageDirectory, pageDirectoryPhysicalAddress)
    PageDirectoryEntry *pageDirectoryEntry = &pageDirectory[pageIdToPageDirectoryEntryIndex(virtualPageId)];
    if (!pageDirectoryEntry->present) {
        kprintf("deallocateVirtualMemoryPage: page %u not mapped (page directory entry not present)\n", virtualPageId);
        RESTORE_TEMPORARY_PAGE
        return;
    }
    PageTable pageTablePhysicalAddress = pageIdToPageAddress(pageDirectoryEntry->physicalPageIdForPageTable);

    ACCESS_PHYSICAL_PAGE_WITH_TEMPORARY_PAGE(PageTable, pageTable, pageTablePhysicalAddress)
    PageTableEntry *pageTableEntry = &pageTable[pageIdToPageTableEntryIndex(virtualPageId)];
    if (!pageTableEntry->present) {
        kprintf("deallocateVirtualMemoryPage: page %u not mapped (page table entry not present)\n", virtualPageId);
        RESTORE_TEMPORARY_PAGE
        return;
    }

    uint32 pageId = pageTableEntry->physicalPageIdForPage;
    deallocatePhysicalPage(pageIdToPageAddress(pageId));
    initializePageTableEntry(pageTableEntry, FALSE, 0);

    uint32 presentPageTableEntryCount = 0;
    for (uint32 i = 0; i < VM_PAGES_PER_PAGE_TABLE; i++)
        if (pageTableEntry[i].present)
            presentPageTableEntryCount++;

    if (presentPageTableEntryCount == 0) {
        // Deallocate the page table
        ACCESS_PHYSICAL_PAGE_WITH_TEMPORARY_PAGE(PageDirectory, pageDirectory, pageDirectoryPhysicalAddress)
        PageDirectoryEntry *pageDirectoryEntry = &pageDirectory[pageIdToPageDirectoryEntryIndex(virtualPageId)];
        uint32 pageId = pageDirectoryEntry->physicalPageIdForPageTable;
        deallocatePhysicalPage(pageIdToPageAddress(pageId));
        initializePageDirectoryEntry(pageDirectoryEntry, FALSE, 0);
    }

    if (pageDirectoryPhysicalAddress == getCurrentPageDirectory())
        flushTlbForSinglePage(pageIdToPageAddress(virtualPageId));

    RESTORE_TEMPORARY_PAGE
}

void allocateVirtualMemoryPages(PageDirectory pageDirectoryPhysicalAddress, uint32 startVirtualPageId, uint32 count) {
    for (uint32 i = 0; i < count; i++)
        allocateVirtualMemoryPage(pageDirectoryPhysicalAddress, startVirtualPageId + i, NULL);
}

void deallocateVirtualMemoryPages(PageDirectory pageDirectoryPhysicalAddress, uint32 startVirtualPageId, uint32 count) {
    for (uint32 i = 0; i < count; i++)
        deallocateVirtualMemoryPage(pageDirectoryPhysicalAddress, startVirtualPageId + i);
}

// buffer size = 4096
void writeshargs(pid32 child, void *buffer) {
    if (proctab[child].prparent != currpid) return;

    allocateVirtualMemoryPages(proctab[child].pageDirectoryPhysicalAddress, VM_SHELL_ARGUMENT_PAGE_ID, 1);
    writeToAnotherVirtualMemorySpacePage(proctab[child].pageDirectoryPhysicalAddress, VM_SHELL_ARGUMENT_PAGE_ID, buffer);
}
