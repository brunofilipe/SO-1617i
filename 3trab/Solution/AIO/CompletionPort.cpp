#include <Windows.h>
#include <tchar.h>
#include <process.h>
#include <stdio.h>
#include <assert.h>

#include "CompletionPort.h"

static HANDLE completionPort;

static UINT WINAPI ProcessOpers(LPVOID arg);

// associa um conjunto de threads à IOCP
static VOID ThreadPoolCreate( ) {
	for (int i = 0; i < MAX_THREADS; ++i) {
		_beginthreadex(NULL, 0, ProcessOpers, (LPVOID)completionPort, 0, NULL);
	}
}

BOOL CompletionPortCreate(int maxConcurrency) {
	completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, maxConcurrency);
	if (completionPort == NULL) return FALSE;
	ThreadPoolCreate();
	return TRUE;
}

VOID CompletionPortClose() {
	CloseHandle(completionPort);
}


BOOL CompletionPortAssociateHandle(HANDLE devHandle, LPVOID completionKey) {
	HANDLE h = CreateIoCompletionPort(devHandle, completionPort, (ULONG_PTR) completionKey, 0);
	return h == completionPort;
}


// função executada pelas threads associadas à IOCP
static UINT WINAPI ProcessOpers(LPVOID arg) {
	HANDLE completionPort = (HANDLE)arg;
	DWORD transferedBytes;
	PAIO_DEV dev;
	OVERLAPPED *ovr;


	//printf("start worker!\n");
	while (TRUE) {
		if (!GetQueuedCompletionStatus(completionPort, &transferedBytes,
			(PULONG_PTR)&dev, &ovr, INFINITE) && GetLastError() != ERROR_HANDLE_EOF) {
			_tprintf(_T("Error %d getting activity packet!\n"), GetLastError());
		}
		
		ProcessTransfer(dev, transferedBytes);
		assert(dev->oper.callback != NULL);
		
		// call operation callback
		dev->oper.callback(dev, transferedBytes, dev->oper.ctx);
	 
	}
}

// low level async read
BOOL AsyncRead(HANDLE sd, LPVOID buffer, DWORD length, OVERLAPPED *ovr)
{
	if (!ReadFile(sd, buffer, length, NULL, ovr)  &&
		GetLastError() != ERROR_IO_PENDING) return FALSE;
	return TRUE;
}


// low level async write
BOOL AsyncWrite(HANDLE sd, LPVOID buffer, DWORD length, OVERLAPPED *ovr) {
	if (!WriteFile(sd, buffer, length,NULL, ovr)  &&
		GetLastError() != ERROR_IO_PENDING) return FALSE;
	return TRUE;

}

