#include "Interface.hpp"
#pragma warning(disable: 4244 4267 4242 4312 4311 4100 4189 4456 4457 4458 4696)
#include "EspPreview.hpp"
#include <cstdio>
#include <atomic>
#include <thread>
#include <Cheat/Options.hpp>
#include <Cheat/WebRemote.hpp>
#include <Cheat/Cheat.hpp>
#include <Cheat/ConfigSystem.hpp>
#include <Security/KeyAuth.hpp>
#include <FrameWork/Dependencies/ImGui/imgui_edited.hpp>
#include <algorithm>
#include <FrameWork/Utilities/Notify.hpp>
#include "SidebarLogo.hpp"

#include <tchar.h>
#include <shellapi.h>
#include <shlobj.h>
#include "../includes/Language.hpp"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

inline Cheat::ConfigManager ConfigManager;

// Flags de resultado da thread de autenticação
static std::atomic<bool> g_AuthCheckDone{ false };
static std::atomic<bool> g_AuthCheckResult{ false };
static std::atomic<bool> g_AutoAuthAttempted{ false };

inline std::string GetLicensePath()
{
	char path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		std::string p = std::string(path) + "\\ScarfaceX";
		CreateDirectoryA(p.c_str(), NULL);
		return p + "\\login.key";
	}
	return "login.key";
}

inline void SaveLicenseKey(const std::string& key)
{
	HANDLE h = CreateFileA(GetLicensePath().c_str(), GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h != INVALID_HANDLE_VALUE)
	{
		DWORD written;
		WriteFile(h, key.c_str(), (DWORD)key.size(), &written, NULL);
		CloseHandle(h);
	}
}

inline std::string LoadLicenseKey()
{
	std::string path = GetLicensePath();
	HANDLE h = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE) return {};
	DWORD sz = GetFileSize(h, NULL);
	std::string buf(sz, '\0');
	DWORD read;
	if (ReadFile(h, &buf[0], sz, &read, NULL))
		buf.resize(read);
	CloseHandle(h);
	return buf;
}

inline void DeleteLicenseKey()
{
	DeleteFileA(GetLicensePath().c_str());
}

struct MarkerT
{
	Vector3D Position;
	std::string Name;
};

	ID3D11ShaderResourceView* SidebarLogo = nullptr;
	int CurrentTab = 0;
static int LegitSub = 0;
static int VisualsSub = 0;
static int MiscSub = 0;
static int PlayerSub = 0;
static int ExploitsSub = 0;
static int WorldSub = 0;
static int SettingsSub = 0;
static int WorldSelPlayerNetId = -1;
static uintptr_t WorldSelVehicleAddr = 0;
std::vector<MarkerT> MarkerList;



namespace
{
	void ApplyOverlayWindowStyle(HWND window, LONG exStyle)
	{
		SetWindowLong(window, GWL_EXSTYLE, exStyle);
		SetWindowPos(
			window,
			HWND_TOPMOST,
			0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW
		);
	}
}

namespace FrameWork
{
	void Interface::RenderActiveFeaturesOverlay()
	{
		if (!g_Options.Misc.ShowActiveFeaturesOverlay)
			return;

		static std::vector<const char*> activeFeatures;
		activeFeatures.clear();

		if (g_Options.LegitBot.AimBot.Enabled) activeFeatures.push_back("Aimbot");
		if (g_Options.LegitBot.SilentAim.Enabled) activeFeatures.push_back("Silent Aim");
		if (g_Options.LegitBot.MagicBullet.Enabled) activeFeatures.push_back("Magic Bullet");
		if (g_Options.LegitBot.Trigger.Enabled) activeFeatures.push_back("Triggerbot");

		if (g_Options.Visuals.ESP.Players.Enabled) activeFeatures.push_back("Player ESP");
		if (g_Options.Visuals.ESP.Vehicles.Enabled) activeFeatures.push_back("Vehicle ESP");

		if (g_Options.Misc.Exploits.LocalPlayer.GodMode) activeFeatures.push_back("Godmode");
		if (g_Options.Misc.Exploits.LocalPlayer.Noclip) activeFeatures.push_back("Noclip");
		if (g_Options.Misc.Exploits.LocalPlayer.InfiniteStamina) activeFeatures.push_back("Inf. Stamina");
		if (g_Options.Misc.Exploits.LocalPlayer.InfiniteCombatRoll) activeFeatures.push_back("Inf. Combat Roll");
		if (g_Options.Misc.Exploits.LocalPlayer.FastRun) activeFeatures.push_back("Fast Run");
		if (g_Options.Misc.Exploits.LocalPlayer.Shrink) activeFeatures.push_back("Shrink");
		if (g_Options.Misc.Exploits.LocalPlayer.AntiAimEnabled) activeFeatures.push_back("Anti-Aim");
		if (g_Options.Misc.Exploits.LocalPlayer.SeatBelt) activeFeatures.push_back("Seat Belt");
		if (g_Options.Misc.Exploits.Weapon.RemoveRecoil) activeFeatures.push_back("No Recoil");
		if (g_Options.Misc.Exploits.Weapon.RemoveSpread) activeFeatures.push_back("No Spread");
		if (g_Options.Misc.Exploits.Weapon.NoReload) activeFeatures.push_back("No Reload");

		if (g_Options.Misc.Exploits.Weapon.InfiniteAmmoEnabled) activeFeatures.push_back("Inf. Ammo");
		if (g_Options.Misc.Exploits.Weapon.Coronhada) activeFeatures.push_back("Unlock All Actions");

		if (g_Options.Misc.Exploits.Vehicle.GodMode) activeFeatures.push_back("Veh Godmode");
		if (g_Options.Misc.Exploits.Vehicle.RocketBoost) activeFeatures.push_back("Rocket Boost");

		if (g_Options.Misc.Trolls.VehicleGrabEnabled) activeFeatures.push_back("Veh Grab");

		if (activeFeatures.empty())
			return;

		ImGuiIO& io = ImGui::GetIO();
		ImDrawList* drawList = ImGui::GetBackgroundDrawList();
		ImVec2 screenSize = io.DisplaySize;

		float padding = 10.0f;
		float itemHeight = 18.0f;
		float headerHeight = 25.0f;
		float width = 150.0f;

		ImGui::PushFont(Assets::InterSemiBold);
		for (const char* feature : activeFeatures)
		{
			float textWidth = ImGui::CalcTextSize(feature).x + 30.0f;
			if (textWidth > width) width = textWidth;
		}
		ImGui::PopFont();

		float height = headerHeight + (activeFeatures.size() * itemHeight) + padding;

		static ImVec2 pos = ImVec2(20, screenSize.y * 0.5f - height * 0.5f);
		static bool isDragging = false;
		static ImVec2 dragOffset;

		bool hoveringHeader = io.MousePos.x >= pos.x && io.MousePos.x <= pos.x + width &&
			io.MousePos.y >= pos.y && io.MousePos.y <= pos.y + headerHeight;
		if (!isDragging && io.MouseDown[0] && hoveringHeader)
		{
			isDragging = true;
			dragOffset = io.MousePos - pos;
		}
		if (isDragging)
		{
			if (io.MouseDown[0])
				pos = io.MousePos - dragOffset;
			else
				isDragging = false;
		}

		pos.x = ImMax(0.0f, ImMin(pos.x, screenSize.x - width));
		pos.y = ImMax(0.0f, ImMin(pos.y, screenSize.y - height));

		drawList->AddRectFilled(pos, pos + ImVec2(width, height), ImColor(0, 0, 0, 200), 6.0f);
		drawList->AddRect(pos, pos + ImVec2(width, height), ImColor(64, 64, 64, 200), 6.0f);

		ImGui::PushFont(Assets::InterBold);
		drawList->AddText(pos + ImVec2(padding, 5), ImColor(255, 255, 255, 255), "ACTIVE FEATURES");
		ImGui::PopFont();

		drawList->AddRectFilled(pos + ImVec2(padding, headerHeight - 2), pos + ImVec2(width - padding, headerHeight), ImColor(255, 0, 51, 255), 1.0f);

		ImGui::PushFont(Assets::InterRegular);
		float currentY = pos.y + headerHeight + 5.0f;
		ImColor accentColor = ImColor(255, 0, 51, 255);

		for (const char* feature : activeFeatures)
		{
			drawList->AddCircleFilled(ImVec2(pos.x + 12, currentY + 9), 2.0f, accentColor);
			drawList->AddText(ImVec2(pos.x + 22, currentY), ImColor(230, 230, 230, 255), feature);
			currentY += itemHeight;
		}
		ImGui::PopFont();
	}

	void Interface::Initialize(HWND Window, HWND TargetWindow, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext)
	{
		hWindow = Window;
		hTargetWindow = TargetWindow;
		IDevice = Device;
		FrameWork::Overlay::g_pd3dDevice = Device;

		if (bIsMenuOpen)
		{
			ApplyOverlayWindowStyle(hWindow, WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
			keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
			SetForegroundWindow(hWindow);
		}

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(hWindow);
		ImGui_ImplDX11_Init(Device, DeviceContext);

		MarkerList.push_back({ Vector3D(212.6681f, -813.7118f, 30.7386f), XorStr("Meeting Point") });
		MarkerList.push_back({ Vector3D(99.7721f, -743.7130f, 45.7547f), XorStr("FIB-Tower") });
		MarkerList.push_back({ Vector3D(-1039.2391f, -2666.4702f, 13.8307f), XorStr("Airport") });
		MarkerList.push_back({ Vector3D(3627.5176f, 3754.3137f, 28.5157f), XorStr("Humanlabs") });
		MarkerList.push_back({ Vector3D(1404.8857f, 3162.1936f, 40.4341f), XorStr("Sandyshores Airfield") });
		MarkerList.push_back({ Vector3D(-2326.4241f, 3053.1711f, 32.8150f), XorStr("Armybase") });
		MarkerList.push_back({ Vector3D(501.6581f, 5604.9321f, 797.9105f), XorStr("Mount Chiliad") });
		MarkerList.push_back({ Vector3D(-1448.0753f, -766.6392f, 23.5332f), XorStr("Del Perro Pier") });
		MarkerList.push_back({ Vector3D(298.7408f, -581.9695f, 43.2608f), XorStr("Pillbox") });

		D3DX11CreateShaderResourceViewFromMemory(Device, SidebarLogoBytes, sizeof(SidebarLogoBytes), NULL, NULL, &SidebarLogo, NULL);
	}

	void Interface::UpdateStyle()
	{
		ImGuiStyle* Style = &ImGui::GetStyle();

		Style->WindowRounding = 3;
		Style->ChildRounding = 2;
		Style->FrameRounding = 2;
		Style->PopupRounding = 6;
		Style->ScrollbarRounding = 9;
		Style->GrabRounding = 12;

		Style->WindowBorderSize = 1;
		Style->ChildBorderSize = 1;
		Style->FrameBorderSize = 0;
		Style->PopupBorderSize = 1;

		Style->WindowPadding = ImVec2(0, 0);
		Style->FramePadding = ImVec2(10, 5);
		Style->ItemSpacing = ImVec2(10, 10);
		Style->ItemInnerSpacing = ImVec2(10, 10);
		Style->ScrollbarSize = 8;

		ImVec4 accentColor = ImVec4(255.0f / 255.0f, 0.0f / 255.0f, 51.0f / 255.0f, 1.00f);
		g_Options.General.PrimaryColor[0] = accentColor.x;
		g_Options.General.PrimaryColor[1] = accentColor.y;
		g_Options.General.PrimaryColor[2] = accentColor.z;
		g_Options.General.PrimaryColor[3] = accentColor.w;

		Style->Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		Style->Colors[ImGuiCol_TextDisabled] = ImVec4(128.f / 255.f, 128.f / 255.f, 128.f / 255.f, 1.00f);
		Style->Colors[ImGuiCol_WindowBg] = ImVec4(0.f, 0.f, 0.f, 1.00f);
		Style->Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		Style->Colors[ImGuiCol_PopupBg] = ImVec4(32.f / 255.f, 32.f / 255.f, 32.f / 255.f, 1.00f);
		Style->Colors[ImGuiCol_Border] = ImVec4(64.f / 255.f, 64.f / 255.f, 64.f / 255.f, 1.00f);
		Style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		Style->Colors[ImGuiCol_FrameBg] = ImVec4(32.f / 255.f, 32.f / 255.f, 32.f / 255.f, 1.00f);
		Style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(64.f / 255.f, 64.f / 255.f, 64.f / 255.f, 1.00f);
		Style->Colors[ImGuiCol_FrameBgActive] = ImVec4(96.f / 255.f, 96.f / 255.f, 96.f / 255.f, 1.00f);
		Style->Colors[ImGuiCol_TitleBg] = ImVec4(0.f, 0.f, 0.f, 1.00f);
		Style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.f, 0.f, 0.f, 1.00f);
		Style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.f, 0.f, 0.f, 1.00f);
		Style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.f, 0.f, 0.f, 1.00f);
		Style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		Style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(64.f / 255.f, 64.f / 255.f, 64.f / 255.f, 1.00f);
		Style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(96.f / 255.f, 96.f / 255.f, 96.f / 255.f, 1.00f);
		Style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(128.f / 255.f, 128.f / 255.f, 128.f / 255.f, 1.00f);
		Style->Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		Style->Colors[ImGuiCol_SliderGrab] = accentColor;
		Style->Colors[ImGuiCol_SliderGrabActive] = accentColor;
		Style->Colors[ImGuiCol_Button] = ImVec4(32.f / 255.f, 32.f / 255.f, 32.f / 255.f, 1.00f);
		Style->Colors[ImGuiCol_ButtonHovered] = ImVec4(64.f / 255.f, 64.f / 255.f, 64.f / 255.f, 1.00f);
		Style->Colors[ImGuiCol_ButtonActive] = accentColor;
		Style->Colors[ImGuiCol_Header] = accentColor;
		Style->Colors[ImGuiCol_HeaderHovered] = accentColor;
		Style->Colors[ImGuiCol_HeaderActive] = accentColor;
		Style->Colors[ImGuiCol_Separator] = ImVec4(64.f / 255.f, 64.f / 255.f, 64.f / 255.f, 1.00f);
		Style->Colors[ImGuiCol_SeparatorHovered] = accentColor;
		Style->Colors[ImGuiCol_SeparatorActive] = accentColor;

		static std::string cachedUserName = FrameWork::Misc::GetDiscordUsername();
		g_Options.General.UserName = cachedUserName;

		if (cachedUserName == XorStr(".llmig") || cachedUserName == XorStr("coto777"))
			g_Options.General.UserRole = XorStr("Developer");
		else
			g_Options.General.UserRole = XorStr("Client");

		Assets::Initialize(IDevice);
		Cheat::g_EspPreview.CreateTexture(IDevice);
	}

	// ── Plexus Particle Background ─────────────────────────────────────────
	struct PlexusParticle
	{
		ImVec2 pos;
		ImVec2 vel;
		float radius;
	};

	static std::vector<PlexusParticle> s_Plexus;
	static bool s_PlexusInit = false;
	static ImVec2 s_PlexusWinSize = ImVec2(0, 0);

	static void InitPlexus(ImVec2 winSize)
	{
		const int count = 80;
		s_Plexus.resize(count);
		for (int i = 0; i < count; i++)
		{
			s_Plexus[i].pos = ImVec2(
				(float(rand()) / RAND_MAX) * winSize.x,
				(float(rand()) / RAND_MAX) * winSize.y
			);
			s_Plexus[i].vel = ImVec2(
				(float(rand()) / RAND_MAX - 0.5f) * 0.4f,
				(float(rand()) / RAND_MAX - 0.5f) * 0.4f
			);
			s_Plexus[i].radius = 1.0f + (float(rand()) / RAND_MAX) * 1.0f;
		}
		s_PlexusInit = true;
	}

	static void RenderParticleBackground(ImDrawList* dl, ImVec2 winPos, ImVec2 winSize)
	{
		if (!s_PlexusInit || winSize.x != s_PlexusWinSize.x || winSize.y != s_PlexusWinSize.y)
		{
			s_PlexusWinSize = winSize;
			InitPlexus(winSize);
		}

		// Update movement + bounce off edges
		for (auto& p : s_Plexus)
		{
			p.pos.x += p.vel.x;
			p.pos.y += p.vel.y;

			if (p.pos.x < 0.f || p.pos.x > winSize.x) p.vel.x *= -1.f;
			if (p.pos.y < 0.f || p.pos.y > winSize.y) p.vel.y *= -1.f;

			p.pos.x = ImClamp(p.pos.x, 0.f, winSize.x);
			p.pos.y = ImClamp(p.pos.y, 0.f, winSize.y);
		}

		// Draw connection lines (gold, fading with distance)
		const float kMaxDist = 95.f;
		const float kMaxDistSq = kMaxDist * kMaxDist;

		for (int i = 0; i < (int)s_Plexus.size(); i++)
		{
			for (int j = i + 1; j < (int)s_Plexus.size(); j++)
			{
				float dx = s_Plexus[i].pos.x - s_Plexus[j].pos.x;
				float dy = s_Plexus[i].pos.y - s_Plexus[j].pos.y;
				float distSq = dx * dx + dy * dy; // squared distance avoids sqrt

				if (distSq < kMaxDistSq)
				{
					// Alpha = 0 at max dist, ~0.4 at min dist (inverse linear)
					float alpha = 1.f - (distSq / kMaxDistSq);
					alpha *= 0.3f;

					dl->AddLine(
						winPos + s_Plexus[i].pos,
						winPos + s_Plexus[j].pos,
						IM_COL32(255, 0, 0, (int)(alpha * 255.f * g_Options.General.PlexusOpacity / 100.f)),
						0.5f
					);
				}
			}
		}

		// Draw particle dots (white)
		for (auto& p : s_Plexus)
		{
			dl->AddCircleFilled(winPos + p.pos, p.radius, IM_COL32(255, 255, 255, (int)(160 * g_Options.General.PlexusOpacity / 100.f)));
		}
	}

	void Interface::RenderGui()
	{
		static std::string s_DiscordName = FrameWork::Misc::GetDiscordUsername();

		if (g_Options.Misc.ShowActiveFeaturesOverlay)
			RenderActiveFeaturesOverlay();

		if (g_Options.General.WaterMark)
		{
			ImDrawList* drawList = ImGui::GetForegroundDrawList();
			ImVec2 screenSize = ImGui::GetIO().DisplaySize;
			ImGui::PushFont(Assets::InterBold);
			std::string watermarkText = "ScarfaceX";
			ImColor wmColor = g_Options.General.WaterMarkCol
				? FrameWork::Misc::Float4ToImColor(g_Options.General.WaterMarkColor)
				: ImColor(255, 255, 255, 200);
			ImVec2 textSize = ImGui::CalcTextSize(watermarkText.c_str());
			drawList->AddText(ImVec2(screenSize.x - textSize.x - 10, 10), wmColor, watermarkText.c_str());
			ImGui::PopFont();
		}

		if (!bIsMenuOpen)
			return;

		// ── Tela de Login ─────────────────────────────────────────────────────
		if (!g_Options.General.IsLoggedIn)
		{
			static char  s_Key[64] = "";
			static bool  s_Wrong = false;
			static bool  s_Checking = false;
			static bool  s_AutoChecking = false;
			static float s_ShakeTimer = 0.f;
			static float s_ShakeOff = 0.f;
			static float s_DotTimer = 0.f;

			// Auto-login com chave salva
			if (!g_AutoAuthAttempted.load() && !s_AutoChecking && !s_Checking && s_Key[0] == '\0')
			{
				std::string savedKey = LoadLicenseKey();
				if (savedKey.empty() && strlen(g_Options.General.LicenseKey) > 0)
					savedKey = g_Options.General.LicenseKey;
				if (!savedKey.empty())
				{
					s_AutoChecking = true;
					strcpy_s(s_Key, savedKey.c_str());
					std::thread([savedKey]()
						{
							bool ok = ::Security::Authenticate(savedKey);
							if (ok)
							{
								g_Options.General.IsLoggedIn = true;
								g_Options.General.DaysLeft = ::Security::CurrentLicense.daysLeft;
								g_Options.General.UserName = ::Security::CurrentLicense.username;
							}
							g_AuthCheckResult = ok;
							g_AuthCheckDone = true;
							g_AutoAuthAttempted = true;
						}).detach();
				}
				else
				{
					g_AutoAuthAttempted = true;
				}
			}

			if (s_AutoChecking)
			{
				s_DotTimer += ImGui::GetIO().DeltaTime;
				if (g_AuthCheckDone.load())
				{
					g_AuthCheckDone = false;
					s_AutoChecking = false;
					s_Checking = false;
					if (!g_AuthCheckResult.load())
					{
						s_Key[0] = '\0';
						s_Wrong = true;
						DeleteLicenseKey();
					}
				}
				else
				{
					// Tela de "Checking saved license..."
					const ImVec2 kSz = { 420.f, 360.f };
					ImVec2 scr = ImGui::GetIO().DisplaySize;
					ImGui::SetNextWindowPos(ImVec2((scr.x - kSz.x) * .5f, (scr.y - kSz.y) * .5f), ImGuiCond_Once);
					ImGui::SetNextWindowSize(kSz);
					ImGui::Begin("##login", nullptr,
						ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
						ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
					{
						ImDrawList* dl = ImGui::GetWindowDrawList();
						ImVec2 wPos = ImGui::GetWindowPos();
						ImVec2 wSz = ImGui::GetWindowSize();
						dl->AddRectFilled(wPos, wPos + wSz, ImColor(0, 0, 0), 4.f);
						{
							static float anim = 0.f;
							anim += ImGui::GetIO().DeltaTime * 0.4f;
							for (int i = 0; i < 28; i++)
							{
								float px = (sinf(anim + i * 0.85f) * .5f + .5f) * wSz.x;
								float py = (cosf(anim + i * 0.55f) * .5f + .5f) * wSz.y;
								dl->AddCircleFilled(wPos + ImVec2(px, py), 1.1f, ImColor(255, 0, 51, 100 * g_Options.General.ParticleOpacity / 100));
							}
						}
						ImGui::PushFont(Assets::InterBold);
						{
							if (SidebarLogo)
							{
								float logoW = 150.f;
								float logoH = logoW;
								float logoX = (wSz.x - logoW) * .5f;
								dl->AddImage(SidebarLogo, wPos + ImVec2(logoX, 16.f), wPos + ImVec2(logoX + logoW, 16.f + logoH));
							}
							const char* t = XorStr("SCARFACEX EXTERNAL");
							ImVec2 ts = ImGui::CalcTextSize(t);
							dl->AddText(Assets::InterBold, 16.f,
								wPos + ImVec2((wSz.x - ts.x) * .5f, 180.f),
								ImColor(255, 255, 255), t);
						}
						ImGui::PopFont();
						ImGui::PushFont(Assets::InterRegular);
						{
							static const char* dots[] = {
								"Checking license   ", "Checking license.  ", "Checking license.. ", "Checking license..."
							};
							const char* msg = dots[(int)(s_DotTimer * 3.f) % 4];
							ImVec2 ss = ImGui::CalcTextSize(msg);
							dl->AddText(Assets::InterRegular, 13.f,
								wPos + ImVec2((wSz.x - ss.x) * .5f, 202.f),
								ImColor(130, 130, 140), msg);
						}
						ImGui::PopFont();
						if (g_Options.General.Plexus)
							RenderParticleBackground(dl, wPos, wSz);
						dl->AddRect(wPos, wPos + wSz, ImColor(64, 64, 64), 4.f, 0, 1.5f);
					}
					ImGui::End();
					return;
				}
			}

			if (s_ShakeTimer > 0.f)
			{
				s_ShakeTimer -= ImGui::GetIO().DeltaTime;
				s_ShakeOff = sinf(s_ShakeTimer * 60.f) * 4.f;
			}
			else s_ShakeOff = 0.f;

			if (s_Checking) s_DotTimer += ImGui::GetIO().DeltaTime;

			const ImVec2 kSz = { 420.f, 360.f };
			ImVec2 scr = ImGui::GetIO().DisplaySize;

			ImGui::SetNextWindowPos(
				ImVec2((scr.x - kSz.x) * .5f, (scr.y - kSz.y) * .5f),
				ImGuiCond_Once);
			ImGui::SetNextWindowSize(kSz);
			ImGui::Begin("##login", nullptr,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			{
				if (s_ShakeOff != 0.f)
				{
					ImVec2 p = ImGui::GetWindowPos();
					ImGui::SetWindowPos(ImVec2(p.x + s_ShakeOff, p.y));
				}

				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 wPos = ImGui::GetWindowPos();
				ImVec2 wSz = ImGui::GetWindowSize();

				dl->AddRectFilled(wPos, wPos + wSz, ImColor(0, 0, 0), 4.f);

				{
					static float anim = 0.f;
					anim += ImGui::GetIO().DeltaTime * 0.4f;
					for (int i = 0; i < 28; i++)
					{
						float px = (sinf(anim + i * 0.85f) * .5f + .5f) * wSz.x;
						float py = (cosf(anim + i * 0.55f) * .5f + .5f) * wSz.y;
						dl->AddCircleFilled(wPos + ImVec2(px, py), 1.1f, ImColor(255, 0, 51, 100 * g_Options.General.ParticleOpacity / 100));
					}
				}

					ImGui::PushFont(Assets::InterBold);
				{
					if (SidebarLogo)
					{
						float logoW = 150.f;
						float logoH = logoW;
						float logoX = (wSz.x - logoW) * .5f;
						dl->AddImage(SidebarLogo, wPos + ImVec2(logoX, 16.f), wPos + ImVec2(logoX + logoW, 16.f + logoH));
					}
					const char* t = XorStr("SCARFACEX EXTERNAL");
					ImVec2 ts = ImGui::CalcTextSize(t);
					dl->AddText(Assets::InterBold, 16.f,
						wPos + ImVec2((wSz.x - ts.x) * .5f, 180.f),
						ImColor(255, 255, 255), t);
				}
				ImGui::PopFont();

				ImGui::PushFont(Assets::InterRegular);
				{
					const char* s = XorStr("Enter your license key to continue");
					ImVec2 ss = ImGui::CalcTextSize(s);
					dl->AddText(Assets::InterRegular, 13.f,
						wPos + ImVec2((wSz.x - ss.x) * .5f, 202.f),
						ImColor(130, 130, 140), s);
				}
				ImGui::PopFont();

				dl->AddRectFilled(wPos + ImVec2(30.f, 225.f),
					wPos + ImVec2(wSz.x - 30.f, 226.f),
					ImColor(35, 35, 45));

				if (g_Options.General.Plexus)
					RenderParticleBackground(dl, wPos, wSz);
				dl->AddRect(wPos, wPos + wSz, ImColor(64, 64, 64), 4.f, 0, 1.5f);

				const float iW = wSz.x - 60.f;
				const float iX = 30.f;
				const float iY = 238.f;

				ImGui::SetCursorPos(ImVec2(iX, iY));
				ImGui::SetNextItemWidth(iW);

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.f, 10.f));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(32, 32, 32).Value);
				ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImColor(64, 64, 64).Value);
				ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImColor(96, 96, 96).Value);
				ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 255, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_Border,
					s_Wrong ? ImColor(220, 60, 60).Value : ImColor(50, 50, 60).Value);

				bool enter = false;
				if (s_Checking)
				{
					ImGui::BeginDisabled();
					ImGui::InputText("##key", s_Key, sizeof(s_Key),
						ImGuiInputTextFlags_Password);
					ImGui::EndDisabled();
				}
				else
				{
					enter = ImGui::InputText("##key", s_Key, sizeof(s_Key),
						ImGuiInputTextFlags_EnterReturnsTrue |
						ImGuiInputTextFlags_Password);
				}

				ImGui::PopStyleColor(5);
				ImGui::PopStyleVar(3);

				if (s_Key[0] == '\0' && !s_Checking)
					dl->AddText(ImGui::GetItemRectMin() + ImVec2(12.f, 10.f),
						ImColor(70, 70, 80), XorStr("XXXX-XXXX-XXXX-XXXX"));

				if (s_Wrong && !s_Checking)
				{
					ImGui::PushFont(Assets::InterRegular);
					std::string authMsg = ::Security::CurrentLicense.lastError;
					if (authMsg.empty()) authMsg = "Invalid key. Try again.";
					if (ImGui::CalcTextSize(authMsg.c_str()).x > wSz.x - 40.f)
						authMsg = authMsg.substr(0, 35) + "...";
					ImVec2 es = ImGui::CalcTextSize(authMsg.c_str());
					dl->AddText(Assets::InterRegular, 12.f,
						wPos + ImVec2((wSz.x - es.x) * .5f, iY + 42.f),
						ImColor(220, 80, 80), authMsg.c_str());
					ImGui::PopFont();
				}

				const float bY = iY + (s_Wrong && !s_Checking ? 62.f : 52.f);
				ImGui::SetCursorPos(ImVec2(iX, bY));

				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 10.f));
				ImGui::PushStyleColor(ImGuiCol_Button,
					s_Checking ? ImColor(50, 50, 60, 200).Value : ImColor(255, 0, 51, 200).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
					s_Checking ? ImColor(50, 50, 60, 200).Value : ImColor(255, 40, 80, 230).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(200, 0, 40, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 255, 255, 255).Value);

				const char* lbl = XorStr("LOGIN");
				if (s_Checking)
				{
					static const char* dots[] = {
						"Checking   ", "Checking.  ", "Checking.. ", "Checking..."
					};
					lbl = dots[(int)(s_DotTimer * 3.f) % 4];
				}

				bool clicked = false;
				if (s_Checking)
				{
					ImGui::BeginDisabled();
					ImGui::Button(lbl, ImVec2(iW, 0.f));
					ImGui::EndDisabled();
				}
				else clicked = ImGui::Button(lbl, ImVec2(iW, 0.f));

				ImGui::PopStyleColor(4);
				ImGui::PopStyleVar(2);

				if ((clicked || enter) && !s_Checking && s_Key[0] != '\0')
				{
					s_Checking = true;
					s_Wrong = false;
					s_DotTimer = 0.f;

					std::string keyCopy(s_Key);
					std::thread([keyCopy]()
						{
							bool ok = ::Security::Authenticate(keyCopy);
							if (ok)
							{
								g_Options.General.IsLoggedIn = true;
								g_Options.General.DaysLeft = ::Security::CurrentLicense.daysLeft;
								g_Options.General.UserName = ::Security::CurrentLicense.username;
								SaveLicenseKey(keyCopy);
							}
							g_AuthCheckResult = ok;
							g_AuthCheckDone = true;
							g_AutoAuthAttempted = true;
						}).detach();
				}

				if (g_AuthCheckDone.load())
				{
					g_AuthCheckDone = false;
					s_Checking = false;
					if (!g_AuthCheckResult.load())
					{
						s_Wrong = true;
						s_ShakeTimer = 0.35f;
					}
				}
			}
			ImGui::End();
			return;
		}

		ImGui::GetBackgroundDrawList()->AddRectFilled(
			ImVec2(0.f, 0.f),
			ImGui::GetIO().DisplaySize,
			ImColor(0, 0, 0, 85)
		);

		ImGui::SetNextWindowSize(ImVec2(895, 535));
		ImVec2 scr = ImGui::GetIO().DisplaySize;
		ImGui::SetNextWindowPos(ImVec2((scr.x - 895) * 0.5f, (scr.y - 640) * 0.5f), ImGuiCond_Once);
		ImGui::Begin(("Menu"), &bIsMenuOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			ImDrawList* DrawList = ImGui::GetWindowDrawList();
			ImVec2 Pos = ImGui::GetWindowPos();
			ImVec2 Size = ImGui::GetWindowSize();

			// Gradient background: solid black → very dark warm gray at base
			DrawList->AddRectFilledMultiColor(
				Pos, Pos + Size,
				IM_COL32(0, 0, 0, 255),     // top-left
				IM_COL32(0, 0, 0, 255),     // top-right
				IM_COL32(14, 10, 6, 255),   // bottom-right  (subtle gold tint)
				IM_COL32(14, 10, 6, 255)    // bottom-left
			);

			if (g_Options.General.Particles)
			{
				static float menuBgAnim = 0.f;
				menuBgAnim += ImGui::GetIO().DeltaTime * 0.5f;
				for (int i = 0; i < 48; i++) {
					float x = (sin(menuBgAnim + i * 0.7f) * 0.5f + 0.5f) * Size.x;
					float y = (cos(menuBgAnim + i * 0.4f) * 0.5f + 0.5f) * Size.y;
					DrawList->AddCircleFilled(Pos + ImVec2(x, y), 1.3f, ImColor(255, 0, 51, 130 * g_Options.General.ParticleOpacity / 100));
				}
			}

			const float kTopBarH = 50.f;
			const float kSidebarW = 200.f;
			const float kSidebarPad = 15.f;
			const float kSidebarVisualW = kSidebarW - kSidebarPad * 2;
			const float kSidebarRightGap = 15.f;
			const float kHeaderGap = 15.f;
			const float kContentTop = kTopBarH + kHeaderGap;
			const float kBottomReserve = 15.f;
			const float kMainChildW = Size.x - (kSidebarW - kSidebarPad) - kSidebarRightGap - 15.f;
			const float kMainChildH = Size.y - kContentTop - kBottomReserve;
			const float kPanelH = ImMax(200.f, kMainChildH - 20.f);

			DrawList->AddRectFilled(Pos, Pos + ImVec2(Size.x, kTopBarH), ImColor(0, 0, 0), ImGui::GetStyle().WindowRounding, ImDrawFlags_RoundCornersTop);

			float mainChildLeft = kSidebarW - kSidebarPad + kSidebarRightGap;

			auto FilterPass = [&](const char* label) -> bool {
				(void)label;
				return true;
			};

			if (SidebarLogo)
			{
				float logoW = 150.f;
				float logoH = logoW;
				float logoX = (kSidebarW - kSidebarPad - logoW) * 0.5f;
				float logoY = (kContentTop + 200.f - logoH) * 0.5f;
				DrawList->AddImage(SidebarLogo, Pos + ImVec2(logoX, logoY), Pos + ImVec2(logoX + logoW, logoY + logoH));
			}

			// Sidebar vertical line (front layer)
			const float sbLineX = Pos.x + kSidebarW - kSidebarPad;
			DrawList->AddLine(ImVec2(sbLineX, Pos.y), ImVec2(sbLineX, Pos.y + Size.y), ImColor(64, 64, 64), 1.f);

			// Plexus on parent DrawList — behind all child windows
			if (g_Options.General.Plexus)
				RenderParticleBackground(DrawList, Pos, Size);

			{
				const float tabH = ImGui::GetFrameHeight();
				const float tabY = (kTopBarH - tabH) * 0.5f;
				ImGui::SetCursorPos(ImVec2(mainChildLeft, tabY));
				if (CurrentTab == 0)
					ImGui::TabHeader(XorStr("LegitBotHeader"), &LegitSub, { _T("Aimbot"), _T("Silent"), _T("Trigger") }, CurrentTab);
				else if (CurrentTab == 1)
					ImGui::TabHeader(XorStr("VisualsHeader"), &VisualsSub, { _T("Players"), _T("Colors"), _T("Vehicles") }, CurrentTab);
				else if (CurrentTab == 2)
					ImGui::TabHeader(XorStr("ExploitsHeader"), &ExploitsSub, { _T("Player"), _T("Vehicles") }, CurrentTab);
				else if (CurrentTab == 3)
					ImGui::TabHeader(XorStr("WorldHeader"), &WorldSub, { _T("Players"), _T("Vehicles") }, CurrentTab);
				else if (CurrentTab == 4)
					ImGui::TabHeader(XorStr("SettingsHeader"), &SettingsSub, { _T("General"), _T("Trolls") }, CurrentTab);
			}

			ImGui::SetCursorPos(ImVec2(kSidebarPad, kContentTop));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::BeginChild(XorStr("Sidebar"), ImVec2(kSidebarVisualW, kMainChildH), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			{
				if (g_Options.General.Particles)
				{
					static float sidebarPartAnim = 0.f;
					sidebarPartAnim += ImGui::GetIO().DeltaTime * 0.5f;
					ImDrawList* sbDl = ImGui::GetWindowDrawList();
					ImVec2 sbPos = ImGui::GetWindowPos();
					ImVec2 sbSize = ImGui::GetWindowSize();
					for (int i = 0; i < 24; i++)
					{
						float px = (sin(sidebarPartAnim + i * 0.7f) * 0.5f + 0.5f) * sbSize.x;
						float py = (cos(sidebarPartAnim + i * 0.4f) * 0.5f + 0.5f) * sbSize.y;
						sbDl->AddCircleFilled(sbPos + ImVec2(px, py), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
					}
				}
				const float itemH = 40.f;
				const float sidebarSpeed = ImGui::GetIO().DeltaTime * 10.f;
				static float sidebarAnimY = 200.f;
				const std::string tabLabels[5] = { _T("Combat"), _T("Visuals"), _T("Exploits"), _T("World"), _T("Settings") };
				const char* tabIcons[5] = { ICON_FA_CROSSHAIRS, ICON_FA_EYE, ICON_FA_VIRUS, ICON_FA_GLOBE, ICON_FA_GEAR };

				float totalTabsH = 5 * itemH;
				float startY = 200.f;
				float targetY = startY + CurrentTab * itemH;
				sidebarAnimY += (targetY - sidebarAnimY) * ImClamp(sidebarSpeed, 0.f, 1.f);
				ImGui::SetCursorPosY(startY);

				for (int i = 0; i < 5; i++)
				{
					const char* label = tabLabels[i].c_str();
					const char* icon = tabIcons[i];
					ImVec2 a = ImGui::GetCursorScreenPos();
					ImVec2 b = a + ImVec2(kSidebarVisualW, itemH);
					bool active = (CurrentTab == i);

					float tabAlpha = ImClamp(1.f - fabs((startY + i * itemH) - sidebarAnimY) / (itemH * 2.f), 0.55f, 1.f);
					ImU32 bgCol = active ? IM_COL32(64, 64, 64, 255) : IM_COL32(0, 0, 0, (int)(tabAlpha * 30));
					ImGui::GetWindowDrawList()->AddRectFilled(a, b, bgCol);

					ImU32 textCol = active ? IM_COL32(255, 255, 255, 255) : IM_COL32(152, 152, 158, (int)(tabAlpha * 255));

					ImGui::PushFont(FrameWork::Assets::FontAwesomeSolid14);
					ImVec2 iconSize = ImGui::CalcTextSize(icon);
					ImGui::GetWindowDrawList()->AddText(a + ImVec2(16.f, (itemH - iconSize.y) * 0.5f), textCol, icon);
					ImGui::PopFont();

					ImGui::PushFont(active ? FrameWork::Assets::InterBold : FrameWork::Assets::InterMedium);
					ImVec2 textSize = ImGui::CalcTextSize(label);
					ImGui::GetWindowDrawList()->AddText(a + ImVec2(16.f + iconSize.x + 8.f, (itemH - textSize.y) * 0.5f), textCol, label);
					ImGui::PopFont();
					ImGui::PushID(i);
					ImGui::InvisibleButton("tab", ImVec2(kSidebarVisualW, itemH));
					if (ImGui::IsItemClicked()) CurrentTab = i;
					ImGui::PopID();
				}

				ImVec2 sbScreen = ImGui::GetWindowPos();
				ImVec2 indicatorA = sbScreen + ImVec2(0.f, sidebarAnimY);
				ImVec2 indicatorB = indicatorA + ImVec2(3.f, itemH);
				ImGui::GetWindowDrawList()->AddRectFilled(indicatorA, indicatorB, IM_COL32(255, 0, 51, 255));
			}
			ImGui::EndChild();
			ImGui::PopStyleVar();

			ImGui::SetCursorPos(ImVec2(kSidebarW - kSidebarPad + kSidebarRightGap, kContentTop));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, ImGui::GetStyle().ItemSpacing.y));
			ImGui::BeginChild(XorStr("MainChild"), ImVec2(kMainChildW, kMainChildH), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			{
				if (CurrentTab == 0) // Legitbot
				{
					ImGui::Columns(2, NULL, false);
					ImGui::SetColumnWidth(0, kMainChildW * 0.5f);
					ImGui::SetColumnWidth(1, kMainChildW * 0.5f);

					float colH = ImGui::GetContentRegionAvail().y;
					ImGui::CustomChild(XorStr("General"), ImVec2(ImGui::GetColumnWidth() - 15, colH));
					{

						if (LegitSub == 0) // Aimbot
						{
							if (g_Options.General.Particles)
							{
								static float legitAnim1 = 0.f;
								legitAnim1 += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(legitAnim1 + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(legitAnim1 + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (FilterPass("Enabled")) { ImGui::Checkbox(_T("Enabled"), &g_Options.LegitBot.AimBot.Enabled); }
							if (FilterPass("Key")) { ImGui::KeyBind(_T("Key"), &g_Options.LegitBot.AimBot.KeyBind, &g_Options.LegitBot.AimBot.KeyBindState); }
							if (FilterPass("Target NPC")) { ImGui::Checkbox(_T("Target NPC"), &g_Options.LegitBot.AimBot.TargetNPC); }
							if (FilterPass("Visible Check")) { ImGui::Checkbox(_T("Visible Check"), &g_Options.LegitBot.AimBot.VisibleCheck); }
							if (FilterPass("Smooth X")) { ImGui::SliderInt(_T("Smooth X"), &g_Options.LegitBot.AimBot.SmoothHorizontal, 0, 100, XorStr("%d")); }
							{ const float bw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
							if (FilterPass("- Smooth X")) { if (ImGui::Button(XorStr("- SX##SmoothX"), ImVec2(bw, 0.0f))) { if (g_Options.LegitBot.AimBot.SmoothHorizontal > 0) g_Options.LegitBot.AimBot.SmoothHorizontal--; } }
							ImGui::SameLine();
							if (FilterPass("+ Smooth X")) { if (ImGui::Button(XorStr("+ SX##SmoothX"), ImVec2(bw, 0.0f))) { if (g_Options.LegitBot.AimBot.SmoothHorizontal < 100) g_Options.LegitBot.AimBot.SmoothHorizontal++; } } }
							if (FilterPass("Smooth Y")) { ImGui::SliderInt(_T("Smooth Y"), &g_Options.LegitBot.AimBot.SmoothVertical, 0, 100, XorStr("%d")); }
							{ const float bw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
							if (FilterPass("- Smooth Y")) { if (ImGui::Button(XorStr("- SY##SmoothY"), ImVec2(bw, 0.0f))) { if (g_Options.LegitBot.AimBot.SmoothVertical > 0) g_Options.LegitBot.AimBot.SmoothVertical--; } }
							ImGui::SameLine();
							if (FilterPass("+ Smooth Y")) { if (ImGui::Button(XorStr("+ SY##SmoothY"), ImVec2(bw, 0.0f))) { if (g_Options.LegitBot.AimBot.SmoothVertical < 100) g_Options.LegitBot.AimBot.SmoothVertical++; } } }
							if (FilterPass("Max Distance")) { ImGui::SliderInt(_T("Max Distance"), &g_Options.LegitBot.AimBot.MaxDistance, 0, 600, XorStr("%dm")); }
						}
						else if (LegitSub == 1) // Silent
						{
							if (g_Options.General.Particles)
							{
								static float legitAnim3 = 0.f;
								legitAnim3 += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(legitAnim3 + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(legitAnim3 + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (FilterPass("Enabled")) { ImGui::Checkbox(_T("Enabled"), &g_Options.LegitBot.SilentAim.Enabled); }
							if (FilterPass("Key")) { ImGui::KeyBind(_T("Key"), &g_Options.LegitBot.SilentAim.KeyBind, &g_Options.LegitBot.SilentAim.KeyBindState); }
							if (FilterPass("Target NPC")) { ImGui::Checkbox(_T("Target NPC"), &g_Options.LegitBot.SilentAim.ShotNPC); }
							if (FilterPass("Visible Check")) { ImGui::Checkbox(_T("Visible Check"), &g_Options.LegitBot.SilentAim.VisibleCheck); }
							if (FilterPass("Max Distance")) { ImGui::SliderInt(_T("Max Distance"), &g_Options.LegitBot.SilentAim.MaxDistance, 0, 600, XorStr("%dm")); }
						}
						else if (LegitSub == 2) // Trigger
						{
							if (g_Options.General.Particles)
							{
								static float legitAnim5 = 0.f;
								legitAnim5 += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(legitAnim5 + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(legitAnim5 + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (FilterPass("Enabled")) { ImGui::Checkbox(_T("Enabled"), &g_Options.LegitBot.Trigger.Enabled); }
							if (FilterPass("Key")) { ImGui::KeyBind(_T("Key"), &g_Options.LegitBot.Trigger.KeyBind, &g_Options.LegitBot.Trigger.KeyBindState); }
							if (FilterPass("Target NPC")) { ImGui::Checkbox(_T("Target NPC"), &g_Options.LegitBot.Trigger.ShotNPC); }
							if (FilterPass("Visible Check")) { ImGui::Checkbox(_T("Visible Check"), &g_Options.LegitBot.Trigger.VisibleCheck); }
							if (FilterPass("Smart Trigger")) { ImGui::Checkbox(_T("Smart Trigger"), &g_Options.LegitBot.Trigger.SmartTrigger); }
							if (FilterPass("Max Distance")) { ImGui::SliderInt(_T("Max Distance"), &g_Options.LegitBot.Trigger.MaxDistance, 0, 600, XorStr("%dm")); }
						}
					}
					ImGui::EndCustomChild();
					
					ImGui::NextColumn();

					ImGui::CustomChild(XorStr("Extra"), ImVec2(ImGui::GetColumnWidth(), colH));
					{
						if (LegitSub == 0) // Aimbot
						{
							if (g_Options.General.Particles)
							{
								static float legitAnim2 = 0.f;
								legitAnim2 += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(legitAnim2 + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(legitAnim2 + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (FilterPass("Field of View")) { ImGui::SliderInt(_T("Field of View"), &g_Options.LegitBot.AimBot.FOV, 0, 800, XorStr("%d px")); }
							{ const float bw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
							if (FilterPass("- FOV")) { if (ImGui::Button(XorStr("- FOV##AimFov"), ImVec2(bw, 0.0f))) { if (g_Options.LegitBot.AimBot.FOV > 0) g_Options.LegitBot.AimBot.FOV--; } }
							ImGui::SameLine();
							if (FilterPass("+ FOV")) { if (ImGui::Button(XorStr("+ FOV##AimFov"), ImVec2(bw, 0.0f))) { if (g_Options.LegitBot.AimBot.FOV < 800) g_Options.LegitBot.AimBot.FOV++; } } }
							if (FilterPass("HitBox")) { ImGui::Combo(_T("HitBox"), &g_Options.LegitBot.AimBot.HitBox, XorStr("Head\0Neck\0Chest\0")); }
							if (FilterPass("Show Fov")) { ImGui::Checkbox(_T("Show Fov"), &g_Options.Misc.Screen.ShowAimbotFov); }
							if (FilterPass("Fov Color")) { ImGui::ColorEdit4(_T("Fov Color"), g_Options.Misc.Screen.AimbotFovColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar); }
						}
						else if (LegitSub == 1) // Silent
						{
							if (g_Options.General.Particles)
							{
								static float legitAnim4 = 0.f;
								legitAnim4 += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(legitAnim4 + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(legitAnim4 + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (FilterPass("Field of View")) { ImGui::SliderInt(_T("Field of View"), &g_Options.LegitBot.SilentAim.Fov, 0, 800, XorStr("%d px")); }
							{ const float bw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
							if (FilterPass("- FOV")) { if (ImGui::Button(XorStr("- FOV##SilFov"), ImVec2(bw, 0.0f))) { if (g_Options.LegitBot.SilentAim.Fov > 0) g_Options.LegitBot.SilentAim.Fov--; } }
							ImGui::SameLine();
							if (FilterPass("+ FOV")) { if (ImGui::Button(XorStr("+ FOV##SilFov"), ImVec2(bw, 0.0f))) { if (g_Options.LegitBot.SilentAim.Fov < 800) g_Options.LegitBot.SilentAim.Fov++; } } }
							if (FilterPass("HitBox")) { ImGui::Combo(_T("HitBox"), &g_Options.LegitBot.SilentAim.HitBox, XorStr("Head\0Neck\0Chest\0")); }
							if (FilterPass("Magic Bullet")) { ImGui::Checkbox(_T("Magic Bullet"), &g_Options.LegitBot.MagicBullet.Enabled); }
							if (g_Options.LegitBot.MagicBullet.Enabled)
							{
								if (FilterPass("Magic Key")) { ImGui::KeyBind(_T("Magic Key"), &g_Options.LegitBot.MagicBullet.KeyBind, &g_Options.LegitBot.MagicBullet.KeyBindState); }
							}
							if (FilterPass("Miss Chance")) { ImGui::SliderInt(_T("Miss Chance"), &g_Options.LegitBot.SilentAim.MissChance, 0, 100, XorStr("%d%%")); }
							{ const float bw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
							if (FilterPass("- Miss")) { if (ImGui::Button(XorStr("- Miss##SilMiss"), ImVec2(bw, 0.0f))) { if (g_Options.LegitBot.SilentAim.MissChance > 0) g_Options.LegitBot.SilentAim.MissChance--; } }
							ImGui::SameLine();
							if (FilterPass("+ Miss")) { if (ImGui::Button(XorStr("+ Miss##SilMiss"), ImVec2(bw, 0.0f))) { if (g_Options.LegitBot.SilentAim.MissChance < 100) g_Options.LegitBot.SilentAim.MissChance++; } } }
							if (FilterPass("Show Fov")) { ImGui::Checkbox(_T("Show Fov"), &g_Options.LegitBot.SilentAim.ShowFov); }
							if (FilterPass("Fov Color")) { ImGui::ColorEdit4(_T("Fov Color"), g_Options.LegitBot.SilentAim.FovColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar); }
						}
						else if (LegitSub == 2) // Trigger
						{
							if (g_Options.General.Particles)
							{
								static float legitAnim6 = 0.f;
								legitAnim6 += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(legitAnim6 + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(legitAnim6 + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (FilterPass("Field of View")) { ImGui::SliderInt(_T("Field of View"), &g_Options.LegitBot.Trigger.Fov, 0, 800, XorStr("%d px")); }
							{ const float bw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
							if (FilterPass("- FOV")) { if (ImGui::Button(XorStr("- FOV##TriFov"), ImVec2(bw, 0.0f))) { if (g_Options.LegitBot.Trigger.Fov > 0) g_Options.LegitBot.Trigger.Fov--; } }
							ImGui::SameLine();
							if (FilterPass("+ FOV")) { if (ImGui::Button(XorStr("+ FOV##TriFov"), ImVec2(bw, 0.0f))) { if (g_Options.LegitBot.Trigger.Fov < 800) g_Options.LegitBot.Trigger.Fov++; } } }
							if (FilterPass("Reaction Time")) { ImGui::SliderInt(_T("Reaction Time"), &g_Options.LegitBot.Trigger.ReactionTime, 0, 500, XorStr("%dms")); }
							{ const float bw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
							if (FilterPass("- RT")) { if (ImGui::Button(XorStr("- RT##TriRT"), ImVec2(bw, 0.0f))) { if (g_Options.LegitBot.Trigger.ReactionTime >= 5) g_Options.LegitBot.Trigger.ReactionTime -= 5; else g_Options.LegitBot.Trigger.ReactionTime = 0; } }
							ImGui::SameLine();
							if (FilterPass("+ RT")) { if (ImGui::Button(XorStr("+ RT##TriRT"), ImVec2(bw, 0.0f))) { if (g_Options.LegitBot.Trigger.ReactionTime <= 495) g_Options.LegitBot.Trigger.ReactionTime += 5; } } }
							if (FilterPass("Show Fov")) { ImGui::Checkbox(_T("Show Fov"), &g_Options.LegitBot.Trigger.ShowFov); }
							if (FilterPass("Fov Color")) { ImGui::ColorEdit4(_T("Fov Color"), g_Options.LegitBot.Trigger.FovColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar); }
						}

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();
						if (FilterPass("Aim Dead")) { ImGui::Checkbox(_T("Aim Dead"), &g_Options.LegitBot.TargetDead); }
					}
					ImGui::EndCustomChild();

					ImGui::Columns(1);
				}
				else if (CurrentTab == 1) // Visuals
				{
					float colH = 0.f;
					if (VisualsSub == 0) // Players - Two columns
					{
						ImGui::Columns(2, NULL, false);
						ImGui::SetColumnWidth(0, kMainChildW * 0.62f);
						ImGui::SetColumnWidth(1, kMainChildW * 0.38f);
						colH = ImGui::GetContentRegionAvail().y;
						ImGui::CustomChild(XorStr("Filter"), ImVec2(ImGui::GetColumnWidth() - 15, colH));
					}
					else // Colors or Vehicles - Single column
					{
						ImGui::CustomChild(XorStr("Filter"), ImVec2(ImGui::GetContentRegionAvail().x - 10, ImGui::GetContentRegionAvail().y));
					}

					{
						if (VisualsSub == 0) // Players
						{
							if (g_Options.General.Particles)
							{
								static float visAnim1 = 0.f;
								visAnim1 += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(visAnim1 + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(visAnim1 + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (FilterPass("Enabled")) { ImGui::Checkbox(_T("Enabled"), &g_Options.Visuals.ESP.Players.Enabled); }
							if (FilterPass("Show NPC")) { ImGui::Checkbox(_T("Show NPC"), &g_Options.Visuals.ESP.Players.ShowNPCs); }
							if (FilterPass("Visible Check")) { ImGui::Checkbox(_T("Visible Check"), &g_Options.Visuals.ESP.Players.VisibleOnly); }
							if (FilterPass("Show Dead")) { ImGui::Checkbox(_T("Show Dead"), &g_Options.Visuals.ESP.Players.ExcludeDeads); }
							if (FilterPass("Show Local")) { ImGui::Checkbox(_T("Show Local"), &g_Options.Visuals.ESP.Players.ShowLocalPlayer); }
							if (FilterPass("Max Distance")) { ImGui::SliderInt(_T("Max Distance"), &g_Options.Visuals.ESP.Players.RenderDistance, 0, 600, XorStr("%dm")); }

							ImGui::Separator();

							if (FilterPass("Name")) { ImGui::Checkbox(_T("Name"), &g_Options.Visuals.ESP.Players.Name); }
							if (FilterPass("Weapon Name")) { ImGui::Checkbox(_T("Weapon Name"), &g_Options.Visuals.ESP.Players.WeaponName); }
							if (FilterPass("Skeleton")) { ImGui::Checkbox(_T("Skeleton"), &g_Options.Visuals.ESP.Players.Skeleton); }
							if (FilterPass("Head Circle")) { ImGui::Checkbox(_T("Head Circle"), &g_Options.Visuals.ESP.Players.HeadCircle); }
							if (FilterPass("Armor Bar")) { ImGui::Checkbox(_T("Armor Bar"), &g_Options.Visuals.ESP.Players.ArmorBar); }
							if (FilterPass("Health Bar")) { ImGui::Checkbox(_T("Health Bar"), &g_Options.Visuals.ESP.Players.HealthBar); }
							if (FilterPass("RGB Mode")) { ImGui::Checkbox(_T("RGB Mode"), &g_Options.Visuals.ESP.Players.RGB); }
							if (g_Options.Visuals.ESP.Players.RGB && FilterPass("RGB Speed"))
								{ ImGui::SliderFloat(_T("RGB Speed"), &g_Options.Visuals.ESP.Players.RGBSpeed, 0.1f, 10.0f, "%.1f"); }
							if (FilterPass("Distance")) { ImGui::Checkbox(_T("Distance"), &g_Options.Visuals.ESP.Players.Distance); }
							if (FilterPass("Box")) { ImGui::Checkbox(_T("Box"), &g_Options.Visuals.ESP.Players.Box); }
							if (FilterPass("Snap Lines")) { ImGui::Checkbox(_T("Snap Lines"), &g_Options.Visuals.ESP.Players.SnapLines); }
							if (FilterPass("Highlight Visible")) { ImGui::Checkbox(_T("Highlight Visible"), &g_Options.Visuals.ESP.Players.HighlightVisible); }
						}
						else if (VisualsSub == 1) // Colors
						{
							if (g_Options.General.Particles)
							{
								static float visAnim5 = 0.f;
								visAnim5 += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(visAnim5 + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(visAnim5 + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (FilterPass("Box Color")) { ImGui::ColorEdit4(_T("Box Color"), g_Options.Visuals.ESP.Players.BoxColor, ImGuiColorEditFlags_AlphaBar); }
							if (FilterPass("Skeleton Color")) { ImGui::ColorEdit4(_T("Skeleton Color"), g_Options.Visuals.ESP.Players.SkeletonColor, ImGuiColorEditFlags_AlphaBar); }
							if (FilterPass("Text Color")) { ImGui::ColorEdit4(_T("Text Color"), g_Options.Visuals.ESP.Players.TextColor, ImGuiColorEditFlags_AlphaBar); }
							if (FilterPass("Snaplines Color")) { ImGui::ColorEdit4(_T("Snaplines Color"), g_Options.Visuals.ESP.Players.SnapLinesColor, ImGuiColorEditFlags_AlphaBar); }
							if (FilterPass("Health Bar Color")) { ImGui::ColorEdit4(_T("Health Bar Color"), g_Options.Visuals.ESP.Players.HealthBarColor, ImGuiColorEditFlags_AlphaBar); }
							if (FilterPass("Armor Bar Color")) { ImGui::ColorEdit4(_T("Armor Bar Color"), g_Options.Visuals.ESP.Players.ArmorColor, ImGuiColorEditFlags_AlphaBar); }
							if (FilterPass("Head Circle Color")) { ImGui::ColorEdit4(_T("Head Circle Color"), g_Options.Visuals.ESP.Players.HeadCircleColor, ImGuiColorEditFlags_AlphaBar); }
							if (FilterPass("Friend Color")) { ImGui::ColorEdit4(_T("Friend Color"), g_Options.Visuals.ESP.Players.FriendSkeletonColor, ImGuiColorEditFlags_AlphaBar); }

						}
						else if (VisualsSub == 2) // Vehicles
						{
							if (g_Options.General.Particles)
							{
								static float visAnim3 = 0.f;
								visAnim3 += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(visAnim3 + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(visAnim3 + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (FilterPass("Enabled")) { ImGui::Checkbox(_T("Enabled"), &g_Options.Visuals.ESP.Vehicles.Enabled); }
							if (FilterPass("Max Distance")) { ImGui::SliderInt(_T("Max Distance"), &g_Options.Visuals.ESP.Vehicles.RenderDistance, 0, 600, XorStr("%dm")); }

							ImGui::Separator();

							if (FilterPass("Name")) { ImGui::Checkbox(_T("Name"), &g_Options.Visuals.ESP.Vehicles.Model); }
							if (FilterPass("Distance")) { ImGui::Checkbox(_T("Distance"), &g_Options.Visuals.ESP.Vehicles.Distance); }
							if (FilterPass("Doors (Lock/Unlock)")) { ImGui::Checkbox(_T("Doors (Lock/Unlock)"), &g_Options.Visuals.ESP.Vehicles.Door); }
							if (FilterPass("Snap Lines")) { ImGui::Checkbox(_T("Snap Lines"), &g_Options.Visuals.ESP.Vehicles.SnapLines); }
							if (FilterPass("Lock Status Icon")) { ImGui::Checkbox(_T("Lock Status Icon"), &g_Options.Visuals.ESP.Vehicles.LockStatus); }

							ImGui::Separator();

							ImGui::Separator();

							if (FilterPass("Text Color")) { ImGui::ColorEdit4(_T("Text Color"), g_Options.Visuals.ESP.Vehicles.TextColor, ImGuiColorEditFlags_AlphaBar); }
							if (FilterPass("Name Color")) { ImGui::ColorEdit4(_T("Name Color"), g_Options.Visuals.ESP.Vehicles.VehicleNameColor, ImGuiColorEditFlags_AlphaBar); }
							if (FilterPass("Distance Color")) { ImGui::ColorEdit4(_T("Distance Color"), g_Options.Visuals.ESP.Vehicles.VehicleDistanceColor, ImGuiColorEditFlags_AlphaBar); }
							if (FilterPass("Snap Lines Color")) { ImGui::ColorEdit4(_T("Snap Lines Color"), g_Options.Visuals.ESP.Vehicles.SnapLinesColor, ImGuiColorEditFlags_AlphaBar); }
							if (FilterPass("Locked Color")) { ImGui::ColorEdit4(_T("Locked Color"), g_Options.Visuals.ESP.Vehicles.LockedColor, ImGuiColorEditFlags_AlphaBar); }
							if (FilterPass("Unlocked Color")) { ImGui::ColorEdit4(_T("Unlocked Color"), g_Options.Visuals.ESP.Vehicles.UnlockedColor, ImGuiColorEditFlags_AlphaBar); }
						}
					}
					ImGui::EndCustomChild();

					if (VisualsSub == 0) // Players - show Preview column
					{
						ImGui::NextColumn();

						ImGui::CustomChild(XorStr("Preview"), ImVec2(ImGui::GetContentRegionAvail().x, colH));
						{
							if (g_Options.General.Particles)
							{
								static float visAnim2 = 0.f;
								visAnim2 += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(visAnim2 + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(visAnim2 + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							Cheat::g_EspPreview.DragDropHandler();
							Cheat::g_EspPreview.Draw();
						}
						ImGui::EndCustomChild();

						ImGui::Columns(1);
					}
				}
				else if (CurrentTab == 2) // Exploits
				{
					static int ExploitsSubDisplay = 0;
					static float ExploitsSubAlpha = 1.f;
					float subSpeed = ImGui::GetIO().DeltaTime * 5.f;
					if (ExploitsSub != ExploitsSubDisplay)
						ExploitsSubAlpha = ImMax(ExploitsSubAlpha - subSpeed, 0.f);
					else
						ExploitsSubAlpha = ImMin(ExploitsSubAlpha + subSpeed, 1.f);
					if (ExploitsSubAlpha == 0.f)
						ExploitsSubDisplay = ExploitsSub;

					auto SubContent = [&]()
					{

					if (ExploitsSubDisplay == 0) // Player
					{
						ImGui::Columns(2, NULL, false);
						ImGui::SetColumnWidth(0, kMainChildW * 0.5f);
						ImGui::SetColumnWidth(1, kMainChildW * 0.5f);

						float colH = ImGui::GetContentRegionAvail().y;
						ImGui::CustomChild(XorStr("PlayerContent"), ImVec2(ImGui::GetColumnWidth() - 15, colH));
						{
							if (g_Options.General.Particles)
							{
								static float playerAnim = 0.f;
								playerAnim += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(playerAnim + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(playerAnim + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}



							if (FilterPass("God Mode")) { ImGui::Checkbox(_T("God Mode"), &g_Options.Misc.Exploits.LocalPlayer.GodMode); }
							if (g_Options.Misc.Exploits.LocalPlayer.GodMode && FilterPass("God Mode Key"))
								{ ImGui::KeyBind(_T("God Mode Key"), &g_Options.Misc.Exploits.LocalPlayer.GodModeKey, &g_Options.Misc.Exploits.LocalPlayer.GodModeKeyState); }
							if (FilterPass("Anti-Aim")) { ImGui::Checkbox(_T("Anti-Aim"), &g_Options.Misc.Exploits.LocalPlayer.AntiAimEnabled); }
							if (g_Options.Misc.Exploits.LocalPlayer.AntiAimEnabled && FilterPass("Anti-Aim Key"))
								{ ImGui::KeyBind(_T("Anti-Aim Key"), &g_Options.Misc.Exploits.LocalPlayer.AntiAimKey, &g_Options.Misc.Exploits.LocalPlayer.AntiAimKeyState); }
							if (FilterPass("Invisible")) { ImGui::Checkbox(_T("Invisible"), &g_Options.Misc.Exploits.LocalPlayer.Invisible); }
							if (FilterPass("Noclip")) { ImGui::Checkbox(_T("Noclip"), &g_Options.Misc.Exploits.LocalPlayer.Noclip); }
							if (g_Options.Misc.Exploits.LocalPlayer.Noclip)
							{
								if (FilterPass("Noclip Key")) { ImGui::KeyBind(_T("Noclip Key"), &g_Options.Misc.Exploits.LocalPlayer.NoclipKey, &g_Options.Misc.Exploits.LocalPlayer.NoclipKeyState); }
								if (FilterPass("Noclip Speed")) { ImGui::SliderFloat(_T("Noclip Speed"), &g_Options.Misc.Exploits.LocalPlayer.NoClipSpeed, 0.1f, 20.f, "%.1f"); }
							}
							if (FilterPass("Infinite Combat Roll")) { ImGui::Checkbox(_T("Infinite Combat Roll"), &g_Options.Misc.Exploits.LocalPlayer.InfiniteCombatRoll); }
							if (FilterPass("Infinite Stamina")) { ImGui::Checkbox(_T("Infinite Stamina"), &g_Options.Misc.Exploits.LocalPlayer.InfiniteStamina); }
							if (FilterPass("Fast Run")) { ImGui::Checkbox(_T("Fast Run"), &g_Options.Misc.Exploits.LocalPlayer.FastRun); }
							if (g_Options.Misc.Exploits.LocalPlayer.FastRun && FilterPass("Run Speed"))
								{ ImGui::SliderFloat(_T("Run Speed"), &g_Options.Misc.Exploits.LocalPlayer.RunSpeed, 1.f, 10.f, "%.1f"); }
							if (FilterPass("Shrink")) { ImGui::Checkbox(_T("Shrink"), &g_Options.Misc.Exploits.LocalPlayer.Shrink); }
							ImGui::Spacing();
							ImGui::SeparatorText(_T("Weapons"));
							ImGui::Spacing();
							if (FilterPass("Infinite Ammo")) { ImGui::Checkbox(_T("Infinite Ammo"), &g_Options.Misc.Exploits.Weapon.InfiniteAmmoEnabled); }
							if (FilterPass("No Reload")) { ImGui::Checkbox(_T("No Reload"), &g_Options.Misc.Exploits.Weapon.NoReload); }
							if (FilterPass("No Recoil")) { ImGui::Checkbox(_T("No Recoil"), &g_Options.Misc.Exploits.Weapon.RemoveRecoil); }
							if (FilterPass("No Spread")) { ImGui::Checkbox(_T("No Spread"), &g_Options.Misc.Exploits.Weapon.RemoveSpread); }
							if (FilterPass("Weapon Scale")) { ImGui::Checkbox(_T("Weapon Scale"), &g_Options.Misc.Exploits.Weapon.WeaponScaleEnabled); }
							if (g_Options.Misc.Exploits.Weapon.WeaponScaleEnabled && FilterPass("Weapon Scale Value"))
								{ ImGui::SliderFloat(_T("Weapon Scale Value"), &g_Options.Misc.Exploits.Weapon.WeaponScale, 0.1f, 10.0f, "%.1f"); }
							if (FilterPass("Unlock All Actions")) { ImGui::Checkbox(_T("Unlock All Actions"), &g_Options.Misc.Exploits.Weapon.Coronhada); }
							if (g_Options.Misc.Exploits.Weapon.Coronhada && FilterPass("Unlock All Actions Key"))
								{ ImGui::KeyBind(_T("Anim Trigger Key"), &g_Options.Misc.Exploits.Weapon.CoronhadaKey, &g_Options.Misc.Exploits.Weapon.CoronhadaKeyState); }
						}
						ImGui::EndCustomChild();

						ImGui::NextColumn();

						ImGui::CustomChild(XorStr("PlayerExtra"), ImVec2(ImGui::GetColumnWidth(), colH));
						{
							if (g_Options.General.Particles)
							{
								static float playerAnim2 = 0.f;
								playerAnim2 += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(playerAnim2 + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(playerAnim2 + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							float btnPad = 12.0f;
							ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));

							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
							if (FilterPass("Full Health")) { if (ImGui::Button(_T("Full Health"), ImVec2(ImGui::GetContentRegionAvail().x - btnPad, 25)))
								Cheat::Exploits::SetLife(); }
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
							if (FilterPass("Full Armor")) { if (ImGui::Button(_T("Full Armor"), ImVec2(ImGui::GetContentRegionAvail().x - btnPad, 25)))
								Cheat::Exploits::SetArmor(); }
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
							if (FilterPass("Revive")) { if (ImGui::Button(_T("Revive"), ImVec2(ImGui::GetContentRegionAvail().x - btnPad, 25)))
								Cheat::Exploits::RevivePlayer(); }
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
							if (FilterPass("Suicide")) { if (ImGui::Button(_T("Suicide"), ImVec2(ImGui::GetContentRegionAvail().x - btnPad, 25)))
								Cheat::Exploits::SuicidePlayer(); }
							ImGui::PopStyleVar();
							ImGui::Spacing();
							ImGui::SeparatorText(_T("Teleport"));
							ImGui::Spacing();
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
							if (FilterPass("Teleport to Waypoint")) { if (ImGui::Button(_T("Teleport to Waypoint"), ImVec2(ImGui::GetContentRegionAvail().x - btnPad, 25)))
								Cheat::Exploits::TeleportToWaypoint(); }
							if (FilterPass("TP Way Key")) { ImGui::KeyBind(_T("TP Way Key"), &g_Options.Misc.Exploits.LocalPlayer.TpWayKey, &g_Options.Misc.Exploits.LocalPlayer.TpWayKeyState); }
							ImGui::Spacing();
							static int selectedLocation = 0;
							const char* locationNames[] = { "Square", "Pier", "Paleto Bay", "Central Bank", "Cassino", "Ls Airport", "Sandy Shores" };
							Vector3D locationCoords[] = {
								Vector3D(156.184f, -1043.17f, 29.3236f),
								Vector3D(-1847.72f, -1223.36f, 13.8745f),
								Vector3D(-397.605f, 6047.57f, 32.1797f),
								Vector3D(221.781f, 217.278f, 106.705f),
								Vector3D(885.322f, 16.8489f, 80.65f),
								Vector3D(-975.532f, -2880.89f, 16.2665f),
								Vector3D(1681.48f, 3251.91f, 40.809f),
							};
							constexpr int locCount = 7;
							struct LocEntry { int idx; float dist; };
							LocEntry sorted[locCount];
							Vector3D localPos;
							if (Cheat::g_Fivem.IsInitialized())
								localPos = Cheat::g_Fivem.GetLocalPlayerInfo().WorldPos;
							for (int i = 0; i < locCount; i++)
							{
								sorted[i].idx = i;
								sorted[i].dist = locationCoords[i].DistTo(localPos);
							}
							std::sort(sorted, sorted + locCount, [](const LocEntry& a, const LocEntry& b) { return a.dist < b.dist; });
							ImGui::TextDisabled(_T("Saved Locations"));
							ImGui::BeginChild("TeleportLocations", ImVec2(ImGui::GetContentRegionAvail().x - 10, ImGui::GetContentRegionAvail().y - 8), true);
							ImGui::Indent(12.f);
							ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.f);
							for (int i = 0; i < locCount; i++)
							{
								int idx = sorted[i].idx;
								float dist = sorted[i].dist;
								char line[288];
								std::snprintf(line, sizeof(line), "%s  |  %.0fm", locationNames[idx], dist);
								ImGui::PushID(93000 + i);
								bool isSel = (selectedLocation == idx);
								if (ImGui::Selectable(line, isSel))
									selectedLocation = idx;
								if (isSel)
								{
									ImVec2 rMin = ImGui::GetItemRectMin();
									ImVec2 rMax = ImGui::GetItemRectMax();
									ImDrawList* dl = ImGui::GetWindowDrawList();
									float w = ImGui::GetContentRegionAvail().x + 4.f;
									float lo = 10.f;
									dl->AddRectFilled(ImVec2(rMin.x - lo, rMin.y), ImVec2(rMin.x + w, rMax.y), IM_COL32(255, 0, 51, 40));
									dl->AddRectFilled(ImVec2(rMin.x - lo, rMin.y), ImVec2(rMin.x - lo + 3, rMax.y), IM_COL32(255, 0, 51, 200));
								}
								ImGui::PopID();
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f);
							}
							ImGui::Unindent(12.f);
							ImGui::EndChild();
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
							if (FilterPass("Teleport to Location")) { if (ImGui::Button(_T("Teleport to Location"), ImVec2(ImGui::GetContentRegionAvail().x - btnPad, 25)))
								Cheat::Exploits::TeleportLocalToCoords(locationCoords[selectedLocation]); }
						}
						ImGui::EndCustomChild();
						ImGui::Columns(1);
					}
					else if (ExploitsSubDisplay == 1) // Vehicles
					{
						ImGui::Columns(2, NULL, false);
						ImGui::SetColumnWidth(0, kMainChildW * 0.5f);
						ImGui::SetColumnWidth(1, kMainChildW * 0.5f);

						float colH = ImGui::GetContentRegionAvail().y;
						ImGui::CustomChild(XorStr("VehiclesContent"), ImVec2(ImGui::GetColumnWidth() - 15, colH));
						{
							if (g_Options.General.Particles)
							{
								static float vehAnim = 0.f;
								vehAnim += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(vehAnim + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(vehAnim + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (FilterPass("Vehicle GodMode")) { ImGui::Checkbox(_T("Vehicle GodMode"), &g_Options.Misc.Exploits.Vehicle.GodMode); }
							if (FilterPass("Seat Belt")) { ImGui::Checkbox(_T("Seat Belt"), &g_Options.Misc.Exploits.LocalPlayer.SeatBelt); }
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();
							float btnPad = 12.0f;
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
							if (FilterPass("Repair Vehicle")) { if (ImGui::Button(_T("Repair Vehicle"), ImVec2(ImGui::GetContentRegionAvail().x - btnPad, 35)))
								Cheat::Exploits::RepairVehicle(); }
							if (FilterPass("Repair Key")) { ImGui::KeyBind(_T("Repair Key"), &g_Options.Misc.Exploits.Vehicle.RepairKey, &g_Options.Misc.Exploits.Vehicle.RepairKeyState); }
							ImGui::Spacing();
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
							if (FilterPass("Break Vehicle")) { if (ImGui::Button(_T("Break Vehicle"), ImVec2(ImGui::GetContentRegionAvail().x - btnPad, 35)))
								Cheat::Exploits::BreakVehicle(); }
							ImGui::Spacing();
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
							if (FilterPass("Unlock Nearby Vehicles")) { if (ImGui::Button(_T("Unlock Nearby Vehicles"), ImVec2(ImGui::GetContentRegionAvail().x - btnPad, 35)))
								Cheat::Exploits::UnlockNearbyVehicles(); }
							if (FilterPass("Unlock Nearby Key")) { ImGui::KeyBind(_T("Unlock Nearby Key"), &g_Options.Misc.Exploits.Vehicle.UnlockNearbyKey, &g_Options.Misc.Exploits.Vehicle.UnlockNearbyKeyState); }
						}
						ImGui::EndCustomChild();

						ImGui::NextColumn();

						ImGui::CustomChild(XorStr("VehicleExtra"), ImVec2(ImGui::GetColumnWidth(), colH));
						{
							if (g_Options.General.Particles)
							{
								static float vehAnimB = 0.f;
								vehAnimB += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(vehAnimB + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(vehAnimB + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (FilterPass("Rocket Boost")) { ImGui::Checkbox(_T("Rocket Boost"), &g_Options.Misc.Exploits.Vehicle.RocketBoost); }
							if (FilterPass("Rocket Boost Key")) { ImGui::KeyBind(_T("Rocket Boost Key"), &g_Options.Misc.Exploits.Vehicle.RocketBoostKey, &g_Options.Misc.Exploits.Vehicle.RocketBoostKeyState); }
							if (FilterPass("Boost Strength")) { ImGui::SliderFloat(_T("Boost Strength"), &g_Options.Misc.Exploits.Vehicle.RocketBoostStrength, 0.0f, 20.0f, XorStr("%.1f")); }
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();
							if (FilterPass("Vehicle Boost")) { ImGui::SliderFloat(_T("Vehicle Boost"), &g_Options.Misc.Exploits.LocalPlayer.v_Boost, 1.0f, 100.0f, XorStr("%.1f")); }
						}
						ImGui::EndCustomChild();

						ImGui::Columns(1);
					}
					};

					SubContent();
				}
				else if (CurrentTab == 3) // World
				{
					ImGui::Columns(2, NULL, false);
					ImGui::SetColumnWidth(0, kMainChildW * 0.5f);
					ImGui::SetColumnWidth(1, kMainChildW * 0.5f);

					float availListY = ImGui::GetContentRegionAvail().y;
					float listH = ImMax(150.f, availListY);
					float optsH = ImMax(100.f, availListY);

					if (WorldSub == 0) // Players
					{
						ImGui::CustomChild(XorStr("WorldPlayers"), ImVec2(ImGui::GetColumnWidth() - 15, listH));
						{
							if (g_Options.General.Particles)
							{
								static float worldAnim = 0.f;
								worldAnim += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(worldAnim + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(worldAnim + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (!Cheat::g_Fivem.IsInitialized())
								ImGui::TextDisabled(_T("Not connected..."));
							else
							{
								auto entities = Cheat::g_Fivem.GetEntitiyList();
								auto local = Cheat::g_Fivem.GetLocalPlayerInfo();
								Vector3D localPos = local.WorldPos;

								std::sort(entities.begin(), entities.end(), [&localPos](const auto& a, const auto& b) {
									if (!a.StaticInfo.Ped) return false;
									if (!b.StaticInfo.Ped) return true;
									return a.Cordinates.DistTo(localPos) < b.Cordinates.DistTo(localPos);
									});

								if (WorldSelPlayerNetId != -1)
								{
									auto it = std::find_if(entities.begin(), entities.end(), [&](const auto& e) { return e.StaticInfo.NetId == WorldSelPlayerNetId; });
									if (it == entities.end())
										WorldSelPlayerNetId = -1;
								}

								ImGui::TextDisabled(_T("Players"));
								ImGui::BeginChild("WorldPlayersList", ImVec2(ImGui::GetContentRegionAvail().x - 10, ImGui::GetContentRegionAvail().y - 8), true);
								ImGui::Indent(12.f);
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.f);
								for (int i = 0; i < (int)entities.size(); i++)
								{
									const auto& e = entities[i];
									float dist = e.Cordinates.DistTo(localPos);
									char line[320];
									if (e.StaticInfo.bIsLocalPlayer)
										std::snprintf(line, sizeof(line), "%s  |  %.0fm  |  %s", e.StaticInfo.Name.c_str(), dist, _T("You"));
									else if (e.StaticInfo.bIsNPC)
										std::snprintf(line, sizeof(line), "%s  |  %.0fm  |  NPC", e.StaticInfo.Name.c_str(), dist);
									else
										std::snprintf(line, sizeof(line), "%s  |  %.0fm  |  ID %d", e.StaticInfo.Name.c_str(), dist, e.StaticInfo.NetId);

									ImGui::PushID(91000 + i);
									bool isSel = (e.StaticInfo.NetId == WorldSelPlayerNetId);
									if (ImGui::Selectable(line, isSel))
									{
										WorldSelPlayerNetId = e.StaticInfo.NetId;
										WorldSelVehicleAddr = 0;
									}
									if (isSel)
									{
										ImVec2 rMin = ImGui::GetItemRectMin();
										ImVec2 rMax = ImGui::GetItemRectMax();
										ImDrawList* dl = ImGui::GetWindowDrawList();
										float w = ImGui::GetContentRegionAvail().x + 4.f;
										float lo = 10.f;
										dl->AddRectFilled(ImVec2(rMin.x - lo, rMin.y), ImVec2(rMin.x + w, rMax.y), IM_COL32(255, 0, 51, 40));
										dl->AddRectFilled(ImVec2(rMin.x - lo, rMin.y), ImVec2(rMin.x - lo + 3, rMax.y), IM_COL32(255, 0, 51, 200));
									}
									ImGui::PopID();
									ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f);
								}
								ImGui::Unindent(12.f);
								ImGui::EndChild();
							}
						}
						ImGui::EndCustomChild();

						ImGui::NextColumn();

						ImGui::CustomChild(XorStr("WorldActions"), ImVec2(ImGui::GetColumnWidth(), optsH));
						{
							if (g_Options.General.Particles)
							{
								static float worldAnim2 = 0.f;
								worldAnim2 += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(worldAnim2 + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(worldAnim2 + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (Cheat::g_Fivem.IsInitialized())
							{
								auto entities = Cheat::g_Fivem.GetEntitiyList();
								float btnPad = 12.0f;
								float btnW = ImMax(80.f, ImGui::GetContentRegionAvail().x - btnPad * 2);

								auto selIt = std::find_if(entities.begin(), entities.end(), [&](const auto& e) { return e.StaticInfo.NetId == WorldSelPlayerNetId; });
								if (selIt == entities.end())
									ImGui::TextDisabled(_T("Select a player from the list"));
								else
								{
									const auto& sel = *selIt;
									{
										ImVec2 p = ImGui::GetCursorScreenPos();
										ImDrawList* dl = ImGui::GetWindowDrawList();
										float w = ImGui::GetContentRegionAvail().x;
										float h = 28.f;
										dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), IM_COL32(255, 0, 51, 30));
										dl->AddRectFilled(p, ImVec2(p.x + 3, p.y + h), IM_COL32(255, 0, 51, 180));
										ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.f);
									}
									ImGui::Indent(12.f);
									ImGui::PushFont(FrameWork::Assets::InterBold);
									ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", sel.StaticInfo.Name.c_str());
									ImGui::PopFont();
									ImGui::Unindent(12.f);
									ImGui::TextDisabled("Player selected");
									if (sel.StaticInfo.bIsLocalPlayer)
									{
										if (Cheat::Exploits::WorldSpectateActive())
										{
										ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
										if (FilterPass("Stop spectating")) { if (ImGui::Button(_T("Stop spectating"), ImVec2(btnW, 24)))
											Cheat::Exploits::WorldSetSpectateTarget(nullptr); }
										}
									}
									else
									{
										ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
										if (FilterPass("Teleport to player")) { if (ImGui::Button(_T("Teleport to player"), ImVec2(btnW, 24)))
										{
											if (sel.StaticInfo.Ped)
												Cheat::Exploits::TeleportLocalToCoords(sel.StaticInfo.Ped->GetCoordinate());
										} }
										const bool spectatingThis = sel.StaticInfo.Ped && Cheat::Exploits::WorldIsSpectatingPed(sel.StaticInfo.Ped);
										ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
										if (ImGui::Button(spectatingThis ? _T("Stop spectate") : _T("Spectate"), ImVec2(btnW, 24)))
										{
											if (spectatingThis)
												Cheat::Exploits::WorldSetSpectateTarget(nullptr);
											else if (sel.StaticInfo.Ped)
												Cheat::Exploits::WorldSetSpectateTarget(sel.StaticInfo.Ped);
										}
										ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
										if (FilterPass("Copy Clothes")) { if (ImGui::Button(_T("Copy Clothes"), ImVec2(btnW, 24)))
										{
											if (sel.StaticInfo.Ped)
												Cheat::Exploits::CopyClothes(sel.StaticInfo.Ped);
										} }
										ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
										if (FilterPass("Pull Player")) { if (ImGui::Button(_T("Pull Player"), ImVec2(btnW, 24)))
										{
											if (sel.StaticInfo.Ped)
											{
												auto* localPed = Cheat::g_Fivem.GetLocalPlayerInfo().Ped;
												if (localPed)
												{
													Cheat::g_Fivem.network_request_control_of_entity((uint64_t)sel.StaticInfo.Ped, (uint64_t)localPed);
													Vector3D playerPos = localPed->GetCoordinate();
													float h = Cheat::GetPlayerHeading();
													const float pullDist = 3.5f;
													Vector3D spawnPos;
													spawnPos.x = playerPos.x - std::sin(h) * pullDist;
													spawnPos.y = playerPos.y + std::cos(h) * pullDist;
													spawnPos.z = playerPos.z;
													uint64_t pedNav = sel.StaticInfo.Ped->GetNavigation();
													uint64_t pedModel = sel.StaticInfo.Ped->GetModelInfo();
													if (pedNav && pedModel)
														Cheat::g_Fivem.TeleportToObject((uintptr_t)sel.StaticInfo.Ped, pedNav, pedModel, spawnPos, spawnPos, true);

												}
											}
										} }
										if (!sel.StaticInfo.bIsNPC && sel.StaticInfo.NetId >= 0)
										{
											const bool isFriend = (Cheat::g_Fivem.FriendList.find(sel.StaticInfo.NetId) != Cheat::g_Fivem.FriendList.end());
											ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
											if (ImGui::Button(isFriend ? _T("Remove friend") : _T("Add friend"), ImVec2(btnW, 24)))
											{
												if (isFriend)
													Cheat::g_Fivem.FriendList.erase(sel.StaticInfo.NetId);
												else
													Cheat::g_Fivem.FriendList[sel.StaticInfo.NetId] = sel.StaticInfo;
											}
										}
									}
								}
							}
						}
						ImGui::EndCustomChild();
					}
					else if (WorldSub == 1) // Vehicles
					{
						ImGui::CustomChild(XorStr("WorldVehiclesList"), ImVec2(ImGui::GetColumnWidth() - 15, listH));
						{
							if (g_Options.General.Particles)
							{
								static float worldVehAnim = 0.f;
								worldVehAnim += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(worldVehAnim + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(worldVehAnim + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (!Cheat::g_Fivem.IsInitialized())
								ImGui::TextDisabled(_T("Not connected..."));
							else
							{
								auto vehs = Cheat::g_Fivem.GetVehicleList();
								auto local = Cheat::g_Fivem.GetLocalPlayerInfo();
								Vector3D localPos = local.WorldPos;

								std::sort(vehs.begin(), vehs.end(), [&localPos](const auto& a, const auto& b) {
									if (!a.Vehicle) return false;
									if (!b.Vehicle) return true;
									return a.Vehicle->GetCoordinate().DistTo(localPos) < b.Vehicle->GetCoordinate().DistTo(localPos);
									});

								if (WorldSelVehicleAddr)
								{
									auto it = std::find_if(vehs.begin(), vehs.end(), [&](const auto& v) { return (uintptr_t)v.Vehicle == WorldSelVehicleAddr; });
									if (it == vehs.end())
										WorldSelVehicleAddr = 0;
								}

								ImGui::TextDisabled(_T("Vehicles"));
								ImGui::BeginChild("WorldVehicleList", ImVec2(ImGui::GetContentRegionAvail().x - 10, ImGui::GetContentRegionAvail().y - 8), true);
								ImGui::Indent(12.f);
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.f);
								for (int i = 0; i < (int)vehs.size(); i++)
								{
									const auto& v = vehs[i];
									if (!v.Vehicle) continue;
									float dist = v.Vehicle->GetCoordinate().DistTo(localPos);
									const char* vname = v.Name.empty() ? _T("Vehicle") : v.Name.c_str();
									char line[288];
									std::snprintf(line, sizeof(line), "%s  |  %.0fm", vname, dist);

									ImGui::PushID(92000 + i);
									bool isSel = ((uintptr_t)v.Vehicle == WorldSelVehicleAddr);
									if (ImGui::Selectable(line, isSel))
									{
										WorldSelVehicleAddr = (uintptr_t)v.Vehicle;
										WorldSelPlayerNetId = -1;
									}
									if (isSel)
									{
										ImVec2 rMin = ImGui::GetItemRectMin();
										ImVec2 rMax = ImGui::GetItemRectMax();
										ImDrawList* dl = ImGui::GetWindowDrawList();
										float w = ImGui::GetContentRegionAvail().x + 4.f;
										float lo = 10.f;
										dl->AddRectFilled(ImVec2(rMin.x - lo, rMin.y), ImVec2(rMin.x + w, rMax.y), IM_COL32(255, 0, 51, 40));
										dl->AddRectFilled(ImVec2(rMin.x - lo, rMin.y), ImVec2(rMin.x - lo + 3, rMax.y), IM_COL32(255, 0, 51, 200));
									}
									ImGui::PopID();
									ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f);
								}
								ImGui::Unindent(12.f);
								ImGui::EndChild();
							}
						}
						ImGui::EndCustomChild();

						ImGui::NextColumn();

						ImGui::CustomChild(XorStr("WorldVehicleOpts"), ImVec2(ImGui::GetColumnWidth(), optsH));
						{
							if (g_Options.General.Particles)
							{
								static float worldVehAnim2 = 0.f;
								worldVehAnim2 += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(worldVehAnim2 + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(worldVehAnim2 + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (Cheat::g_Fivem.IsInitialized())
							{
								auto vehs = Cheat::g_Fivem.GetVehicleList();
								float btnPad = 12.0f;
								float btnW = ImMax(80.f, ImGui::GetContentRegionAvail().x - btnPad * 2);

								auto vIt = std::find_if(vehs.begin(), vehs.end(), [&](const auto& v) { return (uintptr_t)v.Vehicle == WorldSelVehicleAddr; });
								if (vIt == vehs.end() || !vIt->Vehicle)
									ImGui::TextDisabled(_T("Select a vehicle from the list"));
								else
								{
									auto* v = vIt->Vehicle;
									const char* vname = vIt->Name.empty() ? XorStr("Vehicle") : vIt->Name.c_str();
									{
										ImVec2 p = ImGui::GetCursorScreenPos();
										ImDrawList* dl = ImGui::GetWindowDrawList();
										float w = ImGui::GetContentRegionAvail().x;
										float h = 28.f;
										dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), IM_COL32(255, 0, 51, 30));
										dl->AddRectFilled(p, ImVec2(p.x + 3, p.y + h), IM_COL32(255, 0, 51, 180));
										ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.f);
									}
									ImGui::Indent(12.f);
									ImGui::PushFont(FrameWork::Assets::InterBold);
									ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "%s", vname);
									ImGui::PopFont();
									ImGui::Unindent(12.f);
									ImGui::TextDisabled("Vehicle selected");

									ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
									if (FilterPass("Teleport to vehicle")) { if (ImGui::Button(_T("Teleport to vehicle"), ImVec2(btnW, 24)))
										Cheat::Exploits::TeleportLocalToCoords(v->GetCoordinate()); }
									ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
									if (FilterPass("Pull Vehicle")) { if (ImGui::Button(_T("Pull Vehicle"), ImVec2(btnW, 30)))
									{
										auto* localPed = Cheat::g_Fivem.GetLocalPlayerInfo().Ped;
										if (localPed && v)
										{
											Cheat::g_Fivem.network_request_control_of_entity((uint64_t)v, (uint64_t)localPed);
											v->SetLockState(CARLOCK_UNLOCKED);
											Vector3D playerPos = localPed->GetCoordinate();
											float h = Cheat::GetPlayerHeading();
											const float pullDist = 3.5f;
											Vector3D spawnPos;
											spawnPos.x = playerPos.x - std::sin(h) * pullDist;
											spawnPos.y = playerPos.y + std::cos(h) * pullDist;
											spawnPos.z = playerPos.z;
											uint64_t veh_nav = v->GetNavigation();
											uint64_t veh_model = v->GetModelInfo();
											if (veh_nav && veh_model)
												Cheat::g_Fivem.TeleportToObject((uintptr_t)v, veh_nav, veh_model, spawnPos, spawnPos, true);
											v->FixVehicle();

										}
									} }
									ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
									if (FilterPass("Unlock vehicle")) { if (ImGui::Button(_T("Unlock vehicle"), ImVec2(btnW, 24)))
										Cheat::Exploits::WorldUnlockVehicle(v); }
								}
							}
						}
						ImGui::EndCustomChild();
					}

					ImGui::Columns(1);
				}
				else if (CurrentTab == 4) // Settings
				{
					ImGui::Columns(2, NULL, false);
					ImGui::SetColumnWidth(0, kMainChildW * 0.5f);
					ImGui::SetColumnWidth(1, kMainChildW * 0.5f);

					if (SettingsSub == 0) // General
					{
						ImGui::CustomChild(XorStr("SettingsContent"), ImVec2(ImGui::GetColumnWidth() - 15, ImGui::GetContentRegionAvail().y));
						{
							if (g_Options.General.Particles)
							{
								static float setAnim = 0.f;
								setAnim += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(setAnim + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(setAnim + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							{
								ImGui::TextDisabled(_T("Account Information"));
								ImGui::Text("User: %s", s_DiscordName.c_str());
								ImGui::Text("Key: %s", ::Security::CurrentLicense.key.c_str());
								if (g_Options.General.DaysLeft == -1)
									ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), _T("LIFETIME"));
								else if (g_Options.General.DaysLeft > 0)
									ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), _T("Days Left: %d"), g_Options.General.DaysLeft);
								else if (g_Options.General.DaysLeft == 0)
									ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), _T("EXPIRED"));
								ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
								ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.05f, 0.05f, 1.0f));
								if (ImGui::Button(_T("Logout")))
								{
									DeleteLicenseKey();
									g_Options.General.IsLoggedIn = false;
									g_Options.General.UserName.clear();
									g_Options.General.DaysLeft = 0;
									g_AutoAuthAttempted = false;
								}
								ImGui::PopStyleColor(2);
							}

							ImGui::SeparatorText(_T("Configs"));
							{
								static char newName[64] = {};
								static int selectedConfig = -1;
								auto slots = ConfigManager.ListSlots();
								if (slots.empty())
									ImGui::TextDisabled(_T("No saved configs"));
								else
								{
									ImGui::TextDisabled("Saved configs:");
									ImGui::Indent(12.f);
									for (int i = 0; i < (int)slots.size(); i++)
									{
										ImGui::PushID(95000 + i);
										bool isSel = (selectedConfig == i);
										if (ImGui::Selectable(slots[i].c_str(), isSel))
										{
											selectedConfig = i;
											strcpy_s(newName, slots[i].c_str());
										}
										if (isSel)
										{
											ImVec2 rMin = ImGui::GetItemRectMin();
											ImVec2 rMax = ImGui::GetItemRectMax();
											ImDrawList* dl = ImGui::GetWindowDrawList();
											float w = ImGui::GetContentRegionAvail().x + 4.f;
											float lo = 10.f;
											dl->AddRectFilled(ImVec2(rMin.x - lo, rMin.y), ImVec2(rMin.x + w, rMax.y), IM_COL32(255, 0, 51, 40));
											dl->AddRectFilled(ImVec2(rMin.x - lo, rMin.y), ImVec2(rMin.x - lo + 3, rMax.y), IM_COL32(255, 0, 51, 200));
										}
										ImGui::PopID();
									}
									ImGui::Unindent(12.f);
								}
								ImGui::InputText("##SlotName", newName, 64);
								if (ImGui::Button("Save Config"))
								{
									std::string t = newName;
									while (!t.empty() && t.back() == ' ') t.pop_back();
									if (!t.empty())
									{
										ConfigManager.SaveSlot(t);
										memset(newName, 0, sizeof(newName));
									}
								}
								if (ImGui::Button("Load Config"))
								{
									std::string t = newName;
									while (!t.empty() && t.back() == ' ') t.pop_back();
									if (!t.empty())
										ConfigManager.LoadSlot(t);
								}
								if (ImGui::Button("Delete Config"))
								{
									std::string t = newName;
									while (!t.empty() && t.back() == ' ') t.pop_back();
									if (!t.empty())
									{
										ConfigManager.DeleteSlot(t);
										memset(newName, 0, sizeof(newName));
									}
								}
							}
							ImGui::SeparatorText(_T("Clipboard"));
							if (FilterPass("Import Config")) { if (ImGui::Button(_T("Import Config"))) ConfigManager.ImportFromClipboard(); ImGui::SetItemTooltip("Importa uma configuração da área de transferência"); }
							if (FilterPass("Export Config")) { if (ImGui::Button(_T("Export Config"))) ConfigManager.ExportToClipboard(); ImGui::SetItemTooltip("Copia toda a configuração para a área de transferência"); }
						}
						ImGui::EndCustomChild();

						ImGui::NextColumn();

						ImGui::CustomChild(XorStr("SettingsExtra"), ImVec2(ImGui::GetColumnWidth(), ImGui::GetContentRegionAvail().y));
						{
							if (g_Options.General.Particles)
							{
								static float setAnim2 = 0.f;
								setAnim2 += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(setAnim2 + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(setAnim2 + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (FilterPass("Menu Key")) { ImGui::KeyBind(_T("Menu Key"), &g_Options.General.MenuKey, &g_Options.General.KeyBindState); }
							if (FilterPass("Processor Delay")) { ImGui::SliderInt(_T("Processor Delay"), &g_Options.General.ThreadDelay, 1, 15, XorStr("%dms")); }
							if (FilterPass("Stream Mode")) { ImGui::Checkbox(_T("Stream Mode"), &g_Options.General.CaptureBypass); }
							if (FilterPass("Web Remote")) { ImGui::Checkbox(_T("Web Remote"), &g_Options.General.WebRemoteEnabled); ImGui::SetItemTooltip("Ativa o controle remoto via navegador"); }

							if (g_Options.General.WebRemoteEnabled)
							{
								std::string url = Cheat::WebRemote::GetURL();
								if (!url.empty())
								{
									ImGui::TextColored(ImVec4(0.5f, 0.7f, 1.0f, 1.0f), _T("URL: %s"), url.c_str());
									if (FilterPass("Copy URL")) { if (ImGui::Button(_T("Copy URL")))
										ImGui::SetClipboardText(url.c_str()); ImGui::SetItemTooltip("Copia a URL do Web Remote para a área de transferência"); }
								}
								else
									ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), _T("Server starting..."));
							}

							if (FilterPass("Active Features")) { ImGui::Checkbox(_T("Active Features"), &g_Options.Misc.ShowActiveFeaturesOverlay); ImGui::SetItemTooltip("Mostra um overlay com as funcionalidades ativas na tela"); }
							if (FilterPass("Safe Mode")) { ImGui::Checkbox(_T("Safe Mode"), &g_Options.General.SafeMode); }
							if (FilterPass("Particles")) { ImGui::Checkbox(_T("Particles"), &g_Options.General.Particles); }
							if (g_Options.General.Particles)
								if (FilterPass("Particle Opacity")) { ImGui::SliderInt(_T("Particle Opacity"), &g_Options.General.ParticleOpacity, 0, 100, XorStr("%d%%")); }
							if (FilterPass("Plexus")) { ImGui::Checkbox(_T("Plexus"), &g_Options.General.Plexus); }
							if (g_Options.General.Plexus)
								if (FilterPass("Plexus Opacity")) { ImGui::SliderInt(_T("Plexus Opacity"), &g_Options.General.PlexusOpacity, 0, 100, XorStr("%d%%")); }
							if (FilterPass("Second Monitor")) { ImGui::Checkbox(_T("Second Monitor"), &g_Options.General.EspOnSecondaryMonitor); ImGui::SetItemTooltip("Move o overlay ESP para o segundo monitor"); }
							if (FilterPass("Watermark")) { ImGui::Checkbox(_T("Watermark"), &g_Options.General.WaterMark); }
							if (g_Options.General.WaterMark)
							{
								if (FilterPass("Watermark Color")) { ImGui::Checkbox(_T("Watermark Color"), &g_Options.General.WaterMarkCol); }
								if (g_Options.General.WaterMarkCol)
									if (FilterPass("Color")) { ImGui::ColorEdit4(_T("Color"), g_Options.General.WaterMarkColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar); }
							}
							{
								const char* langs[] = { "English", "Portugues" };
								if (FilterPass("Language")) { ImGui::Combo(_T("Language"), &g_Options.General.Language, langs, 2); }
								Language::IsPortuguese = (g_Options.General.Language == 1);
							}
							ImGui::Spacing();

							float btnPad = 12.0f;
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
							if (FilterPass("Unload")) { if (ImGui::Button(_T("Unload"), ImVec2(ImGui::GetContentRegionAvail().x - btnPad, 25))) ExitProcess(0); }
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
							if (FilterPass("Discord")) { if (ImGui::Button(_T("Discord"), ImVec2(ImGui::GetContentRegionAvail().x - btnPad, 25))) ShellExecuteA(NULL, "open", "https://discord.gg/8KvHY9Rhgz", NULL, NULL, SW_SHOWNORMAL); }
						}
						ImGui::EndCustomChild();
					}
					else if (SettingsSub == 1) // Trolls
					{
						ImGui::CustomChild(XorStr("TrollsContent"), ImVec2(ImGui::GetColumnWidth() - 15, ImGui::GetContentRegionAvail().y));
						{
							if (g_Options.General.Particles)
							{
								static float trollAnim = 0.f;
								trollAnim += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(trollAnim + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(trollAnim + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							if (FilterPass("Vehicle Grab")) { ImGui::Checkbox(_T("Vehicle Grab"), &g_Options.Misc.Trolls.VehicleGrabEnabled); }
							if (g_Options.Misc.Trolls.VehicleGrabEnabled)
							{
								if (FilterPass("Grab Key")) { ImGui::KeyBind(_T("Grab Key"), &g_Options.Misc.Trolls.GrabKey, &g_Options.Misc.Trolls.GrabKeyState); }
								ImGui::Separator();
								ImGui::TextDisabled("Hold Settings");
								if (FilterPass("Hold Distance")) { ImGui::SliderFloat(_T("Hold Distance"), &g_Options.Misc.Trolls.HoldDistance, 1.0f, 15.0f, "%.1f m"); }
								if (FilterPass("Hold Height")) { ImGui::SliderFloat(_T("Hold Height"), &g_Options.Misc.Trolls.HoldHeight, -5.0f, 10.0f, "%.1f m"); }
								ImGui::Separator();
								ImGui::TextDisabled("Throw Settings");
								if (FilterPass("Throw Force")) { ImGui::SliderFloat(_T("Throw Force"), &g_Options.Misc.Trolls.ThrowForce, 1.0f, 200.0f, "%.0f%%"); }
								ImGui::Separator();
								if (Cheat::Trolls::IsHoldingVehicle())
								{
									ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Holding vehicle: YES");
									float btnPad = 12.0f;
									ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
									if (FilterPass("Release Vehicle")) { if (ImGui::Button(_T("Release Vehicle"), ImVec2(ImGui::GetContentRegionAvail().x - btnPad, 30)))
										Cheat::Trolls::ReleaseVehicle(false); }
									ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPad);
									if (FilterPass("Throw Vehicle")) { if (ImGui::Button(_T("Throw Vehicle"), ImVec2(ImGui::GetContentRegionAvail().x - btnPad, 30)))
										Cheat::Trolls::ReleaseVehicle(true); }
								}
								else
									ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Holding vehicle: NO");
							}

							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();


						}
						ImGui::EndCustomChild();

						ImGui::NextColumn();

						ImGui::CustomChild(XorStr("TrollsExtra"), ImVec2(ImGui::GetColumnWidth(), ImGui::GetContentRegionAvail().y));
						{
							if (g_Options.General.Particles)
							{
								static float trollAnim2 = 0.f;
								trollAnim2 += ImGui::GetIO().DeltaTime * 0.5f;
								ImVec2 pPos = ImGui::GetWindowPos();
								ImVec2 pSize = ImGui::GetWindowSize();
								for (int i = 0; i < 40; i++) {
									float x = (sin(trollAnim2 + i * 0.7f) * 0.5f + 0.5f) * pSize.x;
									float y = (cos(trollAnim2 + i * 0.4f) * 0.5f + 0.5f) * pSize.y;
									ImGui::GetWindowDrawList()->AddCircleFilled(pPos + ImVec2(x, y), 1.2f, ImColor(255, 0, 51, 180 * g_Options.General.ParticleOpacity / 100));
								}
							}

							ImGui::TextDisabled("Instructions");
							ImGui::BulletText("Enable Vehicle Grab");
							ImGui::BulletText("Set a keybind");
							ImGui::BulletText("Aim at a vehicle and press the key");
							ImGui::BulletText("The vehicle will follow your camera");
							ImGui::BulletText("Press the key again to throw it!");
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();
							if (g_Options.Misc.Trolls.GrabKey != 0)
							{
								std::string mode = g_Options.Misc.Trolls.GrabKeyState == 0 ? "Toggle" : "Hold";
								ImGui::Text("Mode: %s", mode.c_str());
							}
						}
						ImGui::EndCustomChild();
					}

					ImGui::Columns(1);
				}
			}
			ImGui::EndChild();
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();

			DrawList->AddRect(Pos, Pos + Size, ImGui::GetColorU32(ImGuiCol_Border), ImGui::GetStyle().WindowRounding);
		}
		ImGui::End();

		// Admin list code removed to RenderAdminListOverlay()
	}

	void Interface::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_SIZE:
			if (wParam != SIZE_MINIMIZED)
			{
				ResizeWidht = (UINT)LOWORD(lParam);
				ResizeHeight = (UINT)HIWORD(lParam);
			}
			break;
		}

		if (bIsMenuOpen)
		{
			ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		}
	}

	void Interface::HandleMenuKey()
	{
		static bool MenuKeyDown = false;
		static bool cursorHidden = false;

		if (GetAsyncKeyState(g_Options.General.MenuKey) & 0x8000)
		{
			if (!MenuKeyDown)
			{
				MenuKeyDown = true;

				HWND foreground = GetForegroundWindow();

				if (foreground == hTargetWindow || foreground == hWindow)
				{
					bIsMenuOpen = !bIsMenuOpen;

					if (bIsMenuOpen)
					{
						ApplyOverlayWindowStyle(hWindow, WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
						SetForegroundWindow(hWindow);
						if (g_Options.General.CaptureBypass && !cursorHidden)
						{
							while (ShowCursor(FALSE) >= 0);
							cursorHidden = true;
						}
					}
					else
					{
						ApplyOverlayWindowStyle(hWindow, WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED);
						SetForegroundWindow(hTargetWindow);
						if (cursorHidden)
						{
							while (ShowCursor(TRUE) < 0);
							cursorHidden = false;
						}
					}
				}
			}
		}
		else
		{
			MenuKeyDown = false;
		}
	}

	void Interface::ShutDown()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}