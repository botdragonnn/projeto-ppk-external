#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")

#include "WebRemote.hpp"
#include <Cheat/Options.hpp>
#include <Security/KeyAuth.hpp>
#include <FrameWork/Dependencies/NlohmannJson.hpp>
#include <sstream>
#include <vector>
#include <iostream>

using json = nlohmann::json;

namespace Cheat
{
    std::atomic<bool> WebRemote::m_Running = false;
    std::thread WebRemote::m_Thread;
    std::string WebRemote::m_Token;

    const char* HTML_CONTENT = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ScarfaceX Remote Control</title>
    <style>
        * { box-sizing: border-box; }
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #000000; color: #e0e0e0; margin: 0; padding: 20px; }
        .container { max-width: 800px; margin: auto; background: #202020; padding: 20px; border-radius: 12px; border: 1px solid #404040; box-shadow: 0 4px 15px rgba(0,0,0,0.5); }
        h1 { text-align: center; color: #FF0033; font-size: 24px; margin-bottom: 20px; text-transform: uppercase; letter-spacing: 2px; }
        .section { margin-bottom: 20px; padding-bottom: 10px; border-bottom: 1px solid #404040; }
        .section-title { font-weight: bold; color: #FF0033; margin-bottom: 10px; font-size: 18px; cursor: pointer; user-select: none; }
.section-title::before { content: "\25BC"; font-size: 12px; margin-right: 8px; }
.section-title.collapsed::before { content: "\25B6 "; }
        .subsection { margin-left: 15px; margin-bottom: 12px; }
        .subsection-title { font-weight: bold; color: #FF0033; margin-bottom: 8px; font-size: 15px; }
        .control-group { display: flex; align-items: center; justify-content: space-between; margin-bottom: 8px; padding: 3px 0; }
        .label { font-size: 13px; flex: 1; }
        .label small { color: #82828C; font-size: 11px; }
        .switch { position: relative; display: inline-block; width: 40px; height: 20px; flex-shrink: 0; }
        .switch input { opacity: 0; width: 0; height: 0; }
        .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #404040; transition: .3s; border-radius: 20px; }
        .slider:before { position: absolute; content: ""; height: 14px; width: 14px; left: 3px; bottom: 3px; background-color: white; transition: .3s; border-radius: 50%; }
        input:checked + .slider { background-color: #FF0033; }
        input:checked + .slider:before { transform: translateX(20px); }
        input[type=range] { -webkit-appearance: none; width: 100px; background: transparent; flex-shrink: 0; }
        input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; height: 14px; width: 14px; border-radius: 50%; background: #FF0033; cursor: pointer; margin-top: -5px; }
        input[type=range]::-webkit-slider-runnable-track { width: 100%; height: 4px; cursor: pointer; background: #404040; border-radius: 2px; }
        .value-display { color: #FF0033; font-size: 12px; min-width: 28px; text-align: right; flex-shrink: 0; }
        .status-bar { text-align: center; font-size: 12px; color: #82828C; margin-top: 20px; }
        .refresh-btn { display: block; width: 100%; padding: 10px; background: #404040; border: none; color: white; border-radius: 6px; cursor: pointer; margin-top: 10px; transition: .2s; }
        .refresh-btn:hover { background: #2C2C34; }
        .section-content { overflow: hidden; transition: max-height 0.3s ease; }
        .section-content.collapsed { max-height: 0 !important; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ScarfaceX Remote</h1>

        <!-- LegitBot -->
        <div class="section">
            <div class="section-title" onclick="toggleSection(this)">LegitBot</div>
            <div class="section-content">
                <div class="control-group"><span class="label">Aim Dead</span><label class="switch"><input type="checkbox" id="aimdead" onchange="update('aimdead')"><span class="slider"></span></label></div>
                <!-- Aimbot -->
                <div class="subsection">
                    <div class="subsection-title">Aimbot</div>
                    <div class="control-group"><span class="label">Enabled</span><label class="switch"><input type="checkbox" id="aimbot_enabled" onchange="update('aimbot_enabled')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Target NPC</span><label class="switch"><input type="checkbox" id="aimbot_npc" onchange="update('aimbot_npc')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Visible Check</span><label class="switch"><input type="checkbox" id="aimbot_visible" onchange="update('aimbot_visible')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Smooth X <small>(0-100)</small></span><input type="range" id="aimbot_smoothx" min="0" max="100" oninput="showVal(this,'aimbot_smoothx_v')" onchange="update('aimbot_smoothx')"><span class="value-display" id="aimbot_smoothx_v">0</span></div>
                    <div class="control-group"><span class="label">Smooth Y <small>(0-100)</small></span><input type="range" id="aimbot_smoothy" min="0" max="100" oninput="showVal(this,'aimbot_smoothy_v')" onchange="update('aimbot_smoothy')"><span class="value-display" id="aimbot_smoothy_v">0</span></div>
                    <div class="control-group"><span class="label">Max Distance <small>(0-600m)</small></span><input type="range" id="aimbot_dist" min="0" max="600" oninput="showVal(this,'aimbot_dist_v')" onchange="update('aimbot_dist')"><span class="value-display" id="aimbot_dist_v">0</span></div>
                    <div class="control-group"><span class="label">Field of View <small>(0-800)</small></span><input type="range" id="aimbot_fov" min="0" max="800" oninput="showVal(this,'aimbot_fov_v')" onchange="update('aimbot_fov')"><span class="value-display" id="aimbot_fov_v">0</span></div>
                    <div class="control-group"><span class="label">HitBox</span><select id="aimbot_hitbox" onchange="update('aimbot_hitbox')" style="background:#202020;color:#e0e0e0;border:1px solid #404040;border-radius:4px;padding:2px 6px;"><option value="0">Head</option><option value="1">Neck</option><option value="2">Chest</option></select></div>
                    <div class="control-group"><span class="label">Show FOV</span><label class="switch"><input type="checkbox" id="aimbot_showfov" onchange="update('aimbot_showfov')"><span class="slider"></span></label></div>
                </div>
                <!-- Silent Aim -->
                <div class="subsection">
                    <div class="subsection-title">Silent Aim</div>
                    <div class="control-group"><span class="label">Enabled</span><label class="switch"><input type="checkbox" id="silent_enabled" onchange="update('silent_enabled')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Target NPC</span><label class="switch"><input type="checkbox" id="silent_npc" onchange="update('silent_npc')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Visible Check</span><label class="switch"><input type="checkbox" id="silent_visible" onchange="update('silent_visible')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Max Distance <small>(0-600m)</small></span><input type="range" id="silent_dist" min="0" max="600" oninput="showVal(this,'silent_dist_v')" onchange="update('silent_dist')"><span class="value-display" id="silent_dist_v">0</span></div>
                    <div class="control-group"><span class="label">Field of View <small>(0-800)</small></span><input type="range" id="silent_fov" min="0" max="800" oninput="showVal(this,'silent_fov_v')" onchange="update('silent_fov')"><span class="value-display" id="silent_fov_v">0</span></div>
                    <div class="control-group"><span class="label">HitBox</span><select id="silent_hitbox" onchange="update('silent_hitbox')" style="background:#202020;color:#e0e0e0;border:1px solid #404040;border-radius:4px;padding:2px 6px;"><option value="0">Head</option><option value="1">Neck</option><option value="2">Chest</option></select></div>
                    <div class="control-group"><span class="label">Magic Bullet</span><label class="switch"><input type="checkbox" id="silent_magic" onchange="update('silent_magic')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Miss Chance <small>(0-100%)</small></span><input type="range" id="silent_miss" min="0" max="100" oninput="showVal(this,'silent_miss_v')" onchange="update('silent_miss')"><span class="value-display" id="silent_miss_v">0</span></div>
                    <div class="control-group"><span class="label">Show FOV</span><label class="switch"><input type="checkbox" id="silent_showfov" onchange="update('silent_showfov')"><span class="slider"></span></label></div>
                </div>
                <!-- Trigger -->
                <div class="subsection">
                    <div class="subsection-title">Trigger</div>
                    <div class="control-group"><span class="label">Enabled</span><label class="switch"><input type="checkbox" id="trigger_enabled" onchange="update('trigger_enabled')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Target NPC</span><label class="switch"><input type="checkbox" id="trigger_npc" onchange="update('trigger_npc')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Visible Check</span><label class="switch"><input type="checkbox" id="trigger_visible" onchange="update('trigger_visible')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Smart Trigger</span><label class="switch"><input type="checkbox" id="trigger_smart" onchange="update('trigger_smart')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Max Distance <small>(0-600m)</small></span><input type="range" id="trigger_dist" min="0" max="600" oninput="showVal(this,'trigger_dist_v')" onchange="update('trigger_dist')"><span class="value-display" id="trigger_dist_v">0</span></div>
                    <div class="control-group"><span class="label">Field of View <small>(0-800)</small></span><input type="range" id="trigger_fov" min="0" max="800" oninput="showVal(this,'trigger_fov_v')" onchange="update('trigger_fov')"><span class="value-display" id="trigger_fov_v">0</span></div>
                    <div class="control-group"><span class="label">Reaction Time <small>(0-500ms)</small></span><input type="range" id="trigger_reaction" min="0" max="500" oninput="showVal(this,'trigger_reaction_v')" onchange="update('trigger_reaction')"><span class="value-display" id="trigger_reaction_v">0</span></div>
                    <div class="control-group"><span class="label">Show FOV</span><label class="switch"><input type="checkbox" id="trigger_showfov" onchange="update('trigger_showfov')"><span class="slider"></span></label></div>
                </div>
            </div>
        </div>
)HTML"
R"HTML(

        <!-- Visuals -->
        <div class="section">
            <div class="section-title collapsed" onclick="toggleSection(this)">Visuals</div>
            <div class="section-content collapsed">
                <!-- Players -->
                <div class="subsection">
                    <div class="subsection-title">Players ESP</div>
                    <div class="control-group"><span class="label">Enabled</span><label class="switch"><input type="checkbox" id="esp_players" onchange="update('esp_players')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Show Local</span><label class="switch"><input type="checkbox" id="esp_local" onchange="update('esp_local')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Show NPCs</span><label class="switch"><input type="checkbox" id="esp_npcs" onchange="update('esp_npcs')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Visible Only</span><label class="switch"><input type="checkbox" id="esp_visible" onchange="update('esp_visible')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Show Dead</span><label class="switch"><input type="checkbox" id="esp_showdead" onchange="update('esp_showdead')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Max Distance <small>(0-600m)</small></span><input type="range" id="esp_distance" min="0" max="600" oninput="showVal(this,'esp_distance_v')" onchange="update('esp_distance')"><span class="value-display" id="esp_distance_v">0</span></div>
                    <div class="control-group"><span class="label">Name</span><label class="switch"><input type="checkbox" id="esp_name" onchange="update('esp_name')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Weapon Name</span><label class="switch"><input type="checkbox" id="esp_weapon" onchange="update('esp_weapon')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Skeleton</span><label class="switch"><input type="checkbox" id="esp_skeleton" onchange="update('esp_skeleton')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Head Circle</span><label class="switch"><input type="checkbox" id="esp_headcircle" onchange="update('esp_headcircle')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Armor Bar</span><label class="switch"><input type="checkbox" id="esp_armor" onchange="update('esp_armor')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Health Bar</span><label class="switch"><input type="checkbox" id="esp_health" onchange="update('esp_health')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">RGB Mode</span><label class="switch"><input type="checkbox" id="esp_rgb" onchange="update('esp_rgb')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">RGB Speed <small>(0.1-10)</small></span><input type="range" id="esp_rgbspeed" min="1" max="100" oninput="showVal(this,'esp_rgbspeed_v')" onchange="update('esp_rgbspeed')"><span class="value-display" id="esp_rgbspeed_v">0</span></div>
                    <div class="control-group"><span class="label">Distance</span><label class="switch"><input type="checkbox" id="esp_disttxt" onchange="update('esp_disttxt')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Box</span><label class="switch"><input type="checkbox" id="esp_box" onchange="update('esp_box')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Snap Lines</span><label class="switch"><input type="checkbox" id="esp_snaplines" onchange="update('esp_snaplines')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Highlight Visible</span><label class="switch"><input type="checkbox" id="esp_hlvis" onchange="update('esp_hlvis')"><span class="slider"></span></label></div>
                </div>
)HTML"
R"HTML(
                <!-- Colors -->
                <div class="subsection">
                    <div class="subsection-title">Colors</div>
                    <div class="control-group"><span class="label">Box Color</span> <input type="color" id="esp_boxcol" onchange="updateColor('esp_boxcol', this.value)" style="width:40px;height:24px;background:#202020;border:1px solid #404040;border-radius:4px;padding:0;cursor:pointer;"></div>
                    <div class="control-group"><span class="label">Skeleton Color</span> <input type="color" id="esp_skeletoncol" onchange="updateColor('esp_skeletoncol', this.value)" style="width:40px;height:24px;background:#202020;border:1px solid #404040;border-radius:4px;padding:0;cursor:pointer;"></div>
                    <div class="control-group"><span class="label">Text Color</span> <input type="color" id="esp_textcol" onchange="updateColor('esp_textcol', this.value)" style="width:40px;height:24px;background:#202020;border:1px solid #404040;border-radius:4px;padding:0;cursor:pointer;"></div>
                    <div class="control-group"><span class="label">Snap Lines Color</span> <input type="color" id="esp_snapcol" onchange="updateColor('esp_snapcol', this.value)" style="width:40px;height:24px;background:#202020;border:1px solid #404040;border-radius:4px;padding:0;cursor:pointer;"></div>
                    <div class="control-group"><span class="label">Health Bar Color</span> <input type="color" id="esp_healthcol" onchange="updateColor('esp_healthcol', this.value)" style="width:40px;height:24px;background:#202020;border:1px solid #404040;border-radius:4px;padding:0;cursor:pointer;"></div>
                    <div class="control-group"><span class="label">Armor Bar Color</span> <input type="color" id="esp_armorcol" onchange="updateColor('esp_armorcol', this.value)" style="width:40px;height:24px;background:#202020;border:1px solid #404040;border-radius:4px;padding:0;cursor:pointer;"></div>
                    <div class="control-group"><span class="label">Head Circle Color</span> <input type="color" id="esp_headcol" onchange="updateColor('esp_headcol', this.value)" style="width:40px;height:24px;background:#202020;border:1px solid #404040;border-radius:4px;padding:0;cursor:pointer;"></div>
                    <div class="control-group"><span class="label">Friend Color</span> <input type="color" id="esp_friendcol" onchange="updateColor('esp_friendcol', this.value)" style="width:40px;height:24px;background:#202020;border:1px solid #404040;border-radius:4px;padding:0;cursor:pointer;"></div>
                </div>
                <!-- Vehicles -->
                <div class="subsection">
                    <div class="subsection-title">Vehicles ESP</div>
                    <div class="control-group"><span class="label">Enabled</span><label class="switch"><input type="checkbox" id="esp_vehicles" onchange="update('esp_vehicles')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Max Distance <small>(0-600m)</small></span><input type="range" id="esp_vehdist" min="0" max="600" oninput="showVal(this,'esp_vehdist_v')" onchange="update('esp_vehdist')"><span class="value-display" id="esp_vehdist_v">0</span></div>
                    <div class="control-group"><span class="label">Model Name</span><label class="switch"><input type="checkbox" id="esp_vehname" onchange="update('esp_vehname')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Distance</span><label class="switch"><input type="checkbox" id="esp_vehdisttxt" onchange="update('esp_vehdisttxt')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Doors (Lock/Unlock)</span><label class="switch"><input type="checkbox" id="esp_vehdoor" onchange="update('esp_vehdoor')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Snap Lines</span><label class="switch"><input type="checkbox" id="esp_vehsnap" onchange="update('esp_vehsnap')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Lock Status Icon</span><label class="switch"><input type="checkbox" id="esp_vehlock" onchange="update('esp_vehlock')"><span class="slider"></span></label></div>
                </div>
            </div>
        </div>

        <!-- Exploits -->
        <div class="section">
            <div class="section-title collapsed" onclick="toggleSection(this)">Exploits</div>
            <div class="section-content collapsed">
                <!-- Player -->
                <div class="subsection">
                    <div class="subsection-title">Player</div>
                    <div class="control-group"><span class="label">God Mode</span><label class="switch"><input type="checkbox" id="godmode" onchange="update('godmode')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Anti-Aim</span><label class="switch"><input type="checkbox" id="anti-aim" onchange="update('anti-aim')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Noclip</span><label class="switch"><input type="checkbox" id="noclip" onchange="update('noclip')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Noclip Speed <small>(0.1-20)</small></span><input type="range" id="noclipspeed" min="0.1" max="20" step="0.1" oninput="showVal(this,'noclipspeed_v')" onchange="update('noclipspeed')"><span class="value-display" id="noclipspeed_v">0</span></div>
                    <div class="control-group"><span class="label">Infinite Combat Roll</span><label class="switch"><input type="checkbox" id="infroll" onchange="update('infroll')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Infinite Stamina</span><label class="switch"><input type="checkbox" id="infstamina" onchange="update('infstamina')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Fast Run</span><label class="switch"><input type="checkbox" id="fastrun" onchange="update('fastrun')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Run Speed <small>(1-10)</small></span><input type="range" id="fastrunspeed" min="1" max="10" step="0.1" oninput="showVal(this,'fastrunspeed_v')" onchange="update('fastrunspeed')"><span class="value-display" id="fastrunspeed_v">0</span></div>
                    <div class="control-group"><span class="label">Shrink</span><label class="switch"><input type="checkbox" id="shrink" onchange="update('shrink')"><span class="slider"></span></label></div>
                    <div class="subsection-title">Weapons</div>
                    <div class="control-group"><span class="label">Infinite Ammo</span><label class="switch"><input type="checkbox" id="inf_ammo" onchange="update('inf_ammo')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">No Reload</span><label class="switch"><input type="checkbox" id="noreload" onchange="update('noreload')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">No Recoil</span><label class="switch"><input type="checkbox" id="norecoil" onchange="update('norecoil')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">No Spread</span><label class="switch"><input type="checkbox" id="nospread" onchange="update('nospread')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Weapon Scale</span><label class="switch"><input type="checkbox" id="weaponscale" onchange="update('weaponscale')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Weapon Scale Value <small>(0.1-10)</small></span><input type="range" id="weaponscaleval" min="1" max="100" oninput="showVal(this,'weaponscaleval_v')" onchange="update('weaponscaleval')"><span class="value-display" id="weaponscaleval_v">0</span></div>
                </div>
                <!-- Vehicle -->
                <div class="subsection">
                    <div class="subsection-title">Vehicle</div>
                    <div class="control-group"><span class="label">Vehicle God Mode</span><label class="switch"><input type="checkbox" id="vehgod" onchange="update('vehgod')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Seat Belt</span><label class="switch"><input type="checkbox" id="seatbelt" onchange="update('seatbelt')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Rocket Boost</span><label class="switch"><input type="checkbox" id="vehboost" onchange="update('vehboost')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Boost Strength <small>(0-20)</small></span><input type="range" id="vehbooststr" min="0" max="20" oninput="showVal(this,'vehbooststr_v')" onchange="update('vehbooststr')"><span class="value-display" id="vehbooststr_v">0</span></div>
                    <div class="control-group"><span class="label">Vehicle Boost <small>(1-100)</small></span><input type="range" id="vehboostval" min="1" max="100" oninput="showVal(this,'vehboostval_v')" onchange="update('vehboostval')"><span class="value-display" id="vehboostval_v">0</span></div>
                </div>
            </div>
        </div>
)HTML"
R"HTML(

        <!-- Settings -->
        <div class="section">
            <div class="section-title collapsed" onclick="toggleSection(this)">Settings</div>
            <div class="section-content collapsed">
                <div class="subsection">
                    <div class="subsection-title">General</div>
                    <div class="control-group"><span class="label">Safe Mode</span><label class="switch"><input type="checkbox" id="safemode" onchange="update('safemode')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Active Features</span><label class="switch"><input type="checkbox" id="activefeat" onchange="update('activefeat')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Web Remote</span><label class="switch"><input type="checkbox" id="webremote" onchange="update('webremote')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Menu Key</span><select id="menukey" onchange="update('menukey')" style="background:#202020;color:#e0e0e0;border:1px solid #404040;border-radius:4px;padding:2px 6px;"><option value="0">Disabled</option><option value="1">Insert</option><option value="2">F1</option><option value="3">F2</option><option value="4">F3</option><option value="5">F4</option><option value="6">F5</option><option value="7">F6</option><option value="8">F7</option><option value="9">F8</option><option value="10">F9</option><option value="11">F10</option><option value="12">F11</option><option value="13">F12</option></select></div>
                    <div class="control-group"><span class="label">Processor Delay <small>(1-15ms)</small></span><input type="range" id="threaddelay" min="1" max="15" oninput="showVal(this,'threaddelay_v')" onchange="update('threaddelay')"><span class="value-display" id="threaddelay_v">0</span></div>
                    <div class="control-group"><span class="label">Stream Mode</span><label class="switch"><input type="checkbox" id="capture" onchange="update('capture')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Particles</span><label class="switch"><input type="checkbox" id="particles" onchange="update('particles')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Second Monitor</span><label class="switch"><input type="checkbox" id="secmon" onchange="update('secmon')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Watermark</span><label class="switch"><input type="checkbox" id="watermark" onchange="update('watermark')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Language</span><select id="language" onchange="update('language')" style="background:#202020;color:#e0e0e0;border:1px solid #404040;border-radius:4px;padding:2px 6px;"><option value="0">English</option><option value="1">Português</option></select></div>
                </div>
                <div class="subsection">
                    <div class="subsection-title">Trolls</div>
                    <div class="control-group"><span class="label">Vehicle Grab</span><label class="switch"><input type="checkbox" id="troll_grab" onchange="update('troll_grab')"><span class="slider"></span></label></div>
                    <div class="control-group"><span class="label">Hold Distance <small>(1-15m)</small></span><input type="range" id="troll_holddist" min="1" max="15" oninput="showVal(this,'troll_holddist_v')" onchange="update('troll_holddist')"><span class="value-display" id="troll_holddist_v">0</span></div>
                    <div class="control-group"><span class="label">Hold Height <small>(-5-10m)</small></span><input type="range" id="troll_holdht" min="-5" max="10" oninput="showVal(this,'troll_holdht_v')" onchange="update('troll_holdht')"><span class="value-display" id="troll_holdht_v">0</span></div>
                    <div class="control-group"><span class="label">Throw Force <small>(1-200)</small></span><input type="range" id="troll_force" min="1" max="200" oninput="showVal(this,'troll_force_v')" onchange="update('troll_force')"><span class="value-display" id="troll_force_v">0</span></div>
                </div>
            </div>
        </div>

        <button class="refresh-btn" onclick="refresh()">Refresh State</button>
        <div class="status-bar" id="status">Connected</div>
    </div>

    <script>
        var BASE = window.location.pathname.replace(/\/+$/, '');
        function toggleSection(el) {
            el.classList.toggle('collapsed');
            var content = el.nextElementSibling;
            content.classList.toggle('collapsed');
        }
        function showVal(el, id) {
            document.getElementById(id).innerText = el.value;
        }
        function hexToRgb(hex) {
            var r = parseInt(hex.substring(1,3), 16) / 255;
            var g = parseInt(hex.substring(3,5), 16) / 255;
            var b = parseInt(hex.substring(5,7), 16) / 255;
            return {r:r, g:g, b:b};
        }
        function updateColor(key, hex) {
            var rgb = hexToRgb(hex);
            fetch(BASE + '/api/options', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ [key]: [rgb.r, rgb.g, rgb.b, 1.0] })
            }).then(function() { showStatus('Color updated!'); });
        }
        function showStatus(msg) {
            document.getElementById('status').innerText = msg;
            document.getElementById('status').style.color = '#FF0033';
            setTimeout(function() { document.getElementById('status').style.color = '#82828C'; }, 2000);
        }
        function refresh() {
            fetch(BASE + '/api/options')
                .then(function(r) { return r.json(); })
                .then(function(data) {
                    var map = {
                        'aimdead':'legitbot_aimdead',
                        'aimbot_enabled':'legitbot_aimbot_enabled','aimbot_npc':'legitbot_aimbot_npc','aimbot_visible':'legitbot_aimbot_visible',
                        'aimbot_smoothx':'legitbot_aimbot_smoothx','aimbot_smoothy':'legitbot_aimbot_smoothy','aimbot_dist':'legitbot_aimbot_dist',
                        'aimbot_fov':'legitbot_aimbot_fov','aimbot_hitbox':'legitbot_aimbot_hitbox','aimbot_showfov':'legitbot_aimbot_showfov',
                        'silent_enabled':'legitbot_silent_enabled','silent_npc':'legitbot_silent_npc','silent_visible':'legitbot_silent_visible',
                        'silent_dist':'legitbot_silent_dist','silent_fov':'legitbot_silent_fov','silent_hitbox':'legitbot_silent_hitbox',
                        'silent_magic':'legitbot_silent_magic','silent_miss':'legitbot_silent_miss','silent_showfov':'legitbot_silent_showfov',
                        'trigger_enabled':'legitbot_trigger_enabled','trigger_npc':'legitbot_trigger_npc','trigger_visible':'legitbot_trigger_visible',
                        'trigger_smart':'legitbot_trigger_smart','trigger_dist':'legitbot_trigger_dist','trigger_fov':'legitbot_trigger_fov',
                        'trigger_reaction':'legitbot_trigger_reaction','trigger_showfov':'legitbot_trigger_showfov',
                        'esp_players':'visuals_esp_players','esp_local':'visuals_esp_local','esp_npcs':'visuals_esp_npcs',
                        'esp_visible':'visuals_esp_visible','esp_showdead':'visuals_esp_showdead','esp_distance':'visuals_esp_distance',
                        'esp_name':'visuals_esp_name','esp_weapon':'visuals_esp_weapon','esp_skeleton':'visuals_esp_skeleton',
                        'esp_headcircle':'visuals_esp_headcircle','esp_armor':'visuals_esp_armor','esp_health':'visuals_esp_health',
                        'esp_rgb':'visuals_esp_rgb','esp_rgbspeed':'visuals_esp_rgbspeed','esp_disttxt':'visuals_esp_disttxt','esp_box':'visuals_esp_box',
                        'esp_snaplines':'visuals_esp_snaplines',
                        'esp_hlvis':'visuals_esp_hlvis',
                        'esp_vehicles':'visuals_esp_vehicles','esp_vehdist':'visuals_esp_vehdist','esp_vehname':'visuals_esp_vehname',
                        'esp_vehdisttxt':'visuals_esp_vehdisttxt','esp_vehdoor':'visuals_esp_vehdoor','esp_vehsnap':'visuals_esp_vehsnap',
                        'esp_vehlock':'visuals_esp_vehlock',
                        'godmode':'misc_godmode','noclip':'misc_noclip','noclipspeed':'misc_noclipspeed',
                        'infstamina':'misc_infstamina','infroll':'misc_infroll','fastrun':'misc_fastrun',
                        'fastrunspeed':'misc_fastrunspeed','shrink':'misc_shrink','anti-aim':'misc_anti-aim',
                        'inf_ammo':'misc_infammo','noreload':'misc_noreload','nospread':'misc_nospread',
                        'norecoil':'misc_norecoil',
                        'weaponscale':'misc_weaponscale','weaponscaleval':'misc_weaponscaleval',
                        'vehgod':'misc_vehgod','seatbelt':'misc_seatbelt','vehboost':'misc_vehboost',
                        'vehbooststr':'misc_vehbooststr','vehboostval':'misc_vehboostval',
                        'safemode':'general_safemode','activefeat':'general_activefeat','webremote':'general_webremote',
                        'menukey':'general_menukey','threaddelay':'general_threaddelay','capture':'general_capture',
                        'particles':'general_particles','secmon':'general_secmon',
                        'watermark':'general_watermark',
                        'language':'general_language',
                        'troll_grab':'troll_grab','troll_holddist':'troll_holddist',
                        'troll_holdht':'troll_holdht','troll_force':'troll_force'
                    };
                    for (var id in map) {
                        var el = document.getElementById(id);
                        if (!el) continue;
                        var val = data[map[id]];
                        if (val === undefined) continue;
                        if (el.type === 'checkbox') el.checked = val;
                        else if (el.type === 'range') { el.value = val; var vid = id + '_v'; var ve = document.getElementById(vid); if(ve) ve.innerText = val; }
                        else el.value = val;
                    }
                    document.getElementById('status').innerText = 'Last updated: ' + new Date().toLocaleTimeString();
                    document.getElementById('status').style.color = '#82828C';
                })
                .catch(function(e) {
                    document.getElementById('status').innerText = 'Connection Error!';
                    document.getElementById('status').style.color = '#DC5050';
                });
        }
        function update(key) {
            var data = {};
            var el = document.getElementById(key);
            if (el.type === 'checkbox') data[key] = el.checked;
            else if (el.type === 'range') data[key] = parseInt(el.value);
            else data[key] = parseInt(el.value);
            fetch(BASE + '/api/options', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(data)
            }).then(function() { showStatus('Setting updated!'); });
        }
        refresh();
        setInterval(refresh, 2000);
    </script>
</body>
</html>
)HTML"
R"HTML(
)HTML"
R"HTML(
)HTML";

    void WebRemote::Start()
    {
        if (m_Running) return;
        m_Running = true;

        std::string name = Security::CurrentLicense.username;
        if (name.empty())
            m_Token = "access";
        else
            m_Token = name;

        m_Thread = std::thread(ServerThread);
    }

    void WebRemote::Stop()
    {
        m_Running = false;
        if (m_Thread.joinable())
            m_Thread.detach();
    }

    bool WebRemote::IsRunning()
    {
        return m_Running;
    }

    std::string WebRemote::GetURL()
    {
        if (!m_Running) return "";

        char name[256];
        if (gethostname(name, sizeof(name)) == SOCKET_ERROR)
            return "http://127.0.0.1:" + std::to_string(g_Options.General.WebRemotePort) + "/" + m_Token;

        struct hostent* host = gethostbyname(name);
        if (!host)
            return "http://127.0.0.1:" + std::to_string(g_Options.General.WebRemotePort) + "/" + m_Token;

        struct in_addr addr;
        memcpy(&addr, host->h_addr_list[0], sizeof(struct in_addr));
        
        return "http://" + std::string(inet_ntoa(addr)) + ":" + std::to_string(g_Options.General.WebRemotePort) + "/" + m_Token;
    }

    void WebRemote::ServerThread()
    {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return;

        SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocket == INVALID_SOCKET) { WSACleanup(); return; }

        u_long mode = 1;
        ioctlsocket(listenSocket, FIONBIO, &mode);

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons((u_short)g_Options.General.WebRemotePort);

        if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            closesocket(listenSocket);
            WSACleanup();
            return;
        }

        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
        {
            closesocket(listenSocket);
            WSACleanup();
            return;
        }

        while (m_Running)
        {
            SOCKET clientSocket = accept(listenSocket, NULL, NULL);
            if (clientSocket == INVALID_SOCKET)
            {
                if (WSAGetLastError() == WSAEWOULDBLOCK)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
                break;
            }

            DWORD timeout = 2000;
            setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

            char buffer[8192] = { 0 };
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived > 0)
            {
                std::string request(buffer, bytesReceived);
                std::string response = HandleRequest(request);
                send(clientSocket, response.c_str(), (int)response.length(), 0);
            }
            closesocket(clientSocket);
        }

        closesocket(listenSocket);
        WSACleanup();
    }

    std::string WebRemote::HandleRequest(const std::string& request)
    {
        std::string response;
        std::string tokenPath = "/" + m_Token;

        // require token in all requests: look for "GET /token" or "POST /token"
        size_t methodEnd = request.find(' ');
        if (methodEnd == std::string::npos)
            return "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";

        std::string method = request.substr(0, methodEnd);
        std::string path;

        size_t pathStart = request.find(' ', methodEnd + 1);
        if (pathStart != std::string::npos)
            path = request.substr(methodEnd + 1, pathStart - methodEnd - 1);


        if (path.find(tokenPath) != 0)
            return "HTTP/1.1 401 Unauthorized\r\nContent-Length: 0\r\n\r\n";

        std::string rest = path.substr(tokenPath.length());

        if (rest.find("/api/options") == 0)
        {
            if (method == "POST")
            {
                size_t bodyPos = request.find("\r\n\r\n");
                if (bodyPos != std::string::npos)
                {
                    std::string body = request.substr(bodyPos + 4);
                    UpdateOptionsFromJson(body);
                }
                response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nAccess-Control-Allow-Origin: *\r\n\r\n";
            }
            else
            {
                std::string jsonStr = GetOptionsJson();
                response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " + std::to_string(jsonStr.length()) + "\r\nAccess-Control-Allow-Origin: *\r\n\r\n" + jsonStr;
            }
        }
        else
        {
            std::string html = HTML_CONTENT;
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(html.length()) + "\r\n\r\n" + html;
        }
        return response;
    }

    std::string WebRemote::GetOptionsJson()
    {
        json j;
        // LegitBot - shared
        j["legitbot_aimdead"] = g_Options.LegitBot.TargetDead;
        // LegitBot - Aimbot
        j["legitbot_aimbot_enabled"] = g_Options.LegitBot.AimBot.Enabled;
        j["legitbot_aimbot_npc"] = g_Options.LegitBot.AimBot.TargetNPC;
        j["legitbot_aimbot_visible"] = g_Options.LegitBot.AimBot.VisibleCheck;
        j["legitbot_aimbot_smoothx"] = g_Options.LegitBot.AimBot.SmoothHorizontal;
        j["legitbot_aimbot_smoothy"] = g_Options.LegitBot.AimBot.SmoothVertical;
        j["legitbot_aimbot_dist"] = g_Options.LegitBot.AimBot.MaxDistance;
        j["legitbot_aimbot_fov"] = g_Options.LegitBot.AimBot.FOV;
        j["legitbot_aimbot_hitbox"] = g_Options.LegitBot.AimBot.HitBox;
        j["legitbot_aimbot_showfov"] = g_Options.Misc.Screen.ShowAimbotFov;
        // LegitBot - Silent
        j["legitbot_silent_enabled"] = g_Options.LegitBot.SilentAim.Enabled;
        j["legitbot_silent_npc"] = g_Options.LegitBot.SilentAim.ShotNPC;
        j["legitbot_silent_visible"] = g_Options.LegitBot.SilentAim.VisibleCheck;
        j["legitbot_silent_dist"] = g_Options.LegitBot.SilentAim.MaxDistance;
        j["legitbot_silent_fov"] = g_Options.LegitBot.SilentAim.Fov;
        j["legitbot_silent_hitbox"] = g_Options.LegitBot.SilentAim.HitBox;
        j["legitbot_silent_magic"] = g_Options.LegitBot.MagicBullet.Enabled;
        j["legitbot_silent_miss"] = g_Options.LegitBot.SilentAim.MissChance;
        j["legitbot_silent_showfov"] = g_Options.LegitBot.SilentAim.ShowFov;
        // LegitBot - Trigger
        j["legitbot_trigger_enabled"] = g_Options.LegitBot.Trigger.Enabled;
        j["legitbot_trigger_npc"] = g_Options.LegitBot.Trigger.ShotNPC;
        j["legitbot_trigger_visible"] = g_Options.LegitBot.Trigger.VisibleCheck;
        j["legitbot_trigger_smart"] = g_Options.LegitBot.Trigger.SmartTrigger;
        j["legitbot_trigger_dist"] = g_Options.LegitBot.Trigger.MaxDistance;
        j["legitbot_trigger_fov"] = g_Options.LegitBot.Trigger.Fov;
        j["legitbot_trigger_reaction"] = g_Options.LegitBot.Trigger.ReactionTime;
        j["legitbot_trigger_showfov"] = g_Options.LegitBot.Trigger.ShowFov;
        // Visuals - Players
        j["visuals_esp_players"] = g_Options.Visuals.ESP.Players.Enabled;
        j["visuals_esp_local"] = g_Options.Visuals.ESP.Players.ShowLocalPlayer;
        j["visuals_esp_npcs"] = g_Options.Visuals.ESP.Players.ShowNPCs;
        j["visuals_esp_visible"] = g_Options.Visuals.ESP.Players.VisibleOnly;
        j["visuals_esp_showdead"] = g_Options.Visuals.ESP.Players.ExcludeDeads;
        j["visuals_esp_distance"] = g_Options.Visuals.ESP.Players.RenderDistance;
        j["visuals_esp_name"] = g_Options.Visuals.ESP.Players.Name;
        j["visuals_esp_weapon"] = g_Options.Visuals.ESP.Players.WeaponName;
        j["visuals_esp_skeleton"] = g_Options.Visuals.ESP.Players.Skeleton;
        j["visuals_esp_headcircle"] = g_Options.Visuals.ESP.Players.HeadCircle;
        j["visuals_esp_armor"] = g_Options.Visuals.ESP.Players.ArmorBar;
        j["visuals_esp_health"] = g_Options.Visuals.ESP.Players.HealthBar;
        j["visuals_esp_rgb"] = g_Options.Visuals.ESP.Players.RGB;
        j["visuals_esp_rgbspeed"] = (int)(g_Options.Visuals.ESP.Players.RGBSpeed * 10.0f);
        j["visuals_esp_disttxt"] = g_Options.Visuals.ESP.Players.Distance;
        j["visuals_esp_box"] = g_Options.Visuals.ESP.Players.Box;
        j["visuals_esp_snaplines"] = g_Options.Visuals.ESP.Players.SnapLines;
        j["visuals_esp_hlvis"] = g_Options.Visuals.ESP.Players.HighlightVisible;
        // Visuals - Vehicles
        j["visuals_esp_vehicles"] = g_Options.Visuals.ESP.Vehicles.Enabled;
        j["visuals_esp_vehdist"] = g_Options.Visuals.ESP.Vehicles.RenderDistance;
        j["visuals_esp_vehname"] = g_Options.Visuals.ESP.Vehicles.Model;
        j["visuals_esp_vehdisttxt"] = g_Options.Visuals.ESP.Vehicles.Distance;
        j["visuals_esp_vehdoor"] = g_Options.Visuals.ESP.Vehicles.Door;
        j["visuals_esp_vehsnap"] = g_Options.Visuals.ESP.Vehicles.SnapLines;
        j["visuals_esp_vehlock"] = g_Options.Visuals.ESP.Vehicles.LockStatus;
        // Misc - Player
        j["misc_godmode"] = g_Options.Misc.Exploits.LocalPlayer.GodMode;
        j["misc_noclip"] = g_Options.Misc.Exploits.LocalPlayer.Noclip;
        j["misc_noclipspeed"] = g_Options.Misc.Exploits.LocalPlayer.NoClipSpeed;
        j["misc_infstamina"] = g_Options.Misc.Exploits.LocalPlayer.InfiniteStamina;
        j["misc_infroll"] = g_Options.Misc.Exploits.LocalPlayer.InfiniteCombatRoll;
        j["misc_fastrun"] = g_Options.Misc.Exploits.LocalPlayer.FastRun;
        j["misc_fastrunspeed"] = g_Options.Misc.Exploits.LocalPlayer.RunSpeed;
        j["misc_shrink"] = g_Options.Misc.Exploits.LocalPlayer.Shrink;
        j["misc_anti-aim"] = g_Options.Misc.Exploits.LocalPlayer.AntiAimEnabled;
        // Misc - Weapons
        j["misc_infammo"] = g_Options.Misc.Exploits.Weapon.InfiniteAmmoEnabled;
        j["misc_noreload"] = g_Options.Misc.Exploits.Weapon.NoReload;
        j["misc_nospread"] = g_Options.Misc.Exploits.Weapon.RemoveSpread;
        j["misc_norecoil"] = g_Options.Misc.Exploits.Weapon.RemoveRecoil;
        j["misc_weaponscale"] = g_Options.Misc.Exploits.Weapon.WeaponScaleEnabled;
        j["misc_weaponscaleval"] = (int)(g_Options.Misc.Exploits.Weapon.WeaponScale * 10.0f);

        // Misc - Vehicle
        j["misc_vehgod"] = g_Options.Misc.Exploits.Vehicle.GodMode;
        j["misc_seatbelt"] = g_Options.Misc.Exploits.LocalPlayer.SeatBelt;
        j["misc_vehboost"] = g_Options.Misc.Exploits.Vehicle.RocketBoost;
        j["misc_vehbooststr"] = (int)g_Options.Misc.Exploits.Vehicle.RocketBoostStrength;
        j["misc_vehboostval"] = (int)g_Options.Misc.Exploits.LocalPlayer.v_Boost;
        // General
        j["general_safemode"] = g_Options.General.SafeMode;
        j["general_activefeat"] = g_Options.Misc.ShowActiveFeaturesOverlay;
        j["general_webremote"] = g_Options.General.WebRemoteEnabled;
        j["general_menukey"] = g_Options.General.MenuKey;
        j["general_threaddelay"] = g_Options.General.ThreadDelay;
        j["general_capture"] = g_Options.General.CaptureBypass;
        j["general_particles"] = g_Options.General.Particles;
        j["general_secmon"] = g_Options.General.EspOnSecondaryMonitor;
        j["general_watermark"] = g_Options.General.WaterMark;
        j["general_language"] = g_Options.General.Language;
        // Trolls
        j["troll_grab"] = g_Options.Misc.Trolls.VehicleGrabEnabled;
        j["troll_holddist"] = (int)g_Options.Misc.Trolls.HoldDistance;
        j["troll_holdht"] = (int)g_Options.Misc.Trolls.HoldHeight;
        j["troll_force"] = (int)g_Options.Misc.Trolls.ThrowForce;
        return j.dump();
    }

    void WebRemote::UpdateOptionsFromJson(const std::string& json_str)
    {
        try
        {
            json j = json::parse(json_str);
            // LegitBot - shared
            if (j.contains("aimdead")) g_Options.LegitBot.TargetDead = j["aimdead"];
            // Aimbot
            if (j.contains("aimbot_enabled")) g_Options.LegitBot.AimBot.Enabled = j["aimbot_enabled"];
            if (j.contains("aimbot_npc")) g_Options.LegitBot.AimBot.TargetNPC = j["aimbot_npc"];
            if (j.contains("aimbot_visible")) g_Options.LegitBot.AimBot.VisibleCheck = j["aimbot_visible"];
            if (j.contains("aimbot_smoothx")) g_Options.LegitBot.AimBot.SmoothHorizontal = j["aimbot_smoothx"];
            if (j.contains("aimbot_smoothy")) g_Options.LegitBot.AimBot.SmoothVertical = j["aimbot_smoothy"];
            if (j.contains("aimbot_dist")) g_Options.LegitBot.AimBot.MaxDistance = j["aimbot_dist"];
            if (j.contains("aimbot_fov")) g_Options.LegitBot.AimBot.FOV = j["aimbot_fov"];
            if (j.contains("aimbot_hitbox")) g_Options.LegitBot.AimBot.HitBox = j["aimbot_hitbox"];
            if (j.contains("aimbot_showfov")) g_Options.Misc.Screen.ShowAimbotFov = j["aimbot_showfov"];
            // Silent
            if (j.contains("silent_enabled")) g_Options.LegitBot.SilentAim.Enabled = j["silent_enabled"];
            if (j.contains("silent_npc")) g_Options.LegitBot.SilentAim.ShotNPC = j["silent_npc"];
            if (j.contains("silent_visible")) g_Options.LegitBot.SilentAim.VisibleCheck = j["silent_visible"];
            if (j.contains("silent_dist")) g_Options.LegitBot.SilentAim.MaxDistance = j["silent_dist"];
            if (j.contains("silent_fov")) g_Options.LegitBot.SilentAim.Fov = j["silent_fov"];
            if (j.contains("silent_hitbox")) g_Options.LegitBot.SilentAim.HitBox = j["silent_hitbox"];
            if (j.contains("silent_magic")) g_Options.LegitBot.MagicBullet.Enabled = j["silent_magic"];
            if (j.contains("silent_miss")) g_Options.LegitBot.SilentAim.MissChance = j["silent_miss"];
            if (j.contains("silent_showfov")) g_Options.LegitBot.SilentAim.ShowFov = j["silent_showfov"];
            // Trigger
            if (j.contains("trigger_enabled")) g_Options.LegitBot.Trigger.Enabled = j["trigger_enabled"];
            if (j.contains("trigger_npc")) g_Options.LegitBot.Trigger.ShotNPC = j["trigger_npc"];
            if (j.contains("trigger_visible")) g_Options.LegitBot.Trigger.VisibleCheck = j["trigger_visible"];
            if (j.contains("trigger_smart")) g_Options.LegitBot.Trigger.SmartTrigger = j["trigger_smart"];
            if (j.contains("trigger_dist")) g_Options.LegitBot.Trigger.MaxDistance = j["trigger_dist"];
            if (j.contains("trigger_fov")) g_Options.LegitBot.Trigger.Fov = j["trigger_fov"];
            if (j.contains("trigger_reaction")) g_Options.LegitBot.Trigger.ReactionTime = j["trigger_reaction"];
            if (j.contains("trigger_showfov")) g_Options.LegitBot.Trigger.ShowFov = j["trigger_showfov"];
            // Visuals - Players
            if (j.contains("esp_players")) g_Options.Visuals.ESP.Players.Enabled = j["esp_players"];
            if (j.contains("esp_local")) g_Options.Visuals.ESP.Players.ShowLocalPlayer = j["esp_local"];
            if (j.contains("esp_npcs")) g_Options.Visuals.ESP.Players.ShowNPCs = j["esp_npcs"];
            if (j.contains("esp_visible")) g_Options.Visuals.ESP.Players.VisibleOnly = j["esp_visible"];
            if (j.contains("esp_showdead")) g_Options.Visuals.ESP.Players.ExcludeDeads = j["esp_showdead"];
            if (j.contains("esp_distance")) g_Options.Visuals.ESP.Players.RenderDistance = j["esp_distance"];
            if (j.contains("esp_name")) g_Options.Visuals.ESP.Players.Name = j["esp_name"];
            if (j.contains("esp_weapon")) g_Options.Visuals.ESP.Players.WeaponName = j["esp_weapon"];
            if (j.contains("esp_skeleton")) g_Options.Visuals.ESP.Players.Skeleton = j["esp_skeleton"];
            if (j.contains("esp_headcircle")) g_Options.Visuals.ESP.Players.HeadCircle = j["esp_headcircle"];
            if (j.contains("esp_armor")) g_Options.Visuals.ESP.Players.ArmorBar = j["esp_armor"];
            if (j.contains("esp_health")) g_Options.Visuals.ESP.Players.HealthBar = j["esp_health"];
            if (j.contains("esp_rgb")) g_Options.Visuals.ESP.Players.RGB = j["esp_rgb"];
            if (j.contains("esp_rgbspeed")) g_Options.Visuals.ESP.Players.RGBSpeed = (float)(int)j["esp_rgbspeed"] / 10.0f;
            if (j.contains("esp_disttxt")) g_Options.Visuals.ESP.Players.Distance = j["esp_disttxt"];
            if (j.contains("esp_box")) g_Options.Visuals.ESP.Players.Box = j["esp_box"];
            if (j.contains("esp_snaplines")) g_Options.Visuals.ESP.Players.SnapLines = j["esp_snaplines"];
            if (j.contains("esp_hlvis")) g_Options.Visuals.ESP.Players.HighlightVisible = j["esp_hlvis"];
            // Visuals - Vehicles
            if (j.contains("esp_vehicles")) g_Options.Visuals.ESP.Vehicles.Enabled = j["esp_vehicles"];
            if (j.contains("esp_vehdist")) g_Options.Visuals.ESP.Vehicles.RenderDistance = j["esp_vehdist"];
            if (j.contains("esp_vehname")) g_Options.Visuals.ESP.Vehicles.Model = j["esp_vehname"];
            if (j.contains("esp_vehdisttxt")) g_Options.Visuals.ESP.Vehicles.Distance = j["esp_vehdisttxt"];
            if (j.contains("esp_vehdoor")) g_Options.Visuals.ESP.Vehicles.Door = j["esp_vehdoor"];
            if (j.contains("esp_vehsnap")) g_Options.Visuals.ESP.Vehicles.SnapLines = j["esp_vehsnap"];
            if (j.contains("esp_vehlock")) g_Options.Visuals.ESP.Vehicles.LockStatus = j["esp_vehlock"];
            // Misc - Player
            if (j.contains("godmode")) g_Options.Misc.Exploits.LocalPlayer.GodMode = j["godmode"];
            if (j.contains("noclip")) g_Options.Misc.Exploits.LocalPlayer.Noclip = j["noclip"];
            if (j.contains("noclipspeed")) g_Options.Misc.Exploits.LocalPlayer.NoClipSpeed = (float)j["noclipspeed"];
            if (j.contains("infstamina")) g_Options.Misc.Exploits.LocalPlayer.InfiniteStamina = j["infstamina"];
            if (j.contains("infroll")) g_Options.Misc.Exploits.LocalPlayer.InfiniteCombatRoll = j["infroll"];
            if (j.contains("fastrun")) g_Options.Misc.Exploits.LocalPlayer.FastRun = j["fastrun"];
            if (j.contains("fastrunspeed")) g_Options.Misc.Exploits.LocalPlayer.RunSpeed = (float)j["fastrunspeed"];
            if (j.contains("shrink")) g_Options.Misc.Exploits.LocalPlayer.Shrink = j["shrink"];
            if (j.contains("anti-aim")) g_Options.Misc.Exploits.LocalPlayer.AntiAimEnabled = j["anti-aim"];
            // Misc - Weapons
            if (j.contains("inf_ammo")) g_Options.Misc.Exploits.Weapon.InfiniteAmmoEnabled = j["inf_ammo"];
            if (j.contains("noreload")) g_Options.Misc.Exploits.Weapon.NoReload = j["noreload"];
            if (j.contains("nospread")) g_Options.Misc.Exploits.Weapon.RemoveSpread = j["nospread"];
            if (j.contains("norecoil")) g_Options.Misc.Exploits.Weapon.RemoveRecoil = j["norecoil"];
            if (j.contains("weaponscale")) g_Options.Misc.Exploits.Weapon.WeaponScaleEnabled = j["weaponscale"];
            if (j.contains("weaponscaleval")) g_Options.Misc.Exploits.Weapon.WeaponScale = (float)(int)j["weaponscaleval"] / 10.0f;

            // Misc - Vehicle
            if (j.contains("vehgod")) g_Options.Misc.Exploits.Vehicle.GodMode = j["vehgod"];
            if (j.contains("seatbelt")) g_Options.Misc.Exploits.LocalPlayer.SeatBelt = j["seatbelt"];
            if (j.contains("vehboost")) g_Options.Misc.Exploits.Vehicle.RocketBoost = j["vehboost"];
            if (j.contains("vehbooststr")) g_Options.Misc.Exploits.Vehicle.RocketBoostStrength = (float)(int)j["vehbooststr"];
            if (j.contains("vehboostval")) g_Options.Misc.Exploits.LocalPlayer.v_Boost = (float)(int)j["vehboostval"];
            // General
            if (j.contains("safemode")) g_Options.General.SafeMode = j["safemode"];
            if (j.contains("activefeat")) g_Options.Misc.ShowActiveFeaturesOverlay = j["activefeat"];
            if (j.contains("webremote")) g_Options.General.WebRemoteEnabled = j["webremote"];
            if (j.contains("menukey")) g_Options.General.MenuKey = j["menukey"];
            if (j.contains("threaddelay")) g_Options.General.ThreadDelay = j["threaddelay"];
            if (j.contains("capture")) g_Options.General.CaptureBypass = j["capture"];
            if (j.contains("particles")) g_Options.General.Particles = j["particles"];
            if (j.contains("secmon")) g_Options.General.EspOnSecondaryMonitor = j["secmon"];
            if (j.contains("watermark")) g_Options.General.WaterMark = j["watermark"];
            if (j.contains("language")) g_Options.General.Language = j["language"];
            // Trolls
            if (j.contains("troll_grab")) g_Options.Misc.Trolls.VehicleGrabEnabled = j["troll_grab"];
            if (j.contains("troll_holddist")) g_Options.Misc.Trolls.HoldDistance = (float)(int)j["troll_holddist"];
            if (j.contains("troll_holdht")) g_Options.Misc.Trolls.HoldHeight = (float)(int)j["troll_holdht"];
            if (j.contains("troll_force")) g_Options.Misc.Trolls.ThrowForce = (float)(int)j["troll_force"];
        }
        catch (...) {}
    }
}
