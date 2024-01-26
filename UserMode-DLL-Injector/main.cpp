#include "memory.h"
bool inject(DWORD processId, const char* dllPath)
{
    // Open the target process for injection (e.g., Notepad.exe)
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId); // Replace 'processId' with the target process ID

    if (hProcess == NULL) {
        MessageBoxA(0, "Failed to open target process.", "CMOD-Injector ERROR", 0);
        return 1;
    }

    // Allocate memory for the DLL path in the target process
    LPVOID remoteString = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);

    if (remoteString == NULL) {
        MessageBoxA(0, "Failed to allocate memory in the target process.", "CMOD-Injector ERROR", 0);
        CloseHandle(hProcess);
        return 1;
    }

    // Write the DLL path to the allocated memory
    if (!WriteProcessMemory(hProcess, remoteString, dllPath, strlen(dllPath) + 1, NULL))
    {
        MessageBoxA(0, "Failed to write DLL path to target process.", "CMOD-Injector ERROR", 0);
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    // Get the address of LoadLibraryA in the kernel32.dll module
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    FARPROC loadLibrary = GetProcAddress(kernel32, "LoadLibraryA");

    if (loadLibrary == NULL) {
        MessageBoxA(0, "Failed to get address of LoadLibraryA.", "CMOD-Injector ERROR", 0);
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    // Create a remote thread to execute LoadLibraryA with the DLL path as an argument
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibrary, remoteString, 0, NULL);

    if (hThread == NULL) {
        MessageBoxA(0, "Failed to create remote thread.", "CMOD-Injector ERROR", 0);
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    // Wait for the remote thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Clean up resources
    VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
}

int main()
{
    const std::wstring processName = L"Notepad.exe"; //game name
    DWORD processId = GetProcessIdByName(processName.c_str());
    const char* dllPath = "C:\\Users\\Camle\\source\\repos\\CMOD-win11 internal\\x64\\Release\\CMOD Internal.dll"; // Replace with the path to your DLL

    if (inject(processId, dllPath))
    {
        MessageBoxA(0, "DLL injected successfully!", "CMOD-Injector Message", 0);
        return 0;
    }
    else
    {
        MessageBoxA(0, "DLL not injected!", "CMOD-Injector ERROR", 0);
        return 0;
    }

    return 0;
}