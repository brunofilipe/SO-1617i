#include <Windows.h>
#include <process.h>
#include <stdio.h>
#include <assert.h>
#include "../SearchService/SearchService.h"

static PSearchService service;

/*-----------------------------------------------------------------------
This function allows the processing of a selected set of files in a directory
It uses the Windows functions for directory file iteration, namely
"FindFirstFile" and "FindNextFile"
*/

UINT _stdcall Read(LPVOID arg){
	PReader read = (PReader)arg;
	PMULTISEARCH ctx = read->ctx;
	PCHAR windowBuffer = (PCHAR)HeapAlloc(GetProcessHeap(), 0, read->tokenSize + 1);
	DWORD res ;
	DWORD bytesReaded ;
	CHAR c;
	PCHAR answer = read->answer;

	
	while(TRUE){
		int index = InterlockedIncrement(&read->idx);
		if (index >= read->length) break;
		PWIN32_FIND_DATA fileData = read->vals + index;
		_tprintf(_T("ClientID: %d Search on file: %s\n"), ctx->id, fileData->cFileName);
		//FILE_SHARED_READ - permite trabalho Assincrono, valor a 0 não permite
		HANDLE hFile = CreateFile(fileData->cFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		_tprintf(_T("%s\n"), fileData->cFileName);
		assert(hFile != INVALID_HANDLE_VALUE);

		// clear windowBuffer
		memset(windowBuffer, 0, read->tokenSize + 1);

		res = ReadFile(hFile, &c, 1, &bytesReaded, NULL);
		while (res && bytesReaded == 1) {

			// slide window to accommodate new char
			memmove_s(windowBuffer,read->tokenSize, windowBuffer + 1, read->tokenSize - 1);
			windowBuffer[read->tokenSize - 1] = c;

			// test accumulated bytes with token
			if (memcmp(windowBuffer, read->entry->value, read->tokenSize) == 0) {

				// append filename to answer and go to next file
				strcat_s(&*answer, MAX_CHARS, fileData->cFileName);
				strcat_s(&*answer, MAX_CHARS, "\n");
				break;
			}
			res = ReadFile(hFile, &c, 1, &bytesReaded, NULL);
		}
		CloseHandle(hFile);
	}
	HeapFree(GetProcessHeap(), 0, windowBuffer);
	return 0;
}

int CountFiles(PCHAR path){
	int counter = 0;
	HANDLE iterator;
	WIN32_FIND_DATA fileData;
	TCHAR buffer[MAX_PATH];
	_stprintf_s(buffer, _T("%s\\%s"), path, _T("*.*"));
	if ((iterator = FindFirstFile(buffer, &fileData)) == INVALID_HANDLE_VALUE)
		return counter;
	do {

		if (fileData.dwFileAttributes == FILE_ATTRIBUTE_ARCHIVE) {
			++counter;
		}
	} while (FindNextFile(iterator, &fileData));
	return counter;
}


UINT _stdcall processEntry(LPVOID arg) {
	PMULTISEARCH ct = (PMULTISEARCH)arg;
	PEntry entry = ct->entry;
	PCHAR path = ct->path;
	HANDLE iterator;
	WIN32_FIND_DATA fileData;
	TCHAR buffer[MAX_PATH];		// auxiliary buffer
	DWORD tokenSize;
	Sleep(1000);
	DWORD ncpu = ct->nCPU;
	_tprintf(_T("Token to search: %s\n"), entry->value);

	// the buffer is needed to define a match string that guarantees 
	// a priori selection for all files
	_stprintf_s(buffer, _T("%s\\%s"), path, _T("*.*"));

	// start iteration
	if ((iterator = FindFirstFile(buffer, &fileData)) == INVALID_HANDLE_VALUE)
		goto error;
	

	// alloc buffer to hold bytes readed from file stream
	tokenSize = strlen(entry->value);
	//PCHAR windowBuffer = (PCHAR)HeapAlloc(GetProcessHeap(), 0, tokenSize + 1);
	// set auxiliary vars
	PCHAR answer;
	PSharedBlock pSharedBlock = (PSharedBlock)service->sharedMem;
	answer = pSharedBlock->answers[entry->answIdx];
	memset(answer, 0, MAX_CHARS);
	
	int numberofFiles = CountFiles(path);
	PWIN32_FIND_DATA vals = (PWIN32_FIND_DATA)malloc(sizeof(WIN32_FIND_DATA)*numberofFiles);
	//Sleep(9000);
	// process only file entries
	int idxVal = 0;
	do {
		
		if (fileData.dwFileAttributes == FILE_ATTRIBUTE_ARCHIVE) {
			vals[idxVal++] = fileData;
		}
	} while (FindNextFile(iterator, &fileData));

	HANDLE*threads = (HANDLE*)malloc(sizeof(HANDLE)*ct->nCPU);
	PReader reader = (PReader)malloc(sizeof(Reader));
	reader->answer = answer;
	//reader->windowBuffer = windowBuffer;
	reader->ctx = ct;
	reader->entry = entry;
	reader->vals = vals;
	reader->length = idxVal;
	reader->tokenSize = tokenSize;
	reader->idx = -1; //de modo a que o primeiro incremento retorne 0
	for (DWORD i = 0; i<ncpu; ++i) {
		threads[i] = (HANDLE)_beginthreadex(NULL, 0, Read,reader , 0, NULL);
	}
	WaitForMultipleObjects(ncpu, threads, TRUE, INFINITE);
	// sinalize client and finish answer
	SetEvent(entry->answReadyEvt);

	CloseHandle(entry->answReadyEvt);
	FindClose(iterator);
	
	
	free(ct->entry);
	free(ct);
	for (DWORD idx= 0; idx < ncpu; ++idx){
		CloseHandle(threads[idx]);
	}
	free(threads);
	
error:  ;
	return 0;

}



DWORD GetNumCPUs() {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

INT main(DWORD argc, PCHAR argv[]) {
	
	PCHAR name;
	PCHAR path;
	DWORD res;
	CHAR pathname[MAX_CHARS*4];
	if (argc < 3) {
		printf("Use > %s <service_name> <repository pathname>\n", argv[0]);
		name = "Service1";
		res = GetCurrentDirectory(MAX_CHARS, pathname);
		assert(res > 0);
		path = pathname;
		printf("Using > %s %s \"%s\"\n", argv[0], name, path);
	}
	else {
		name = argv[1];
		path = argv[2];
	}
	printf("Server app: Create service with name = %s. Repository name = %s\n", name, path);
	service = SearchCreate(name); assert(service != NULL);
	Entry entry;
	int id = -1;
	DWORD nCpu = GetNumCPUs();
	HANDLE sem = CreateSemaphore(NULL, nCpu, nCpu, NULL);
	int i = 0;
	while (1) {
		PMULTISEARCH ctx =(PMULTISEARCH) malloc(sizeof(MULTISEARCH));
		++id;
		res = SearchGet(service, &entry);
		ctx->id = id;
		ctx->path = path;
		ctx->nCPU = nCpu;
		ctx->sem = sem;
		PEntry dummy = (PEntry)malloc(sizeof(Entry));
		memcpy(dummy,&entry,sizeof(Entry));
		ctx->entry = dummy;
		if (res == FALSE) break;
		_beginthreadex(NULL, 0, processEntry, ctx, 0, NULL); //alinea c 
	}
	printf("Server app: Close service name = %s and exit\n", name);
	SearchClose(service);

	return 0;
}




