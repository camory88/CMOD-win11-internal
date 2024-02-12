#pragma once
#include <Windows.h>
#include <iostream>
#include "signature.h"
#include "skCrypter.h"

#define E skCrypt

static bool MainMenu = true;
static bool InfoMenu = true;

bool getKeyDown(int key)
{
	return (GetAsyncKeyState(key) & 0x8000) != 0;
}


void ListModules(DWORD processID) {
    HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
    MODULEENTRY32 me32;

    // Take a snapshot of all modules in the specified process.
    hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
    if (hModuleSnap == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateToolhelp32Snapshot failed." << std::endl;
        return;
    }

    // Set the size of the structure before using it.
    me32.dwSize = sizeof(MODULEENTRY32);

    // Retrieve information about the first module.
    if (!Module32First(hModuleSnap, &me32)) {
        std::cerr << "Module32First failed." << std::endl;
        CloseHandle(hModuleSnap);
        return;
    }


    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);


    // Now walk the module list of the process.
    do {
        std::wcout << "Module Name: " << me32.szModule << std::endl;
    } while (Module32Next(hModuleSnap, &me32));

    FreeConsole();
    CloseHandle(hModuleSnap);
}