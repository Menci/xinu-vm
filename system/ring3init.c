#include <xinu.h>

struct gdt_entry {
	unsigned int limit_low              : 16;
	unsigned int base_low               : 24;
	unsigned int accessed               :  1;
	unsigned int read_write             :  1; // readable for code, writable for data
	unsigned int conforming_expand_down :  1; // conforming for code, expand down for data
	unsigned int code                   :  1; // 1 for code, 0 for data
	unsigned int code_data_segment      :  1; // should be 1 for everything but TSS and LDT
	unsigned int DPL                    :  2; // privilege level
	unsigned int present                :  1;
	unsigned int limit_high             :  4;
	unsigned int available              :  1; // only used in software; has no effect on hardware
	unsigned int long_mode              :  1;
	unsigned int big                    :  1; // 32-bit opcodes for code, uint32 stack for data
	unsigned int gran                   :  1; // 1 to use 4k page addressing, 0 for byte addressing
	unsigned int base_high              :  8;
} __attribute__((packed));

struct tss_entry {
	uint32 prev_tss; // The previous TSS - with hardware task switching these form a kind of backward linked list.
	uint32 esp0;     // The stack pointer to load when changing to kernel mode.
	uint32 ss0;      // The stack segment to load when changing to kernel mode.
	// Everything below here is unused.
	uint32 esp1; // esp and ss 1 and 2 would be used when switching to rings 1 or 2.
	uint32 ss1;
	uint32 esp2;
	uint32 ss2;
	uint32 cr3;
	uint32 eip;
	uint32 eflags;
	uint32 eax;
	uint32 ecx;
	uint32 edx;
	uint32 ebx;
	uint32 esp;
	uint32 ebp;
	uint32 esi;
	uint32 edi;
	uint32 es;
	uint32 cs;
	uint32 ss;
	uint32 ds;
	uint32 fs;
	uint32 gs;
	uint32 ldt;
	uint16 trap;
	uint16 iomap_base;
} __attribute__((packed));

struct tss_entry tss;

void inittss(struct gdt_entry *g) {
	// Compute the base and limit of the TSS for use in the GDT entry.
	uint32 base = (uint32)&tss;
	uint32 limit = sizeof(tss);
 
	// Add a TSS descriptor to the GDT.
	g->limit_low = limit;
	g->base_low = base;
	g->accessed = 1; // With a system entry (`code_data_segment` = 0), 1 indicates TSS and 0 indicates LDT
	g->read_write = 0; // For a TSS, indicates busy (1) or not busy (0).
	g->conforming_expand_down = 0; // always 0 for TSS
	g->code = 1; // For a TSS, 1 indicates 32-bit (1) or 16-bit (0).
	g->code_data_segment = 0; // indicates TSS/LDT (see also `accessed`)
	g->DPL = 0; // ring 0, see the comments below
	g->present = 1;
	g->limit_high = (limit & (0xf << 16)) >> 16; // isolate top nibble
	g->available = 0; // 0 for a TSS
	g->long_mode = 0;
	g->big = 0; // should leave zero according to manuals.
	g->gran = 0; // limit is in bytes, not pages
	g->base_high = (base & (0xff << 24)) >> 24; //isolate top byte
 
	// Ensure the TSS is initially zero'd.
	memset(&tss, 0, sizeof(tss));
 
	tss.ss0  = (2 << 3) | 0;  // Set the kernel stack segment.
	tss.esp0 = 0; // Set the kernel stack pointer.
	//note that CS is loaded from the IDT entry and should be the regular kernel code segment
}

// Set kernel stack
void setkstk(uint32 esp0) {
	// Update TSS
	tss.esp0 = esp0;

	// Set IA32_SYSENTER_ESP (0x175)
	asm volatile (
		"xor %%edx, %%edx\n\t"  // high32: 0
		"mov %0, %%eax\n\t"     // low32 : esp0
		"mov $0x175, %%ecx\n\t" // IA32_SYSENTER_ESP
		"wrmsr\n\t"
		:
		:"m"(esp0)
		:"edx", "eax", "ecx"
	);
}

void initgdt(void *pgdt) {
	struct gdt_entry *gdt = pgdt;

	// gdt[1]: kernel code
	// gdt[2]: kernel data/stack
	// gdt[3]: user code
	// gdt[4]: user data/stack
	// gdt[5]: tss

	gdt[3] = gdt[1];
	gdt[3].DPL = 3;
	gdt[4] = gdt[2];
	gdt[4].DPL = 3;
	
	inittss(&gdt[5]);

	void initring3();
	initring3();
}

void initring3() {
	extern void syscall_dispatch();
	uint32 syscall_dispatch_addr = (uint32)&syscall_dispatch;

	// Set IA32_SYSENTER_CS (0x174)
	asm volatile (
		"xor %%edx, %%edx\n\t"        // high32: 0
		"mov $(1 * 8) | 0, %%eax\n\t" // low32 : kernel CS
		"mov $0x174, %%ecx\n\t"       // IA32_SYSENTER_CS
		"wrmsr\n\t"
		:
		:
		:"edx", "eax", "ecx"
	);

	// Set IA32_SYSENTER_EIP (0x176)
	asm volatile (
		"xor %%edx, %%edx\n\t"        // high32: 0
		"mov %0, %%eax\n\t"           // low32 : addr of syscall_dispatch()
		"mov $0x176, %%ecx\n\t"       // IA32_SYSENTER_EIP
		"wrmsr\n\t"
		:
		:"m"(syscall_dispatch_addr)
		:"edx", "eax", "ecx"
	);
}

void loadtss() {
	// Apple TSS
	asm volatile (
		"mov $(5 * 8) | 0, %%ax\n\t"
		"ltr %%ax"
		:
		:
		:"ax"
	);
}
