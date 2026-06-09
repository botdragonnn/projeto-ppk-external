#include "ConfigSystem.hpp"
#include <FrameWork/Utilities/Base64.hpp>
#include "Options.hpp"
#include <fstream>

namespace Cheat
{

	void ConfigManager::AddItem(void* Pointer, const char* Name, const std::string& Type)
	{
		Items.push_back(new CConfigItem(std::string(Name), Pointer, Type));
	}

	void ConfigManager::SetupItem(int* Pointer, float Value, const std::string& Name)
	{
		AddItem(Pointer, Name.c_str(), XorStr("int"));
		*Pointer = Value;
	}

	void ConfigManager::SetupItem(float* Pointer, float Value, const std::string& Name)
	{
		AddItem(Pointer, Name.c_str(), XorStr("float"));
		*Pointer = Value;
	}

	void ConfigManager::SetupItem(bool* Pointer, float Value, const std::string& Name)
	{
		AddItem(Pointer, Name.c_str(), XorStr("bool"));
		*Pointer = Value;
	}

	void ConfigManager::SetupItem(float(*Pointer)[4], float v0, float v1, float v2, float v3, const std::string& Name)
	{
		AddItem(Pointer, Name.c_str(), XorStr("float4"));
		(*Pointer)[0] = v0;
		(*Pointer)[1] = v1;
		(*Pointer)[2] = v2;
		(*Pointer)[3] = v3;
	}

	void ConfigManager::SetupItem(float(*Pointer)[2], float v0, float v1, const std::string& Name)
	{
		AddItem(Pointer, Name.c_str(), XorStr("float2"));
		(*Pointer)[0] = v0;
		(*Pointer)[1] = v1;
	}

	void ConfigManager::SetupItem(char(*Pointer)[64], const char* Value, const std::string& Name)
	{
		AddItem(Pointer, Name.c_str(), XorStr("char64"));
		strcpy_s(*Pointer, 64, Value);
	}

	void ConfigManager::SetupItem(uint64_t* Pointer, uint64_t Value, const std::string& Name)
	{
		AddItem(Pointer, Name.c_str(), XorStr("uint64"));
		*Pointer = Value;
	}

	void ConfigManager::SetupItem(std::string* Pointer, const std::string& Value, const std::string& Name)
	{
		AddItem(Pointer, Name.c_str(), XorStr("string"));
		*Pointer = Value;
	}

    void ConfigManager::Setup()
    {
        // LegitBot - AimBot
        SetupItem(&g_Options.LegitBot.AimBot.Enabled, false, "LegitBot.AimBot.Enabled");
        SetupItem(&g_Options.LegitBot.AimBot.LegitMode, false, "LegitBot.AimBot.LegitMode");
        SetupItem(&g_Options.LegitBot.AimBot.ClosestFov, false, "LegitBot.AimBot.ClosestFov");
        SetupItem(&g_Options.LegitBot.AimBot.KeyBind, 0, "LegitBot.AimBot.KeyBind");
        SetupItem(&g_Options.LegitBot.AimBot.KeyBindState, 0, "LegitBot.AimBot.KeyBindState");
        SetupItem(&g_Options.LegitBot.AimBot.TargetNPC, false, "LegitBot.AimBot.TargetNPC");
        SetupItem(&g_Options.LegitBot.AimBot.VisibleCheck, false, "LegitBot.AimBot.VisibleCheck");
        SetupItem(&g_Options.LegitBot.AimBot.HitBox, 0, "LegitBot.AimBot.HitBox");
        SetupItem(&g_Options.LegitBot.AimBot.MaxDistance, 250, "LegitBot.AimBot.MaxDistance");
        SetupItem(&g_Options.LegitBot.AimBot.FOV, 10, "LegitBot.AimBot.FOV");
        SetupItem(&g_Options.LegitBot.AimBot.SmoothHorizontal, 2, "LegitBot.AimBot.SmoothHorizontal");
        SetupItem(&g_Options.LegitBot.AimBot.SmoothVertical, 2, "LegitBot.AimBot.SmoothVertical");
        SetupItem(&g_Options.LegitBot.AimBot.Prediction, false, "LegitBot.AimBot.Prediction");
        SetupItem(&g_Options.LegitBot.TargetDead, false, "LegitBot.TargetDead");

        // LegitBot - MagicBullet
        SetupItem(&g_Options.LegitBot.MagicBullet.Enabled, false, "LegitBot.MagicBullet.Enabled");
        SetupItem(&g_Options.LegitBot.MagicBullet.KeyBind, 0, "LegitBot.MagicBullet.KeyBind");
        SetupItem(&g_Options.LegitBot.MagicBullet.KeyBindState, 0, "LegitBot.MagicBullet.KeyBindState");

        // LegitBot - TriggerBot
        SetupItem(&g_Options.LegitBot.Trigger.Enabled, false, "LegitBot.Trigger.Enabled");
        SetupItem(&g_Options.LegitBot.Trigger.KeyBind, 0, "LegitBot.Trigger.KeyBind");
        SetupItem(&g_Options.LegitBot.Trigger.KeyBindState, 0, "LegitBot.Trigger.KeyBindState");
        SetupItem(&g_Options.LegitBot.Trigger.ShotNPC, false, "LegitBot.Trigger.ShotNPC");
        SetupItem(&g_Options.LegitBot.Trigger.VisibleCheck, false, "LegitBot.Trigger.VisibleCheck");
        SetupItem(&g_Options.LegitBot.Trigger.MaxDistance, 250, "LegitBot.Trigger.MaxDistance");
        SetupItem(&g_Options.LegitBot.Trigger.ReactionTime, 0, "LegitBot.Trigger.ReactionTime");
        SetupItem(&g_Options.LegitBot.Trigger.SmartTrigger, false, "LegitBot.Trigger.SmartTrigger");
        SetupItem(&g_Options.LegitBot.Trigger.Fov, 10, "LegitBot.Trigger.Fov");
        SetupItem(&g_Options.LegitBot.Trigger.ShowFov, false, "LegitBot.Trigger.ShowFov");
        SetupItem(&g_Options.LegitBot.Trigger.FovColor, 1.f, 1.f, 1.f, 1.f, "LegitBot.Trigger.FovColor");

        // LegitBot - SilentAim
        SetupItem(&g_Options.LegitBot.SilentAim.LegitMode, false, "LegitBot.SilentAim.LegitMode");
        SetupItem(&g_Options.LegitBot.SilentAim.Enabled, false, "LegitBot.SilentAim.Enabled");
        SetupItem(&g_Options.LegitBot.SilentAim.ClosestFov, false, "LegitBot.SilentAim.ClosestFov");
        SetupItem(&g_Options.LegitBot.SilentAim.Fov, 10, "LegitBot.SilentAim.Fov");
        SetupItem(&g_Options.LegitBot.SilentAim.KeyBind, 0, "LegitBot.SilentAim.KeyBind");
        SetupItem(&g_Options.LegitBot.SilentAim.KeyBindState, 0, "LegitBot.SilentAim.KeyBindState");
        SetupItem(&g_Options.LegitBot.SilentAim.MissChance, 1, "LegitBot.SilentAim.MissChance");
        SetupItem(&g_Options.LegitBot.SilentAim.ShotNPC, false, "LegitBot.SilentAim.ShotNPC");
        SetupItem(&g_Options.LegitBot.SilentAim.VisibleCheck, false, "LegitBot.SilentAim.VisibleCheck");
        SetupItem(&g_Options.LegitBot.SilentAim.MaxDistance, 250, "LegitBot.SilentAim.MaxDistance");
        SetupItem(&g_Options.LegitBot.SilentAim.HitBox, 0, "LegitBot.SilentAim.HitBox");
        SetupItem(&g_Options.LegitBot.SilentAim.ShowFov, false, "LegitBot.SilentAim.ShowFov");
        SetupItem(&g_Options.LegitBot.SilentAim.MagicBullets, false, "LegitBot.SilentAim.MagicBullets");
        SetupItem(&g_Options.LegitBot.SilentAim.FovColor, 1.f, 1.f, 0.f, 1.f, "LegitBot.SilentAim.FovColor");

        // Visuals - ESP - Players
        SetupItem(&g_Options.Visuals.ESP.Players.Enabled, true, "Visuals.ESP.Players.Enabled");
        SetupItem(&g_Options.Visuals.ESP.Players.ShowLocalPlayer, false, "Visuals.ESP.Players.ShowLocalPlayer");
        SetupItem(&g_Options.Visuals.ESP.Players.ShowNPCs, false, "Visuals.ESP.Players.ShowNPCs");
        SetupItem(&g_Options.Visuals.ESP.Players.UpdateESP, false, "Visuals.ESP.Players.UpdateESP");
        SetupItem(&g_Options.Visuals.ESP.Players.VisibleOnly, false, "Visuals.ESP.Players.VisibleOnly");
        SetupItem(&g_Options.Visuals.ESP.Players.ExcludeDeads, true, "Visuals.ESP.Players.ExcludeDeads");
        SetupItem(&g_Options.Visuals.ESP.Players.RenderDistance, 200, "Visuals.ESP.Players.RenderDistance");
        SetupItem(&g_Options.Visuals.ESP.Players.Box, false, "Visuals.ESP.Players.Box");
        SetupItem(&g_Options.Visuals.ESP.Players.Minimap, false, "Visuals.ESP.Players.Minimap");
        SetupItem(&g_Options.Visuals.ESP.Players.Skeleton, true, "Visuals.ESP.Players.Skeleton");
        SetupItem(&g_Options.Visuals.ESP.Players.Name, true, "Visuals.ESP.Players.Name");
        SetupItem(&g_Options.Visuals.ESP.Players.HealthBar, false, "Visuals.ESP.Players.HealthBar");
        SetupItem(&g_Options.Visuals.ESP.Players.ArmorBar, false, "Visuals.ESP.Players.ArmorBar");
        SetupItem(&g_Options.Visuals.ESP.Players.WeaponName, false, "Visuals.ESP.Players.WeaponName");
        SetupItem(&g_Options.Visuals.ESP.Players.Distance, false, "Visuals.ESP.Players.Distance");
        SetupItem(&g_Options.Visuals.ESP.Players.SnapLines, false, "Visuals.ESP.Players.SnapLines");
        SetupItem(&g_Options.Visuals.ESP.Players.HighlightVisible, false, "Visuals.ESP.Players.HighlightVisible");
        SetupItem(&g_Options.Visuals.ESP.Players.NamePosNum, 2, "Visuals.ESP.Players.NamePosNum");
        SetupItem(&g_Options.Visuals.ESP.Players.NameState, 2, "Visuals.ESP.Players.NameState");
        SetupItem(&g_Options.Visuals.ESP.Players.HealthBarPosNum, 3, "Visuals.ESP.Players.HealthBarPosNum");
        SetupItem(&g_Options.Visuals.ESP.Players.HealthBarState, 0, "Visuals.ESP.Players.HealthBarState");
        SetupItem(&g_Options.Visuals.ESP.Players.ArmorBarPosNum, 3, "Visuals.ESP.Players.ArmorBarPosNum");
        SetupItem(&g_Options.Visuals.ESP.Players.ArmorBarState, 0, "Visuals.ESP.Players.ArmorBarState");
        SetupItem(&g_Options.Visuals.ESP.Players.WeaponNamePosNum, 0, "Visuals.ESP.Players.WeaponNamePosNum");
        SetupItem(&g_Options.Visuals.ESP.Players.WeaponNameState, 0, "Visuals.ESP.Players.WeaponNameState");
        SetupItem(&g_Options.Visuals.ESP.Players.DistancePosNum, 0, "Visuals.ESP.Players.DistancePosNum");
        SetupItem(&g_Options.Visuals.ESP.Players.DistanceState, 0, "Visuals.ESP.Players.DistanceState");
        SetupItem(&g_Options.Visuals.ESP.Players.HeadCircle, false, "Visuals.ESP.Players.HeadCircle");
        SetupItem(&g_Options.Visuals.ESP.Players.RGB, false, "Visuals.ESP.Players.RGB");
        SetupItem(&g_Options.Visuals.ESP.Players.RGBSpeed, 1.0f, "Visuals.ESP.Players.RGBSpeed");
        SetupItem(&g_Options.Visuals.ESP.Players.TextColor, 1.f, 1.f, 1.f, 1.f, "Visuals.ESP.Players.TextColor");
        SetupItem(&g_Options.Visuals.ESP.Players.BoxColor, 1.f, 1.f, 1.f, 1.f, "Visuals.ESP.Players.BoxColor");
        SetupItem(&g_Options.Visuals.ESP.Players.SkeletonColor, 1.f, 1.f, 1.f, 1.f, "Visuals.ESP.Players.SkeletonColor");
        SetupItem(&g_Options.Visuals.ESP.Players.HealthBarColor, 0.f, 1.f, 0.f, 1.f, "Visuals.ESP.Players.HealthBarColor");
        SetupItem(&g_Options.Visuals.ESP.Players.ArmorColor, 0.1f, 0.1f, 1.f, 1.f, "Visuals.ESP.Players.ArmorColor");
        SetupItem(&g_Options.Visuals.ESP.Players.SnapLinesColor, 1.f, 1.f, 1.f, 1.f, "Visuals.ESP.Players.SnapLinesColor");
        SetupItem(&g_Options.Visuals.ESP.Players.HeadCircleColor, 1.f, 1.f, 1.f, 1.f, "Visuals.ESP.Players.HeadCircleColor");
        SetupItem(&g_Options.Visuals.ESP.Players.FriendSkeletonColor, 0.f, 1.f, 0.f, 1.f, "Visuals.ESP.Players.FriendSkeletonColor");
        SetupItem(&g_Options.Visuals.ESP.Players.NameRawPos, 0.f, 0.f, "Visuals.ESP.Players.NameRawPos");
        SetupItem(&g_Options.Visuals.ESP.Players.HealthBarRawPos, 0.f, 0.f, "Visuals.ESP.Players.HealthBarRawPos");
        SetupItem(&g_Options.Visuals.ESP.Players.ArmorBarRawPos, 0.f, 0.f, "Visuals.ESP.Players.ArmorBarRawPos");
        SetupItem(&g_Options.Visuals.ESP.Players.WeaponNameRawPos, 0.f, 0.f, "Visuals.ESP.Players.WeaponNameRawPos");
        SetupItem(&g_Options.Visuals.ESP.Players.DistanceRawPos, 0.f, 0.f, "Visuals.ESP.Players.DistanceRawPos");


        // Visuals - ESP - Vehicles
        SetupItem(&g_Options.Visuals.ESP.Vehicles.Enabled, false, "Visuals.ESP.Vehicles.Enabled");
        SetupItem(&g_Options.Visuals.ESP.Vehicles.Marker, false, "Visuals.ESP.Vehicles.Marker");
        SetupItem(&g_Options.Visuals.ESP.Vehicles.Door, false, "Visuals.ESP.Vehicles.Door");
        SetupItem(&g_Options.Visuals.ESP.Vehicles.Name, false, "Visuals.ESP.Vehicles.Name");
        SetupItem(&g_Options.Visuals.ESP.Vehicles.RenderDistance, 250, "Visuals.ESP.Vehicles.RenderDistance");
        SetupItem(&g_Options.Visuals.ESP.Vehicles.Distance, false, "Visuals.ESP.Vehicles.Distance");
        SetupItem(&g_Options.Visuals.ESP.Vehicles.Model, false, "Visuals.ESP.Vehicles.Model");
        SetupItem(&g_Options.Visuals.ESP.Vehicles.SnapLines, false, "Visuals.ESP.Vehicles.SnapLines");
        SetupItem(&g_Options.Visuals.ESP.Vehicles.LockStatus, false, "Visuals.ESP.Vehicles.LockStatus");
        SetupItem(&g_Options.Visuals.ESP.Vehicles.TextColor, 1.f, 1.f, 1.f, 1.f, "Visuals.ESP.Vehicles.TextColor");
        SetupItem(&g_Options.Visuals.ESP.Vehicles.MarkerColor, 1.f, 1.f, 1.f, 1.f, "Visuals.ESP.Vehicles.MarkerColor");
        SetupItem(&g_Options.Visuals.ESP.Vehicles.SnapLinesColor, 1.f, 1.f, 1.f, 1.f, "Visuals.ESP.Vehicles.SnapLinesColor");
        SetupItem(&g_Options.Visuals.ESP.Vehicles.VehicleNameColor, 1.f, 1.f, 1.f, 1.f, "Visuals.ESP.Vehicles.VehicleNameColor");
        SetupItem(&g_Options.Visuals.ESP.Vehicles.VehicleDistanceColor, 1.f, 1.f, 1.f, 1.f, "Visuals.ESP.Vehicles.VehicleDistanceColor");
        SetupItem(&g_Options.Visuals.ESP.Vehicles.LockedColor, 1.f, 0.f, 0.f, 1.f, "Visuals.ESP.Vehicles.LockedColor");
        SetupItem(&g_Options.Visuals.ESP.Vehicles.UnlockedColor, 0.f, 1.f, 0.f, 1.f, "Visuals.ESP.Vehicles.UnlockedColor");

        // Misc - Screen
        SetupItem(&g_Options.Misc.Screen.ShowAimbotFov, false, "Misc.Screen.ShowAimbotFov");

        SetupItem(&g_Options.Misc.Screen.AimbotFovColor, 1.f, 1.f, 1.f, 1.f, "Misc.Screen.AimbotFovColor");

        // Misc - Exploits - LocalPlayer
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.v_Boost, 1.2f, "Misc.Exploits.LocalPlayer.v_Boost");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.Shrink, false, "Misc.Exploits.LocalPlayer.Shrink");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.StealCarEnabled, false, "Misc.Exploits.LocalPlayer.StealCarEnabled");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.InfiniteCombatRoll, false, "Misc.Exploits.LocalPlayer.InfiniteCombatRoll");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.InfiniteStamina, false, "Misc.Exploits.LocalPlayer.InfiniteStamina");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.RemoveCollisions, false, "Misc.Exploits.LocalPlayer.RemoveCollisions");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.Invisible, false, "Misc.Exploits.LocalPlayer.Invisible");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.SeatBelt, false, "Misc.Exploits.LocalPlayer.SeatBelt");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.GodMode, false, "Misc.Exploits.LocalPlayer.GodMode");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.GodModeKey, 0, "Misc.Exploits.LocalPlayer.GodModeKey");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.GodModeKeyState, 0, "Misc.Exploits.LocalPlayer.GodModeKeyState");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.AntiAimEnabled, false, "Misc.Exploits.LocalPlayer.AntiAimEnabled");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.AntiAimKey, 0, "Misc.Exploits.LocalPlayer.AntiAimKey");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.AntiAimKeyState, 0, "Misc.Exploits.LocalPlayer.AntiAimKeyState");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.FastRun, false, "Misc.Exploits.LocalPlayer.FastRun");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.Noclip, false, "Misc.Exploits.LocalPlayer.Noclip");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.RunSpeed, 1.0f, "Misc.Exploits.LocalPlayer.RunSpeed");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.LockAllCars, false, "Misc.Exploits.LocalPlayer.LockAllCars");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.UnLockAllCars, false, "Misc.Exploits.LocalPlayer.UnLockAllCars");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.NoclipKey, 0, "Misc.Exploits.LocalPlayer.NoclipKey");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.NoclipKeyState, 0, "Misc.Exploits.LocalPlayer.NoclipKeyState");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.NoClipSpeed, 2.0f, "Misc.Exploits.LocalPlayer.NoClipSpeed");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.TpWayKey, 0, "Misc.Exploits.LocalPlayer.TpWayKey");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.TpWayKeyState, 0, "Misc.Exploits.LocalPlayer.TpWayKeyState");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.DamageMultiplier, 1.0f, "Misc.Exploits.LocalPlayer.DamageMultiplier");
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.Boost, 1.0f, "Misc.Exploits.LocalPlayer.Boost");

        // Misc - Exploits - Vehicle
        SetupItem(&g_Options.Misc.Exploits.Vehicle.GodMode, false, "Misc.Exploits.Vehicle.GodMode");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.JumpingCar, false, "Misc.Exploits.Vehicle.JumpingCar");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.RocketBoost, false, "Misc.Exploits.Vehicle.RocketBoost");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.RocketBoostJumping, false, "Misc.Exploits.Vehicle.RocketBoostJumping");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.RocketBoostKey, 0, "Misc.Exploits.Vehicle.RocketBoostKey");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.JumpingCarKey, 0, "Misc.Exploits.Vehicle.JumpingCarKey");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.UnlockNearbyKey, 0, "Misc.Exploits.Vehicle.UnlockNearbyKey");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.RocketBoostStrength, 5.0f, "Misc.Exploits.Vehicle.RocketBoostStrength");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.JumpGravity, 3.0f, "Misc.Exploits.Vehicle.JumpGravity");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.RepairKey, 0, "Misc.Exploits.Vehicle.RepairKey");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.RepairKeyState, 0, "Misc.Exploits.Vehicle.RepairKeyState");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.RocketBoostKeyState, 0, "Misc.Exploits.Vehicle.RocketBoostKeyState");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.UnlockNearbyKeyState, 0, "Misc.Exploits.Vehicle.UnlockNearbyKeyState");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.ModifyHandling, false, "Misc.Exploits.Vehicle.ModifyHandling");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.HandlingAccel, 0, "Misc.Exploits.Vehicle.HandlingAccel");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.HandlingBrake, 0, "Misc.Exploits.Vehicle.HandlingBrake");
        SetupItem(&g_Options.Misc.Exploits.Vehicle.HandlingTraction, 0, "Misc.Exploits.Vehicle.HandlingTraction");

        // Misc - Exploits - Weapon
        SetupItem(&g_Options.Misc.Exploits.Weapon.InfiniteAmmoEnabled, false, "Misc.Exploits.Weapon.InfiniteAmmoEnabled");
        SetupItem(&g_Options.Misc.Exploits.Weapon.RemoveSpread, false, "Misc.Exploits.Weapon.RemoveSpread");
        SetupItem(&g_Options.Misc.Exploits.Weapon.RemoveRecoil, false, "Misc.Exploits.Weapon.RemoveRecoil");
        SetupItem(&g_Options.Misc.Exploits.Weapon.NoReload, false, "Misc.Exploits.Weapon.NoReload");
        SetupItem(&g_Options.Misc.Exploits.Weapon.WeaponScaleEnabled, false, "Misc.Exploits.Weapon.WeaponScaleEnabled");
        SetupItem(&g_Options.Misc.Exploits.Weapon.WeaponScale, 1.0f, "Misc.Exploits.Weapon.WeaponScale");
        SetupItem(&g_Options.Misc.Exploits.Weapon.Coronhada, false, "Misc.Exploits.Weapon.Coronhada");


        // Misc - Trolls
        SetupItem(&g_Options.Misc.Trolls.VehicleGrabEnabled, false, "Misc.Trolls.VehicleGrabEnabled");
        SetupItem(&g_Options.Misc.Trolls.GrabKey, 0, "Misc.Trolls.GrabKey");
        SetupItem(&g_Options.Misc.Trolls.GrabKeyState, 0, "Misc.Trolls.GrabKeyState");
        SetupItem(&g_Options.Misc.Trolls.ThrowForce, 50.0f, "Misc.Trolls.ThrowForce");
        SetupItem(&g_Options.Misc.Trolls.HoldDistance, 4.0f, "Misc.Trolls.HoldDistance");
        SetupItem(&g_Options.Misc.Trolls.HoldHeight, 2.5f, "Misc.Trolls.HoldHeight");

        // Misc
        SetupItem(&g_Options.Misc.ShowActiveFeaturesOverlay, false, "Misc.ShowActiveFeaturesOverlay");

        // Misc - Exploits - LocalPlayer (extras)
        SetupItem(&g_Options.Misc.Exploits.LocalPlayer.BoomFist, false, "Misc.Exploits.LocalPlayer.BoomFist");


        // Misc - GiveWeapon
        SetupItem(&g_Options.Misc.GiveWeapon.Enabled, false, "Misc.GiveWeapon.Enabled");
        SetupItem(&g_Options.Misc.GiveWeapon.KeyBind, 0, "Misc.GiveWeapon.KeyBind");
        SetupItem(&g_Options.Misc.GiveWeapon.KeyBindState, 0, "Misc.GiveWeapon.KeyBindState");
        SetupItem(&g_Options.Misc.GiveWeapon.WeaponHash, 0, "Misc.GiveWeapon.WeaponHash");
        SetupItem(&g_Options.Misc.GiveWeapon.WeaponName, "", "Misc.GiveWeapon.WeaponName");

        // General
        SetupItem(&g_Options.General.ShutDown, false, "General.ShutDown");
        SetupItem(&g_Options.General.SafeMode, false, "General.SafeMode");
        SetupItem(&g_Options.General.MenuKey, VK_INSERT, "General.MenuKey");
        SetupItem(&g_Options.General.KeyBind, 0, "General.KeyBind");
        SetupItem(&g_Options.General.KeyBindState, 0, "General.KeyBindState");
        SetupItem(&g_Options.General.CaptureBypass, true, "General.CaptureBypass");
        SetupItem(&g_Options.General.WaterMark, false, "General.WaterMark");
        SetupItem(&g_Options.General.Particles, true, "General.Particles");
        SetupItem(&g_Options.General.ParticleOpacity, 50, "General.ParticleOpacity");
        SetupItem(&g_Options.General.Plexus, true, "General.Plexus");
        SetupItem(&g_Options.General.PlexusOpacity, 50, "General.PlexusOpacity");
        SetupItem(&g_Options.General.ThreadDelay, 1, "General.ThreadDelay");
        SetupItem(&g_Options.General.EspOnSecondaryMonitor, false, "General.EspOnSecondaryMonitor");
        SetupItem(&g_Options.General.WaterMarkCol, false, "General.WaterMarkCol");
        SetupItem(&g_Options.General.Language, 0, "General.Language");
        SetupItem(&g_Options.General.WebRemoteEnabled, false, "General.WebRemoteEnabled");
        SetupItem(&g_Options.General.WebRemotePort, 8080, "General.WebRemotePort");
        SetupItem(&g_Options.General.PrimaryColor, 0.55f, 0.55f, 0.55f, 1.f, "General.PrimaryColor");
        SetupItem(&g_Options.General.TextColor, 1.f, 1.f, 1.f, 1.f, "General.TextColor");
        SetupItem(&g_Options.General.WaterMarkColor, 1.f, 1.f, 1.f, 1.f, "General.WaterMarkColor");
        SetupItem(&g_Options.General.LicenseKey, "", "General.LicenseKey");
        SetupItem(&g_Options.General.UserName, "", "General.UserName");
        SetupItem(&g_Options.General.UserRole, "", "General.UserRole");
        SetupItem(&g_Options.General.AvatarURL, "", "General.AvatarURL");

        // CheatGlobalUsage
        SetupItem(&g_Options.CheatGlobalUsage.bIsSpectatingEntity, false, "CheatGlobalUsage.bIsSpectatingEntity");
        SetupItem(&g_Options.CheatGlobalUsage.bIsRemoteControllingVehicle, false, "CheatGlobalUsage.bIsRemoteControllingVehicle");
        SetupItem(&g_Options.CheatGlobalUsage.SpectatingEntity, (uint64_t)0, "CheatGlobalUsage.SpectatingEntity");
        SetupItem(&g_Options.CheatGlobalUsage.ControllingVehicle, (uint64_t)0, "CheatGlobalUsage.ControllingVehicle");

	}

	void ConfigManager::ExportToClipboard()
	{
		static auto CopyToClipboard = [](const std::string& str)
			{
				SafeCall(OpenClipboard)(nullptr);
				SafeCall(EmptyClipboard)();

				void* hg = SafeCall(GlobalAlloc)(GMEM_MOVEABLE, str.size() + 1);

				if (!hg) {
					SafeCall(CloseClipboard)();
					return;
				}

				memcpy(SafeCall(GlobalLock)(hg), str.c_str(), str.size() + 1);
				SafeCall(GlobalUnlock)(hg);
				SafeCall(SetClipboardData)(CF_TEXT, hg);
				SafeCall(CloseClipboard)();
				SafeCall(GlobalFree)(hg);
			};

		nlohmann::json allJson;
		std::set<std::string> seenItems;

		for (auto it : Items)
		{
			if (seenItems.count(it->Name) > 0) {
				continue;
			}

			nlohmann::json j;

			j[XorStr("name")] = it->Name;
			j[XorStr("type")] = it->Type;

			if (!it->Type.compare(XorStr("int")))
				j[XorStr("value")] = (int)*(int*)it->Pointer;
			else if (!it->Type.compare(XorStr("float")))
				j[XorStr("value")] = (float)*(float*)it->Pointer;
			else if (!it->Type.compare(XorStr("bool")))
				j[XorStr("value")] = (bool)*(bool*)it->Pointer;
			else if (!it->Type.compare(XorStr("float4")))
				j[XorStr("value")] = nlohmann::json::array({ ((float*)it->Pointer)[0], ((float*)it->Pointer)[1], ((float*)it->Pointer)[2], ((float*)it->Pointer)[3] });
			else if (!it->Type.compare(XorStr("float2")))
				j[XorStr("value")] = nlohmann::json::array({ ((float*)it->Pointer)[0], ((float*)it->Pointer)[1] });
			else if (!it->Type.compare(XorStr("char64")))
				j[XorStr("value")] = std::string((char*)it->Pointer);
			else if (!it->Type.compare(XorStr("uint64")))
				j[XorStr("value")] = *(uint64_t*)it->Pointer;
			else if (!it->Type.compare(XorStr("string")))
				j[XorStr("value")] = *(std::string*)it->Pointer;

			allJson.push_back(j);
			seenItems.insert(it->Name);
		}

		auto str = base64::Encode((std::string(XorStr("stps5m- ")).append(allJson.dump(-1, '~'/*, true*/))).c_str());
		CopyToClipboard(str);
	}

	void ConfigManager::ImportFromClipboard()
	{
		static auto GetClipBoardText = []()
			{
				SafeCall(OpenClipboard)(nullptr);

				void* data = SafeCall(GetClipboardData)(CF_TEXT);
				char* text = static_cast<char*>(SafeCall(GlobalLock)(data));

				std::string str_text(text);

				SafeCall(GlobalUnlock)(data);
				SafeCall(CloseClipboard)();

				return str_text;
			};

		static auto find_item = [](std::vector< CConfigItem* > items, std::string name) -> CConfigItem*
			{
				for (int i = 0; i < (int)items.size(); i++)
					if (!items[i]->Name.compare(name))
						return items[i];

				return nullptr;
			};

		if (GetClipBoardText().empty()) {
			return;
		}

		auto decoded_string = base64::Decode(GetClipBoardText());
		// this a cutiehook config?
		if (decoded_string[0] != 's' ||
			decoded_string[1] != 't' ||
			decoded_string[2] != 'p' ||
			decoded_string[3] != 's' ||
			decoded_string[4] != '5' ||
			decoded_string[5] != 'm' ||
			decoded_string[6] != '-' ||
			decoded_string[7] != ' ')
			return;

		auto parsed_config = nlohmann::json::parse(decoded_string.erase(0, 8));

		nlohmann::json allJson = parsed_config;

		for (auto it = allJson.begin(); it != allJson.end(); ++it)
		{
			nlohmann::json j = *it;

			std::string name = j[XorStr("name")];
			std::string type = j[XorStr("type")];

			auto item = find_item(Items, name);

			if (item)
			{
				if (!type.compare(XorStr("int")))
					*(int*)item->Pointer = j[XorStr("value")].get<int>();
				else if (!type.compare(XorStr("float")))
					*(float*)item->Pointer = j[XorStr("value")].get<float>();
				else if (!type.compare(XorStr("bool")))
					*(bool*)item->Pointer = j[XorStr("value")].get<bool>();
				else if (!type.compare(XorStr("float4")))
				{
					auto& arr = j[XorStr("value")];
					for (int i = 0; i < 4 && i < (int)arr.size(); i++)
						((float*)item->Pointer)[i] = arr[i].get<float>();
				}
				else if (!type.compare(XorStr("float2")))
				{
					auto& arr = j[XorStr("value")];
					for (int i = 0; i < 2 && i < (int)arr.size(); i++)
						((float*)item->Pointer)[i] = arr[i].get<float>();
				}
				else if (!type.compare(XorStr("char64")))
					strcpy_s((char*)item->Pointer, 64, j[XorStr("value")].get<std::string>().c_str());
				else if (!type.compare(XorStr("uint64")))
					*(uint64_t*)item->Pointer = j[XorStr("value")].get<uint64_t>();
				else if (!type.compare(XorStr("string")))
					*(std::string*)item->Pointer = j[XorStr("value")].get<std::string>();
			}
		}
	}

	void ConfigManager::SaveToFile()
	{
		nlohmann::json allJson;
		std::set<std::string> seenItems;

		for (auto it : Items)
		{
			if (seenItems.count(it->Name) > 0)
				continue;

			nlohmann::json j;
			j["name"] = it->Name;
			j["type"] = it->Type;

			if (!it->Type.compare("int"))
				j["value"] = *(int*)it->Pointer;
			else if (!it->Type.compare("float"))
				j["value"] = *(float*)it->Pointer;
			else if (!it->Type.compare("bool"))
				j["value"] = *(bool*)it->Pointer;
			else if (!it->Type.compare("float4"))
				j["value"] = nlohmann::json::array({ ((float*)it->Pointer)[0], ((float*)it->Pointer)[1], ((float*)it->Pointer)[2], ((float*)it->Pointer)[3] });
			else if (!it->Type.compare("float2"))
				j["value"] = nlohmann::json::array({ ((float*)it->Pointer)[0], ((float*)it->Pointer)[1] });
			else if (!it->Type.compare("char64"))
				j["value"] = std::string((char*)it->Pointer);
			else if (!it->Type.compare("uint64"))
				j["value"] = *(uint64_t*)it->Pointer;
			else if (!it->Type.compare("string"))
				j["value"] = *(std::string*)it->Pointer;

			allJson.push_back(j);
			seenItems.insert(it->Name);
		}

		std::ofstream file("config.json");
		if (file.is_open())
		{
			file << allJson.dump(4);
			file.close();
		}
	}

	void ConfigManager::LoadFromFile()
	{
		std::ifstream file("config.json");
		if (!file.is_open())
			return;

		nlohmann::json allJson;
		try
		{
			file >> allJson;
		}
		catch (...)
		{
			file.close();
			return;
		}
		file.close();

		static auto find_item = [](std::vector<CConfigItem*> items, std::string name) -> CConfigItem*
		{
			for (int i = 0; i < (int)items.size(); i++)
				if (!items[i]->Name.compare(name))
					return items[i];
			return nullptr;
		};

		for (auto& j : allJson)
		{
			std::string name = j["name"];
			std::string type = j["type"];

			auto item = find_item(Items, name);
			if (item)
			{
				if (!type.compare("int"))
					*(int*)item->Pointer = j["value"].get<int>();
				else if (!type.compare("float"))
					*(float*)item->Pointer = j["value"].get<float>();
				else if (!type.compare("bool"))
					*(bool*)item->Pointer = j["value"].get<bool>();
				else if (!type.compare("float4"))
				{
					auto& arr = j["value"];
					for (int i = 0; i < 4 && i < (int)arr.size(); i++)
						((float*)item->Pointer)[i] = arr[i].get<float>();
				}
				else if (!type.compare("float2"))
				{
					auto& arr = j["value"];
					for (int i = 0; i < 2 && i < (int)arr.size(); i++)
						((float*)item->Pointer)[i] = arr[i].get<float>();
				}
				else if (!type.compare("char64"))
					strcpy_s((char*)item->Pointer, 64, j["value"].get<std::string>().c_str());
				else if (!type.compare("uint64"))
					*(uint64_t*)item->Pointer = j["value"].get<uint64_t>();
				else if (!type.compare("string"))
					*(std::string*)item->Pointer = j["value"].get<std::string>();
			}
		}
	}

	std::vector<std::string> ConfigManager::ListSlots()
	{
		std::vector<std::string> slots;
		WIN32_FIND_DATAA ffd;
		HANDLE hFind = FindFirstFileA("config_*.json", &ffd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do {
				std::string fn = ffd.cFileName;
				if (fn.size() > 7 && fn.substr(0, 7) == "config_")
				{
					std::string name = fn.substr(7, fn.size() - 12);
					slots.push_back(name);
				}
			} while (FindNextFileA(hFind, &ffd) != 0);
			FindClose(hFind);
		}
		return slots;
	}

	void ConfigManager::SaveSlot(const std::string& name)
	{
		std::string path = "config_" + name + ".json";
		nlohmann::json allJson;
		std::set<std::string> seenItems;

		for (auto it : Items)
		{
			if (!it || seenItems.count(it->Name) > 0)
				continue;

			nlohmann::json j;
			j["name"] = it->Name;
			j["type"] = it->Type;

			try
			{
				if (!it->Type.compare("int"))
					j["value"] = *(int*)it->Pointer;
				else if (!it->Type.compare("float"))
					j["value"] = *(float*)it->Pointer;
				else if (!it->Type.compare("bool"))
					j["value"] = *(bool*)it->Pointer;
				else if (!it->Type.compare("float4"))
					j["value"] = nlohmann::json::array({ ((float*)it->Pointer)[0], ((float*)it->Pointer)[1], ((float*)it->Pointer)[2], ((float*)it->Pointer)[3] });
				else if (!it->Type.compare("float2"))
					j["value"] = nlohmann::json::array({ ((float*)it->Pointer)[0], ((float*)it->Pointer)[1] });
				else if (!it->Type.compare("char64"))
					j["value"] = std::string((char*)it->Pointer);
				else if (!it->Type.compare("uint64"))
					j["value"] = *(uint64_t*)it->Pointer;
				else if (!it->Type.compare("string"))
					j["value"] = *(std::string*)it->Pointer;
			}
			catch (...) { }

			allJson.push_back(j);
			seenItems.insert(it->Name);
		}

		std::ofstream file(path);
		if (file.is_open())
		{
			file << allJson.dump(4);
			file.close();
		}
	}

	void ConfigManager::LoadSlot(const std::string& name)
	{
		std::string path = "config_" + name + ".json";
		std::ifstream file(path);
		if (!file.is_open())
			return;

		nlohmann::json allJson;
		try { file >> allJson; }
		catch (...) { file.close(); return; }
		file.close();

		for (auto& j : allJson)
		{
			try
			{
				auto name = j["name"].get<std::string>();
				auto type = j["type"].get<std::string>();
				if (name.empty() || type.empty()) continue;

				CConfigItem* item = nullptr;
				for (auto& it : Items)
				{
					if (it && !it->Name.compare(name)) { item = it; break; }
				}
				if (!item) continue;

				if (!type.compare("int"))
					*(int*)item->Pointer = j["value"].get<int>();
				else if (!type.compare("float"))
					*(float*)item->Pointer = j["value"].get<float>();
				else if (!type.compare("bool"))
					*(bool*)item->Pointer = j["value"].get<bool>();
				else if (!type.compare("float4"))
				{
					auto& arr = j["value"];
					for (int i = 0; i < 4 && i < (int)arr.size(); i++)
						((float*)item->Pointer)[i] = arr[i].get<float>();
				}
				else if (!type.compare("float2"))
				{
					auto& arr = j["value"];
					for (int i = 0; i < 2 && i < (int)arr.size(); i++)
						((float*)item->Pointer)[i] = arr[i].get<float>();
				}
				else if (!type.compare("char64"))
					strcpy_s((char*)item->Pointer, 64, j["value"].get<std::string>().c_str());
				else if (!type.compare("uint64"))
					*(uint64_t*)item->Pointer = j["value"].get<uint64_t>();
				else if (!type.compare("string"))
					*(std::string*)item->Pointer = j["value"].get<std::string>();
			}
			catch (...) { }
		}
	}

	void ConfigManager::DeleteSlot(const std::string& name)
	{
		std::string path = "config_" + name + ".json";
		DeleteFileA(path.c_str());
	}
}