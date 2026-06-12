	#pragma once

	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif

	#include <Windows.h>
	#include <FrameWork/Dependencies/ImGui/imgui.h>
	#include <string>

	namespace Cheat
	{
		class Options
		{
		public:
			struct LegitBot
			{
				bool TargetDead = false;
				struct AimBot
				{
					bool Enabled = false;
					bool LegitMode = false;
					bool ClosestFov = false;
					int KeyBind = 0;
					int KeyBindState = 0;
					bool TargetNPC = false;
					bool VisibleCheck = false;
					int HitBox = 0;
					int MaxDistance = 250;
					int FOV = 10;
					int SmoothHorizontal = 2;
					int SmoothVertical = 2;
					bool Prediction = false;
				}AimBot;
				struct MagicBullet
				{
					bool Enabled = false;
					int KeyBind = 0;
					int KeyBindState = 0;
				}MagicBullet;
				struct TriggerBot
				{
					bool Enabled = false;
					int KeyBind = 0;
					int KeyBindState = 0;
					bool ShotNPC = false;
					bool VisibleCheck = false;
					int MaxDistance = 250;
					int ReactionTime = 0;
					int Fov = 10;
					bool SmartTrigger = false;
					bool ShowFov = false;
					float FovColor[4] = { 1.f, 1.f, 1.f, 1.f };
				}Trigger;
				struct SilentAim
				{
					bool LegitMode = false;
					bool Enabled = false;
					bool ClosestFov = false;
					int Fov = 10;
					int KeyBind = 0;
					int KeyBindState = 0;
					int MissChance = 1;
					bool ShotNPC = false;
					bool VisibleCheck = false;
					int MaxDistance = 250;
					int HitBox = 0;
					bool ShowFov = false;
					float FovColor[4] = { 1.f, 1.f, 0.f, 1.f };
					bool MagicBullets = false;
				}SilentAim;
			}LegitBot;
			struct Visuals
			{


				struct ESP
				{
					struct Players
					{
						bool Enabled = false;
						bool ShowLocalPlayer = false;
						bool ShowNPCs = false;
						bool UpdateESP = false;
						bool VisibleOnly = false;
				bool ExcludeDeads = false;
						int RenderDistance = 200;
						bool Box = false;
						bool Minimap = false;
						bool Skeleton = false;
						bool Name = false;          float NameRawPos[2] = {0,0}; int NamePosNum = 2; int NameState = 2;
						bool HealthBar = false;      float HealthBarRawPos[2] = {0,0}; int HealthBarPosNum = 3; int HealthBarState = 0;
						bool ArmorBar = false;              float ArmorBarRawPos[2] = {0,0}; int ArmorBarPosNum = 3; int ArmorBarState = 0;
						bool WeaponName = false;           float WeaponNameRawPos[2] = {0,0}; int WeaponNamePosNum = 0; int WeaponNameState = 0;
						bool Distance = false;             float DistanceRawPos[2] = {0,0}; int DistancePosNum = 0; int DistanceState = 0;
						bool SnapLines = false;
						float FriendSkeletonColor[4] = { 0.f, 1.f, 0.f, 1.f };
						float TextColor[4] = { 1.f, 1.f, 1.f, 1.f };
						float BoxColor[4] = { 1.f, 1.f, 1.f, 1.f };
						float SkeletonColor[4] = { 1.f, 1.f, 1.f, 1.f };
						float HealthBarColor[4] = { 0.f, 1.f, 0.f, 1.f };
						float ArmorColor[4] = { 0.1f, 0.1f, 1.f, 1.f };
						float SnapLinesColor[4] = { 1.f, 1.f, 1.f, 1.f };

						// Head Circle
						bool HeadCircle = false;
						float HeadCircleColor[4] = { 1.f, 1.f, 1.f, 1.f };

						// RGB Mode
						bool RGB = false;
						float RGBSpeed = 1.0f;

					// Highlight Visible
					bool HighlightVisible = false;


					}Players;

					struct Vehicles
					{
						bool Enabled = false;
						float TextColor[4] = { 1.f, 1.f, 1.f, 1.f };
						float MarkerColor[4] = { 1.f, 1.f, 1.f, 1.f };
						bool Marker = false;
						bool Door = false;
						bool Name = false;
						int RenderDistance = 250;
						bool Distance = false;
				bool Model = false;
					bool SnapLines = false;
					bool LockStatus = false;
					float SnapLinesColor[4] = { 1.f, 1.f, 1.f, 1.f };
					float VehicleNameColor[4] = { 1.f, 1.f, 1.f, 1.f };
					float VehicleDistanceColor[4] = { 1.f, 1.f, 1.f, 1.f };
					float LockedColor[4] = { 1.f, 0.f, 0.f, 1.f };
					float UnlockedColor[4] = { 0.f, 1.f, 0.f, 1.f };
					}Vehicles;
				}ESP;
			}Visuals;
			struct Misc
			{
				struct Screen
				{
					bool ShowAimbotFov = false;
	
					float AimbotFovColor[4] = { 1.f, 1.f, 1.f, 1.f };
				}Screen;
				struct Exploits
				{
					struct LocalPlayer
					{
						float v_Boost = 1.2f;
						bool Shrink = false;
						bool StealCarEnabled = false;
						bool InfiniteCombatRoll = false;
						bool InfiniteStamina = false;
						bool RemoveCollisions = false;
						bool Invisible = false;
						bool SeatBelt = false;
						bool GodMode = false;
						int GodModeKey = 0;
						int GodModeKeyState = 0;
						bool AntiAimEnabled = false;
						int AntiAimKey = 0;
						int AntiAimKeyState = 0;
					bool FastRun = false;
						bool Noclip = false;
						float RunSpeed = 1.0f;
				bool BoomFist = false;
				bool LockAllCars = false;
						bool UnLockAllCars = false;
						int NoclipKey = 0;
						int NoclipKeyState = 0;
						float NoClipSpeed = 2.f;
						int TpWayKey = 0;
						int TpWayKeyState = 0;
					float DamageMultiplier = 1.0f;
					float Boost = 1.0f;
				}LocalPlayer;
				struct Vehicle
				{
					bool GodMode = false;
					bool JumpingCar = false;
					bool RocketBoost = false;
					bool RocketBoostJumping = false;

					int RepairKey = 0;
					int RepairKeyState = 0;
					int RocketBoostKey = 0;
					int RocketBoostKeyState = 0;
					int JumpingCarKey = 0;
				int UnlockNearbyKey = 0;
				int UnlockNearbyKeyState = 0;
				float RocketBoostStrength = 5.0f;
				float JumpGravity = 3.0f;
				bool ModifyHandling = false;
				int HandlingAccel = 0;
				int HandlingBrake = 0;
				int HandlingTraction = 0;
				bool HandlingResetRequested = false;
			}Vehicle;
				struct Weapon
				{
					bool InfiniteAmmoEnabled = false;
				bool RemoveSpread = false;
				bool RemoveRecoil = false;

				bool NoReload = false;
				bool WeaponScaleEnabled = false;
					float WeaponScale = 1.0f;
				bool Coronhada = false;

				}Weapon;

			}Exploits;
			struct GiveWeapon
			{
				bool Enabled = false;
				int KeyBind = 0;
				int KeyBindState = 0;
				int WeaponHash = 0;
				char WeaponName[64] = "";
			}GiveWeapon;
				bool ShowActiveFeaturesOverlay = false;

			struct Trolls
			{
				bool VehicleGrabEnabled = false;
				int GrabKey = 0;
				int GrabKeyState = 0;
				float ThrowForce = 50.0f;
				float HoldDistance = 4.0f;
				float HoldHeight = 2.5f;
			}Trolls;
			}Misc;
			struct General
			{
				bool WebRemoteEnabled = false;
				int WebRemotePort = 8080;
				float PrimaryColor[4] = { 0.55f, 0.55f, 0.55f, 1.00f };
				float TextColor[4] = { 1.00f, 1.00f, 1.00f, 1.00f };

				bool ShutDown = false;
				bool EspOnSecondaryMonitor = false;
				bool SafeMode = false;
				int MenuKey = VK_INSERT;
				int KeyBind = 0;
				int KeyBindState = 0;
				bool CaptureBypass = true;
		bool WaterMark = false;
		bool WaterMarkCol = false;
		float WaterMarkColor[4] = { 1.f, 1.f, 1.f, 1.f };
				bool Particles = false;
				int ParticleOpacity = 50;
				bool Plexus = true;
				int PlexusOpacity = 100;
				bool AnimatedBorder = true;
				int BorderSpeed = 160;
				int BorderThickness = 6;
				int BorderGradientLen = 300;
				int ThreadDelay = 1;
				bool IsLoggedIn = false;
				char LicenseKey[64] = "TAMOJUNTO";
				std::string UserName;
				std::string UserRole;
				std::string AvatarURL;

				int DaysLeft = 0;  // -1 = lifetime, 0 = expirado, >0 = dias restantes
				int Language = 0;  // 0 = English, 1 = Portuguese

			}General;
			struct CheatGlobalUsage
			{
				bool bIsSpectatingEntity = false;
				bool bIsRemoteControllingVehicle = false;
				uint64_t SpectatingEntity = 0;
				uint64_t ControllingVehicle = 0;
			}CheatGlobalUsage;
		};
	}

	inline Cheat::Options g_Options;
