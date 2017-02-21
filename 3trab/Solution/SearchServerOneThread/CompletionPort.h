#pragma once

#include <Windows.h>

// used default concurrency level
#define MAX_CONCURRENCY 0

// maximum number of threads associated with IOCP
#define MAX_THREADS	16

/* completion port public functions */
BOOL CompletionPortCreate(int maxConcurrency);
BOOL CompletionPortAssociateHandle(HANDLE devHandle, LPVOID completionKey);
VOID CompletionPortClose();

/* operation management */
VOID SearchSessionProcessCurrentFile(LPOVERLAPPED ovr, DWORD transferedBytes);
VOID SearchSessionProcessNextFile(LPOVERLAPPED ovr);
