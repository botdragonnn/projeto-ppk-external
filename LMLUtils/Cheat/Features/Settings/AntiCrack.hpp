#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

namespace Cheat
{
    namespace AntiCrack
    {
        void Initialize();
        void RunThread();
        bool GetMaxPrivileges(HANDLE hProc);
    }
}
