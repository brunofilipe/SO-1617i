// Alive.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\Include\Uthread.h"




HANDLE tA, tB, tC,tD;

VOID ThreadA(UT_ARGUMENT Argument) {
	UCHAR Char = (UCHAR)Argument;
	printf("Is Alive ? %d \n", UtAlive(tA));
	printf("Thread %c start\n", Char);
	UtJoin(tB);
	printf("Thread %c end\n", Char);
}
VOID ThreadB(UT_ARGUMENT Argument) {
	UCHAR Char = (UCHAR)Argument;

	printf("Thread %c start\n", Char);
	printf("Thread %c end\n", Char);
}
VOID ThreadC(UT_ARGUMENT Argument) {
	UCHAR Char = (UCHAR)Argument;

	printf("Thread %c start\n", Char);
	UtJoin(tB);
	printf("Thread %c end\n", Char);
}

VOID ThreadD(UT_ARGUMENT Argument) {
	UCHAR Char = (UCHAR)Argument;
	ShowUtTick(tD, 10000);
}
/*testing UtAlive*/
VOID AliveTest() {
	printf("\n :: Test 1 - BEGIN :: \n\n");
	tA = UtCreate(ThreadA, (UT_ARGUMENT)'A');
	tB = UtCreate(ThreadB, (UT_ARGUMENT)'B');
	tC = UtCreate(ThreadC, (UT_ARGUMENT)'C');
	UtRun();
	printf("\n\n :: Test 1 - END :: \n");
}

VOID CountTest() {
	printf("\n :: Test 1 - BEGIN :: \n\n");
	tD = UtCreate(ThreadD, (UT_ARGUMENT)'D');
	UtRun();
	printf("\n\n :: Test 1 - END :: \n");
}

int main() {
	UtInit();
	CountTest();
	printf("Press any key to finish");
	getchar();
	UtEnd();
	return 0;
}

