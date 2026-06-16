#include "Cheat.hpp"
#include "WebRemote.hpp"
#include "FivemSDK/Fivem.hpp"
#include <thread>
#include <FrameWork/FrameWork.hpp>
#include <FrameWork/Utilities/Notify.hpp>
#include "ConfigSystem.hpp"
#include <FrameWork/includes/Language.hpp>
#include "Features/Combat/MagicBullet.hpp"

namespace Cheat
{
    struct MonitorInfoEx
    {
        ImVec2 Pos;
        ImVec2 Size;
        bool   Valid = false;
    };

    static MonitorInfoEx g_SecondMonitor;

    BOOL CALLBACK EnumMonitorsProc(HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam)
    {
        auto monitors = reinterpret_cast<std::vector<MONITORINFOEXA>*>(lParam);

        MONITORINFOEXA info{};
        info.cbSize = sizeof(info);
        if (GetMonitorInfoA(hMonitor, &info))
            monitors->push_back(info);

        return TRUE;
    }

    void UpdateSecondMonitor()
    {
        static auto lastUpdate = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if (g_SecondMonitor.Valid && std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdate).count() < 5)
            return;

        lastUpdate = now;

        std::vector<MONITORINFOEXA> monitors;
        EnumDisplayMonitors(nullptr, nullptr, EnumMonitorsProc, (LPARAM)&monitors);

        if (monitors.size() < 2)
        {
            g_SecondMonitor.Valid = false;
            return;
        }

        auto& m = monitors[1];

        g_SecondMonitor.Pos = ImVec2((float)m.rcMonitor.left, (float)m.rcMonitor.top);
        g_SecondMonitor.Size = ImVec2(
            (float)(m.rcMonitor.right - m.rcMonitor.left),
            (float)(m.rcMonitor.bottom - m.rcMonitor.top)
        );

        g_SecondMonitor.Valid = true;
    }

   

     std::vector<std::string> GetMonitorNames()
    {
        std::vector<MONITORINFOEXA> monitors;
        EnumDisplayMonitors(nullptr, nullptr, EnumMonitorsProc, (LPARAM)&monitors);

        std::vector<std::string> names;
        for (size_t i = 0; i < monitors.size(); i++)
        {
            names.push_back("Monitor " + std::to_string(i + 1));
        }

        return names;
    }


    void UpdateWindowPos()
    {
        static int lastW = 0;
        static int lastH = 0;

        HWND hOverlay = FrameWork::Overlay::GetOverlayWindow();
        HWND hTarget = FrameWork::Overlay::GetTargetWindow();
        if (!hOverlay || !hTarget)
            return;

        // Check if game is minimized
        if (IsIconic(hTarget))
        {
            if (IsWindowVisible(hOverlay))
                ShowWindow(hOverlay, SW_HIDE);
            return;
        }

        // Check if game is the foreground window OR menu is open
        HWND foreground = GetForegroundWindow();
        bool isMenuOpen = false;
        // We can't easily check Interface::GetMenuOpen() from here without an instance, 
        // but we can check if the overlay itself is the foreground window.
        if (foreground == hTarget || foreground == hOverlay)
        {
            if (!IsWindowVisible(hOverlay))
                ShowWindow(hOverlay, SW_SHOWNOACTIVATE);
        }
        else
        {
            // If not in focus and not on secondary monitor, hide it
            if (!g_Options.General.EspOnSecondaryMonitor)
            {
                if (IsWindowVisible(hOverlay))
                    ShowWindow(hOverlay, SW_HIDE);
                return;
            }
        }

        ImVec2 pos, size;

        if (g_Options.General.EspOnSecondaryMonitor)
        {
            UpdateSecondMonitor();

            if (!g_SecondMonitor.Valid)
                return;

            pos = g_SecondMonitor.Pos;
            size = g_SecondMonitor.Size;
        }
        else
        {
            RECT rc{};
            GetClientRect(hTarget, &rc);
            MapWindowPoints(hTarget, nullptr, reinterpret_cast<POINT*>(&rc), 2);

            pos = ImVec2((float)rc.left, (float)rc.top);
            size = ImVec2((float)(rc.right - rc.left), (float)(rc.bottom - rc.top));
        }

        int w = (int)size.x;
        int h = (int)size.y;

        // Only move/resize if changed to optimize
        static float lastPosX = -1, lastPosY = -1;
        if (pos.x != lastPosX || pos.y != lastPosY || w != lastW || h != lastH)
        {
            MoveWindow(hOverlay, (int)pos.x, (int)pos.y, w, h, FALSE);
            lastPosX = pos.x;
            lastPosY = pos.y;
        }

        if (w != lastW || h != lastH)
        {
            lastW = w;
            lastH = h;

            FrameWork::Overlay::dxCleanupRenderTarget();
            FrameWork::Overlay::dxGetSwapChain()->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
            FrameWork::Overlay::dxCreateRenderTarget();
        }
    }
    void SwitchOverlayToSecondaryMonitor()
    {
        UpdateSecondMonitor();
        if (!g_SecondMonitor.Valid)
            return;

        HWND hOverlay = FrameWork::Overlay::GetOverlayWindow();
        int w = (int)g_SecondMonitor.Size.x;
        int h = (int)g_SecondMonitor.Size.y;
        MoveWindow(hOverlay, (int)g_SecondMonitor.Pos.x, (int)g_SecondMonitor.Pos.y, w, h, TRUE);

        FrameWork::Overlay::dxCleanupRenderTarget();
        FrameWork::Overlay::dxGetSwapChain()->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
        FrameWork::Overlay::dxCreateRenderTarget();
    }

    void Initialize()
    {
        static bool lastEspOnSecondaryMonitor = false;

        while (!g_Fivem.IsInitialized())
        {
            g_Fivem.Intialize();

            if (!g_Fivem.IsInitialized())
                std::this_thread::sleep_for(std::chrono::seconds(5));
        }


        FrameWork::Overlay::Setup(g_Fivem.GetPid());
        FrameWork::Overlay::Initialize();

        std::thread(TriggerBot::RunThread).detach();
        std::thread(AimBot::RunThread).detach();
        std::thread(SilentAim::RunThread).detach();
		std::thread(Exploits::RunThread).detach();
		std::thread(Trolls::RunThread).detach();
		std::thread(UpdateNames::RunThread).detach();
		std::thread(&ResourceManager::cResourceList::RunThread, &ResourceManager::g_ResourceList).detach();
		std::thread(AntiCrack::RunThread).detach();

		Language::IsPortuguese = (g_Options.General.Language == 1);

		std::thread([]()
            {
                while (!g_Options.General.ShutDown)
                {
                    Exploits::ToggleCarLock();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }
            }).detach();

        std::thread([]()
            {
                while (!g_Options.General.ShutDown)
                {
                    g_Fivem.UpdateEntities();
                    
                    static auto lastUpdate2 = std::chrono::steady_clock::now();
                    auto now2 = std::chrono::steady_clock::now();
                    if (std::chrono::duration_cast<std::chrono::milliseconds>(now2 - lastUpdate2).count() > 150) {
                        g_Fivem.UpdateVehicles();
                        lastUpdate2 = now2;
                    }
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(8));
                }
            }).detach();

        if (FrameWork::Overlay::IsInitialized())
        {
            FrameWork::Interface Interface(
                FrameWork::Overlay::GetOverlayWindow(),
                FrameWork::Overlay::GetTargetWindow(),
                FrameWork::Overlay::dxGetDevice(),
                FrameWork::Overlay::dxGetDeviceContext()
            );

            Interface.UpdateStyle();
            FrameWork::Overlay::SetupWindowProcHook(std::bind(
                &FrameWork::Interface::WindowProc, &Interface,
                std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3, std::placeholders::_4));

            MSG msg{};


            while (msg.message != WM_QUIT)
            {
                auto frame_start = std::chrono::high_resolution_clock::now();

                static auto lastProcessCheck = std::chrono::steady_clock::now();
                auto nowCheck = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::seconds>(nowCheck - lastProcessCheck).count() > 2)
                {
                    DWORD exitCode;
                    if (GetExitCodeProcess(g_Fivem.GetProcHandle(), &exitCode) && exitCode != STILL_ACTIVE)
                    {
                        g_Options.General.ShutDown = true;
                        break;
                    }
                    lastProcessCheck = nowCheck;
                }

                while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                    if (msg.message == WM_QUIT)
                    {
                        g_Options.General.ShutDown = true;
                        break;
                    }
                }

                if (g_Options.General.ShutDown)
                    break;

                ImGui::GetIO().MouseDrawCursor = Interface.GetMenuOpen();




                if (Interface.ResizeHeight != 0 || Interface.ResizeWidht != 0)
                {
                    FrameWork::Overlay::dxCleanupRenderTarget();
                    FrameWork::Overlay::dxGetSwapChain()->ResizeBuffers(
                        0, Interface.ResizeWidht, Interface.ResizeHeight,
                        DXGI_FORMAT_UNKNOWN, 0);
                    Interface.ResizeHeight = Interface.ResizeWidht = 0;
                    FrameWork::Overlay::dxCreateRenderTarget();
                }

                Interface.HandleMenuKey();

                // Web Remote Management
                if (g_Options.General.WebRemoteEnabled && !WebRemote::IsRunning())
                    WebRemote::Start();
                else if (!g_Options.General.WebRemoteEnabled && WebRemote::IsRunning())
                    WebRemote::Stop();

                // Monitor toggle detection (must be before UpdateWindowPos)
                if (g_Options.General.EspOnSecondaryMonitor != lastEspOnSecondaryMonitor)
                {
                    lastEspOnSecondaryMonitor = g_Options.General.EspOnSecondaryMonitor;

                    if (g_Options.General.EspOnSecondaryMonitor)
                    {
                        SwitchOverlayToSecondaryMonitor();
                    }
                    else
                    {
                        HWND hOverlay = FrameWork::Overlay::GetOverlayWindow();
                        HWND hTarget = FrameWork::Overlay::GetTargetWindow();
                        if (hOverlay && hTarget)
                        {
                            RECT rc{};
                            GetClientRect(hTarget, &rc);
                            MapWindowPoints(hTarget, nullptr, reinterpret_cast<POINT*>(&rc), 2);
                            int w = rc.right - rc.left;
                            int h = rc.bottom - rc.top;
                            MoveWindow(hOverlay, rc.left, rc.top, w, h, TRUE);
                            FrameWork::Overlay::dxCleanupRenderTarget();
                            FrameWork::Overlay::dxGetSwapChain()->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
                            FrameWork::Overlay::dxCreateRenderTarget();
                        }
                    }
                }

                if (!g_Options.General.EspOnSecondaryMonitor)
                {
                    UpdateWindowPos();
                }

                // Se a overlay estiver escondida e o menu não estiver aberto, não precisa renderizar
                if (!IsWindowVisible(FrameWork::Overlay::GetOverlayWindow()) && !Interface.GetMenuOpen())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }

                static bool captureBypass = false;
                if (captureBypass != g_Options.General.CaptureBypass)
                {
                    SafeCall(SetWindowDisplayAffinity)(
                        FrameWork::Overlay::GetOverlayWindow(),
                        g_Options.General.CaptureBypass ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE
                        );
                    captureBypass = g_Options.General.CaptureBypass;
                }

                ImGui_ImplDX11_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();

                g_Fivem.UpdateViewMatrix();

                {
                    auto io = ImGui::GetIO();

                    HWND foreground = GetForegroundWindow();
                    bool isFiveMInFocus = (foreground == FrameWork::Overlay::GetTargetWindow() || foreground == FrameWork::Overlay::GetOverlayWindow());
                    NotifyManager::Render();

                    ImVec2 center;

                    HWND hOverlay = FrameWork::Overlay::GetOverlayWindow();
                    RECT rc;
                    GetClientRect(hOverlay, &rc);
                    center = ImVec2(
                        (float)(rc.right - rc.left) * 0.5f,
                        (float)(rc.bottom - rc.top) * 0.5f
                    );


                    if (g_Options.LegitBot.AimBot.Enabled && g_Options.Misc.Screen.ShowAimbotFov)
                    {
                        ImGui::GetBackgroundDrawList()->AddCircle(
                            center,
                            (float)g_Options.LegitBot.AimBot.FOV,
                            FrameWork::Misc::Float4ToImColor(g_Options.Misc.Screen.AimbotFovColor),
                            360, 1.0f
                        );
                    }

                    if (g_Options.LegitBot.Trigger.Enabled && g_Options.LegitBot.Trigger.ShowFov)
                    {
                        float triggerFov = (float)g_Options.LegitBot.Trigger.Fov;
                        ImGui::GetBackgroundDrawList()->AddCircle(
                            center,
                            triggerFov,
                            FrameWork::Misc::Float4ToImColor(g_Options.LegitBot.Trigger.FovColor),
                            360, 1.0f
                        );
                    }

                    if (g_Options.LegitBot.SilentAim.ShowFov && g_Options.LegitBot.SilentAim.Enabled)
                    {
                        ImGui::GetBackgroundDrawList()->AddCircle(
                            center,
                            (float)g_Options.LegitBot.SilentAim.Fov,
                            FrameWork::Misc::Float4ToImColor(g_Options.LegitBot.SilentAim.FovColor),
                            360, 1.0f
                        );
                    }

                    if (isFiveMInFocus)
                    {
      
                        if (g_Options.LegitBot.MagicBullet.Enabled) {
                            Cheat::MagicBullet::RunThread();
                        }
                        

                        if (g_Options.Visuals.ESP.Vehicles.Enabled)
                            ESP::Vehicles();

                        if (g_Options.Visuals.ESP.Players.Enabled)
                            ESP::Players();


                        if (g_Options.General.KeyBind > 0) // só checa se tem tecla setada
                        {
                            if (GetAsyncKeyState(g_Options.General.KeyBind) & 1) // apenas no "press"
                            {
                                auto ped = Cheat::g_Fivem.GetLocalPlayerInfo().Ped;
                                if (ped && ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle))
                                {
                                    auto vehicle = ped->GetLastVehicle();
                                    if (vehicle)
                                        vehicle->FixVehicle();
                                }
                            }
                        }
                        if (g_Options.Misc.Exploits.LocalPlayer.GodModeKey > 0)
                        {
                            if (GetAsyncKeyState(g_Options.Misc.Exploits.LocalPlayer.GodModeKey) & 1)
                            {
                                g_Options.Misc.Exploits.LocalPlayer.GodMode = !g_Options.Misc.Exploits.LocalPlayer.GodMode;
                                const bool godOn = g_Options.Misc.Exploits.LocalPlayer.GodMode;

                                auto ped = Cheat::g_Fivem.GetLocalPlayerInfo().Ped;
                                if (ped)
                                    ped->GodMode(godOn);


                            }
                        }
                        if (g_Options.Misc.Exploits.LocalPlayer.AntiAimKey > 0)
                        {
                            if (GetAsyncKeyState(g_Options.Misc.Exploits.LocalPlayer.AntiAimKey) & 1)
                            {
                                g_Options.Misc.Exploits.LocalPlayer.AntiAimEnabled = !g_Options.Misc.Exploits.LocalPlayer.AntiAimEnabled;
                                const bool aaOn = g_Options.Misc.Exploits.LocalPlayer.AntiAimEnabled;

                                auto ped = Cheat::g_Fivem.GetLocalPlayerInfo().Ped;
                                if (ped)
                                {
                                    ped->SetConfigFlag(CPED_CONFIG_FLAG_DisablePlayerLockon, aaOn);
                                    ped->SetConfigFlag(CPED_CONFIG_FLAG_AllowPlayerLockOnIfFriendly, aaOn);
                                    ped->SetConfigFlag(CPED_CONFIG_FLAG_TreatAsFriendlyForTargetingAndDamage, aaOn);
                                    ped->SetConfigFlag(CPED_CONFIG_FLAG_NeverEverTargetThisPed, aaOn);
                                    ped->SetConfigFlag(CPED_CONFIG_FLAG_DisableLockonToRandomPeds, aaOn);
                                    ped->SetConfigFlag(CPED_CONFIG_FLAG_BlockGroupPedAimedAtResponse, aaOn);
                                }


                            }
                        }
                        if (g_Options.LegitBot.MagicBullet.KeyBind > 0)
                        {
                            if (GetAsyncKeyState(g_Options.LegitBot.MagicBullet.KeyBind) & 1)
                            {
                                g_Options.LegitBot.MagicBullet.Enabled = !g_Options.LegitBot.MagicBullet.Enabled;
                                const bool mbOn = g_Options.LegitBot.MagicBullet.Enabled;


                            }
                        }

                    }

                    Interface.RenderGui();
                }

                ImGui::EndFrame();
                ImGui::Render();

                FrameWork::Overlay::dxRefresh();
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
                FrameWork::Overlay::dxGetSwapChain()->Present(1, 0);

                auto frame_end = std::chrono::high_resolution_clock::now();
                auto frame_duration = std::chrono::duration_cast<std::chrono::milliseconds>(frame_end - frame_start);

                // Limitador de FPS mais flexível (ex: 144 FPS)
                const int target_fps = (Interface.GetMenuOpen()) ? 144 : 240; 
                const int target_ms = 1000 / target_fps;
                
                if (frame_duration.count() < target_ms)
                    std::this_thread::sleep_for(std::chrono::milliseconds(target_ms - (int)frame_duration.count()));
            }

            Cheat::ShutDown();


            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            TerminateProcess(GetCurrentProcess(), 0);
        }
    }

    void ShutDown()
    {
        g_Options.General.ShutDown = true;

        MagicBullet::Restore();
        WebRemote::Stop();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        FrameWork::Overlay::ShutDown();
        FrameWork::Overlay::dxShutDown();

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}
