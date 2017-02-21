#include <windows.h>
#include <stdio.h>

#define ARRAY_SIZE 256
DWORD array[ARRAY_SIZE];

DWORD WINAPI MyThread() {
	DWORD start_time, end_time;
	DWORD array_size = ARRAY_SIZE;
	DWORD sum = 0;
	start_time = GetTickCount();
	for (DWORD i = 0; i < array_size; i++) {
		sum += array[i];
	}
	return sum;
}

DWORD WINAPI MyThread1() {
	DWORD start_time, end_time;
	DWORD array_size = ARRAY_SIZE;
	DWORD sum = 0;
	start_time = GetTickCount();
	for (DWORD i = 0; i < array_size; i++) {
		sum *= array[i];
	}
	return sum;
}

int main(){
	DWORD dwThreadId;
	HANDLE handle1 = CreateThread(NULL,0,MyThread,NULL,0,&dwThreadId );
	HANDLE handle2 = CreateThread(NULL,0,MyThread1,NULL,0,&dwThreadId);
	int n = 0;
	DWORD initial = GetTickCount();
	while(n < 100){
		SwitchToThread();
		++n;
	}
	DWORD final = GetTickCount();
	DWORD res = final - initial;
	printf("************** threads do mesmo processo\n");
	printf("%ld\n", res);
	CloseHandle(handle1);
	CloseHandle(handle2);


/*****************************************************************************************************/
	HANDLE handle3 = CreateThread(NULL,0,MyThread,NULL,0,&dwThreadId );
	PROCESS_INFORMATION proc_inf;
	STARTUPINFO startup;
	CreateProcess(NULL,NULL,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,&startup,&proc_inf);
	HANDLE handle4 = CreateRemoteThread(proc_inf.hProcess,NULL,0,LPTHREAD_START_ROUTINE,MyThread1,STACK_SIZE_PARAM_IS_A_RESERVATION,NULL);
	int y = 0;
	DWORD initialone = GetTickCount();
	while(y < 100){
		SwitchToThread();
		++y;
	}
	DWORD finalone = GetTickCount();
	DWORD resone = finalone - initialone;
	printf("************** threads de processos diferentes\n");
	printf("%ld\n", resone);
	return 0;

}