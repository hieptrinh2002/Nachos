#include "syscall.h"
#include "copyright.h"
// TASK I, Xu ly sort = bubble sort
int main()
{
	int n, i, j, temp;
	int a[100];
	PrintString("Input an n array of max 100 elements:");
	n = ReadInt();
	if(n>100) return;
	PrintString("Input your array below: ");
	for (i = 0; i < n; i++) {
		a[i] = ReadInt();
	} 
	for (i = 0; i < n-1; i++) {
		for (j = i+1; j < n; j++) {
			if (a[j] < a[i])
			{
				temp = a[j];
				a[j] = a[i];
				a[i] = temp;
			}
		}
	}
	PrintString("This is your sorted array:");
	for (i = 0; i < n; i++) {
		PrintInt(a[i]);
		PrintChar(' ');
	}
	PrintString("\n");
	return 0;
}

