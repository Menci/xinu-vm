#pragma once
#include <kernel.h>

#define VM_PAGE_SIZE                                    4096
#define VM_MAX_PAGE_COUNT                               65536 // Support max 256 MiB dynamically-paged physical memory
#define VM_PAGES_PER_PAGE_TABLE                         (VM_PAGE_SIZE / sizeof(void *))
#define VM_PAGE_TABLES_PER_DIRECTORY                    (VM_PAGE_SIZE / sizeof(void *))
#define VM_MEMORY_SIZE_PER_PAGE_TABLE                   (VM_PAGES_PER_PAGE_TABLE * VM_PAGE_SIZE)
#define VM_KERNEL_PAGE_TABLE_COUNT                      8
#define VM_KERNEL_PAGE_COUNT                            (VM_KERNEL_PAGE_TABLE_COUNT * VM_PAGES_PER_PAGE_TABLE)
#define VM_KERNEL_MEMORY_SIZE                           (VM_PAGES_PER_PAGE_TABLE * VM_KERNEL_PAGE_TABLE_COUNT)
#define VM_TEMPORARY_PAGE_ID                            ((1u << 20) - 1)
#define VM_TEMPORARY_PAGE_ADDRESS                       (pageIdToPageAddress(VM_TEMPORARY_PAGE_ID))
#define VM_PAGE_TABLE_FOR_TEMPORARY_PAGE_PAGE_ID        (VM_TEMPORARY_PAGE_ID - 1)
#define VM_PAGE_TABLE_FOR_TEMPORARY_PAGE_PAGE_ADDRESS   (pageIdToPageAddress(VM_PAGE_TABLE_FOR_TEMPORARY_PAGE_PAGE_ID))

#define VM_STACK_PAGES_PER_PROCESS                      1024
#define VM_STACK_SIZE_PER_PROCESS                       (VM_STACK_PAGES_PER_PROCESS * VM_PAGE_SIZE)
#define VM_STACK_VIRTUAL_PAGE_ID_END                    VM_PAGE_TABLE_FOR_TEMPORARY_PAGE_PAGE_ID
#define VM_STACK_VIRTUAL_PAGE_ID_BEGIN                  (VM_STACK_VIRTUAL_PAGE_ID_END - VM_STACK_PAGES_PER_PROCESS)
#define VM_STACK_VIRTUAL_ADDRESS_HIGH                   (pageIdToPageAddress(VM_STACK_VIRTUAL_PAGE_ID_END))
#define VM_STACK_VIRTUAL_ADDRESS_LOW                    (pageIdToPageAddress(VM_STACK_VIRTUAL_PAGE_ID_BEGIN))

#define VM_KSTACK_PAGES_PER_PROCESS                     1024
#define VM_KSTACK_SIZE_PER_PROCESS                      (VM_KSTACK_PAGES_PER_PROCESS * VM_PAGE_SIZE)
#define VM_KSTACK_VIRTUAL_PAGE_ID_END                   (VM_STACK_VIRTUAL_PAGE_ID_BEGIN - 1)
#define VM_KSTACK_VIRTUAL_PAGE_ID_BEGIN                 (VM_KSTACK_VIRTUAL_PAGE_ID_END - VM_KSTACK_PAGES_PER_PROCESS)
#define VM_KSTACK_VIRTUAL_ADDRESS_HIGH                  (pageIdToPageAddress(VM_KSTACK_VIRTUAL_PAGE_ID_END))
#define VM_KSTACK_VIRTUAL_ADDRESS_LOW                   (pageIdToPageAddress(VM_KSTACK_VIRTUAL_PAGE_ID_BEGIN))

#define VM_SHELL_ARGUMENT_PAGE_ID                       (VM_KSTACK_VIRTUAL_PAGE_ID_BEGIN - 1)
#define VM_SHELL_ARGUMENT_PAGE_ADDRESS                  (pageIdToPageAddress(VM_SHELL_ARGUMENT_PAGE_ID))

#define VM_HEAP_START_PAGE_ID                           VM_KERNEL_PAGE_COUNT
#define VM_HEAP_START_PAGE_ADDRESS                      (pageIdToPageAddress(VM_KERNEL_PAGE_COUNT))

typedef struct PageDirectoryEntry {
    uint32 present                    : 1;
    uint32 readWrite                  : 1;
    uint32 userSupervisor             : 1;
    uint32 writeThrough               : 1;
    uint32 disableCache               : 1;
    uint32 accessed                   : 1;
    uint32 dirty                      : 1;
    uint32 global                     : 1;
    uint32 zero1                      : 1;
    uint32 zero2                      : 3;
    uint32 physicalPageIdForPageTable : 20;
} __attribute__((packed)) PageDirectoryEntry;

typedef PageDirectoryEntry *PageDirectory;

typedef struct PageTableEntry {
    uint32 present               : 1;
    uint32 readWrite             : 1;
    uint32 userSupervisor        : 1;
    uint32 writeThrough          : 1;
    uint32 disableCache          : 1;
    uint32 accessed              : 1;
    uint32 zero1                 : 1;
    uint32 isLargePage           : 1;
    uint32 zero2                 : 1;
    uint32 zero3                 : 3;
    uint32 physicalPageIdForPage : 20;
} __attribute__((packed)) PageTableEntry;

typedef PageTableEntry *PageTable;

inline static void moveMemory(void *destination, void *source, uint32 length) {
    byte *byteDestination = (byte *)destination;
    byte *byteSource = (byte *)source;
    if (destination < source)
        for (uint32 i = 0; i < length; i++)
            byteDestination[i] = byteSource[i];
    else
        for (uint32 i = 0; i < length; i++)
            byteDestination[length - i - 1] = byteSource[length - i - 1];
}

inline static void *alignAddressUpToPage(void *address) {
    uint32 x = (uint32)address;
    uint32 r = x % VM_PAGE_SIZE;
    if (r == 0) return address;
    return (void *)(x - r + VM_PAGE_SIZE);
}

inline static uint32 getRoundedUpPageCountOfMemorySize(uint32 size) {
    return size / VM_PAGE_SIZE + !!(size % VM_PAGE_SIZE);
}

inline static uint32 getRoundedDownPageCountOfMemorySize(uint32 size) {
    return size / VM_PAGE_SIZE;
}

inline static uint32 pageAddressToPageId(void *page) {
    return ((uint32)page) >> 12;
}

inline static void *pageIdToPageAddress(uint32 id) {
    return (void *)(id << 12);
}

inline static uint32 pageIdToPageDirectoryEntryIndex(uint32 pageId) {
    return pageId / VM_PAGES_PER_PAGE_TABLE;
}

inline static uint32 pageIdToPageTableEntryIndex(uint32 pageId) {
    return pageId % VM_PAGES_PER_PAGE_TABLE;
}

void appendNewAvailableMemoryToFreePages(void *start, uint32 size);
PageDirectory initializeVirtualMemory();
void writeToAnotherVirtualMemorySpacePage(PageDirectory pageDirectoryPhysicalAddress, uint32 virtualPageId, void *src);
PageDirectory allocateVirtualMemorySpace();
void deallocateVirtualMemorySpace(PageDirectory pageDirectoryPhysicalAddress);
void allocateVirtualMemoryPages(PageDirectory pageDirectoryPhysicalAddress, uint32 startVirtualPageId, uint32 count);
void deallocateVirtualMemoryPages(PageDirectory pageDirectoryPhysicalAddress, uint32 startVirtualPageId, uint32 count);
