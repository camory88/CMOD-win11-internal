#include "memory.hpp"

int main()
{
    const wchar_t* processName = L"r5apex.exe"; //game name
    DWORD processId = GetProcessIdByName(processName);
    const char* dllPath = "C:\\Users\\Camle\\source\\repos\\CMOD-win11 internal\\x64\\Release\\CMOD Internal.dll"; // Replace with the path to your DLL

    if (CHollowInject(processId, dllPath))
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