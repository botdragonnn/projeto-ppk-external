	#pragma once
	#pragma warning(disable: 4244 4267 4242 4312 4311)

	// ImGui
	#include <FrameWork/Dependencies/ImGui/imgui.h>
	#include <FrameWork/Dependencies/ImGui/imgui_impl_dx11.h>
	#include <FrameWork/Dependencies/ImGui/imgui_impl_win32.h>
	#include <FrameWork/Dependencies/ImGui/imgui_internal.h>

	// FivemSDK
	#include "FivemSDK/Fivem.hpp"

	// Features
	#include "Features/Visuals/PlayerESP.hpp"
	#include "Features/Visuals/VehicleESP.hpp"
	#include "Features/Combat/AimBot.hpp"
	#include "Features/Combat/TriggerBot.hpp"
	#include "Features/Combat/MagicBullet.hpp"
	#include "Features/Combat/SilentAim.hpp"
	#include "Features/Exploits/Player/Exploits.hpp"
	#include "Features/Exploits/Vehicle/Exploits.hpp"
	#include "Features/Exploits/Weapon/Exploits.hpp"
	#include "Features/Settings/Trolls.hpp"
	#include "Features/Settings/AntiCrack.hpp"
	#include "Features/World/UpdateNames.hpp"
	#include "Features/Exploits/Weapon/GiveWeapon.hpp"
	#include "Features/Settings/ResourceList.hpp"

	#include "Options.hpp"

	namespace Cheat
	{
		 std::vector<std::string> GetMonitorNames();
		void UpdateWindowPos();
		void Initialize();
		void ShutDown();
	}