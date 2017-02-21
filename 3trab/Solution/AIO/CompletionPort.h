#pragma once


// é usado o nível de concorrencia por omissao
#define MAX_CONCURRENCY 0

// threads associadas à completion port
#define MAX_THREADS	16

struct _aio_dev;

// callback signature
typedef VOID(*AIO_CALLBACK)(struct _aio_dev *dev,  INT transferedBytes, LPVOID ctx);

// oper context and callback
typedef struct _aio_oper {
	AIO_CALLBACK callback;
	LPVOID ctx;
} AIO_OPER, *PAIO_OPER;

// Represents an async device (file or socket).
// The model only supports sequential access and
// at most one operation in course per device.
typedef struct _aio_dev {
	HANDLE dev;
	OVERLAPPED ioStatus;
	AIO_OPER oper;
	BOOL isSocket;
} AIO_DEV, *PAIO_DEV;


/* completion port public functions */
BOOL CompletionPortCreate(int maxConcurrency);
BOOL CompletionPortAssociateHandle(HANDLE devHandle, LPVOID completionKey);
VOID CompletionPortClose();

/* low level async I/O wrappers */
BOOL AsyncRead(HANDLE sd, LPVOID buffer, DWORD length, OVERLAPPED *ovr);
BOOL AsyncWrite(HANDLE sd, LPVOID buffer, DWORD length, OVERLAPPED *ovr);


/* aio devices creation and destruction */
BOOL InitAioDev(PAIO_DEV aio, HANDLE dev, BOOL isSocket);
BOOL InitAioSocket(PAIO_DEV aio, SOCKET s);
VOID CloseAsync(PAIO_DEV ad);

/* aiodev management */
VOID SetAioOper(PAIO_DEV dev, AIO_CALLBACK cb, LPVOID uCtx);
VOID ProcessTransfer(PAIO_DEV dev, DWORD transferedBytes);


// async operations
BOOL ReadAsync(PAIO_DEV dev, LPVOID buffer, int bsize, AIO_CALLBACK cb, LPVOID ctx);
BOOL WriteAsync(PAIO_DEV dev, LPVOID buffer, int bsize, AIO_CALLBACK cb, LPVOID ctx); 

