#pragma once

#include <FrameWork/FrameWork.hpp>
#include <winternl.h>
#include <string>
#include "../Dependencies/ImGui/imgui.h"

typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

namespace FrameWork
{
	namespace Misc
	{
		int RandomInt(int Lower = -100, int Max = 100);
		std::string Wstring2String(std::wstring Input);
		std::wstring String2WString(std::string Input);
		void InitializeConsole();
		void ShutDownConsole();
		std::string DownloadServerInfo(std::wstring IP, int PORT);
		std::string HttpRequest(const std::wstring& host, int port, const std::wstring& path, const std::string& method = "GET", const std::string& body = "", bool secure = true);
		RTL_OSVERSIONINFOW GetRealOSVersion();
		std::string GetWindowsFullBuildNumber();
		ImColor Float4ToImColor(float* Input);
		std::string GetDiscordUsername();
	}
}