# LMLUtils — Agent Guide

## Build
- **VS 2022** (v143), C++17 (`stdcpp17`), Windows 10 SDK, x64 Debug/Release
- Open `LMLUtils.sln`, build, run as Administrator (Debug: `HighestAvailable`, Release: `RequireAdministrator`)
- No tests, no linter, no CI

## Entrypoint
`LMLUtils.cpp:62` → `wWinMain()` → `MainThread()` → `AntiCrack::Initialize()` → `Cheat::Initialize()` → message loop → `Cheat::ShutDown()` → `TerminateProcess()`. Fatal termination on shutdown is by design.

## Key Architecture
- **Rendering**: Custom ImGui fork in `FrameWork/Dependencies/ImGui/` — `imgui.cpp` (edited), `imguiedited.cpp` (custom combo/widgets), `imgui_settomgs.h` (color palette in `c::` namespace)
- **Palette in code**: `c::bg::filling`, `c::child::filling`, `c::element::text`, etc. — see `imgui_settomgs.h:30-105`
- **Layout constants** (hardcoded in `Interface.cpp`): sidebar=155px, top bar=64px, gaps=15px
- **Config**: `config.json` in working directory (relative path, `ConfigSystem::SaveToFile` writes `config.json` at cwd). Config slot pattern: `config_<name>.json`. 177 unique items; some in the file are legacy/unused.
- **Language**: Dual EN/PT via `Language::Translations` map. Controlled by `g_Options.General.Language` (0=EN,1=PT). Uses `_T(text)` macro for translations
- **Auth**: KeyAuth v1.3 (`Security/KeyAuth.hpp:159-161`). `ownerid="Iwdu7Lrjv1"`, `secret="027905926a62c53716a0a3786f9fca4e4a77bce5391d915ded2eca3c53d07bb3"`. Flow: `type=init` → sessionid → `type=license` with HWID
- **Persistent login**: key saved to `%appdata%/ScarfaceX/login.key`, auto-loaded on startup
- **SDK**: FiveM memory reading via `FivemSDK/Fivem.cpp` (~1844 lines). Both `GetBonePosByInstFragAndID` and `GetBonePosFromCPed` exist but combat only uses the former
- **Options**: `Cheat/Options.hpp` — single `g_Options` global of struct `Cheat::Options` with nested structs
- **ProcessPriority**: 0=Normal, 1=AboveNormal, 2=High, 3=Realtime (set via `SetProcessPriority()` in `LMLUtils.cpp:28`)
- **FPS cap**: 144 when menu open, 240 when closed (`Cheat.cpp:481`)
- **Fatal exit**: On FiveM process death detection or `WM_QUIT`, `ShutDown()` runs then `TerminateProcess()` kills the cheat process

## Critical Source Files
| File | Purpose |
|------|---------|
| `Cheat/Options.hpp` | All toggle/variable definitions (`g_Options`) |
| `FrameWork/Render/Interface.cpp` (~2102 lines) | Main UI rendering |
| `FrameWork/Dependencies/ImGui/imgui_settomgs.h` | Color palette `c::` namespace |
| `FrameWork/Dependencies/ImGui/imgui.cpp` (~line 5450) | `CustomChild` widget with hardcoded colors |
| `FrameWork/Dependencies/ImGui/imguiedited.cpp` | Custom combo/input widgets |
| `Cheat/ConfigSystem.cpp` / `ConfigSystem.hpp` | Config save/load with 8 type overloads |
| `Security/KeyAuth.hpp` | KeyAuth v1.3 WinHTTP auth |
| `Cheat/FivemSDK/Fivem.cpp` | Memory SDK (~1844 lines) |
| `FrameWork/includes/Language.hpp` | EN/PT translation map |
| `Cheat/WebRemote.cpp` | Embedded HTTP server for web remote control |
| `FrameWork/Render/SidebarLogo.hpp` | Logo byte array |
| `FrameWork/Render/Logo.hpp` | Login screen logo byte array |
| `game.hpp` | Empty file (exists but has no content) |

## Combat
- Hitbox offsets: Head `z += 0.04f`, Neck `SKEL_Neck_1`, Chest `SKEL_Spine3`
- No Recoil: written to `weaponInfo + 0x2E8` (hardcoded in `Weapon/Exploits.cpp:57`)
- No Spread: `Offsets::m_Spread` is resolved dynamically via pattern scan in `Fivem.cpp:747`
- `Weapon/Exploits.cpp::RunTick()` called from Exploits thread

## ESP
- Max player render distance defaults to 200m (configurable via `g_Options.Visuals.ESP.Players.RenderDistance`)
- Head circle: center at head bone + 0.04f Z, radius = box Height/15
- RGB Mode in `Options.hpp:115-116` (`Players::RGB`, `Players::RGBSpeed`)

## Config System
- 8 overloads: `int*`, `float*`, `bool*`, `float[4]*`, `float[2]*`, `char[64]*`, `uint64_t*`, `std::string*`
- Slots: `ListSlots()` / `SaveSlot()` / `LoadSlot()` / `DeleteSlot()` — files named `config_<name>.json`
- `Setup()` called from `ConfigSystem.cpp` registers ~154 items (config.json has 177 unique items; leftovers from prior versions)

## Other Notable Files
- `tutorial.md` at `LMLUtils/` — guide for setting up the companion Node.js auth panel
- `NoclipCar.txt`, `PullVehicle.txt` at repo root — reference snippets for feature implementation
- `pattern_scanner.exe` at repo root — standalone offset scanner tool

## Shellcode Infrastructure
Two shellcode variants are injected into GTA5.exe via `CreateRemoteThread`:

1. **Raycast shellcode** (`InitRaycastShellcode` in `Fivem.cpp:802`): Calls `WorldProbe` + `GetShapeTestResult` via `__vectorcall`. Params at `m_ParamsAddr` (0x40 bytes). Used by `CheckLOS` / `HasLineOfSight`.

2. **Vehicle spawn shellcode** (`InitSpawnVehicleShellcode` in `Fivem.cpp:944`): Calls GTA V's `CREATE_VEHICLE` function via `__fastcall` with int + float args. Params at `m_SpawnVehParamsAddr` (0x30 bytes layout: modelHash, x, y, z, heading, isNetwork, bScriptHostVeh, funAddr, result). `CreateVehicleAddr` resolved via 3 candidate patterns in `Fivem.cpp:757-779`. `WorldSpawnVehicle` writes params → `CreateRemoteThread` → reads result. `HandleToVehicle` converts the handle to `CVehicle*` via pool index.

## Clone Vehicle
`WorldCloneVehicle` (`Vehicle/Exploits.cpp:26`): Reads source vehicle's model hash from `ModelInfo+0x18`, calls `WorldSpawnVehicle` with position 4.5m in front of the player, copies color ints at offsets 0x94C/0x950/0x954/0x958, and unlocks the new vehicle. If spawning fails (pattern not found), shows error notification — no crash.

## Known Issues / Edge Cases
- **Pull Player freeze**: inherent — FiveM freezes ped when another client takes network control. No fix without avoiding `network_request_control_of_entity`
- `GetBonePosFromCPed` in SDK exists but is unused in combat (only `GetBonePosByInstFragAndID` is used)
- No `/Language` directory — translations are inline in `Language.hpp`
- **ForceWeaponWheel**: **Perpetually broken** on b3570+. All pattern-patching approaches removed due to false-positive crashes (`GTA5_b3570.exe+1F710`). Only `SetConfigFlag(BlockWeaponSwitching, false)` runs per-frame. FiveM handles `DISABLE_CONTROL_ACTION` in its own memory, bypassing GTA5.exe. No fix without runtime RE.
- **Clone Vehicle**: spawns via shellcode calling `CREATE_VEHICLE`. If the pattern scan fails to find the function address, the button shows "Clone failed" notification and does nothing.
