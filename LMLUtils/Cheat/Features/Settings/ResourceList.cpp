#include "ResourceList.hpp"
#include "../../Options.hpp"
#include <FrameWork/Utilities/Memory.hpp>
#include <FrameWork/Utilities/Notify.hpp>
#include <thread>
#include <chrono>
#include <sstream>

namespace Cheat
{
    namespace ResourceManager
    {
        // Pattern to find fwResourceManager singleton in five.dll
        // This references: lea rcx, [fwResourceManager]
        // Common across many FiveM versions
        const std::vector<uint8_t> g_ResourceManagerPattern = {
            0x48, 0x8D, 0x0D, 0x00, 0x00, 0x00, 0x00,       // lea rcx, [?]
            0x48, 0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,       // mov rcx, [rcx]
            0x48, 0x85, 0xC9,                                 // test rcx, rcx
            0x75, 0x00,                                       // jnz short
            0x48, 0x8D, 0x0D, 0x00, 0x00, 0x00, 0x00        // lea rcx, [?]
        };

        bool cResourceList::FindManager()
        {
            if (m_ManagerAddr != 0)
                return true;

            uint64_t addr = FrameWork::Memory::FindSignatureInAllModules(g_ResourceManagerPattern);
            if (!addr)
                return false;

            int rel = FrameWork::Memory::ReadMemory<int>(addr + 3);
            m_ManagerAddr = addr + 7 + rel;

            m_Initialized = m_ManagerAddr != 0;
            return m_Initialized;
        }

        std::string cResourceList::ReadResourceName(uintptr_t resourcePtr)
        {
            if (!resourcePtr)
                return "";

            auto& fivem = g_Fivem;
            HANDLE hProc = fivem.GetProcHandle();
            if (!hProc || hProc == INVALID_HANDLE_VALUE)
                return "";

            // Try reading the resource name at common offsets
            // fwResource typically has name at offset +0x10 (as std::string or char*)
            char nameBuf[128] = { 0 };

            // Method 1: Try reading as char* at offset 0x10 (typical for older builds)
            uint64_t namePtr1 = FrameWork::Memory::ReadMemory<uint64_t>(resourcePtr + 0x10);
            if (namePtr1 > 0x10000 && namePtr1 < 0x7FFFFFFFFFFFFF)
            {
                FrameWork::Memory::ReadProcessMemoryImpl(namePtr1, nameBuf, sizeof(nameBuf) - 1);
                if (nameBuf[0] != 0)
                    return std::string(nameBuf);
            }

            // Method 2: Try reading directly at offset 0x10 (UTF-16 or inline string)
            FrameWork::Memory::ReadProcessMemoryImpl(resourcePtr + 0x10, nameBuf, sizeof(nameBuf) - 1);
            if (nameBuf[0] >= 0x20 && nameBuf[0] <= 0x7E)
                return std::string(nameBuf);

            // Method 3: Try at offset 0x08 (GTA netObject style)
            FrameWork::Memory::ReadProcessMemoryImpl(resourcePtr + 0x08, nameBuf, sizeof(nameBuf) - 1);
            if (nameBuf[0] >= 0x20 && nameBuf[0] <= 0x7E && nameBuf[0] != 0)
                return std::string(nameBuf);

            // Method 4: Try reading std::string at offset 0x18 (pointer + size)
            FrameWork::Memory::ReadProcessMemoryImpl(resourcePtr + 0x18, nameBuf, sizeof(nameBuf) - 1);
            if (nameBuf[0] >= 0x20 && nameBuf[0] <= 0x7E && nameBuf[0] != 0)
                return std::string(nameBuf);

            // Method 5: Try as char* at offset 0x18
            uint64_t namePtr2 = FrameWork::Memory::ReadMemory<uint64_t>(resourcePtr + 0x18);
            if (namePtr2 > 0x10000 && namePtr2 < 0x7FFFFFFF)
            {
                memset(nameBuf, 0, sizeof(nameBuf));
                FrameWork::Memory::ReadProcessMemoryImpl(namePtr2, nameBuf, sizeof(nameBuf) - 1);
                if (nameBuf[0] >= 0x20 && nameBuf[0] <= 0x7E)
                    return std::string(nameBuf);
            }

            // Method 6: Try at offset 0x20 (some builds store name further in)
            uint64_t namePtr3 = FrameWork::Memory::ReadMemory<uint64_t>(resourcePtr + 0x20);
            if (namePtr3 > 0x10000 && namePtr3 < 0x7FFFFFFF)
            {
                memset(nameBuf, 0, sizeof(nameBuf));
                FrameWork::Memory::ReadProcessMemoryImpl(namePtr3, nameBuf, sizeof(nameBuf) - 1);
                if (nameBuf[0] >= 0x20 && nameBuf[0] <= 0x7E)
                    return std::string(nameBuf);
            }

            // Method 7: Try wide string at offset 0x10 -> convert to narrow
            wchar_t wNameBuf[128] = { 0 };
            uint64_t wNamePtr = FrameWork::Memory::ReadMemory<uint64_t>(resourcePtr + 0x10);
            if (wNamePtr > 0x10000 && wNamePtr < 0x7FFFFFFF)
            {
                FrameWork::Memory::ReadProcessMemoryImpl(wNamePtr, wNameBuf, sizeof(wNameBuf) - 2);
                if (wNameBuf[0] != 0)
                {
                    char narrow[128] = { 0 };
                    WideCharToMultiByte(CP_UTF8, 0, wNameBuf, -1, narrow, sizeof(narrow) - 1, NULL, NULL);
                    return std::string(narrow);
                }
            }

            return "";
        }

        void cResourceList::Refresh()
        {
            if (!FindManager())
                return;

            auto& fivem = g_Fivem;
            if (!fivem.IsInitialized())
                return;

            HANDLE hProc = fivem.GetProcHandle();
            if (!hProc || hProc == INVALID_HANDLE_VALUE)
                return;

            // Read the manager pointer (it's a pointer to the singleton)
            uint64_t managerPtr = FrameWork::Memory::ReadMemory<uint64_t>(m_ManagerAddr);
            if (!managerPtr || managerPtr == 0xCCCCCCCCCCCCCC)
                return;

            // Try to find the resource array in the manager
            // Typically: manager + 0x20 = array pointer, manager + 0x28 = count
            // Structure can vary significantly between FiveM versions

            std::vector<Resources_t> newResources;

            // Method 1: Try common layout (vector at +0x20, +0x28)
            uint64_t resourceArray = FrameWork::Memory::ReadMemory<uint64_t>(managerPtr + 0x20);
            uint64_t resourceCount = FrameWork::Memory::ReadMemory<uint64_t>(managerPtr + 0x28);

            if (resourceArray && resourceCount > 0 && resourceCount < 500)
            {
                for (uint64_t i = 0; i < resourceCount; i++)
                {
                    uintptr_t resPtr = FrameWork::Memory::ReadMemory<uintptr_t>(resourceArray + i * 8);
                    if (!resPtr || resPtr < 0x10000)
                        continue;

                    Resources_t res;
                    res.Pointer = resPtr;
                    res.Name = ReadResourceName(resPtr);
                    res.State = (eResourceState)FrameWork::Memory::ReadMemory<uint32_t>(resPtr + 0x28);

                    // Try state at other offsets
                    if (res.State > Stopping)
                    {
                        for (int off = 0x20; off < 0x50; off += 4)
                        {
                            uint32_t testState = FrameWork::Memory::ReadMemory<uint32_t>(resPtr + off);
                            if (testState <= Stopping)
                            {
                                res.State = (eResourceState)testState;
                                break;
                            }
                        }
                    }

                    newResources.push_back(res);
                }
            }

            // Method 2: If method 1 failed, try different offsets
            if (newResources.empty())
            {
                // Try manager + 0x30 (array), manager + 0x38 (count)
                uint64_t resArray2 = FrameWork::Memory::ReadMemory<uint64_t>(managerPtr + 0x30);
                uint64_t resCount2 = FrameWork::Memory::ReadMemory<uint64_t>(managerPtr + 0x38);

                if (resArray2 && resCount2 > 0 && resCount2 < 500)
                {
                    for (uint64_t i = 0; i < resCount2; i++)
                    {
                        uintptr_t resPtr = FrameWork::Memory::ReadMemory<uintptr_t>(resArray2 + i * 8);
                        if (!resPtr || resPtr < 0x10000)
                            continue;

                        Resources_t res;
                        res.Pointer = resPtr;
                        res.Name = ReadResourceName(resPtr);
                        res.State = (eResourceState)FrameWork::Memory::ReadMemory<uint32_t>(resPtr + 0x28);

                        if (res.State > Stopping)
                            res.State = eResourceState::Uninitialized;

                        newResources.push_back(res);
                    }
                }
            }

            // Method 3: Try reading at manager + 0x18
            if (newResources.empty())
            {
                uint64_t resArray3 = FrameWork::Memory::ReadMemory<uint64_t>(managerPtr + 0x18);
                uint64_t resCount3 = FrameWork::Memory::ReadMemory<uint64_t>(managerPtr + 0x10);

                if (resArray3 && resCount3 > 0 && resCount3 < 500)
                {
                    for (uint64_t i = 0; i < resCount3; i++)
                    {
                        uintptr_t resPtr = FrameWork::Memory::ReadMemory<uintptr_t>(resArray3 + i * 8);
                        if (!resPtr || resPtr < 0x10000)
                            continue;

                        Resources_t res;
                        res.Pointer = resPtr;
                        res.Name = ReadResourceName(resPtr);
                        res.State = (eResourceState)FrameWork::Memory::ReadMemory<uint32_t>(resPtr + 0x28);

                        if (res.State > Stopping)
                            res.State = eResourceState::Uninitialized;

                        newResources.push_back(res);
                    }
                }
            }

            // Update the global list
            {
                std::scoped_lock lock(g_ResourceMutex);
                vResources = newResources;
            }
        }

        bool cResourceList::Stop(uintptr_t ResourcePtr)
        {
            if (!ResourcePtr || ResourcePtr < 0x10000)
                return false;

            auto& fivem = g_Fivem;
            HANDLE hProc = fivem.GetProcHandle();
            if (!hProc || hProc == INVALID_HANDLE_VALUE)
                return false;

            // Read the vtable to get the Stop function pointer
            uint64_t vtable = FrameWork::Memory::ReadMemory<uint64_t>(ResourcePtr);
            if (!vtable || vtable < 0x10000 || vtable == 0xCCCCCCCCCCCCCC)
                return false;

            // The Stop method is typically at vtable index 2 (0x10) or index 3 (0x18)
            // We'll try both
            uint64_t stopFunc = 0;

            // Try vtable offset 0x10 (index 2)
            stopFunc = FrameWork::Memory::ReadMemory<uint64_t>(vtable + 0x10);
            if (!stopFunc || stopFunc < 0x10000 || stopFunc == 0xCCCCCCCCCCCCCC)
            {
                // Try vtable offset 0x18 (index 3)
                stopFunc = FrameWork::Memory::ReadMemory<uint64_t>(vtable + 0x18);
            }

            if (!stopFunc || stopFunc < 0x10000 || stopFunc == 0xCCCCCCCCCCCCCC)
                return false;

            // Shellcode: mov rcx, resourcePtr; call stopFunc; ret
            uint8_t shellcode[] = {
                0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rcx, ResourcePtr
                0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, stopFunc
                0xFF, 0xD0,                                                 // call rax
                0x33, 0xC0,                                                 // xor eax, eax
                0xC3                                                        // ret
            };

            memcpy(shellcode + 2, &ResourcePtr, sizeof(ResourcePtr));
            memcpy(shellcode + 12, &stopFunc, sizeof(stopFunc));

            void* remoteAddr = VirtualAllocEx(hProc, NULL, sizeof(shellcode), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (!remoteAddr)
                return false;

            WriteProcessMemory(hProc, remoteAddr, shellcode, sizeof(shellcode), NULL);

            HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)remoteAddr, NULL, 0, NULL);
            if (hThread)
            {
                WaitForSingleObject(hThread, 10000);
                CloseHandle(hThread);
                VirtualFreeEx(hProc, remoteAddr, 0, MEM_RELEASE);
                return true;
            }

            VirtualFreeEx(hProc, remoteAddr, 0, MEM_RELEASE);
            return false;
        }

        void cResourceList::RunThread()
        {
            while (!g_Options.General.ShutDown)
            {
                Refresh();
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }
    }
}
