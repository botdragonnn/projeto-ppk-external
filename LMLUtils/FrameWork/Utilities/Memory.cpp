#include "Memory.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Ponteiros Nt resolvidos uma única vez na inicialização
// ─────────────────────────────────────────────────────────────────────────────
static pNtReadVirtualMemory  s_NtRead  = nullptr;
static pNtWriteVirtualMemory s_NtWrite = nullptr;

static void EnsureNtPointers()
{
    if (s_NtRead && s_NtWrite) return;
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll) return;
    s_NtRead  = (pNtReadVirtualMemory) GetProcAddress(ntdll, "NtReadVirtualMemory");
    s_NtWrite = (pNtWriteVirtualMemory)GetProcAddress(ntdll, "NtWriteVirtualMemory");
}

// ─────────────────────────────────────────────────────────────────────────────
// EnumWindows helper
// ─────────────────────────────────────────────────────────────────────────────
struct TGetWindowHandleData
{
    DWORD        Pid;
    std::wstring WindowName;
    HWND         hWnd;
};

static BOOL CALLBACK EnumWindowsCallback(HWND Handle, LPARAM lParam)
{
    TGetWindowHandleData& Data = *(TGetWindowHandleData*)lParam;

    if (Data.Pid == 0)
    {
        int Length = GetWindowTextLengthW(Handle);
        if (Length == 0) return TRUE;

        std::wstring Buffer(Length + 1, L'\0');
        GetWindowTextW(Handle, &Buffer[0], Length + 1);

        if (Data.WindowName != Buffer) return TRUE;
        Data.hWnd = Handle;
        return FALSE;
    }
    else
    {
        DWORD Pid = 0;
        GetWindowThreadProcessId(Handle, &Pid);
        if (Data.Pid != Pid) return TRUE;
        Data.hWnd = Handle;
        return FALSE;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Variáveis globais de estado (definidas aqui, declaradas inline no .hpp)
// ─────────────────────────────────────────────────────────────────────────────
// (inline no .hpp já gera a definição em C++17 — não redeclarar aqui)

namespace FrameWork
{
    // ── Janela / Processo ─────────────────────────────────────────────────────

    HWND Memory::GetWindowHandleByPID(DWORD Pid)
    {
        TGetWindowHandleData d{ Pid, L"", nullptr };
        EnumWindows(EnumWindowsCallback, (LPARAM)&d);
        return d.hWnd;
    }

    HWND Memory::GetWindowHandleByName(std::wstring WindowName)
    {
        TGetWindowHandleData d{ 0, std::move(WindowName), nullptr };
        EnumWindows(EnumWindowsCallback, (LPARAM)&d);
        return d.hWnd;
    }

    DWORD Memory::GetProcessPidByName(std::wstring ProcessName)
    {
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (!hSnap || hSnap == INVALID_HANDLE_VALUE) return 0;

        PROCESSENTRY32W pe{ sizeof(pe) };
        DWORD pid = 0;
        if (Process32FirstW(hSnap, &pe))
        {
            do {
                if (_wcsicmp(pe.szExeFile, ProcessName.c_str()) == 0)
                { pid = pe.th32ProcessID; break; }
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);
        return pid;
    }

    uint64_t Memory::GetModuleBaseByName(DWORD Pid, std::wstring ModuleName)
    {
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, Pid);
        if (!hSnap || hSnap == INVALID_HANDLE_VALUE) return 0;

        MODULEENTRY32W me{ sizeof(me) };
        uint64_t base = 0;
        if (Module32FirstW(hSnap, &me))
        {
            do {
                if (_wcsicmp(me.szModule, ModuleName.c_str()) == 0)
                { base = (uint64_t)me.modBaseAddr; break; }
            } while (Module32NextW(hSnap, &me));
        }
        CloseHandle(hSnap);
        return base;
    }

    uint64_t Memory::GetModuleSizeByName(DWORD Pid, std::wstring ModuleName)
    {
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, Pid);
        if (!hSnap || hSnap == INVALID_HANDLE_VALUE) return 0;

        MODULEENTRY32W me{ sizeof(me) };
        uint64_t size = 0;
        if (Module32FirstW(hSnap, &me))
        {
            do {
                if (_wcsicmp(me.szModule, ModuleName.c_str()) == 0)
                { size = (uint64_t)me.modBaseSize; break; }
            } while (Module32NextW(hSnap, &me));
        }
        CloseHandle(hSnap);
        return size;
    }

    bool Memory::IsProcessRunning(DWORD Pid)
    {
        if (!Pid) return false;
        HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, Pid);
        if (!h || h == INVALID_HANDLE_VALUE) return false;
        DWORD code = 0;
        bool ok = GetExitCodeProcess(h, &code) && code == STILL_ACTIVE;
        CloseHandle(h);
        return ok;
    }

    // ── Attach / Detach ───────────────────────────────────────────────────────

    void Memory::AttachProces(DWORD Pid)
    {
        EnsureNtPointers();

        // Tenta acesso completo → mínimo necessário → só leitura
        static const DWORD kAccessLevels[] = {
            PROCESS_ALL_ACCESS,
            PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION |
                PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION,
            PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
        };

        Memory::AttachedProcessHandle = nullptr;
        for (DWORD access : kAccessLevels)
        {
            HANDLE h = OpenProcess(access, FALSE, Pid);
            if (h && h != INVALID_HANDLE_VALUE)
            { Memory::AttachedProcessHandle = h; break; }
        }

        Memory::AttachedProcessPid = Memory::AttachedProcessHandle ? Pid : 0;
    }

    void Memory::DetachProcess()
    {
        if (Memory::AttachedProcessHandle)
            CloseHandle(Memory::AttachedProcessHandle);
        Memory::AttachedProcessHandle = nullptr;
        Memory::AttachedProcessPid    = 0;
    }

    // ── Leitura / Escrita — hot path ──────────────────────────────────────────

    void Memory::ReadProcessMemoryImpl(uint64_t ReadAddress, LPVOID Read, SIZE_T Size)
    {
        if (!Memory::AttachedProcessHandle) return;

        // Usa NtReadVirtualMemory diretamente — evita overhead do SafeCall
        if (s_NtRead)
        {
            s_NtRead(Memory::AttachedProcessHandle,
                     (PVOID)ReadAddress, Read, Size, nullptr);
            return;
        }
        // Fallback
        ReadProcessMemory(Memory::AttachedProcessHandle,
                          (LPCVOID)ReadAddress, Read, Size, nullptr);
    }

    bool Memory::WriteProcessMemoryImpl(uint64_t WriteAddress, LPVOID Value, SIZE_T Size)
    {
        if (!Memory::AttachedProcessHandle) return false;

        if (s_NtWrite)
            return s_NtWrite(Memory::AttachedProcessHandle,
                             (PVOID)WriteAddress, Value, Size, nullptr) == 0;

        return WriteProcessMemory(Memory::AttachedProcessHandle,
                                  (LPVOID)WriteAddress, Value, Size, nullptr) != 0;
    }

    bool Memory::ZwWriteVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress,
                                       PVOID Buffer, SIZE_T BufferSize,
                                       PSIZE_T NumberOfBytesWritten)
    {
        EnsureNtPointers();
        if (!s_NtWrite) return false;
        return s_NtWrite(ProcessHandle, BaseAddress, Buffer,
                         BufferSize, NumberOfBytesWritten) == 0;
    }

    // ── String — lê tudo de uma vez em vez de byte a byte ────────────────────

    std::string Memory::ReadProcessMemoryString(uint64_t ReadAddress, SIZE_T StringSize)
    {
        const SIZE_T kMax = 256;
        SIZE_T sz = (StringSize < kMax) ? StringSize : kMax;

        char buf[256]{};
        SIZE_T read = 0;

        // Uma única syscall em vez de 256 chamadas individuais
        if (s_NtRead)
            s_NtRead(Memory::AttachedProcessHandle,
                     (PVOID)ReadAddress, buf, sz, &read);
        else
            ReadProcessMemory(Memory::AttachedProcessHandle,
                              (LPCVOID)ReadAddress, buf, sz, &read);

        buf[read < kMax ? read : kMax - 1] = '\0';
        return std::string(buf);
    }

    // ── Bytes ─────────────────────────────────────────────────────────────────

    std::vector<uint8_t> Memory::ReadBytes(uintptr_t Addr, size_t Size)
    {
        std::vector<uint8_t> buf(Size, 0);
        SIZE_T read = 0;
        ReadProcessMemoryImpl(Addr, buf.data(), Size);
        return buf;
    }

    bool Memory::WriteBytes(uintptr_t address, const std::vector<uint8_t>& bytes)
    {
        return WriteProcessMemoryImpl(address,
                                      const_cast<uint8_t*>(bytes.data()),
                                      bytes.size());
    }

    bool Memory::PatchFunc(uintptr_t Addr, int NopCount)
    {
        if (!NopCount) return false;
        // Stack-allocated para patches pequenos — sem heap alloc
        if (NopCount <= 64)
        {
            uint8_t nops[64];
            memset(nops, 0x90, NopCount);
            return WriteProcessMemoryImpl(Addr, nops, NopCount);
        }
        std::vector<uint8_t> v(NopCount, 0x90);
        return WriteBytes(Addr, v);
    }

    bool Memory::HookJump(uintptr_t HookAddress, uintptr_t JmpToAddress)
    {
        // 14-byte absolute jmp: FF 25 00 00 00 00 [8-byte addr]
        uint8_t patch[14]{};
        patch[0] = 0xFF; patch[1] = 0x25;
        memcpy(patch + 6, &JmpToAddress, sizeof(uintptr_t));
        return WriteProcessMemoryImpl(HookAddress, patch, sizeof(patch));
    }

    uintptr_t Memory::CreateCodeCave(size_t Size)
    {
        if (!Memory::AttachedProcessHandle) return 0;
        void* p = VirtualAllocEx(Memory::AttachedProcessHandle, nullptr,
                                  Size, MEM_COMMIT | MEM_RESERVE,
                                  PAGE_EXECUTE_READWRITE);
        return (uintptr_t)p;
    }

    bool Memory::FreeCave(uintptr_t CaveAddress)
    {
        if (!Memory::AttachedProcessHandle) return false;
        return VirtualFreeEx(Memory::AttachedProcessHandle,
                              (LPVOID)CaveAddress, 0, MEM_RELEASE) != 0;
    }

    // ── FindSignature — with overlap fix ──────────────────────────────────
    uint64_t Memory::FindSignature(std::vector<uint8_t> Signature,
                                    uintptr_t ModuleBase, uintptr_t ModuleBaseSize)
    {
        const size_t sigSz = Signature.size();
        if (!sigSz || !Memory::AttachedProcessHandle) return 0;

        const size_t kBlock = 0x10000;
        std::vector<uint8_t> buf(kBlock + sigSz - 1);

        for (uintptr_t addr = ModuleBase;
             addr < ModuleBase + ModuleBaseSize;
             addr += kBlock)
        {
            SIZE_T toRead = kBlock;
            if (addr + toRead > ModuleBase + ModuleBaseSize)
                toRead = (ModuleBase + ModuleBaseSize) - addr;

            if (toRead < sigSz) break;

            SIZE_T got = 0;
            NTSTATUS st = s_NtRead
                ? s_NtRead(Memory::AttachedProcessHandle,
                           (PVOID)addr, buf.data(), toRead, &got)
                : (ReadProcessMemory(Memory::AttachedProcessHandle,
                                     (LPCVOID)addr, buf.data(), toRead, &got)
                   ? 0 : -1);

            if (st < 0 || got == 0) continue;

            for (size_t i = 0; i + sigSz <= got; i++)
            {
                bool match = true;
                for (size_t j = 0; j < sigSz; j++)
                {
                    if (Signature[j] != 0x00 && buf[i + j] != Signature[j])
                    { match = false; break; }
                }
                if (match) return addr + i;
            }
        }

        return 0;
    }

    // ── FindSignatureInAllMemory — scan all readable/executable regions ───
    uint64_t Memory::FindSignatureInAllMemory(std::vector<uint8_t> Signature)
    {
        const size_t sigSz = Signature.size();
        if (!sigSz || !Memory::AttachedProcessHandle) return 0;

        uint64_t found = 0;
        uintptr_t addr = 0;
        MEMORY_BASIC_INFORMATION mbi;

        while (VirtualQueryEx(Memory::AttachedProcessHandle, (LPCVOID)addr, &mbi, sizeof(mbi)) == sizeof(mbi))
        {
            if (mbi.State == MEM_COMMIT &&
                (mbi.Protect & (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_READONLY | PAGE_READWRITE)))
            {
                found = FindSignature(Signature, (uintptr_t)mbi.BaseAddress, mbi.RegionSize);
                if (found) break;
            }
            addr = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;
        }

        return found;
    }

	uint64_t Memory::FindSignatureInAllModules(std::vector<uint8_t> Signature)
	{
		if (!Memory::AttachedProcessPid || !Memory::AttachedProcessHandle)
			return 0;

		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
		                                        Memory::AttachedProcessPid);
		if (!hSnap || hSnap == INVALID_HANDLE_VALUE)
			return 0;

		MODULEENTRY32W me{ sizeof(me) };
		uint64_t found = 0;
		if (Module32FirstW(hSnap, &me))
		{
			do {
				uint64_t base = (uint64_t)me.modBaseAddr;
				uint64_t size = (uint64_t)me.modBaseSize;
				if (base && size)
				{
					found = FindSignature(Signature, base, size);
					if (found)
						break;
				}
			} while (Module32NextW(hSnap, &me));
		}
		CloseHandle(hSnap);
		return found;
	}

} // namespace FrameWork
