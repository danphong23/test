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

void handle_SC_PrintInt(){
    int value = machine->ReadRegister(4);

    if(value == 0){
        gSynchConsole->Write("0", 1);
        return IncreasePC();
    }else if(value == -2147483648){
        gSynchConsole->Write("-", 1);
        gSynchConsole->Write("2147483648", 10);
        return IncreasePC();
    }

    int i = 0;
    int MAX_BUFFER = 255; 
    char* buffer = new char[MAX_BUFFER + 1]; 
    memset(buffer, '0', MAX_BUFFER + 1);
    if(value < 0){
        buffer[i++] = '-';
        value *= -1;
    }
    int length = i;
    int temp = value; 
    while(temp){ 
        length++;
        temp /= 10;
    }
    for(int j = length - 1;j >= i;j--){
        buffer[j] = char((value % 10) + 48);
        value /= 10;
    }

    gSynchConsole->Write(buffer, length);
    delete[] buffer;
    return IncreasePC(); 
}
void handle_SC_ReadInt()
{
	int number = 0; // biến để lưu số nguyên đọc được
	int sign = 1; // biến để lưu dấu của số nguyên
	char buffer = new char[255]; // mảng để lưu chuỗi biểu diễn số nguyên
	int i = 0; // biến chỉ số cho mảng
	int len = synchConsole->Read(buffer, 256); // đọc một dòng từ console vào mảng
	if (len == -1) // nếu người dùng nhập CTRL-A, thì trả về -1
	{
		machine->WriteRegister(2, -1);
		break;
	}
	while (buffer[i] == ' ' || buffer[i] == '\t' || buffer[i] == '\n') // bỏ qua các ký tự trắng
		i++;
	if (buffer[i] == '-') // nếu ký tự là dấu trừ, thì đặt sign bằng -1
	{
		sign = -1;
		i++;
	}
	else if (buffer[i] == '+') // nếu ký tự là dấu cộng, thì bỏ qua nó
		i++;
	while (buffer[i] >= '0' && buffer[i] <= '9') // nếu ký tự là một chữ số, thì cộng dồn vào number
	{
		number = number * 10 + (buffer[i] - '0');
		i++;
	}
	number = number * sign; // nhân với dấu của số nguyên
	machine->WriteRegister(2, number); // ghi kết quả vào thanh ghi 2
	delete[] buffer;

	IncreasePC(); // Tăng Program Counter 
}
void handle_SC_ReadChar(){
    char* buffer = new char;
    gSynchConsole->Read(buffer, 1);
    machine->WriteRegister(2,*buffer);
    return IncreasePC(); 
}
void handle_SC_PrintChar(){
    char c;
    c = (char)machine->ReadRegister(4);
    gSynchConsole->Write(&c, 1);
    return IncreasePC(); 
}
void handle_SC_ReadString()
{
	// Input: Buffer(char*), do dai toi da cua chuoi nhap vao(int)
	// Output: Khong co
	// Cong dung: Doc vao mot chuoi voi tham so la buffer va do dai toi da
	int virtAddr, length;
	char* buffer;
	virtAddr = machine->ReadRegister(4); // Lay dia chi tham so buffer truyen vao tu thanh ghi so 4
	length = machine->ReadRegister(5); // Lay do dai toi da cua chuoi nhap vao tu thanh ghi so 5
	buffer = User2System(virtAddr, length); // Copy chuoi tu vung nho User Space sang System Space
	gSynchConsole->Read(buffer, length); // Goi ham Read cua SynchConsole de doc chuoi
	System2User(virtAddr, length, buffer); // Copy chuoi tu vung nho System Space sang vung nho User Space
	delete buffer;
	IncreasePC(); // Tang Program Counter 
}
void handle_SC_ReadPrintString()
{
	// Input: Buffer(char*)
	// Output: Chuoi doc duoc tu buffer(char*)
	// Cong dung: Xuat mot chuoi la tham so buffer truyen vao ra man hinh
	int virtAddr;
	char* buffer;
	virtAddr = machine->ReadRegister(4); // Lay dia chi cua tham so buffer tu thanh ghi so 4
	buffer = User2System(virtAddr, 255); // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai 255 ki tu
	int length = 0;
	while (buffer[length] != 0) length++; // Dem do dai that cua chuoi
	gSynchConsole->Write(buffer, length + 1); // Goi ham Write cua SynchConsole de in chuoi
	delete buffer;
	IncreasePC(); // Tang Program Counter 
}
void ExceptionHandler(ExceptionType which)
// Cau c) Tang Program Counter
void IncreasePC()
{
	int counter = machine->ReadRegister(PCReg);
   	machine->WriteRegister(PrevPCReg, counter);
    counter = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PCReg, counter);
   	machine->WriteRegister(NextPCReg, counter + 4);
}

// Input: Khong gian dia chi User(int) - gioi han cua buffer(int)
// Output: Bo nho dem Buffer(char*)
// Chuc nang: Sao chep vung nho User sang vung nho System
char* User2System(int virtAddr, int limit)
{
	int i; //chi so index
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit + 1]; //can cho chuoi terminal
	if (kernelBuf == NULL)
		return kernelBuf;
		
	memset(kernelBuf, 0, limit + 1);
	
	for (i = 0; i < limit; i++)
	{
		machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

// Input: Khong gian vung nho User(int) - gioi han cua buffer(int) - bo nho dem buffer(char*)
// Output: So byte da sao chep(int)
// Chuc nang: Sao chep vung nho System sang vung nho User
int System2User(int virtAddr, int len, char* buffer)
{
	if (len < 0) return -1;
	if (len == 0)return len;
	int i = 0;
	int oneChar = 0;
	do{
		oneChar = (int)buffer[i];
		machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}


void ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);
	switch (which) {
	case NoException:
		return;
	case SyscallException:
	{
		switch (type) {
		case SC_Halt:
			DEBUG('a', "\nShutdown, initiated by user program. ");
			printf("\nShutdown, initiated by user program. ");
			interrupt->Halt();
			return;
		case SC_ReadInt:
		{
			handle_SC_ReadInt();
			break;
		}
		case SC_PrintInt:
		{
			handle_SC_PrintInt();
			break;
		}
		case SC_ReadChar:
		{
			handle_SC_ReadChar();
			break;
		}
		case SC_PrintChar:
		{
			handle_SC_PrintChar();
			break;
		}
		case SC_ReadString:
		{
			handle_SC_ReadString();
			break;
		}
		case SC_PrintString:
		{
			handle_SC_PrintString();
			break;
		}
		}
		break;
	}
	case PageFaultException: // Có lỗi trang
	{
		DEBUG('a', "[Page Fault Exception] No valid translation found\n");
		printf("[Page Fault Exception] No valid translation found\n");
		interrupt->Halt();
		break;
	}
	case ReadOnlyException: // Có lỗi chỉ đọc
	{
		DEBUG('a', "[Read Only Exception] Write attempted to page marked read-only\n");
		printf("[Page Fault Exception] Write attempted to page marked read-only\n");
		interrupt->Halt();
		break;
	}
	case BusErrorException: // Có lỗi bus
	{
		DEBUG('a', "[Bus Error Exception] Translation resulted in an invalid physical address\n");
		printf("[Page Fault Exception] Translation resulted in an invalid physical address\n");
		interrupt->Halt();
		break;
	}
	case AddressErrorException: // Có lỗi địa chỉ
	{
		DEBUG('a', "[Address Error Exception] Unaligned reference or one that was beyond the end of the address space\n");
		printf("[Page Fault Exception] Unaligned reference or one that was beyond the end of the address space\n");
		interrupt->Halt();
		break;
	}
	case OverflowException: // Có lỗi tràn số
	{
		DEBUG('a', "[Overflow Exception] Integer overflow in add or sub\n");
		printf("[Page Fault Exception] Integer overflow in add or sub\n");
		interrupt->Halt();
		break;
	}
	case IllegalInstrException: // Có lỗi lệnh bất hợp pháp
	{
		DEBUG('a', "[Illegal Instr Exception] Unimplemented or reserved instr\n");
		printf("[Page Fault Exception] Unimplemented or reserved instr\n");
		interrupt->Halt();
		break;
	}
	case NumExceptionTypes: // Có lỗi loại không xác định
	{
		DEBUG('a', "[Num Exception Types] Unknown exception type\n");
		printf("[Page Fault Exception] Unknown exception type\n");
		interrupt->Halt();
		break;
	}
	}
}
