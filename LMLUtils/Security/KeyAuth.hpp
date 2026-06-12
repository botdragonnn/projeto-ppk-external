#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <winhttp.h>
#include <string>
#include <chrono>
#include <cstdio>

#pragma comment(lib, "winhttp.lib")

#include <FrameWork/Dependencies/NlohmannJson.hpp>

namespace Security
{
    struct LicenseInfo
    {
        bool valid = false;
        std::string key = "";
        std::string expiryDate = "";
        int daysLeft = 0;
        bool lifetime = false;
        std::string username = "";
        std::string lastError = "";
    };

    inline LicenseInfo CurrentLicense;

    inline std::string GetHWID()
    {
        HW_PROFILE_INFOA info{};
        if (GetCurrentHwProfileA(&info))
            return info.szHwProfileGuid;
        return "unknown";
    }

    inline int CalculateDaysLeft(const std::string& expiryDate)
    {
        if (expiryDate.empty() || expiryDate == "0")
            return -1;

        int year, month, day;
        if (sscanf_s(expiryDate.c_str(), "%d-%d-%d", &year, &month, &day) == 3)
        {
            std::tm tm_expiry = {};
            tm_expiry.tm_year = year - 1900;
            tm_expiry.tm_mon = month - 1;
            tm_expiry.tm_mday = day;
            auto expiry_time = std::chrono::system_clock::from_time_t(std::mktime(&tm_expiry));
            auto now = std::chrono::system_clock::now();
            auto days = std::chrono::duration_cast<std::chrono::hours>(expiry_time - now).count() / 24;
            return (int)days;
        }

        long long timestamp = 0;
        if (sscanf_s(expiryDate.c_str(), "%lld", &timestamp) == 1 && timestamp > 0)
        {
            time_t t = (time_t)timestamp;
            struct tm tm_et;
            localtime_s(&tm_et, &t);
            char buf[11];
            strftime(buf, sizeof(buf), "%Y-%m-%d", &tm_et);

            std::tm tm_expiry = {};
            tm_expiry.tm_year = tm_et.tm_year;
            tm_expiry.tm_mon = tm_et.tm_mon;
            tm_expiry.tm_mday = tm_et.tm_mday;
            auto expiry_time = std::chrono::system_clock::from_time_t(std::mktime(&tm_expiry));
            auto now = std::chrono::system_clock::now();
            auto days = std::chrono::duration_cast<std::chrono::hours>(expiry_time - now).count() / 24;
            return (int)days;
        }

        return -1;
    }

    inline std::string KeyAuthRequest(const std::string& postData)
    {
        std::string response;
        HINTERNET hSession = WinHttpOpen(L"ScarfaceX-External/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) return response;

        HINTERNET hConnect = WinHttpConnect(hSession, L"keyauth.win", INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (!hConnect) { WinHttpCloseHandle(hSession); return response; }

        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/api/1.3/", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
        if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return response; }

        DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE | SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
        WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));

        std::wstring headers = L"Content-Type: application/x-www-form-urlencoded\r\n";
        WinHttpAddRequestHeaders(hRequest, headers.c_str(), -1L, WINHTTP_ADDREQ_FLAG_ADD);

        if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)postData.c_str(), (DWORD)postData.length(), (DWORD)postData.length(), 0))
        {
            if (WinHttpReceiveResponse(hRequest, NULL))
            {
                DWORD dwSize = 0, dwDownloaded = 0;
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
                            if (WinHttpReadData(hRequest, pszOutBuffer, dwSize, &dwDownloaded))
                                response.append(pszOutBuffer, dwDownloaded);
                            delete[] pszOutBuffer;
                        }
                    }
                } while (dwSize > 0);
            }
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return response;
    }

    inline bool GetJsonBool(const nlohmann::json& j, const std::string& key)
    {
        if (!j.contains(key)) return false;
        auto& v = j[key];
        if (v.is_boolean()) return v.get<bool>();
        if (v.is_string())
        {
            std::string s = v.get<std::string>();
            return s == "true" || s == "1";
        }
        if (v.is_number_integer()) return v.get<int>() != 0;
        return false;
    }

    inline std::string GetJsonStr(const nlohmann::json& j, const std::string& key)
    {
        if (!j.contains(key)) return "";
        auto& v = j[key];
        if (v.is_string()) return v.get<std::string>();
        if (v.is_number_integer()) return std::to_string(v.get<int>());
        return v.dump();
    }

    inline bool Authenticate(const std::string& key)
    {
        CurrentLicense.valid = false;
        CurrentLicense.key = key;
        CurrentLicense.lastError = "";

        if (key.empty()) { CurrentLicense.lastError = "Key is empty"; return false; }

        const std::string name = "ScarfaceX 2.0";
        const std::string ownerid = "qs0kSLr3B9";
        const std::string secret = "3af39c6065eabed8edc9bd46a770231628d0363106dfb7949fc2fac9ff1dc2c7";
        const std::string version = "1.0";

        std::string hwid = GetHWID();
        std::string initData = "type=init&ver=" + version + "&name=" + name + "&ownerid=" + ownerid + "&secret=" + secret + "&hwid=" + hwid;
        std::string initResp = KeyAuthRequest(initData);
        if (initResp.empty()) { CurrentLicense.lastError = "Connection failed (init)"; return false; }

        nlohmann::json initJson;
        try { initJson = nlohmann::json::parse(initResp); }
        catch (...) { CurrentLicense.lastError = "Invalid JSON (init): " + initResp; return false; }

        if (!GetJsonBool(initJson, "success"))
        {
            CurrentLicense.lastError = GetJsonStr(initJson, "message");
            if (CurrentLicense.lastError.empty()) CurrentLicense.lastError = "Init failed (check app name & ownerid)";
            return false;
        }

        std::string sessionid = GetJsonStr(initJson, "sessionid");

        std::string loginData = "type=license&key=" + key + "&sessionid=" + sessionid + "&name=" + name + "&ownerid=" + ownerid + "&hwid=" + hwid + "&ver=" + version;
        std::string loginResp = KeyAuthRequest(loginData);
        if (loginResp.empty()) { CurrentLicense.lastError = "Connection failed (login)"; return false; }

        nlohmann::json loginJson;
        try { loginJson = nlohmann::json::parse(loginResp); }
        catch (...) { CurrentLicense.lastError = "Invalid JSON (login): " + loginResp; return false; }

        if (!GetJsonBool(loginJson, "success"))
        {
            CurrentLicense.lastError = GetJsonStr(loginJson, "message");
            if (CurrentLicense.lastError.empty()) CurrentLicense.lastError = loginResp;
            return false;
        }

        if (loginJson.contains("info"))
        {
            auto& info = loginJson["info"];
            if (info.contains("username"))
                CurrentLicense.username = GetJsonStr(info, "username");

            if (info.contains("subscriptions") && !info["subscriptions"].empty())
            {
                auto& sub = info["subscriptions"][0];
                std::string expiry = GetJsonStr(sub, "expiry");

                if (expiry.empty() || expiry == "0")
                {
                    CurrentLicense.lifetime = true;
                    CurrentLicense.daysLeft = -1;
                    CurrentLicense.expiryDate = "LIFETIME";
                }
                else
                {
                    long long ts = 0;
                    if (sscanf_s(expiry.c_str(), "%lld", &ts) == 1 && ts > 0)
                    {
                        time_t t = (time_t)ts;
                        struct tm tm_et;
                        localtime_s(&tm_et, &t);
                        char buf[11];
                        strftime(buf, sizeof(buf), "%Y-%m-%d", &tm_et);
                        CurrentLicense.expiryDate = buf;
                    }
                    else
                    {
                        CurrentLicense.expiryDate = expiry;
                    }
                    CurrentLicense.daysLeft = CalculateDaysLeft(CurrentLicense.expiryDate);
                }
            }
        }

        CurrentLicense.valid = true;
        return true;
    }
}
