#include <stdio.h>
#include <tchar.h>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#include "CompletionPort.h"



// cria e inicia um novo async device  
BOOL InitAioDev(PAIO_DEV aio, HANDLE dev, BOOL isSocket) {
	aio->dev = dev;
	aio->isSocket = isSocket;
	ZeroMemory(&aio->ioStatus, sizeof(OVERLAPPED));
	return CompletionPortAssociateHandle(aio->dev, aio);
}

BOOL InitAioSocket(PAIO_DEV aio, SOCKET s) {
	return InitAioDev(aio, (HANDLE)s, TRUE);
}

VOID CloseAsync(PAIO_DEV ad) {
	if (ad->isSocket) {
		printf("End session!\n");
		shutdown((SOCKET)ad->dev, SD_BOTH);
		closesocket((SOCKET)ad->dev);
	}
	else {
		CloseHandle(ad->dev);
	}
}
 
VOID SetAioOper(PAIO_DEV dev, AIO_CALLBACK cb, LPVOID uCtx) {
	dev->oper.callback = cb;
	dev->oper.ctx = uCtx;
}


// adjust current position for seekable devices
VOID ProcessTransfer(PAIO_DEV dev, DWORD transferedBytes) {
	if (!dev->isSocket && transferedBytes > 0) {
		LARGE_INTEGER pos;
		LPOVERLAPPED ovr = &dev->ioStatus;
		pos.HighPart = ovr->OffsetHigh; pos.LowPart = ovr->Offset;
		pos.QuadPart += transferedBytes;
		ovr->OffsetHigh = pos.HighPart; ovr->Offset = pos.LowPart;
	}
}