#include "UpdateNames.hpp"
#include "../../Options.hpp"
#include <FrameWork/Dependencies/httplib.h>
#include <FrameWork/Utilities/Notify.hpp>
#include <fstream>
#include <regex>
#include <algorithm>

using json = nlohmann::json;

namespace Cheat
{
    namespace UpdateNames
    {
        static std::string g_ServerIp;
        static bool g_Initialized = false;

        static std::string GetFiveMDir()
        {
            char value[255];
            DWORD BufferSize = 8192;

            HKEY hKey;
            if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\CitizenFX\\FiveM", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
                return "";

            LONG result = RegQueryValueExA(hKey, "Last Run Location", NULL, NULL, (LPBYTE)value, &BufferSize);
            RegCloseKey(hKey);

            if (result != ERROR_SUCCESS)
                return "";

            return std::string(value);
        }

        static std::string ExtractServerIp()
        {
            std::string fivemDir = GetFiveMDir();
            if (fivemDir.empty())
                return "";

            std::string crashometryPath = fivemDir + "\\data\\cache\\crashometry";
            std::ifstream file(crashometryPath, std::ios::binary);
            if (!file.is_open())
                return "";

            std::string line;
            std::string serverIp;
            while (std::getline(file, line))
            {
                size_t pos = line.find("last_server_url");
                if (pos != std::string::npos)
                {
                    size_t start = line.find("last_server", pos);
                    if (start != std::string::npos)
                    {
                        size_t colon = line.find(":", start + 12);
                        if (colon != std::string::npos)
                        {
                            serverIp = line.substr(start + 12, colon - (start + 12) + 6);
                            break;
                        }
                    }
                }
            }

            file.close();
            return serverIp;
        }

        static void FetchPlayerNames()
        {
            if (g_ServerIp.empty())
            {
                g_ServerIp = ExtractServerIp();
                if (g_ServerIp.empty())
                    return;
            }

            try
            {
                httplib::Client cli("http://" + g_ServerIp);
                cli.set_connection_timeout(5);
                cli.set_read_timeout(5);

                auto res = cli.Get("/players.json");
                if (res && res->status == 200)
                {
                    json playersJson = json::parse(res->body);

                    if (playersJson.is_array())
                    {
                        for (const auto& player : playersJson)
                        {
                            if (!player.contains("id") || !player.contains("name"))
                                continue;

                            int playerId = player["id"].get<int>();
                            std::string playerName = player["name"].get<std::string>();

                            PlayerNetworkInfo info;
                            info.UserName = playerName;

                            if (player.contains("identifiers") && player["identifiers"].is_array())
                            {
                                for (const auto& identifier : player["identifiers"])
                                {
                                    if (!identifier.is_string()) continue;
                                    std::string idVal = identifier.get<std::string>();

                                    if (idVal.find("discord:") == 0 && idVal.length() > 8)
                                        info.DiscordId = idVal.substr(8);

                                    if (idVal.find("steam:") == 0 && idVal.length() > 6)
                                        info.SteamId = idVal.substr(6);
                                }
                            }

                            NetworkMap[playerId] = info;
                        }
                    }
                    else if (playersJson.contains("Data") && playersJson["Data"].contains("players"))
                    {
                        auto& players = playersJson["Data"]["players"];
                        for (const auto& player : players)
                        {
                            if (!player.contains("id") || !player.contains("name"))
                                continue;

                            int playerId = player["id"].get<int>();
                            std::string playerName = player["name"].get<std::string>();

                            PlayerNetworkInfo info;
                            info.UserName = playerName;

                            if (player.contains("identifiers") && player["identifiers"].is_array())
                            {
                                for (const auto& identifier : player["identifiers"])
                                {
                                    if (!identifier.is_string()) continue;
                                    std::string idVal = identifier.get<std::string>();

                                    if (idVal.find("discord:") == 0 && idVal.length() > 8)
                                        info.DiscordId = idVal.substr(8);

                                    if (idVal.find("steam:") == 0 && idVal.length() > 6)
                                        info.SteamId = idVal.substr(6);
                                }
                            }

                            NetworkMap[playerId] = info;
                        }
                    }
                }
            }
            catch (...)
            {
            }
        }

        std::string GetPlayerName(int playerId)
        {
            auto it = NetworkMap.find(playerId);
            if (it != NetworkMap.end())
                return it->second.UserName;
            return "";
        }

        std::string GetPlayerDiscord(int playerId)
        {
            auto it = NetworkMap.find(playerId);
            if (it != NetworkMap.end())
                return it->second.DiscordId;
            return "";
        }

        std::string GetPlayerSteam(int playerId)
        {
            auto it = NetworkMap.find(playerId);
            if (it != NetworkMap.end())
                return it->second.SteamId;
            return "";
        }

        void RunThread()
        {
            while (!g_Options.General.ShutDown)
            {
                FetchPlayerNames();
                std::this_thread::sleep_for(std::chrono::seconds(6));
            }
        }
    }
}
