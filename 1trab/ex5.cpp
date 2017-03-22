#include "stdio.h"
#include "Windows.h"
#define DLL __declspec( )

DLL void GetImportedFunctions();

void PrintDLL(PIMAGE_DOS_HEADER dos_header, PIMAGE_IMPORT_DESCRIPTOR dec) {
	PCHAR  ptr = (PCHAR)dos_header + dec->Name;
	printf("DLL name = %s\n", ptr);
}

void PrintDLLFuctions(PIMAGE_DOS_HEADER dos_header, IMAGE_THUNK_DATA* thunksmethods, IMAGE_THUNK_DATA* thunksaddress) {
	PIMAGE_IMPORT_BY_NAME imagemethods = (PIMAGE_IMPORT_BY_NAME)((BYTE*)dos_header + thunksmethods->u1.AddressOfData);
	PCHAR  methods = imagemethods->Name;	//nome dos metodos
	long address = thunksaddress->u1.AddressOfData;		//endereços dos metodos
	printf("	Function %s (%x) \n", methods, address);
}

void GetImportedFunctions() {
	HANDLE handle = GetModuleHandle(NULL);
	PIMAGE_DOS_HEADER  dos_header = (PIMAGE_DOS_HEADER)handle;
	PIMAGE_NT_HEADERS  ntHeaders = (PIMAGE_NT_HEADERS)(/*(PBYTE)*/dos_header + dos_header->e_lfanew);
	PIMAGE_OPTIONAL_HEADER  header = /*(PIMAGE_OPTIONAL_HEADER)*/&ntHeaders->OptionalHeader;
	PIMAGE_IMPORT_DESCRIPTOR  dec = (PIMAGE_IMPORT_DESCRIPTOR)((PBYTE)dos_header + header->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	for (; dec->Characteristics != 0; ++dec) {
		PrintDLL(dos_header, dec);
		IMAGE_THUNK_DATA * thunksmethods = (IMAGE_THUNK_DATA*)((PBYTE)dos_header + dec->OriginalFirstThunk);
		IMAGE_THUNK_DATA * thunksaddress = (IMAGE_THUNK_DATA*)((PBYTE)dos_header + dec->FirstThunk);
		for (; thunksmethods->u1.AddressOfData != NULL; ++thunksmethods, ++thunksaddress)
			PrintDLLFuctions(dos_header, thunksmethods, thunksaddress);
	}
}

int main(int argc, char const *argv[])
{
	GetImportedFunctions();
	return 0;
}