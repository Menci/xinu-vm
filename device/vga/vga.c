#include <xinu.h>

void memset16(void *dest, int32 count, uint16 value) {
    uint16 *dest16 = (uint16 *)dest;
    for (int32 i = 0; i < count; i++)
        dest16[i] = value;
}

void memmove(void *dest, void *src, int32 length) {
    uint8 *dest8 = (uint8 *)dest;
    uint8 *src8 = (uint8 *)src;
    if (dest < src)
        for (int32 i = 0; i < length; i++)
            dest8[i] = src8[i];
    else
        for (int32 i = 0; i < length; i++)
            dest8[length - i - 1] = src8[length - i - 1];
}

struct CursorPosition {
    uint8 row, column;
} curr = { 0, 0 };

const uint8 SCREEN_WIDTH = 80;
const uint8 SCREEN_HEIGHT = 25;
uint16 *const TEXT_MODE_BUFFER = (uint16 *)0xB8000;
const uint8 BLACK_WHITE = 0x07;

uint16 getScreenPositionIndex(uint8 row, uint8 column) {
    return (uint16)row * SCREEN_WIDTH + column;
}

uint16 combineCharacterWithMode(char ascii, uint8 mode) {
    return ((uint16)mode << 8) | ascii;
}

void writeCharacterToScreenMemory(uint8 row, uint8 column, char ascii, uint8 mode) {
    uint16 index = getScreenPositionIndex(row, column);
    TEXT_MODE_BUFFER[index] = combineCharacterWithMode(ascii, mode);
}

void clearScreen() {
    memset16(
        TEXT_MODE_BUFFER,
        (int32)SCREEN_WIDTH * SCREEN_HEIGHT,
        combineCharacterWithMode(' ', BLACK_WHITE)
    );
}

void setCursorPosition(uint8 row, uint8 column) {
    uint16 index = getScreenPositionIndex(row, column);
 
    outb(0x3D4, 0x0F);
    outb(0x3D5, index & 0xFF);
    outb(0x3D4, 0x0E);
    outb(0x3D5, (index >> 8) & 0xFF);

    curr.row = row;
    curr.column = column;
}

void scrollUpScreen() {
    // Move everything in the buffer except the last line
    memmove(
        TEXT_MODE_BUFFER,                // Buffer address from 0-th line
        TEXT_MODE_BUFFER + SCREEN_WIDTH, // Buffer address from 1-st line
        (int32)SCREEN_WIDTH * (SCREEN_HEIGHT - 1) * sizeof(uint16)
    );

    // Fill the last line
    memset16(
        TEXT_MODE_BUFFER + (int32)SCREEN_WIDTH * (SCREEN_HEIGHT - 1),
        SCREEN_WIDTH,
        combineCharacterWithMode(' ', BLACK_WHITE)
    );
}

void moveToNewLine() {
    if (curr.row == SCREEN_HEIGHT - 1) {
        scrollUpScreen();
        setCursorPosition(SCREEN_HEIGHT - 1, 0);
    } else {
        setCursorPosition(curr.row + 1, 0);
    }
}

void backspace() {
    if (curr.column == 0 && curr.row == 0)
        // At first position of screen
        return;

    if (curr.column != 0)
        // In current line
        setCursorPosition(curr.row, curr.column - 1);
    else
        // Move to previous line
        setCursorPosition(curr.row - 1, SCREEN_WIDTH - 1);
    writeCharacterToScreenMemory(curr.row, curr.column, ' ', BLACK_WHITE);
}

void outputAsciiCharacter(char ascii, uint8 mode) {
    switch (ascii) {
    case '\n':
        // Newline
        moveToNewLine();
        break;
    case '\t':
        // Tab
        for (int32 i = 0; i < 8 - (curr.column % 8); i++)
            outputAsciiCharacter(' ', mode);
        break;
    case '\b':
        // Backspace
        backspace();
        break;
    default:
        writeCharacterToScreenMemory(curr.row, curr.column, ascii, mode);
        if (curr.column == SCREEN_WIDTH - 1)
            moveToNewLine();
        else
            setCursorPosition(curr.row, curr.column + 1);
    }
}

devcall vgainit(struct dentry *devptr) {
    clearScreen();
    setCursorPosition(0, 0);
    return OK;
}

devcall vgaputc(struct dentry *devptr, char ch) {
    kprintf("vgaputc: char = [%d] %c\n", (int)ch, ch);
    outputAsciiCharacter(ch, BLACK_WHITE);
    return OK;
}

devcall vgawrite(struct dentry *devptr, char *buff, int32 count) {
    for (int32 i = 0; i < count; i++)
        outputAsciiCharacter(buff[i], BLACK_WHITE);

    return OK;
}
