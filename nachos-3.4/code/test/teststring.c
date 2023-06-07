/* halt.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

int
main()
{
	char * s;
	int n;
	//printf("\nNhap so ki tu cua chuoi: ");
	n = ReadInt(n);
	//printf("\nNhap chuoi: ");
	ReadString(s, n);
	//printf("\nChuoi ban vua nhap la: ");
	PrintString(s);
    	Halt();
    /* not reached */
}
