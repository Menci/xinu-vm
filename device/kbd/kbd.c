#include "kbd.h"

#define KBD_BUFFER_SIZE 4

sid32 kbdSem; // kbdSem = buffer length - 1
char kbdBuffer[KBD_BUFFER_SIZE];
int32 kbdBufferHead;

devcall kbdinit(struct dentry *devptr) {
    kbdSem = semcreate(0);
    if (kbdSem == SYSERR)
        return SYSERR;

    kbdBufferHead = 0;

    set_evec(devptr->dvirq, (uint32)devptr->dvintr);

    return OK;
}

void kbdhandler(void) {
    reschedule_cntl(DEFER_START);

    while (1) {
        int32 bufferLength = semcount(kbdSem);
        if (bufferLength < 0) bufferLength = 0;

        // Get key
        char newChar = readKeyboardBuffer();
        if (newChar == -1 || newChar == 0) break;

        if (bufferLength >= KBD_BUFFER_SIZE) {
            // Buffer full
            kprintf("kbdhandler: buffer full, buffer head = %d, buffer length = %d, char = [%d] '%c'\n", kbdBufferHead, bufferLength, (int)newChar, newChar);
            break;
        }

        // Append to buffer
        kprintf("kbdhandler: got key, buffer head = %d, buffer length = %d, char = [%d] '%c'\n", kbdBufferHead, bufferLength, (int)newChar, newChar);

        int32 pos = (kbdBufferHead + bufferLength) % KBD_BUFFER_SIZE;
        kbdBuffer[pos] = newChar;
        signal(kbdSem);
    }

    reschedule_cntl(DEFER_STOP);
}

devcall	kbdgetc(struct dentry *devptr) {
    wait(kbdSem);
    int i = kbdBufferHead++;
    char gotChar = kbdBuffer[i];
    kbdBufferHead %= KBD_BUFFER_SIZE;
    kprintf("kbdgetc: got char, buffer head = %d, char = [%d] '%c'\n", i, (int)gotChar, gotChar);
    return gotChar;
}

devcall	kbdread(struct dentry *devptr, char	*buff, int32 count) {
    char ch;
    int32 i = 0;
    while (i < count && (ch = kbdgetc(devptr)) != '\n')
        buff[i++] = ch;
    return i;
}
