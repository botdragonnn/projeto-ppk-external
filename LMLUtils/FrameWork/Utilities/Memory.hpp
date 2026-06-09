#pragma once

// Windows deve vir antes de winternl para garantir que NTSTATUS, HANDLE, etc. estejam definidos
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <winternl.h>
#include <TlHelp32.h>

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

// Ponteiros de função para NtReadVirtualMemory / NtWriteVirtualMemory via GetProcAddress
// (evita dependência de ntdll.lib que pode não estar disponível em todos os ambientes)
typedef NTSTATUS(NTAPI* pNtWriteVirtualMemory)(
    HANDLE ProcessHandle, PVOID BaseAddress,
    PVOID Buffer, SIZE_T BufferSize, PSIZE_T NumberOfBytesWritten);

typedef NTSTATUS(NTAPI* pNtReadVirtualMemory)(
    HANDLE ProcessHandle, PVOID BaseAddress,
    PVOID Buffer, SIZE_T BufferSize, PSIZE_T NumberOfBytesRead);

namespace FrameWork
{
    namespace Memory
    {
        // ── Handles globais ──────────────────────────────────────────────────
        inline HANDLE AttachedProcessHandle = nullptr;
        inline DWORD  AttachedProcessPid    = 0;

        // ── Processo ─────────────────────────────────────────────────────────
        void  AttachProces(DWORD Pid);
        void  DetachProcess();
        bool  IsProcessRunning(DWORD Pid);

        // ── Janela / Módulo ───────────────────────────────────────────────────
        HWND     GetWindowHandleByPID(DWORD Pid);
        HWND     GetWindowHandleByName(std::wstring WindowName);
        DWORD    GetProcessPidByName(std::wstring ProcessName);
        uint64_t GetModuleBaseByName(DWORD Pid, std::wstring ModuleName);
        uint64_t GetModuleSizeByName(DWORD Pid, std::wstring ModuleName);

        // ── Leitura / Escrita ─────────────────────────────────────────────────
        void ReadProcessMemoryImpl(uint64_t ReadAddress, LPVOID Read, SIZE_T Size);
        bool WriteProcessMemoryImpl(uint64_t WriteAddress, LPVOID Write, SIZE_T Size);

        std::string          ReadProcessMemoryString(uint64_t ReadAddress, SIZE_T StringSize = 256);
        std::vector<uint8_t> ReadBytes(uintptr_t Addr, size_t Size);
        bool                 WriteBytes(uintptr_t address, const std::vector<uint8_t>& bytes);
        bool                 ZwWriteVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress,
                                                  PVOID Buffer, SIZE_T BufferSize,
                                                  PSIZE_T NumberOfBytesWritten);

        // ── Utilitários ───────────────────────────────────────────────────────
        bool      PatchFunc(uintptr_t Addr, int NopCount);
        bool      HookJump(uintptr_t HookAddress, uintptr_t JmpToAddress);
        uintptr_t CreateCodeCave(size_t Size);
        bool      FreeCave(uintptr_t CaveAddress);
		uint64_t  FindSignature(std::vector<uint8_t> Signature,
		                        uintptr_t ModuleBase, uintptr_t ModuleBaseSize);
		uint64_t  FindSignatureInAllModules(std::vector<uint8_t> Signature);
		uint64_t  FindSignatureInAllMemory(std::vector<uint8_t> Signature);

        // ── Templates ─────────────────────────────────────────────────────────
        template <typename T, typename B>
        T ReadMemory(B ReadAddress)
        {
            T Read{};
            ReadProcessMemoryImpl((uint64_t)ReadAddress, &Read, sizeof(T));
            return Read;
        }

        template <typename T, typename B>
        bool WriteMemory(B WriteAddress, T Value)
        {
            return WriteProcessMemoryImpl((uint64_t)WriteAddress, &Value, sizeof(T));
        }
    }
}
