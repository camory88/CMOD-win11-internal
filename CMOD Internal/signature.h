#pragma once
#include <cstdlib>
#include <Windows.h>
#include <vector>
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>

uintptr_t find_signature(const wchar_t* moduleName, const char* signature)
{
    MODULEENTRY32 moduleEntry = { sizeof(MODULEENTRY32) };
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());

    if (Module32First(snapshot, &moduleEntry))
    {
        do
        {
            if (wcscmp(moduleEntry.szModule, moduleName) == 0)
            {
                uintptr_t moduleBase = reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);
                uintptr_t moduleEnd = moduleBase + moduleEntry.modBaseSize;

                const char* signaturePtr = signature;
                uintptr_t address = 0;

                for (uintptr_t i = moduleBase; i < moduleEnd; ++i)
                {
                    if (*signaturePtr == '\0')
                        return address;

                    if (*signaturePtr == '?' || *reinterpret_cast<const BYTE*>(signaturePtr) == *reinterpret_cast<const BYTE*>(i))
                    {
                        if (address == 0)
                            address = i;

                        if (*(++signaturePtr) == '\0')
                            return address;
                    }
                    else
                    {
                        signaturePtr = signature;
                        address = 0;
                    }
                }
            }
        } while (Module32Next(snapshot, &moduleEntry));
    }

    CloseHandle(snapshot);
    return 0;
}

namespace util
{
    uintptr_t ida_signature(uintptr_t moduleAdress, const char* signature)
    {
        static auto patternToByte = [](const char* pattern)
            {
                auto       bytes = std::vector<int>{ };
                const auto start = const_cast<char*>(pattern);
                const auto end = const_cast<char*>(pattern) + strlen(pattern);

                for (auto current = start; current < end; ++current)
                {
                    if (*current == '?')
                    {
                        ++current;
                        if (*current == '?')
                            ++current;
                        bytes.push_back(-1);
                    }
                    else { bytes.push_back(strtoul(current, &current, 16)); }
                }
                return bytes;
            };

        const auto dosHeader = (PIMAGE_DOS_HEADER)moduleAdress;
        const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)moduleAdress + dosHeader->e_lfanew);

        const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto       patternBytes = patternToByte(signature);
        const auto scanBytes = reinterpret_cast<std::uint8_t*>(moduleAdress);

        const auto s = patternBytes.size();
        const auto d = patternBytes.data();

        for (auto i = 0ul; i < sizeOfImage - s; ++i)
        {
            bool found = true;
            for (auto j = 0ul; j < s; ++j)
            {
                if (scanBytes[i + j] != d[j] && d[j] != -1)
                {
                    found = false;
                    break;
                }
            }
            if (found) { return reinterpret_cast<uintptr_t>(&scanBytes[i]); }
        }
        return NULL;
    }
}