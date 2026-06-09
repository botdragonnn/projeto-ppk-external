#pragma once

#include <string>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <vector>
#include <FrameWork/FrameWork.hpp>
#include <FrameWork/Dependencies/NlohmannJson.hpp>

namespace Cheat
{
    struct PlayerNetworkInfo
    {
        std::string UserName;
        std::string DiscordId;
        std::string SteamId;
    };

    namespace UpdateNames
    {
        void RunThread();
        std::string GetPlayerName(int playerId);
        std::string GetPlayerDiscord(int playerId);
        std::string GetPlayerSteam(int playerId);

        inline std::unordered_map<int, PlayerNetworkInfo> NetworkMap;
    }
}
