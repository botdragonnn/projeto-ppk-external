#include "AntiCrack.hpp"
#include "../../Options.hpp"
#include <FrameWork/Utilities/Notify.hpp>
#include <FrameWork/FrameWork.hpp>
#include <algorithm>
#include <string>
#include <vector>

namespace Cheat
{
    namespace AntiCrack
    {
        static bool g_bMaxPrivileges = false;

        static const std::vector<std::string> BadTitles =
        {
            "httpdebugger",
            "http debugger",
            "x64dbg",
            "debugger",
            "disassembler",
            "decompiler",
            "fiddler",
            "wireshark",
            "string search",
            "process list",
            "memory viewer",
            "system informer",
            "process hacker",
            "ghidra",
            "binary ninja",
            "hyperdbg",
            "process explorer - sysinternals",
            "extreme dumper",
            "add address",
            "process telerik",
            "scylla",
            "referenced strings",
            "dissect code",
            "beamer",
            "windbg",
            "ksdumper",
            "import reconstructor",
            "httpdebuggerui",
            "analysis tool",
            "ollydbg",
            "network traffic dump tool",
            "petool",
            "part of sysinternals suite",
            "network analyzer",
            "cheat engine",
            "cheatengine",
        };

        BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam)
        {
            int length = GetWindowTextLengthA(hWnd);
            if (length == 0 || !IsWindowVisible(hWnd))
                return TRUE;

            char* buffer = new char[length + 1];
            GetWindowTextA(hWnd, buffer, length + 1);
            std::string title(buffer);
            delete[] buffer;

            std::transform(title.begin(), title.end(), title.begin(), ::tolower);

            for (const auto& bad : BadTitles)
            {
                if (title.find(bad) != std::string::npos)
                {
                    bool* found = reinterpret_cast<bool*>(lParam);
                    if (found)
                        *found = true;



                    return FALSE;
                }
            }

            return TRUE;
        }

        bool GetMaxPrivileges(HANDLE hProc)
        {
            HANDLE hToken = NULL;
            TOKEN_PRIVILEGES tp;
            LUID luid;

            if (!OpenProcessToken(hProc, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
                return false;

            if (!LookupPrivilegeValueA(NULL, "SeDebugPrivilege", &luid))
            {
                CloseHandle(hToken);
                return false;
            }

            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
            {
                CloseHandle(hToken);
                return false;
            }

            CloseHandle(hToken);
            return true;
        }

        void Initialize()
        {
            HANDLE hProc = GetCurrentProcess();
            g_bMaxPrivileges = GetMaxPrivileges(hProc);
        }

        void RunThread()
        {
            while (!g_Options.General.ShutDown)
            {
                bool foundDebugger = false;
                EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&foundDebugger));

                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }
    }
}
