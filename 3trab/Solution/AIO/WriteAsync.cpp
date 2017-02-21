#include <stdio.h>
#include <tchar.h>
#include <WinSock2.h>
#include <Windows.h>

#include <assert.h>
#include "CompletionPort.h"

BOOL WriteAsync(PAIO_DEV dev, LPVOID buffer, int bsize, AIO_CALLBACK cb, LPVOID ctx) {
	SetAioOper(dev, cb, ctx);
	return AsyncWrite(dev->dev, buffer, bsize, &dev->ioStatus);
}

