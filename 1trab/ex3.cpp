
#include <stdio.h>
#include <windows.h>
#include <psapi.h>

MEMORYSTATUSEX GetMEMORYSTATUSEX() {
	MEMORYSTATUSEX mem;
	mem.dwLength = sizeof(mem);
	GlobalMemoryStatusEx(&mem);
	return mem;
}

PROCESS_MEMORY_COUNTERS GetPROCESS_MEMORY_COUNTERS(HANDLE processHandle) {
	PROCESS_MEMORY_COUNTERS proc;
	proc.cb = sizeof(proc);
	GetProcessMemoryInfo(processHandle, &proc , proc.cb);
	return proc;
}

PERFORMANCE_INFORMATION GetPERFORMANCE_INFORMATION() {
	PERFORMANCE_INFORMATION perfInfo;
	perfInfo.cb = sizeof(perfInfo);
	GetPerformanceInfo(&perfInfo, perfInfo.cb);
	return perfInfo;
}

int GetNumberOfPrivatePages(HANDLE processHandle) {
	PSAPI_WORKING_SET_INFORMATION wsi_dummy, *wsi;
	DWORD wsi_size;
	wsi_dummy.NumberOfEntries = 0;
	QueryWorkingSet(processHandle, (LPVOID)&wsi_dummy, sizeof(wsi));
	wsi_size = sizeof(PSAPI_WORKING_SET_INFORMATION) + (sizeof(PSAPI_WORKING_SET_BLOCK) * wsi_dummy.NumberOfEntries);
	wsi = (PSAPI_WORKING_SET_INFORMATION*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, wsi_size);
	QueryWorkingSet(processHandle, (LPVOID)wsi, wsi_size);
	int privatePagesCounter = 0;
	for (int i = 0; i < wsi->NumberOfEntries; i++)
		if (!wsi->WorkingSetInfo[i].Shared)
			privatePagesCounter++;
	return privatePagesCounter;
}

void PrintGlobalInformation(const MEMORYSTATUSEX& mem, const PERFORMANCE_INFORMATION& perfInfo) {
	printf("There are %lld total Bytes of physical memory. \n", mem.ullTotalPhys);
	printf("There are %lld occupied Bytes of physical memory. \n", (mem.ullTotalPhys - mem.ullAvailPhys));
	printf("There are %lld total Bytes of virtual memory. \n", mem.ullTotalPageFile);
	printf("There are %lld occupied Bytes of virtual memory. \n", (mem.ullTotalPageFile - mem.ullAvailPageFile));
	printf("There are %d total Bytes of paging file. \n", perfInfo.PageSize);
}

void PrintLocalProcessInformation(const MEMORYSTATUSEX& mem, const PROCESS_MEMORY_COUNTERS& memoryStatistics, const PERFORMANCE_INFORMATION& perfInfo, int number_of_private_pages) {
	printf("The available address space is %lld Bytes \n", mem.ullTotalVirtual);
	printf("The occupied address space is %lld Bytes \n", (mem.ullTotalVirtual - mem.ullAvailVirtual));
	printf("The number of (hard) Page Faults is %d Bytes. \n", memoryStatistics.PageFaultCount);
	printf("The size of the Working Set is %d Bytes. \n", memoryStatistics.WorkingSetSize);
	printf("The size of the Private Working Set is %d Bytes. \n", (number_of_private_pages*perfInfo.PageSize));
}

void PrintMemInfo(){
	HANDLE processHandle = GetCurrentProcess();
	
	MEMORYSTATUSEX currentMemoryState = GetMEMORYSTATUSEX();
	PROCESS_MEMORY_COUNTERS processMemoryStatistics = GetPROCESS_MEMORY_COUNTERS(processHandle);
	PERFORMANCE_INFORMATION performanceInfo = GetPERFORMANCE_INFORMATION();
	int numberOfPrivatePages = GetNumberOfPrivatePages(processHandle);
	
	printf("/* -------------- GLOBAL INFORMATION -------------------/*\n");
	PrintGlobalInformation(currentMemoryState, performanceInfo);
	printf("\n/* -------------- LOCAL PROCESS INFORMATION -------------------/*\n");
	PrintLocalProcessInformation(currentMemoryState, processMemoryStatistics, performanceInfo, numberOfPrivatePages);
}

int main(){
	PrintMemInfo();
	return 0;
}




