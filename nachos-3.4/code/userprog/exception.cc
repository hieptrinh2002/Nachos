// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "math.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

#define MaxFileLength 32

void IncreaseProgramCounter()
{
	// machine.h
	//#define PCReg	34	// Current program counter
	//#define NextPCReg	35	// Next program counter (for branch delay)
	//#define PrevPCReg	36	// Previous program counter (for debugging)

	// 3 steps
	machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));		 // update Previous program counter to Current program counter
	machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));		 // update Current program counter to Next program counter
	machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4); // update Next program counter to next of next program counter
}

void ExceptionHandler(ExceptionType which)
{
	switch (which)
	{
	case NoException:
		return;
	case PageFaultException:
		DEBUG('a', "\n\nNo valid translation found. \n\n");
		printf("\n\n No valid translation found. \n\n");
		interrupt->Halt();
		break;
	case ReadOnlyException:
		DEBUG('a', "\n\nWrite attempted to page marked read-only. \n\n");
		printf("\n\n Write attempted to page marked read-only. \n\n");
		interrupt->Halt();
		break;
	case BusErrorException:
		DEBUG('a', "\n\nTranslation resulted invalid physical address. \n\n");
		printf("\n\n Translation resulted invalid physical address. \n\n");
		interrupt->Halt();
		break;
	case AddressErrorException:
		DEBUG('a', "\n\n Unaligned reference or one that was beyond the end of the address space. \n\n");
		printf("\n\n Unaligned reference or one that was beyond the end of the address space. \n\n");
		interrupt->Halt();
		break;
	case OverflowException:
		DEBUG('a', "\n\nInteger overflow in add or sub. \n\n");
		printf("\n\n Integer overflow in add or sub. \n\n");
		interrupt->Halt();
		break;
	case IllegalInstrException:
		DEBUG('a', "\n\n Unimplemented or reserved instr.\n\n");
		printf("\n\n Unimplemented or reserved instr.\n\n");
		interrupt->Halt();
		break;
	case NumExceptionTypes:
		DEBUG('a', "\n\n Number exception types.\n\n");
		printf("\n\n Number exception types.\n\n");
		interrupt->Halt();
		break;
	case SyscallException:
	{
		int type = machine->ReadRegister(2);
		switch (type)
		{
		case SC_Halt:
		{
			// STOP HDH
			DEBUG('a', "\n Shutdown, initiated by user program.");
			printf("\n\nShutdown, initiated by user program. \n");
			interrupt->Halt();
			return;
		}
		case SC_ReadInt:
		{
			char *buffer = new char[255];
			int len = gSynchConsole->Read(buffer, 256);
			int i = 0;
			if (buffer[0] == '-')
			{
				i++; // increase counter if num is negative
			}
			// validate int
			for (; i < len; ++i)
			{
				if (buffer[i] < '0' || buffer[i] > '9')
				{
					printf("Not an integer!");
					i = len + 1; // falsy flag instead of bool
				}
			}

			if (i == len + 1)
			{
				machine->WriteRegister(2, 0);
			}
			else
			{
				machine->WriteRegister(2, atoi(buffer));
			}
			// 0 if not an int
			break;
		}
		case SC_PrintInt:
		{
			int num = machine->ReadRegister(4);
			if (num == 0)
			{
				gSynchConsole->Write("0\0", 2);
				break;
			}
			bool sign = num < 0;
			int len = 0;
			if (sign)
			{
				num = -num;
				len++;
			}
			len += (int)log10(num) + 1;

			char *output = new char[len + 1];
			if (sign)
			{
				output[0] = '-';
			}
			output[len] = '\0';

			for (int i = len - 1; i >= (int)sign; --i, num /= 10)
			{
				output[i] = (char)((num % 10) + '0');
			}

			gSynchConsole->Write(output, len + 1);
			break;
		}
		case SC_ReadChar:
		{
			char ch;
			gSynchConsole->Read(&ch, 1);
			machine->WriteRegister(2, ch);
			break;
		}

		case SC_PrintChar:
		{
			char ch = machine->ReadRegister(4);
			gSynchConsole->Write(&ch, 1);
			break;
		}
		case SC_ReadString:
		{
			// Lay dia chi ki tu dau tien cua chuoi
			int addr = machine->ReadRegister(4);
			// Lay do dai chuoi cho phep
			int length = machine->ReadRegister(5);

			char *buff = new char[length + 1]; // Cap phat bo nho dua tren do dai chuoi cho phep
			if (buff == NULL)
				break;
			memset(buff, 0, length + 1);				  // khoi tao gia tri 0 cho chuoi buf
			int size = gSynchConsole->Read(buff, length); // Lay so byte thuc su doc duoc
			machine->System2User(addr, size, buff);		  // copy chuoi tu vung nho system space sang user space co do dai la so byte thuc su cua chuoi nguoi dung nhap
			break;
		}
		case SC_PrintString:
		{
			int addr = machine->ReadRegister(4); // Lay dia chi ki tu dau tien cua chuoi
			char *buff = new char[256];
			buff = machine->User2System(addr, 255); // Copy chuoi tu vung nho user space sang system space
			if (buff == NULL)
				break;

			int i = 0;
			while (buff[i] != 0 && buff[i] != '\n')
			{
				gSynchConsole->Write(buff + i, 1); // Xuat tung ky tu cua chuoi ra man hinh
				i++;
			}
			buff[i] = '\n'; // Ket thuc chuoi bang ki tu xuong dong.

			gSynchConsole->Write(buff + i, 1); // Xuat ki tu xuong dong ra man hinh
			machine->WriteRegister(2, i - 1);

			break;
		}
		case SC_CreateFile:
		{
			int virtAddr;
			char *filename;

			//  printf("\n SC_Create call...");
			// printf("\n Reading virtual address of file name.");
			DEBUG('a', "\n SC_Create call ...");
			DEBUG('a', "\n Reading virtual address of filename");

			// check for exception
			virtAddr = machine->ReadRegister(4);
			DEBUG('a', "\n Reading filename.");
			filename = machine->User2System(virtAddr, MaxFileLength + 1);
			if (filename == NULL)
			{
				printf("\n Not enough memory in system");
				DEBUG('a', "\n Not enough memory in system");
				machine->WriteRegister(2, -1);
				delete filename;
				break;
			}

			if (strlen(filename) == 0 || (strlen(filename) >= MaxFileLength + 1))
			{
				printf("\n Too many characters in filename: %s", filename);
				DEBUG('a', "\n Too many characters in filename");
				machine->WriteRegister(2, -1);
				delete filename;
				break;
			}

			// printf("\n Finish reading filename.");
			// printf("\n File name: '%s'",filename);
			DEBUG('a', "\n Finish reading filename.");
			// DEBUG('a',"\n File name : '"<<filename<<"'");

			// Create file with size = 0
			if (!fileSystem->Create(filename, 0))
			{
				printf("\n Error create file '%s'", filename);
				delete filename;
				machine->WriteRegister(2, -1);
				delete filename;
				break;
			}
			// printf("\n Create file '%s' success",filename);

			machine->WriteRegister(2, 0);

			delete filename;
			break;
		}
		case SC_Open:
		{
			//  printf("\n Calling SC_Open.");
			int virtAddr = machine->ReadRegister(4);
			int type = machine->ReadRegister(5);

			if (type < 0 || type > 3)
			{
				printf("\n SC_OpenError: unexpected file type: %d", type);
				break;
			}

			int freeSlot = fileSystem->FindFreeSlot();
			if (freeSlot < 0)
			{
				printf("\n SC_OpenError: No free slot.");
				break;
			}

			char *filename = machine->User2System(virtAddr, MaxFileLength + 1);

			if (filename == NULL)
			{
				printf("\n Not enough memory in system");
				DEBUG('a', "\n Not enough memory in system");
				machine->WriteRegister(2, -1);
				delete filename;
				break;
			}

			if (strlen(filename) == 0 || (strlen(filename) >= MaxFileLength + 1))
			{
				printf("\n Too many characters in filename: %s", filename);
				DEBUG('a', "\n Too many characters in filename");
				machine->WriteRegister(2, -1);
				delete filename;
				break;
			}

			if (type == 0 || type == 1) // chi xu li khi type = 0 hoac 1
			{

				if ((fileSystem->openf[freeSlot] = fileSystem->Open(filename, type)) != NULL) // Mo file thanh cong
				{
					machine->WriteRegister(2, freeSlot); // tra ve OpenFileID
				}
			}
			else if (type == 2) // xu li stdin voi type quy uoc la 2
			{
				machine->WriteRegister(2, 0); // tra ve OpenFileID
			}
			else // xu li stdout voi type quy uoc la 3
			{
				machine->WriteRegister(2, 1); // tra ve OpenFileID
			}

			if (fileSystem->openf[freeSlot] == NULL)
			{
				printf("\n Error opening file:  %s", filename);
				DEBUG('a', "\n Error opening file.");
				machine->WriteRegister(2, -1);
				delete filename;
				break;
			}

			machine->WriteRegister(2, freeSlot);

			break;
		}
		case SC_Close:
		{
			int id = machine->ReadRegister(4);
			if (id < 0 || id > 14)
			{
				printf("\n CloseError: Unexpected file id: %d", id);
				machine->WriteRegister(2, -1);
				break;
			}
			if (!fileSystem->openf[id])
			{
				printf("\n CloseError: closing file id %d is not opened", id);
				machine->WriteRegister(2, -1);
				break;
			}

			// currentThread->
			delete fileSystem->openf[id];
			fileSystem->openf[id] = NULL;
			machine->WriteRegister(2, 0);
			break;
		}
		case SC_Read:
		{
			// Input: buffer(char*), so ky tu(int), id cua file(OpenFileID)
			// Output: -1: Loi, So byte read thuc su: Thanh cong, -2: Thanh cong
			// Cong dung: Doc file voi tham so la buffer, so ky tu cho phep va id cua file
			int virtAddr = machine->ReadRegister(4);  // Lay dia chi cua tham so buffer tu thanh ghi so 4
			int charcount = machine->ReadRegister(5); // Lay charcount tu thanh ghi so 5
			int id = machine->ReadRegister(6);		  // Lay id cua file tu thanh ghi so 6
			int OldPos;
			int NewPos;
			char *buf;
			// Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
			if (id < 0 || id > 14)
			{
				printf("\nID isn't valid.");
				machine->WriteRegister(2, -1);
				break;
			}
			// Kiem tra file co ton tai khong
			if (fileSystem->openf[id] == NULL)
			{
				printf("\nFile doesn't exist.");
				machine->WriteRegister(2, -1);
				break;
			}
			if (fileSystem->openf[id]->type == 3) // Xet truong hop doc file stdout (type quy uoc la 3) thi tra ve -1
			{
				printf("\nCan't read file stdout.");
				machine->WriteRegister(2, -1);
				break;
			}
			OldPos = fileSystem->openf[id]->GetCurrentPos(); // Kiem tra thanh cong thi lay vi tri OldPos
			buf = machine->User2System(virtAddr, charcount); // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charcount
			// Xet truong hop doc file stdin (type quy uoc la 2)
			if (fileSystem->openf[id]->type == 2)
			{
				// Su dung ham Read cua lop SynchConsole de tra ve so byte thuc su doc duoc
				int size = gSynchConsole->Read(buf, charcount);
				machine->System2User(virtAddr, size, buf); // Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer co do dai la so byte thuc su
				machine->WriteRegister(2, size);		   // Tra ve so byte thuc su doc duoc
				delete buf;
				break;
			}
			// Xet truong hop doc file binh thuong thi tra ve so byte thuc su
			if ((fileSystem->openf[id]->Read(buf, charcount)) > 0)
			{
				// So byte thuc su = NewPos - OldPos
				NewPos = fileSystem->openf[id]->GetCurrentPos();
				// Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer co do dai la so byte thuc su
				machine->System2User(virtAddr, NewPos - OldPos, buf);
				machine->WriteRegister(2, NewPos - OldPos);
			}
			else
			{
				// Truong hop con lai la doc file co noi dung la NULL tra ve -2
				// printf("\nDoc file rong.");
				machine->WriteRegister(2, -2);
			}
			delete buf;
			break;
		}
		case SC_Write:
		{
			// Input: buffer(char*), so ky tu(int), id cua file(OpenFileID)
			// Output: -1: Loi, So byte write thuc su: Thanh cong, -2: Thanh cong
			// Cong dung: Ghi file voi tham so la buffer, so ky tu cho phep va id cua file
			int virtAddr = machine->ReadRegister(4);  // Lay dia chi cua tham so buffer tu thanh ghi so 4
			int charcount = machine->ReadRegister(5); // Lay charcount tu thanh ghi so 5
			int id = machine->ReadRegister(6);		  // Lay id cua file tu thanh ghi so 6
			int OldPos;
			int NewPos;
			char *buf;
			// Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
			if (id < 0 || id > 14)
			{
				printf("\nID isn't valid.");
				machine->WriteRegister(2, -1);
				break;
			}
			// Kiem tra file co ton tai khong
			if (fileSystem->openf[id] == NULL)
			{
				printf("\nFile doesn't exist.");
				machine->WriteRegister(2, -1);
				break;
			}
			// Xet truong hop ghi file only read (type quy uoc la 1) hoac file stdin (type quy uoc la 2) thi tra ve -1
			if (fileSystem->openf[id]->type == 1 || fileSystem->openf[id]->type == 2)
			{
				printf("\nCan't write file stdin (or file only read).");
				machine->WriteRegister(2, -1);
				break;
			}
			OldPos = fileSystem->openf[id]->GetCurrentPos(); // Kiem tra thanh cong thi lay vi tri OldPos
			buf = machine->User2System(virtAddr, charcount); // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charcount
			// Xet truong hop ghi file read & write (type quy uoc la 0) thi tra ve so byte thuc su
			if (fileSystem->openf[id]->type == 0)
			{
				if ((fileSystem->openf[id]->Write(buf, charcount)) > 0)
				{
					// So byte thuc su = NewPos - OldPos
					NewPos = fileSystem->openf[id]->GetCurrentPos();
					machine->WriteRegister(2, NewPos - OldPos);
					delete buf;
					break;
				}
			}
			if (fileSystem->openf[id]->type == 3) // Xet truong hop con lai ghi file stdout (type quy uoc la 3)
			{
				int i = 0;
				while (buf[i] != 0 && buf[i] != '\n') // Vong lap de write den khi gap ky tu '\n'
				{
					gSynchConsole->Write(buf + i, 1); // Su dung ham Write cua lop SynchConsole
					i++;
				}
				buf[i] = '\n';
				gSynchConsole->Write(buf + i, 1); // Write ky tu '\n'
				machine->WriteRegister(2, i - 1); // Tra ve so byte thuc su write duoc
				delete buf;
				break;
			}
		}
		case SC_Exec:
		{
			int virtAddr = machine->ReadRegister(4);
			char *name;
			name = machine->User2System(virtAddr, MaxFileLength + 1);

			if (name == NULL)
			{
				DEBUG('a', "\n Not enough memory !!");
				printf("\n Not enough memory !!");
				machine->WriteRegister(2, -1); // return -1
				break;
			}

			OpenFile *oFile = fileSystem->Open(name);

			if (oFile == NULL)
			{
				DEBUG('a', "\n Can't open this file !!");
				printf("\nExec:: Can't open this file !!");
				machine->WriteRegister(2, -1); // return -1
				break;
			}
			delete oFile;

			// Return child process id
			int id = pTab->ExecUpdate(name);
			machine->WriteRegister(2, id); // return id to register2

			delete[] name;

			break;
		}
		case SC_Join:
		{
			// int Join(SpaceId id)
			// Input: id dia chi cua thread
			// Output:
			if (id < 0 || id > 14)
			{
				printf("\nID isn't valid.");
				machine->WriteRegister(2, -1);
				break;
			}
			// Kiem tra file co ton tai khong
			if (fileSystem->openf[id] == NULL)
			{
				printf("\nFile doesn't exist.");
				machine->WriteRegister(2, -1);
				break;
			}
			// Xet truong hop ghi file only read (type quy uoc la 1) hoac file stdin (type quy uoc la 2) thi tra ve -1
			if (fileSystem->openf[id]->type == 1 || fileSystem->openf[id]->type == 2)
			{
				printf("\nCan't write file stdin (or file only read).");
				machine->WriteRegister(2, -1);
				break;
			}

			int id = machine->ReadRegister(4);

			int res = pTab->JoinUpdate(id);

			machine->WriteRegister(2, res);
			break;
		}
		case SC_Exit:
		{
			printf("\n\n Calling SC_Exit.");
			DEBUG('a', "\n\n Calling SC_Exit.");

			// avoid harry
			IntStatus oldLevel = interrupt->SetLevel(IntOff);

			int exitStatus;
			// ProcessHashData *processData;

			exitStatus = machine->ReadRegister(4);

			// if process exited with error, print error
			if (exitStatus != 0)
				printf("\nProcess %s exited with error level %d", currentThread->getName(), exitStatus);

			//  currentThread->Finish();
			(void)interrupt->SetLevel(oldLevel);
			interrupt->Halt();
			machine->WriteRegister(2, 0);
			break;
		}
		case SC_CreateSemaphore:
		{
			// Cú pháp: int CreateSemaphore(char* name, int semval);
			// Input: tên của semaphore (name), giá trị cua semaphore (semval)
			// Output: Success: 0 - Failed: -1
			// Cong dung:  Tạo Semaphore mới

			int virtAddr = machine->ReadRegister(4); //Đọc địa chỉ “name” từ thanh ghi r4.
			int semval = machine->ReadRegister(5);	 //Đọc giá trị “semval” từ thanh ghi r5.

			//"name" lúc này đang ở trong user space. Gọi hàm User2System đã được khai báo
			//để chuyển vùng nhớ user space tới vùng nhớ system space.
			char *name;
			name = machine->User2System(virtAddr, MaxFileLength + 1); // Lay ten Semaphore, nap vao kernel

			// Kiem tra name rong
			if (name == NULL)
			{
				DEBUG('a', "\nCan not create semaphore with null name");
				printf("\nCan not create semaphore with null name");
				machine->WriteRegister(2, -1);
				break;
			}

			// Gọi thực hiện hàm semTab->Create(name,semval) để tạo Semaphore, nếu có lỗi thì báo lỗi.
			int res = semTab->Create(name, semval);

			if (res == -1)
			{
				DEBUG('a', "\nCan not create semaphore (%s, %d)", name, &semval);
				printf("\nCan not create semaphore (%s, %d)", name, &semval);
				machine->WriteRegister(2, -1);
				delete[] name;
				break;
			}

			// Lưu kết quả thực hiện vào thanh ghi r2
			machine->WriteRegister(2, res);
			delete[] name;
			break;
		}
		case SC_Wait:
		{
			// int Wait(char* name)
			int virtAddr = machine->ReadRegister(4);

			char *name = machine->User2System(virtAddr, MaxFileLength + 1);
			if (name == NULL)
			{
				DEBUG('a', "\n Not enough memory in System");
				printf("\n Not enough memory in System");
				machine->WriteRegister(2, -1);
				delete[] name;
				break;
			}

			int res = semTab->Wait(name);

			if (res == -1)
			{
				DEBUG('a', "\n Semaphore name doesn't exist!");
				printf("\n Semaphore name doesn't exist!");
				machine->WriteRegister(2, -1);
				delete[] name;
				break;
			}

			delete[] name;
			machine->WriteRegister(2, res);
			break;
		}
		case SC_Signal:
		{
			// int Signal(char* name)
			int virtAddr = machine->ReadRegister(4);

			char *name = machine->User2System(virtAddr, MaxFileLength + 1);
			if (name == NULL)
			{
				DEBUG('a', "\n Not enough memory in System");
				printf("\n Not enough memory in System");
				machine->WriteRegister(2, -1);
				delete[] name;
				break;
			}

			int res = semTab->Signal(name);

			if (res == -1)
			{
				DEBUG('a', "\n Semaphore name doesn't exist!");
				printf("\n Semaphore name doesn't exist!");
				machine->WriteRegister(2, -1);
				delete[] name;
				break;
			}

			delete[] name;
			machine->WriteRegister(2, res);
			break;
		}

		default:
		{
			printf("\n\n Shutdown, initiated by user program. \n");
			interrupt->Halt();
			break;
		}
		}
		IncreaseProgramCounter();
		break;
	}

	default:
		// printf("\n Unexpected user mode exception (%d %d)", which, type);
		break;
	}
}
