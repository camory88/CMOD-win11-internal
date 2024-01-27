#include "includes.h"

DWORD GetProcessIdByName(const wchar_t* processName)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, processName) == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return 0;
}

bool CManualMap(const wchar_t* targetProcessName, const char* dllPath) {
    // Find the process ID of the target process
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD targetProcessID = 0;

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (_wcsicmp(pe32.szExeFile, targetProcessName) == 0) {
                targetProcessID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);

    if (targetProcessID == 0) {
        return FALSE; // Target process not found
    }

    // Open the target process
    HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, targetProcessID);

    if (hProcess == NULL) {
        return FALSE; // Failed to open target process
    }

    // Read the image headers of the DLL to be injected
    HANDLE hFile = CreateFileA(dllPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        CloseHandle(hProcess);
        return false; // Failed to open DLL file
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    LPVOID fileBuffer = VirtualAlloc(NULL, fileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (fileBuffer == NULL) {
        CloseHandle(hFile);
        CloseHandle(hProcess);
        return false; // Failed to allocate memory for DLL file
    }

    DWORD bytesRead;
    ReadFile(hFile, fileBuffer, fileSize, &bytesRead, NULL);
    CloseHandle(hFile);

    // Allocate memory in the target process for the DLL path
    LPVOID remotePath = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (remotePath == NULL) {
        VirtualFree(fileBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false; // Failed to allocate memory in the target process
    }

    // Write the DLL path into the target process
    WriteProcessMemory(hProcess, remotePath, dllPath, strlen(dllPath) + 1, NULL);

    // Get the address of LoadLibraryA function in kernel32.dll
    LPVOID loadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");

    if (loadLibraryAddr == NULL) {
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        VirtualFree(fileBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false; // LoadLibraryA not found
    }

    // Create a remote thread in the target process to execute LoadLibraryA with the DLL path
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, remotePath, 0, NULL);

    if (hThread == NULL) {
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        VirtualFree(fileBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false; // Failed to create remote thread
    }

    // Wait for the remote thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Clean up
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
    VirtualFree(fileBuffer, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return true;
}

bool CLoadLibrary(DWORD processId, const char* dllPath)
{
    // Open the target process for injection (e.g., Notepad.exe)
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId); // Replace 'processId' with the target process ID

    if (hProcess == NULL) {
        MessageBoxA(0, "Failed to open target process.", "CMOD-Injector ERROR", 0);
        return FALSE;
    }

    // Allocate memory for the DLL path in the target process
    LPVOID remoteString = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);

    if (remoteString == NULL) {
        MessageBoxA(0, "Failed to allocate memory in the target process.", "CMOD-Injector ERROR", 0);
        CloseHandle(hProcess);
        return FALSE;
    }

    // Write the DLL path to the allocated memory
    if (!WriteProcessMemory(hProcess, remoteString, dllPath, strlen(dllPath) + 1, NULL))
    {
        MessageBoxA(0, "Failed to write DLL path to target process.", "CMOD-Injector ERROR", 0);
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    // Get the address of LoadLibraryA in the kernel32.dll module
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    FARPROC loadLibrary = GetProcAddress(kernel32, "LoadLibraryA");

    if (loadLibrary == NULL) {
        MessageBoxA(0, "Failed to get address of LoadLibraryA.", "CMOD-Injector ERROR", 0);
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    // Create a remote thread to execute LoadLibraryA with the DLL path as an argument
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibrary, remoteString, 0, NULL);

    if (hThread == NULL) {
        MessageBoxA(0, "Failed to create remote thread.", "CMOD-Injector ERROR", 0);
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    // Wait for the remote thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Clean up resources
    VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
}


bool CHollowInject(const wchar_t* targetProcessName, const char* dllPath) {
    // Find the process ID of the target process
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD targetProcessID = 0;

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (_wcsicmp(pe32.szExeFile, targetProcessName) == 0) {
                targetProcessID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);

    if (targetProcessID == 0) {
        return false; // Target process not found
    }

    // Open the target process
    HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, targetProcessID);

    if (hProcess == NULL) {
        return false; // Failed to open target process
    }

    // Allocate memory for the DLL path in the target process
    LPVOID remoteMemory = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (remoteMemory == NULL) {
        CloseHandle(hProcess);
        return false; // Failed to allocate memory
    }

    // Write the DLL path to the target process
    WriteProcessMemory(hProcess, remoteMemory, dllPath, strlen(dllPath) + 1, NULL);

    // Get the address of the LoadLibrary function
    LPVOID loadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

    // Create a remote thread in the target process to load the DLL
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, remoteMemory, 0, NULL);

    if (hThread == NULL) {
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false; // Failed to create remote thread
    }

    // Wait for the remote thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Clean up
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return true;
}