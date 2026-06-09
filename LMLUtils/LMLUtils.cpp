#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#include <winsock2.h>
#include <Windows.h>
#include <thread>
#include <tlhelp32.h>
#include <chrono>
#include <string>
#include <iostream>
#include <timeapi.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ws2_32.lib")

#include <FrameWork/FrameWork.hpp>
#include <Cheat/Cheat.hpp>
#include <Cheat/Features/Settings/AntiCrack.hpp>

constexpr auto kMainLoopDelay = std::chrono::seconds(2);

DWORD WINAPI MainThread(LPVOID lpParam)
{
	timeBeginPeriod(1);

	Cheat::AntiCrack::Initialize();
	SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);

	Cheat::Initialize();

	while (!g_Options.General.ShutDown) {
		std::this_thread::sleep_for(kMainLoopDelay);
	}

	Cheat::ShutDown();
	timeEndPeriod(1);

	SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);

	return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	return (int)MainThread(NULL);
}