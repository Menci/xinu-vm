/* xsh_kbdtest.c - xsh_kbdtest */

#include <xinu.h>

shellcmd xsh_kbdtest(int nargs, char *args[]) {
	while (1) {
		char ch = fgetc(KEYBOARD);
		fputc(ch, SCREEN);
	}
	return 0;
}
