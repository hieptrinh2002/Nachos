#include "syscall.h"
//In bang ASCII

int main() {
	int i = 32;
    for (; i < 127; i++) 
    {
        PrintInt(i); //Printing Integers
        PrintString(" - ");
        PrintChar(i); //Printing Characters of i
        PrintString("\n");
    }
}

