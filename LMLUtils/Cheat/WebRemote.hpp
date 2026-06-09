#pragma once

#include <string>
#include <atomic>
#include <thread>

namespace Cheat
{
    class WebRemote
    {
    public:
        static void Start();
        static void Stop();
        static bool IsRunning();
        static std::string GetURL();

    private:
        static void ServerThread();
        static std::string HandleRequest(const std::string& request);
        static std::string GetOptionsJson();
        static void UpdateOptionsFromJson(const std::string& json_str);

        static std::atomic<bool> m_Running;
        static std::thread m_Thread;
        static std::string m_Token;
    };
}
