#include "Fivem.hpp"

#include <fstream>
#include <filesystem>
#include <thread>

#include "../Options.hpp"
#include <regex>

namespace Cheat
{
	float GetPlayerHeading()
	{
		auto localPed = g_Fivem.GetLocalPlayerInfo().Ped;
		if (!localPed) return 0.0f;
		return FrameWork::Memory::ReadMemory<float>((uintptr_t)localPed + 0x3C);
	}

	namespace Offsets
	{
		uint64_t m_InfiniteAmmo0;
		uint64_t m_InfiniteAmmo1;
		uint64_t EntityType;
		uint64_t m_DamageHandler;
		uint64_t Armor;
		uint64_t BoneManager;
		uint64_t PlayerInfo;
		uint64_t PlayerNetID;
		uint64_t WeaponManager;
		uint64_t FragInsNmGTA;
		uint64_t ConfigFlags;
		uint64_t MaxHealth;
		uint64_t LastVehicle;
		uint64_t VehicleBodyHealth;
		uint64_t VehicleTankHealth;
		uint64_t VisibleFlag;
		uint64_t SpeedModifier;
		uint64_t VehicleDriver;
		uint64_t FrameFlag;
		uint64_t SteeringAngle;
		uint64_t AcceleratorForce;
		uint64_t BrakeForce;
		uint64_t DoorLock;
		uint64_t SeatBelt;
		uint64_t SeatBeltWindShield;
		uint64_t SetVehicleCustomPrimaryColour; // E8 ? ? ? ? F6 43 50 02
		uint64_t Handling;
		uint64_t VehicleState;                  // 8A 87 ? ? ? ? C0 E8 ? 41 22 C5 88 83 ? ? ? ? 8A 87 ? ? ? ? C0 E8 ? 88 83
		uint64_t Stamina;
		uint64_t Clothes;
		uint64_t ArmsKinematics;                //E8 ?? ?? ?? ?? 48 83 C3 60 48 FF CF 75 E6 48 8B 5C 24  
		uint64_t LegsKinematics;                //E8 ?? ?? ?? ?? 48 83 C3 60 48 FF CF 75 DF 48 8B 5C 24 ?? 48 8B 6C 24 ?? 48 8B 74 24
		uint64_t MagicPatch;
	}

	std::string trim(const std::string& str) {
		size_t first = str.find_first_not_of(' ');
		if (first == std::string::npos) return "";
		size_t last = str.find_last_not_of(' ');
		return str.substr(first, last - first + 1);
	}

	int GetBuild() {
		std::wstring appDataPath = _wgetenv(L"LOCALAPPDATA");
		std::wstring iniFilePath = appDataPath + L"\\FiveM\\FiveM.app\\CitizenFX.ini";

		std::ifstream iniFile(iniFilePath);
		if (!iniFile.is_open()) {
			std::cerr << "Failed to open CitizenFX.ini" << std::endl;
			return -1;
		}

		std::string line;
		while (std::getline(iniFile, line)) {
			line = trim(line);

			if (line == "ReplaceExecutable=0") {
				return 3258;
			}
		}

		iniFile.clear();
		iniFile.seekg(0);
		while (std::getline(iniFile, line)) {
			line = trim(line);

			if (line.find(("SavedBuildNumber=")) != std::string::npos) {
				std::string buildNumberStr = line.substr(line.find("=") + 1);
				buildNumberStr = trim(buildNumberStr);
				return std::stoi(buildNumberStr);
			}
		}

		std::cout << ("Version not found") << std::endl;
		return -1;
	}

	void FivemSDK::Intialize()
	{
		if (bIsIntialized)
			return;

		static const std::vector<std::wstring> ProcessList = {
			XorStr(L"FiveM_b2060_GameProcess.exe"),
			XorStr(L"FiveM_b2060_GTAProcess.exe"),
			XorStr(L"FiveM_b2189_GameProcess.exe"),
			XorStr(L"FiveM_b2189_GTAProcess.exe"),
			XorStr(L"FiveM_b2372_GameProcess.exe"),
			XorStr(L"FiveM_b2372_GTAProcess.exe"),
			XorStr(L"FiveM_b2545_GameProcess.exe"),
			XorStr(L"FiveM_b2545_GTAProcess.exe"),
			XorStr(L"FiveM_b2612_GameProcess.exe"),
			XorStr(L"FiveM_b2612_GTAProcess.exe"),
			XorStr(L"FiveM_b2699_GameProcess.exe"),
			XorStr(L"FiveM_b2699_GTAProcess.exe"),
			XorStr(L"FiveM_b2802_GameProcess.exe"),
			XorStr(L"FiveM_b2802_GTAProcess.exe"),
			XorStr(L"FiveM_b2944_GameProcess.exe"),
			XorStr(L"FiveM_b2944_GTAProcess.exe"),
			XorStr(L"FiveM_b3095_GameProcess.exe"),
			XorStr(L"FiveM_b3095_GTAProcess.exe"),
			XorStr(L"FiveM_b3258_GameProcess.exe"),
			XorStr(L"FiveM_b3258_GTAProcess.exe"),
			XorStr(L"FiveM_GameProcess.exe"),
			XorStr(L"FiveM_GTAProcess.exe"),
			XorStr(L"FiveM_b3323_GameProcess.exe"),
			XorStr(L"FiveM_b3323_GTAProcess.exe"),
			XorStr(L"FiveM_b3407_GameProcess.exe"),
			XorStr(L"FiveM_b3407_GTAProcess.exe"),
			XorStr(L"FiveM_b3570_GameProcess.exe"),
			XorStr(L"FiveM_b3570_GTAProcess.exe"),
		};

		for (size_t i = 0; i < ProcessList.size(); i++)
		{
			Pid = FrameWork::Memory::GetProcessPidByName(ProcessList.at(i).c_str());
			if (Pid)
			{
				ModuleName = FrameWork::Misc::Wstring2String(ProcessList.at(i));
				ModuleBase = FrameWork::Memory::GetModuleBaseByName(Pid, ProcessList.at(i).c_str());
				ModuleBaseSize = FrameWork::Memory::GetModuleSizeByName(Pid, ProcessList.at(i).c_str());

				std::regex Regex(XorStr(R"_(FiveM_b(\d+))_"));
				std::smatch Match;

				if (std::regex_search(ModuleName, Match, Regex)) {
					GameVersion = std::stoi(Match[1].str());
				}
				else {
					GameVersion = 3258;
				}

				FrameWork::Memory::AttachProces(Pid);
				break;
			}
		}

		RealGameVersion = GetBuild();

		auto ResolvePattern = [&](const char* pattern, int offset) -> uint64_t
			{
				std::vector<uint8_t> signature;
				std::string p = pattern;
				for (size_t i = 0; i < p.size(); i++) {
					if (p[i] == ' ') continue;
					if (p[i] == '?') {
						signature.push_back(0x00);
					}
					else {
						char hex[3] = { p[i], p[i + 1], '\0' };
						signature.push_back((uint8_t)strtol(hex, nullptr, 16));
						i++;
					}
				}
				uint64_t addr = FrameWork::Memory::FindSignature(signature, ModuleBase, ModuleBaseSize);
				if (!addr) return 0;

				int rel = FrameWork::Memory::ReadMemory<int>(addr + offset);
				return addr + offset + 4 + rel;
			};

		if (RealGameVersion == 2372)
		{
			World = ResolvePattern("48 8B 05 ? ? ? ? 33 D2 48 8B 40 08 8A CA 48 85 C0 74 16 48 8B", 3);
			ReplayInterface = ResolvePattern("48 8D 0D ? ? ? ? 48 ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 8A D8 E8 ? ? ? ? 84 DB 75 13 48 8D 0D ? ? ? ? 48 8B D7 E8 ? ? ? ? 84 C0 74 BC 8B 8F", 3);
			ViewPort = ResolvePattern("48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD", 3);
			Camera = ResolvePattern("4C 8B 35 ? ? ? ? 33 FF 32 DB", 3);
			bIsPlayerAiming = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3); // Using PlayerAimingAt pattern for bIsPlayerAiming if not sure, but usually they are separate. Wait, 3570 block used PlayerAimingAt for PlayerAimingAt.
			PlayerAimingAt = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			std::vector<uint8_t> bullet_sig = { 0xF3, 0x41, 0x0F, 0x10, 0x19, 0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04 };
			HandleBullet = FrameWork::Memory::FindSignature(bullet_sig, ModuleBase, ModuleBaseSize);
			BlipList = ResolvePattern("4C 8D 05 ? ? ? ? 0F B7 C1", 3);
			GameplayCamHolder = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 48 08 48 85 C9 74 07", 3);
			GameplayCamTarget = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 40 08 48 85 C0 74 07", 3);
			CanCombatRoll = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 0B 48 8D 0D", 3);

			Offsets::EntityType = 0x10B8;
			Offsets::Armor = 0x14E0;
			Offsets::BoneManager = 0x430;
			Offsets::PlayerInfo = 0x10C8;
			Offsets::PlayerNetID = 0x88;
			Offsets::WeaponManager = 0x10D8;
			Offsets::FragInsNmGTA = 0x1400;
			Offsets::ConfigFlags = 0x1414;
			Offsets::MaxHealth = 0x2A0;
			Offsets::LastVehicle = 0xD30;
			Offsets::m_DamageHandler = 0xD10;
			Offsets::VehicleDriver = 0xC68;
			Offsets::DoorLock = 0x1390;
			Offsets::SteeringAngle = 0x9B4;
			Offsets::AcceleratorForce = Offsets::SteeringAngle + 0x8;
			Offsets::BrakeForce = Offsets::SteeringAngle + 0xC;
			Offsets::Clothes = 0x10C0;
			Offsets::Stamina = 0xCD4;
			Offsets::VisibleFlag = 0x142C;
			Offsets::Handling = 0x938;
			Offsets::SpeedModifier = 0xCF0;
			Offsets::VehicleState = 0x94A;
			Offsets::SeatBelt = 0x140C;
			Offsets::SeatBeltWindShield = 0x1418;
			Offsets::FrameFlag = 0x0218;

			bIsIntialized = true;
		}
		else if (RealGameVersion == 3407)
		{
			World = ResolvePattern("48 8B 05 ? ? ? ? 33 D2 48 8B 40 08 8A CA 48 85 C0 74 16 48 8B", 3);
			ReplayInterface = ResolvePattern("48 8D 0D ? ? ? ? 48 ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 8A D8 E8 ? ? ? ? 84 DB 75 13 48 8D 0D ? ? ? ? 48 8B D7 E8 ? ? ? ? 84 C0 74 BC 8B 8F", 3);
			ViewPort = ResolvePattern("48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD", 3);
			Camera = ResolvePattern("4C 8B 35 ? ? ? ? 33 FF 32 DB", 3);
			bIsPlayerAiming = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			PlayerAimingAt = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			BlipList = ResolvePattern("4C 8D 05 ? ? ? ? 0F B7 C1", 3);

			CanCombatRoll = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 0B 48 8D 0D", 3);
			GameplayCamHolder = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 48 08 48 85 C9 74 07", 3);
			GameplayCamTarget = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 40 08 48 85 C0 74 07", 3);
			std::vector<uint8_t> bullet_sig = { 0xF3, 0x41, 0x0F, 0x10, 0x19, 0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04 };
			HandleBullet = FrameWork::Memory::FindSignature(bullet_sig, ModuleBase, ModuleBaseSize);

			Offsets::MaxHealth = 0x284;;
			Offsets::WeaponManager = 0x10B8;
			Offsets::EntityType = 0x1098;
			Offsets::LastVehicle = 0xD10;
			Offsets::PlayerInfo = 0x10A8;
			Offsets::m_DamageHandler = 0xD70;
			Offsets::VisibleFlag = 0x0270;
			Offsets::FragInsNmGTA = 0x1430;
			Offsets::PlayerNetID = 0xE8;
			Offsets::Armor = 0x150C;
			Offsets::ConfigFlags = 0x1444;
			Offsets::VehicleState = 0x972;
			Offsets::FrameFlag = 0x0270;
			Offsets::Clothes = 0x10C0;

			bIsIntialized = true;
		}
		else if (RealGameVersion == 2612)
		{
			World = ResolvePattern("48 8B 05 ? ? ? ? 33 D2 48 8B 40 08 8A CA 48 85 C0 74 16 48 8B", 3);
			ReplayInterface = ResolvePattern("48 8D 0D ? ? ? ? 48 ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 8A D8 E8 ? ? ? ? 84 DB 75 13 48 8D 0D ? ? ? ? 48 8B D7 E8 ? ? ? ? 84 C0 74 BC 8B 8F", 3);
			ViewPort = ResolvePattern("48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD", 3);
			Camera = ResolvePattern("4C 8B 35 ? ? ? ? 33 FF 32 DB", 3);
			bIsPlayerAiming = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			PlayerAimingAt = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			BlipList = ResolvePattern("4C 8D 05 ? ? ? ? 0F B7 C1", 3);

			CanCombatRoll = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 0B 48 8D 0D", 3);
			GameplayCamHolder = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 48 08 48 85 C9 74 07", 3);
			GameplayCamTarget = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 40 08 48 85 C0 74 07", 3);
			std::vector<uint8_t> bullet_sig = { 0xF3, 0x41, 0x0F, 0x10, 0x19, 0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04 };
			HandleBullet = FrameWork::Memory::FindSignature(bullet_sig, ModuleBase, ModuleBaseSize);

			Offsets::EntityType = 0x10B8;
			Offsets::Armor = 0x1530;
			Offsets::BoneManager = 0x430;
			Offsets::PlayerInfo = 0x10C8;
			Offsets::PlayerNetID = 0x88;
			Offsets::WeaponManager = 0x10D8;
			Offsets::FragInsNmGTA = 0x1450;
			Offsets::ConfigFlags = 0x1464;
			Offsets::m_DamageHandler = 0xD10;
			Offsets::MaxHealth = 0x2A0;
			Offsets::LastVehicle = 0xD30;
			Offsets::VehicleDriver = 0xC68;
			Offsets::DoorLock = 0x1390;
			Offsets::SteeringAngle = 0x9B4;
			Offsets::AcceleratorForce = Offsets::SteeringAngle + 0x8;
			Offsets::BrakeForce = Offsets::SteeringAngle + 0xC;
			Offsets::Clothes = 0x10C0;
			Offsets::Stamina = 0xCF4;
			Offsets::VisibleFlag = 0x147C;
			Offsets::SpeedModifier = 0xCF0;
			Offsets::VehicleState = 0x94A;
			Offsets::SeatBelt = 0x145C;
			Offsets::SeatBeltWindShield = 0x1468;
			Offsets::FrameFlag = 0x0218;

			bIsIntialized = true;
		}
		else if (RealGameVersion == 2699)
		{
			World = ResolvePattern("48 8B 05 ? ? ? ? 33 D2 48 8B 40 08 8A CA 48 85 C0 74 16 48 8B", 3);
			ReplayInterface = ResolvePattern("48 8D 0D ? ? ? ? 48 ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 8A D8 E8 ? ? ? ? 84 DB 75 13 48 8D 0D ? ? ? ? 48 8B D7 E8 ? ? ? ? 84 C0 74 BC 8B 8F", 3);
			ViewPort = ResolvePattern("48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD", 3);
			Camera = ResolvePattern("4C 8B 35 ? ? ? ? 33 FF 32 DB", 3);
			bIsPlayerAiming = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			PlayerAimingAt = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			BlipList = ResolvePattern("4C 8D 05 ? ? ? ? 0F B7 C1", 3);

			CanCombatRoll = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 0B 48 8D 0D", 3);
			GameplayCamHolder = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 48 08 48 85 C9 74 07", 3);
			GameplayCamTarget = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 40 08 48 85 C0 74 07", 3);
			std::vector<uint8_t> bullet_sig = { 0xF3, 0x41, 0x0F, 0x10, 0x19, 0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04 };
			HandleBullet = FrameWork::Memory::FindSignature(bullet_sig, ModuleBase, ModuleBaseSize);

			Offsets::EntityType = 0x10B8;
			Offsets::Armor = 0x1530;
			Offsets::BoneManager = 0x430;
			Offsets::PlayerInfo = 0x10C8;
			Offsets::PlayerNetID = 0x88;
			Offsets::WeaponManager = 0x10D8;
			Offsets::FragInsNmGTA = 0x1450;
			Offsets::SteeringAngle = 0x9B4;
			Offsets::AcceleratorForce = Offsets::SteeringAngle + 0x8;
			Offsets::BrakeForce = Offsets::SteeringAngle + 0xC;
			Offsets::ConfigFlags = 0x1464;
			Offsets::MaxHealth = 0x2A0;
			Offsets::LastVehicle = 0xD30;
			Offsets::VehicleDriver = 0xC68;
			Offsets::m_DamageHandler = 0xD10;
			Offsets::DoorLock = 0x1390;
			Offsets::Clothes = 0x10C0;
			Offsets::Stamina = 0xCF4;
			Offsets::VisibleFlag = 0x147C;
			Offsets::SpeedModifier = 0xCF0;
			Offsets::Handling = 0x938;
			Offsets::VehicleState = 0x94A;
			Offsets::SeatBelt = 0x145C;
			Offsets::SeatBeltWindShield = 0x1468;
			Offsets::FrameFlag = 0x0218;

			bIsIntialized = true;
		}
		else if (RealGameVersion == 2189)
		{
			World = ResolvePattern("48 8B 05 ? ? ? ? 33 D2 48 8B 40 08 8A CA 48 85 C0 74 16 48 8B", 3);
			ReplayInterface = ResolvePattern("48 8D 0D ? ? ? ? 48 ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 8A D8 E8 ? ? ? ? 84 DB 75 13 48 8D 0D ? ? ? ? 48 8B D7 E8 ? ? ? ? 84 C0 74 BC 8B 8F", 3);
			ViewPort = ResolvePattern("48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD", 3);
			Camera = ResolvePattern("4C 8B 35 ? ? ? ? 33 FF 32 DB", 3);
			bIsPlayerAiming = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			PlayerAimingAt = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			BlipList = ResolvePattern("4C 8D 05 ? ? ? ? 0F B7 C1", 3);

			CanCombatRoll = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 0B 48 8D 0D", 3);
			GameplayCamHolder = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 48 08 48 85 C9 74 07", 3);
			GameplayCamTarget = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 40 08 48 85 C0 74 07", 3);
			std::vector<uint8_t> bullet_sig = { 0xF3, 0x41, 0x0F, 0x10, 0x19, 0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04 };
			HandleBullet = FrameWork::Memory::FindSignature(bullet_sig, ModuleBase, ModuleBaseSize);

			Offsets::EntityType = 0x10B8;
			Offsets::Armor = 0x14E0;
			Offsets::BoneManager = 0x430;
			Offsets::PlayerInfo = 0x10C8;
			Offsets::PlayerNetID = 0x78;
			Offsets::WeaponManager = 0x10D8;
			Offsets::FragInsNmGTA = 0x1400;
			Offsets::ConfigFlags = 0x1414;
			Offsets::MaxHealth = 0x2A0;
			Offsets::LastVehicle = 0xD30;
			Offsets::VehicleDriver = 0xC68;
			Offsets::DoorLock = 0x1390;
			Offsets::m_DamageHandler = 0xC9C;
			Offsets::Clothes = 0x10C0;
			Offsets::Stamina = 0xCD4;
			Offsets::VisibleFlag = 0x142C;
			Offsets::SpeedModifier = 0xCD0;
			Offsets::VehicleState = 0x94A;
			Offsets::SteeringAngle = 0x9B4;
			Offsets::AcceleratorForce = Offsets::SteeringAngle + 0x8;
			Offsets::BrakeForce = Offsets::SteeringAngle + 0xC;
			Offsets::Handling = 0x938;
			Offsets::SeatBelt = 0x140C;
			Offsets::SeatBeltWindShield = 0x1418;
			Offsets::FrameFlag = 0x0218;

			bIsIntialized = true;
		}
		else if (RealGameVersion == 2802)
		{
			World = ResolvePattern("48 8B 05 ? ? ? ? 33 D2 48 8B 40 08 8A CA 48 85 C0 74 16 48 8B", 3);
			ReplayInterface = ResolvePattern("48 8D 0D ? ? ? ? 48 ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 8A D8 E8 ? ? ? ? 84 DB 75 13 48 8D 0D ? ? ? ? 48 8B D7 E8 ? ? ? ? 84 C0 74 BC 8B 8F", 3);
			ViewPort = ResolvePattern("48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD", 3);
			Camera = ResolvePattern("4C 8B 35 ? ? ? ? 33 FF 32 DB", 3);
			bIsPlayerAiming = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			PlayerAimingAt = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			BlipList = ResolvePattern("4C 8D 05 ? ? ? ? 0F B7 C1", 3);

			CanCombatRoll = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 0B 48 8D 0D", 3);
			GameplayCamHolder = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 48 08 48 85 C9 74 07", 3);
			GameplayCamTarget = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 40 08 48 85 C0 74 07", 3);
			std::vector<uint8_t> bullet_sig = { 0xF3, 0x41, 0x0F, 0x10, 0x19, 0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04 };
			HandleBullet = FrameWork::Memory::FindSignature(bullet_sig, ModuleBase, ModuleBaseSize);

			Offsets::EntityType = 0x1098;
			Offsets::Armor = 0x1530;
			Offsets::BoneManager = 0x410;
			Offsets::PlayerInfo = 0x10A8;
			Offsets::PlayerNetID = 0x88;
			Offsets::WeaponManager = 0x10B8;
			Offsets::FragInsNmGTA = 0x1430;
			Offsets::ConfigFlags = 0x1444;
			Offsets::MaxHealth = 0x284;
			Offsets::LastVehicle = 0xD10;
			Offsets::VehicleDriver = 0xC48;
			Offsets::DoorLock = 0x1370;
			Offsets::Clothes = 0x10C0;
			Offsets::Stamina = 0xCF4;
			Offsets::m_DamageHandler = 0xD10;
			Offsets::SteeringAngle = 0x994;
			Offsets::AcceleratorForce = Offsets::SteeringAngle + 0x8;
			Offsets::BrakeForce = Offsets::SteeringAngle + 0xC;
			Offsets::VisibleFlag = 0x145C;
			Offsets::Handling = 0x918;
			Offsets::SpeedModifier = 0xCF0;
			Offsets::VehicleState = 0x92A;
			Offsets::SeatBelt = 0x143C;
			Offsets::SeatBeltWindShield = 0x1448;
			Offsets::FrameFlag = 0x0270;

			bIsIntialized = true;
		}
		else if (RealGameVersion == 2060)
		{
			World = ResolvePattern("48 8B 05 ? ? ? ? 33 D2 48 8B 40 08 8A CA 48 85 C0 74 16 48 8B", 3);
			ReplayInterface = ResolvePattern("48 8D 0D ? ? ? ? 48 ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 8A D8 E8 ? ? ? ? 84 DB 75 13 48 8D 0D ? ? ? ? 48 8B D7 E8 ? ? ? ? 84 C0 74 BC 8B 8F", 3);
			ViewPort = ResolvePattern("48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD", 3);
			Camera = ResolvePattern("4C 8B 35 ? ? ? ? 33 FF 32 DB", 3);
			bIsPlayerAiming = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			PlayerAimingAt = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			BlipList = ResolvePattern("4C 8D 05 ? ? ? ? 0F B7 C1", 3);

			CanCombatRoll = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 0B 48 8D 0D", 3);
			GameplayCamHolder = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 48 08 48 85 C9 74 07", 3);
			GameplayCamTarget = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 40 08 48 85 C0 74 07", 3);
			std::vector<uint8_t> bullet_sig = { 0xF3, 0x41, 0x0F, 0x10, 0x19, 0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04 };
			HandleBullet = FrameWork::Memory::FindSignature(bullet_sig, ModuleBase, ModuleBaseSize);

			Offsets::EntityType = 0x10B8;
			Offsets::Armor = 0x14E0;
			Offsets::BoneManager = 0x430;
			Offsets::PlayerInfo = 0x10A8;
			Offsets::PlayerNetID = 0x78;
			Offsets::WeaponManager = 0x10D8;
			Offsets::FragInsNmGTA = 0x1400;
			Offsets::ConfigFlags = 0x1414;
			Offsets::MaxHealth = 0x2A0;
			Offsets::LastVehicle = 0xD28;
			Offsets::VehicleDriver = 0xC68;
			Offsets::m_DamageHandler = 0xC9C;
			Offsets::DoorLock = 0x1390;
			Offsets::Clothes = 0x10C0;
			Offsets::Stamina = 0xCD4;
			Offsets::SteeringAngle = 0x9B4;
			Offsets::AcceleratorForce = Offsets::SteeringAngle + 0x8;
			Offsets::BrakeForce = Offsets::SteeringAngle + 0xC;
			Offsets::VisibleFlag = 0x142C;
			Offsets::SpeedModifier = 0xCD0;
			Offsets::VehicleState = 0x94A;
			Offsets::Handling = 0x938;
			Offsets::SeatBelt = 0x140C;
			Offsets::SeatBeltWindShield = 0x1418;
			Offsets::FrameFlag = 0x0218;

			bIsIntialized = true;
		}
		else if (RealGameVersion == 2545)
		{
			World = ResolvePattern("48 8B 05 ? ? ? ? 33 D2 48 8B 40 08 8A CA 48 85 C0 74 16 48 8B", 3);
			ReplayInterface = ResolvePattern("48 8D 0D ? ? ? ? 48 ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 8A D8 E8 ? ? ? ? 84 DB 75 13 48 8D 0D ? ? ? ? 48 8B D7 E8 ? ? ? ? 84 C0 74 BC 8B 8F", 3);
			ViewPort = ResolvePattern("48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD", 3);
			Camera = ResolvePattern("4C 8B 35 ? ? ? ? 33 FF 32 DB", 3);
			bIsPlayerAiming = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			PlayerAimingAt = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			BlipList = ResolvePattern("4C 8D 05 ? ? ? ? 0F B7 C1", 3);

			CanCombatRoll = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 0B 48 8D 0D", 3);
			GameplayCamHolder = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 48 08 48 85 C9 74 07", 3);
			GameplayCamTarget = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 40 08 48 85 C0 74 07", 3);
			std::vector<uint8_t> bullet_sig = { 0xF3, 0x41, 0x0F, 0x10, 0x19, 0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04 };
			HandleBullet = FrameWork::Memory::FindSignature(bullet_sig, ModuleBase, ModuleBaseSize);

			Offsets::EntityType = 0x10B8;
			Offsets::Armor = 0x1530;
			Offsets::BoneManager = 0x430;
			Offsets::PlayerInfo = 0x10A8;
			Offsets::PlayerNetID = 0x88;
			Offsets::WeaponManager = 0x10D8;
			Offsets::FragInsNmGTA = 0x1450;
			Offsets::ConfigFlags = 0x1464;
			Offsets::MaxHealth = 0x2A0;

			Offsets::LastVehicle = 0xD30;
			Offsets::VehicleDriver = 0xC68;
			Offsets::DoorLock = 0x1390;
			Offsets::Handling = 0x938;
			Offsets::Clothes = 0x10C0;
			Offsets::Stamina = 0xCF4;
			Offsets::SteeringAngle = 0x9B4;
			Offsets::m_DamageHandler = 0xD10;
			Offsets::AcceleratorForce = Offsets::SteeringAngle + 0x8;
			Offsets::BrakeForce = Offsets::SteeringAngle + 0xC;
			Offsets::VisibleFlag = 0x147C;
			Offsets::SpeedModifier = 0xCF0;
			Offsets::VehicleState = 0x94A;
			Offsets::SeatBelt = 0x145C;
			Offsets::SeatBeltWindShield = 0x1468;
			Offsets::FrameFlag = 0x0218;

			bIsIntialized = true;
		}
		else if (RealGameVersion == 2944)
		{
			World = ResolvePattern("48 8B 05 ? ? ? ? 33 D2 48 8B 40 08 8A CA 48 85 C0 74 16 48 8B", 3);
			ReplayInterface = ResolvePattern("48 8D 0D ? ? ? ? 48 ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 8A D8 E8 ? ? ? ? 84 DB 75 13 48 8D 0D ? ? ? ? 48 8B D7 E8 ? ? ? ? 84 C0 74 BC 8B 8F", 3);
			ViewPort = ResolvePattern("48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD", 3);
			Camera = ResolvePattern("4C 8B 35 ? ? ? ? 33 FF 32 DB", 3);
			bIsPlayerAiming = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			PlayerAimingAt = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			BlipList = ResolvePattern("4C 8D 05 ? ? ? ? 0F B7 C1", 3);

			CanCombatRoll = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 0B 48 8D 0D", 3);
			GameplayCamHolder = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 48 08 48 85 C9 74 07", 3);
			GameplayCamTarget = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 40 08 48 85 C0 74 07", 3);
			std::vector<uint8_t> bullet_sig = { 0xF3, 0x41, 0x0F, 0x10, 0x19, 0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04 };
			HandleBullet = FrameWork::Memory::FindSignature(bullet_sig, ModuleBase, ModuleBaseSize);

			Offsets::EntityType = 0x1098;
			Offsets::Armor = 0x150C;
			Offsets::BoneManager = 0x430;
			Offsets::PlayerInfo = 0x10A8;
			Offsets::PlayerNetID = 0xE8;
			Offsets::WeaponManager = 0x10B8;
			Offsets::SteeringAngle = 0x994;
			Offsets::AcceleratorForce = Offsets::SteeringAngle + 0x8;
			Offsets::BrakeForce = Offsets::SteeringAngle + 0xC;
			Offsets::FragInsNmGTA = 0x1430;
			Offsets::ConfigFlags = 0x1444;
			Offsets::MaxHealth = 0x284;
			Offsets::LastVehicle = 0xD10;
			Offsets::VehicleDriver = 0xC48;
			Offsets::DoorLock = 0x1370;
			Offsets::Clothes = 0x10C0;
			Offsets::Stamina = 0xCF4;
			Offsets::VisibleFlag = 0x145C;
			Offsets::Handling = 0x918;
			Offsets::m_DamageHandler = 0xD60;
			Offsets::SpeedModifier = 0xD40;
			Offsets::VehicleState = 0x92A;
			Offsets::SeatBelt = 0x143C;
			Offsets::SeatBeltWindShield = 0x1448;
			Offsets::FrameFlag = 0x0270;

			bIsIntialized = true;
		}
		else if (RealGameVersion == 3095)
		{
			World = ResolvePattern("48 8B 05 ? ? ? ? 33 D2 48 8B 40 08 8A CA 48 85 C0 74 16 48 8B", 3);
			ReplayInterface = ResolvePattern("48 8D 0D ? ? ? ? 48 ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 8A D8 E8 ? ? ? ? 84 DB 75 13 48 8D 0D ? ? ? ? 48 8B D7 E8 ? ? ? ? 84 C0 74 BC 8B 8F", 3);
			ViewPort = ResolvePattern("48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD", 3);
			Camera = ResolvePattern("4C 8B 35 ? ? ? ? 33 FF 32 DB", 3);
			bIsPlayerAiming = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			PlayerAimingAt = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			BlipList = ResolvePattern("4C 8D 05 ? ? ? ? 0F B7 C1", 3);

			CanCombatRoll = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 0B 48 8D 0D", 3);
			GameplayCamHolder = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 48 08 48 85 C9 74 07", 3);
			GameplayCamTarget = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 40 08 48 85 C0 74 07", 3);
			std::vector<uint8_t> bullet_sig = { 0xF3, 0x41, 0x0F, 0x10, 0x19, 0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04 };
			HandleBullet = FrameWork::Memory::FindSignature(bullet_sig, ModuleBase, ModuleBaseSize);

			Offsets::EntityType = 0x1098;
			Offsets::Armor = 0x150C;
			Offsets::BoneManager = 0x430;
			Offsets::PlayerInfo = 0x10A8;
			Offsets::PlayerNetID = 0xE8;
			Offsets::WeaponManager = 0x10B8;
			Offsets::FragInsNmGTA = 0x1430;
			Offsets::ConfigFlags = 0x1444;
			Offsets::MaxHealth = 0x284;
			Offsets::LastVehicle = 0xD10;
			Offsets::VehicleDriver = 0xC90;
			Offsets::DoorLock = 0x13C0;
			Offsets::m_DamageHandler = 0xD60;
			Offsets::SteeringAngle = 0x994;
			Offsets::AcceleratorForce = Offsets::SteeringAngle + 0x8;
			Offsets::BrakeForce = Offsets::SteeringAngle + 0xC;
			Offsets::Handling = 0x960;
			Offsets::Clothes = 0x10C0;
			Offsets::Stamina = 0xCF4;
			Offsets::VisibleFlag = 0x145C;
			Offsets::SpeedModifier = 0xD40;
			Offsets::VehicleState = 0x972;
			Offsets::SeatBelt = 0x143C;
			Offsets::SeatBeltWindShield = 0x1448;
			Offsets::FrameFlag = 0x0270;

			bIsIntialized = true;
		}
		else if (RealGameVersion == 3258)
		{
			World = ResolvePattern("48 8B 05 ? ? ? ? 33 D2 48 8B 40 08 8A CA 48 85 C0 74 16 48 8B", 3);
			ReplayInterface = ResolvePattern("48 8D 0D ? ? ? ? 48 ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 8A D8 E8 ? ? ? ? 84 DB 75 13 48 8D 0D ? ? ? ? 48 8B D7 E8 ? ? ? ? 84 C0 74 BC 8B 8F", 3);
			ViewPort = ResolvePattern("48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD", 3);
			Camera = ResolvePattern("4C 8B 35 ? ? ? ? 33 FF 32 DB", 3);
			bIsPlayerAiming = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			PlayerAimingAt = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			BlipList = ResolvePattern("4C 8D 05 ? ? ? ? 0F B7 C1", 3);

			CanCombatRoll = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 0B 48 8D 0D", 3);
			GameplayCamHolder = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 48 08 48 85 C9 74 07", 3);
			GameplayCamTarget = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 40 08 48 85 C0 74 07", 3);
			std::vector<uint8_t> bullet_sig = { 0xF3, 0x41, 0x0F, 0x10, 0x19, 0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04 };
			HandleBullet = FrameWork::Memory::FindSignature(bullet_sig, ModuleBase, ModuleBaseSize);

			Offsets::EntityType = 0x1098;
			Offsets::Armor = 0x150C;
			Offsets::BoneManager = 0x430;
			Offsets::PlayerInfo = 0x10A8;
			Offsets::PlayerNetID = 0xE8;
			Offsets::WeaponManager = 0x10B8;
			Offsets::FragInsNmGTA = 0x1430;
			Offsets::ConfigFlags = 0x1444;
			Offsets::MaxHealth = 0x284;
			Offsets::LastVehicle = 0xD10;
			Offsets::VisibleFlag = 0x145C;
			Offsets::SpeedModifier = 0xD50;
			Offsets::m_DamageHandler = 0xD70;
			Offsets::DoorLock = 0x13C0;
			Offsets::SeatBelt = 0x143C;
			Offsets::SeatBeltWindShield = 0x1448;
			Offsets::Handling = 0x960;
			Offsets::VehicleState = 0x972;
			Offsets::Clothes = 0x10C0;
			Offsets::Stamina = 0xCF4;
			Offsets::FrameFlag = 0x0270;

			bIsIntialized = true;
		}
		else if (RealGameVersion == 3323)
		{
			World = ResolvePattern("48 8B 05 ? ? ? ? 33 D2 48 8B 40 08 8A CA 48 85 C0 74 16 48 8B", 3);
			ReplayInterface = ResolvePattern("48 8D 0D ? ? ? ? 48 ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 8A D8 E8 ? ? ? ? 84 DB 75 13 48 8D 0D ? ? ? ? 48 8B D7 E8 ? ? ? ? 84 C0 74 BC 8B 8F", 3);
			ViewPort = ResolvePattern("48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD", 3);
			Camera = ResolvePattern("4C 8B 35 ? ? ? ? 33 FF 32 DB", 3);
			bIsPlayerAiming = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			BlipList = ResolvePattern("4C 8D 05 ? ? ? ? 0F B7 C1", 3);

			CanCombatRoll = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 0B 48 8D 0D", 3);
			GameplayCamHolder = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 48 08 48 85 C9 74 07", 3);
			GameplayCamTarget = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 40 08 48 85 C0 74 07", 3);
			PlayerAimingAt = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			std::vector<uint8_t> bullet_sig = { 0xF3, 0x41, 0x0F, 0x10, 0x19, 0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04 };
			HandleBullet = FrameWork::Memory::FindSignature(bullet_sig, ModuleBase, ModuleBaseSize);

			Offsets::EntityType = 0x1098;
			Offsets::Armor = 0x150C;
			Offsets::BoneManager = 0x430;
			Offsets::PlayerInfo = 0x10A8;
			Offsets::PlayerNetID = 0xE8;
			Offsets::WeaponManager = 0x10B8;
			Offsets::FragInsNmGTA = 0x1430;
			Offsets::ConfigFlags = 0x1444;
			Offsets::MaxHealth = 0x284;
			Offsets::LastVehicle = 0xD30;
			Offsets::VisibleFlag = 0x147C;
			Offsets::SpeedModifier = 0xD40;
			Offsets::DoorLock = 0x13C0;
			Offsets::SeatBelt = 0x145C;
			Offsets::m_DamageHandler = 0xD70;
			Offsets::SeatBeltWindShield = 0x1468;
			Offsets::Handling = 0x938;
			Offsets::Clothes = 0x10C0;
			Offsets::Stamina = 0xCF4;

			bIsIntialized = true;
		}
		else if (RealGameVersion == 3570)
		{
			World = ResolvePattern("48 8B 05 ? ? ? ? 33 D2 48 8B 40 08 8A CA 48 85 C0 74 16 48 8B", 3);
			ReplayInterface = ResolvePattern("48 8D 0D ? ? ? ? 48 ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 8A D8 E8 ? ? ? ? 84 DB 75 13 48 8D 0D ? ? ? ? 48 8B D7 E8 ? ? ? ? 84 C0 74 BC 8B 8F", 3);
			ViewPort = ResolvePattern("48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD", 3);
			Camera = ResolvePattern("4C 8B 35 ? ? ? ? 33 FF 32 DB", 3);
			bIsPlayerAiming = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			BlipList = ResolvePattern("4C 8D 05 ? ? ? ? 0F B7 C1", 3);

			CanCombatRoll = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 0B 48 8D 0D", 3);
			GameplayCamHolder = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 48 08 48 85 C9 74 07", 3);
			GameplayCamTarget = ResolvePattern("48 8B 05 ? ? ? ? 48 8B 40 08 48 85 C0 74 07", 3);

			PlayerAimingAt = ResolvePattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", 3);
			std::vector<uint8_t> bullet_sig = { 0xF3, 0x41, 0x0F, 0x10, 0x19, 0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04 };
			HandleBullet = FrameWork::Memory::FindSignature(bullet_sig, ModuleBase, ModuleBaseSize);

			Offsets::MaxHealth = 0x284;
			Offsets::WeaponManager = 0x10B8;
			Offsets::EntityType = 0x1098;
			Offsets::LastVehicle = 0xD10;
			Offsets::PlayerInfo = 0x10A8;
			Offsets::FragInsNmGTA = 0x1430;
			Offsets::PlayerNetID = 0xE8;
			Offsets::Armor = 0x150C;
			Offsets::ConfigFlags = 0x1444;

			bIsIntialized = true;
		}

		CitizemPlayerNamesModule = FrameWork::Memory::GetModuleBaseByName(Pid, XorStr(L"citizen-playernames-five.dll"));
		NetIdToNamesPtr = CitizemPlayerNamesModule + 0x30D98;

		if (bIsIntialized)
			FrameWork::Memory::AttachProces(Pid);

		uintptr_t HandlingSigAddr = FrameWork::Memory::FindSignature({ 0x48, 0x8b, 0x83,0x00,0x00,0x00,0x00, 0x8b, 0xd5, 0x48, 0x89, 0x44, 0x24,0x00, 0x4c, 0x89, 0x74, 0x24 }, ModuleBase, ModuleBaseSize);
		Offsets::m_Handling = FrameWork::Memory::ReadMemory<int>(HandlingSigAddr + 3);

		uintptr_t VehicleEngineSigAddr = FrameWork::Memory::FindSignature({ 0xf3, 0x0f, 0x11, 0x80, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8b, 0x87, 0x00, 0x00, 0x00, 0x00, 0xf3, 0x0f, 0x10, 0x05 }, ModuleBase, ModuleBaseSize);
		if (VehicleEngineSigAddr)
			Offsets::m_VehicleEngineHealth = FrameWork::Memory::ReadMemory<int>(VehicleEngineSigAddr + 4);

		uintptr_t VehicleBodyHealthSigAddr = FrameWork::Memory::FindSignature({ 0xF3, 0x0F, 0x11, 0x83, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x5C, 0x24, 0x30 }, ModuleBase, ModuleBaseSize);
		if (VehicleBodyHealthSigAddr)
			Offsets::m_VehicleBodyHealth = FrameWork::Memory::ReadMemory<int>(VehicleBodyHealthSigAddr + 4);

		uintptr_t SeatBealtAddr = FrameWork::Memory::FindSignature({ 0xf6, 0x81,0x00 ,0x00 ,0x00 ,0x00 ,0x00 , 0x75,0x00 , 0x8b, 0x83 }, ModuleBase, ModuleBaseSize);
		Offsets::m_SeatBealt = FrameWork::Memory::ReadMemory<int>(SeatBealtAddr + 2);

		Offsets::ArmsKinematics = FrameWork::Memory::FindSignature({ 0xE8, 0x00, 0x00, 0x00, 0x00, 0x48, 0x83, 0xC3, 0x60, 0x48, 0xFF, 0xCF, 0x75, 0xE6, 0x48, 0x8B, 0x5C, 0x24 }, ModuleBase, ModuleBaseSize);
		Offsets::LegsKinematics = FrameWork::Memory::FindSignature({ 0xE8, 0x00, 0x00, 0x00, 0x00, 0x48, 0x83, 0xC3, 0x60, 0x48, 0xFF, 0xCF, 0x75, 0xDF, 0x48, 0x8B, 0x5C, 0x24, 0x00, 0x48, 0x8B, 0x6C, 0x24, 0x00, 0x48, 0x8B, 0x74, 0x24 }, ModuleBase, ModuleBaseSize);


		Offsets::m_InfiniteAmmo0 = FrameWork::Memory::FindSignature({ 0x41, 0x2B, 0xC9, 0x3B, 0xC8, 0x0F, 0x4D, 0xC8 }, ModuleBase, ModuleBaseSize);
		Offsets::m_InfiniteAmmo1 = FrameWork::Memory::FindSignature({ 0x41, 0x2B, 0xD1, 0xE8 }, ModuleBase, ModuleBaseSize);

		// ── Raycast table pattern (used for occlusion visibility check) ──
		{
			uintptr_t RaycastSigAddr = FrameWork::Memory::FindSignature({ 0x48, 0x8D, 0x1D, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xC1 }, ModuleBase, ModuleBaseSize);
			if (RaycastSigAddr)
			{
				int32_t rel = FrameWork::Memory::ReadMemory<int32_t>(RaycastSigAddr + 3);
				Offsets::m_RaycastTable = RaycastSigAddr + 7 + rel;
			}
		}

		uintptr_t SpreadSigAddr = FrameWork::Memory::FindSignature({ 0xF3, 0x41, 0x0F, 0x10, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xCA }, ModuleBase, ModuleBaseSize);
		if (SpreadSigAddr)
			Offsets::m_Spread = FrameWork::Memory::ReadMemory<int>(SpreadSigAddr + 5);

		ProcHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Pid);

		// ── Pattern scan for WorldProbe (raycast function) ──
		{
			std::vector<uint8_t> wpPattern = {
				0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10,
				0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x41, 0x54, 0x41, 0x55,
				0x41, 0x56, 0x41, 0x57, 0x48, 0x83, 0xEC, 0x30, 0x48, 0x8B,
				0xF9, 0x0F, 0x28, 0xF2
			};
			WorldProbeAddr = FrameWork::Memory::FindSignature(wpPattern, ModuleBase, ModuleBaseSize);
		}

		// ── Pattern scan for GetShapeTestResult ──
		{
			std::vector<uint8_t> grPattern = {
				0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x4C, 0x24, 0x08,
				0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xD9, 0x48, 0x8B,
				0xFA, 0x48, 0x8B, 0xF2
			};
			GetShapeTestResultAddr = FrameWork::Memory::FindSignature(grPattern, ModuleBase, ModuleBaseSize);
		}

		// ── Shellcode init ──
		InitRaycastShellcode();
	}

	// Shellcode layout:
	// +0x00: fromX, fromY, fromZ (12 bytes)
	// +0x0C: toX, toY, toZ (12 bytes)
	// +0x18: WorldProbeFn (8 bytes)
	// +0x20: GetShapeTestResultFn (8 bytes)
	// +0x28: result (int32) output (0=blocked,1=visible)
	// +0x2C: padding (4)
	// +0x30: handle (int64) output
	void FivemSDK::InitRaycastShellcode()
	{
		if (m_ShellcodeReady)
			return;
		if (!WorldProbeAddr || !GetShapeTestResultAddr)
			return;

		std::vector<uint8_t> sc;

		auto emit = [&](uint8_t b) { sc.push_back(b); };

		// push rbx                   ; save non-volatile register
		emit(0x53);
		// push rbp
		emit(0x55);
		// mov rbp, rsp
		emit(0x48); emit(0x89); emit(0xE5);
		// sub rsp, 0x30
		emit(0x48); emit(0x83); emit(0xEC); emit(0x30);

		// mov rbx, rcx               ; params pointer → rbx
		emit(0x48); emit(0x89); emit(0xCB);

		// ── Call WorldProbe (__vectorcall) ──
		// xmm0=fromX, xmm1=fromY, xmm2=fromZ, xmm3=toX, xmm4=toY, xmm5=toZ, r8d=flags
		// movss xmm0, [rbx]
		emit(0xF3); emit(0x0F); emit(0x10); emit(0x03);
		// movss xmm1, [rbx+4]
		emit(0xF3); emit(0x0F); emit(0x10); emit(0x4B); emit(0x04);
		// movss xmm2, [rbx+8]
		emit(0xF3); emit(0x0F); emit(0x10); emit(0x53); emit(0x08);
		// movss xmm3, [rbx+0x0C]
		emit(0xF3); emit(0x0F); emit(0x10); emit(0x5B); emit(0x0C);
		// movss xmm4, [rbx+0x10]
		emit(0xF3); emit(0x0F); emit(0x10); emit(0x63); emit(0x10);
		// movss xmm5, [rbx+0x14]
		emit(0xF3); emit(0x0F); emit(0x10); emit(0x6B); emit(0x14);

		// mov r8d, 0x1FF (flags = 511)
		emit(0x41); emit(0xB8);
		emit(0xFF); emit(0x01); emit(0x00); emit(0x00);

		// sub rsp, 0x20 (shadow space, restored after)
		emit(0x48); emit(0x83); emit(0xEC); emit(0x20);
		// mov rax, [rbx+0x18] (WorldProbeFn)
		emit(0x48); emit(0x8B); emit(0x43); emit(0x18);
		// call rax
		emit(0xFF); emit(0xD0);
		// add rsp, 0x20 (restore stack)
		emit(0x48); emit(0x83); emit(0xC4); emit(0x20);

		// Save handle to [rbx+0x30]
		emit(0x48); emit(0x89); emit(0x43); emit(0x30);

		// ── Poll GetShapeTestResult ──
		// poll_loop (current offset in sc = let's store and calculate)
		int poll_start = (int)sc.size();

		// mov rcx, [rbx+0x30]     ; handle
		emit(0x48); emit(0x8B); emit(0x4B); emit(0x30);
		// lea rdx, [rbx+0x28]     ; &result
		emit(0x48); emit(0x8D); emit(0x53); emit(0x28);
		// xor r8d, r8d            ; hitPos = nullptr
		emit(0x45); emit(0x31); emit(0xC0);
		// xor r9d, r9d            ; hitNormal = nullptr
		emit(0x45); emit(0x31); emit(0xC9);
		// mov [rsp+0x28], r8      ; entityType = nullptr (on stack)
		emit(0x4C); emit(0x89); emit(0x44); emit(0x24); emit(0x28);

		// mov rax, [rbx+0x20]     ; GetShapeTestResultFn
		emit(0x48); emit(0x8B); emit(0x43); emit(0x20);
		// call rax
		emit(0xFF); emit(0xD0);

		// test eax, eax
		emit(0x85); emit(0xC0);
		// jz short poll_loop
		int poll_end = (int)sc.size() + 2;
		int disp = poll_start - poll_end;
		emit(0x74); emit((uint8_t)(disp & 0xFF));

		// ── Convert result ──
		// mov eax, [rbx+0x28]     ; read didHit
		emit(0x8B); emit(0x43); emit(0x28);
		// test eax, eax
		emit(0x85); emit(0xC0);
		// sete al                  ; al = 1 if didHit == 0 (visible)
		emit(0x0F); emit(0x94); emit(0xC0);
		// mov [rbx+0x28], eax     ; store result
		emit(0x89); emit(0x43); emit(0x28);

		// ── Epilogue ──
		if (sc.size() < 200)
		{
			// add rsp, 0x30
			emit(0x48); emit(0x83); emit(0xC4); emit(0x30);
		}
		else
		{
			// Need larger adjustment - unlikely but handle it
			emit(0x48); emit(0x81); emit(0xC4);
			emit(0x30); emit(0x00); emit(0x00); emit(0x00);
		}
		// pop rbp
		emit(0x5D);
		// pop rbx
		emit(0x5B);
		// ret
		emit(0xC3);

		// Allocate and write to target
		uint64_t cave = FrameWork::Memory::CreateCodeCave(sc.size());
		if (!cave)
			return;

		if (!FrameWork::Memory::WriteBytes(cave, sc))
		{
			FrameWork::Memory::FreeCave(cave);
			return;
		}

		// Pre-allocate params struct
		uint64_t params = FrameWork::Memory::CreateCodeCave(0x40);
		if (!params)
		{
			FrameWork::Memory::FreeCave(cave);
			return;
		}

		m_ShellcodeAddr = cave;
		m_ParamsAddr = params;
		m_ShellcodeReady = true;
	}

	// ── InitSpawnVehicleShellcode ─────────────────────────────
	void FivemSDK::InitSpawnVehicleShellcode()
	{
		if (m_SpawnVehShellcodeReady)
			return;
		if (!CreateVehicleAddr)
			return;

		std::vector<uint8_t> sc;

		auto emit = [&](uint8_t b) { sc.push_back(b); };

		// push rbx
		emit(0x53);
		// push rbp
		emit(0x55);
		// mov rbp, rsp
		emit(0x48); emit(0x89); emit(0xE5);
		// sub rsp, 0x50
		emit(0x48); emit(0x83); emit(0xEC); emit(0x50);

		// mov rbx, rcx              ; params -> rbx
		emit(0x48); emit(0x89); emit(0xCB);

		// mov ecx, [rbx]           ; modelHash
		emit(0x8B); emit(0x0B);

		// movss xmm1, [rbx+8]      ; x
		emit(0xF3); emit(0x0F); emit(0x10); emit(0x4B); emit(0x08);
		// movss xmm2, [rbx+12]     ; y
		emit(0xF3); emit(0x0F); emit(0x10); emit(0x53); emit(0x0C);
		// movss xmm3, [rbx+16]     ; z
		emit(0xF3); emit(0x0F); emit(0x10); emit(0x5B); emit(0x10);

		// heading -> [rsp+0x20]
		emit(0xF3); emit(0x0F); emit(0x10); emit(0x43); emit(0x14);
		emit(0xF3); emit(0x0F); emit(0x11); emit(0x44); emit(0x24); emit(0x20);

		// isNetwork -> [rsp+0x28]
		emit(0x8B); emit(0x43); emit(0x18);
		emit(0x89); emit(0x44); emit(0x24); emit(0x28);

		// bScriptHostVeh -> [rsp+0x30]
		emit(0x8B); emit(0x43); emit(0x1C);
		emit(0x89); emit(0x44); emit(0x24); emit(0x30);

		// mov rax, [rbx+0x20]      ; funAddr
		emit(0x48); emit(0x8B); emit(0x43); emit(0x20);
		// call rax
		emit(0xFF); emit(0xD0);

		// mov [rbx+0x28], rax      ; save result
		emit(0x48); emit(0x89); emit(0x43); emit(0x28);

		// leave
		emit(0xC9);
		// pop rbx
		emit(0x5B);
		// ret
		emit(0xC3);

		uint64_t cave = FrameWork::Memory::CreateCodeCave(sc.size());
		if (!cave)
			return;

		if (!FrameWork::Memory::WriteBytes(cave, sc))
		{
			FrameWork::Memory::FreeCave(cave);
			return;
		}

		uint64_t params = FrameWork::Memory::CreateCodeCave(0x30);
		if (!params)
		{
			FrameWork::Memory::FreeCave(cave);
			return;
		}

		m_SpawnVehShellcodeAddr = cave;
		m_SpawnVehParamsAddr = params;
		m_SpawnVehShellcodeReady = true;
	}

	// ── WorldSpawnVehicle ─────────────────────────────────────
	uint64_t FivemSDK::WorldSpawnVehicle(uint32_t modelHash, Vector3D pos, float heading)
	{
		if (!m_SpawnVehShellcodeReady)
			return 0;

		using namespace FrameWork::Memory;

		WriteMemory<uint32_t>(m_SpawnVehParamsAddr, modelHash);
		WriteMemory<float>(m_SpawnVehParamsAddr + 0x08, pos.x);
		WriteMemory<float>(m_SpawnVehParamsAddr + 0x0C, pos.y);
		WriteMemory<float>(m_SpawnVehParamsAddr + 0x10, pos.z);
		WriteMemory<float>(m_SpawnVehParamsAddr + 0x14, heading);
		WriteMemory<int32_t>(m_SpawnVehParamsAddr + 0x18, 0);
		WriteMemory<int32_t>(m_SpawnVehParamsAddr + 0x1C, 0);
		WriteMemory<uint64_t>(m_SpawnVehParamsAddr + 0x20, CreateVehicleAddr);
		WriteMemory<uint64_t>(m_SpawnVehParamsAddr + 0x28, 0);

		HANDLE hThread = CreateRemoteThread(
			ProcHandle, NULL, 0,
			(LPTHREAD_START_ROUTINE)m_SpawnVehShellcodeAddr,
			(LPVOID)m_SpawnVehParamsAddr, 0, NULL
		);

		if (!hThread)
			return 0;

		WaitForSingleObject(hThread, 5000);
		CloseHandle(hThread);

		return ReadMemory<uint64_t>(m_SpawnVehParamsAddr + 0x28);
	}

	// ── HandleToVehicle ───────────────────────────────────────
	CVehicle* FivemSDK::HandleToVehicle(uint64_t handle)
	{
		if (!handle) return nullptr;

		if (handle > 0x10000)
			return (CVehicle*)handle;

		int poolIndex = (int)(handle >> 8);
		if (!pVehicleInterface) return nullptr;

		auto vehList = pVehicleInterface->VehicleList();
		if (!vehList) return nullptr;

		return vehList->Vehicle(poolIndex);
	}

	// ── CheckLOS ─────────────────────────────────────────────
	bool FivemSDK::CheckLOS(Vector3D from, Vector3D to)
	{
		if (m_ShellcodeReady)
			return HasLineOfSight(from, to);

		ImVec2 screen = WorldToScreen(to);
		if (screen.x == 0.0f && screen.y == 0.0f)
			return false;

		Vector3D lineDir;
		lineDir.x = to.x - from.x;
		lineDir.y = to.y - from.y;
		lineDir.z = to.z - from.z;
		float lenSqr = lineDir.LengthSqr();
		if (lenSqr < 1.0f)
			return true;

		// ── Static thread-local cache to reduce allocations ──
		thread_local std::vector<Entity> ents;
		thread_local std::vector<VehicleInfo> vehs;
		ents.clear();
		vehs.clear();

		GetEntitiyListSnapshot(ents);
		GetVehicleListSnapshot(vehs);

		// ── Pedestrian check ──
		for (size_t ei = 0; ei < ents.size(); ei++)
		{
			const auto& e = ents[ei];
			if (!e.StaticInfo.Ped || e.StaticInfo.bIsLocalPlayer)
				continue;

			Vector3D toTarget;
			toTarget.x = to.x - e.Cordinates.x;
			toTarget.y = to.y - e.Cordinates.y;
			toTarget.z = to.z - e.Cordinates.z;
			if (toTarget.LengthSqr() < 4.0f)
				continue; // skip target itself

			// Project ped position onto the line segment
			Vector3D toEnt;
			toEnt.x = e.Cordinates.x - from.x;
			toEnt.y = e.Cordinates.y - from.y;
			toEnt.z = e.Cordinates.z - from.z;
			float t = (toEnt.x * lineDir.x + toEnt.y * lineDir.y + toEnt.z * lineDir.z) / lenSqr;
			if (t < 0.0f || t > 1.0f)
				continue;

			// Find closest point on segment
			Vector3D closest;
			closest.x = from.x + lineDir.x * t;
			closest.y = from.y + lineDir.y * t;
			closest.z = from.z + lineDir.z * t;

			Vector3D offset;
			offset.x = e.Cordinates.x - closest.x;
			offset.y = e.Cordinates.y - closest.y;
			offset.z = e.Cordinates.z - closest.z;
			if (offset.LengthSqr() < 2.25f) // ped radius ~1.5
				return false;
		}

		// ── Vehicle check ──
		// Find which entity is at 'to' to skip their own vehicle
		uint64_t targetPed = 0;
		for (size_t ei = 0; ei < ents.size(); ei++)
		{
			const auto& e = ents[ei];
			if (!e.StaticInfo.Ped)
				continue;
			Vector3D diff;
			diff.x = to.x - e.Cordinates.x;
			diff.y = to.y - e.Cordinates.y;
			diff.z = to.z - e.Cordinates.z;
			if (diff.LengthSqr() < 1.0f)
			{
				targetPed = reinterpret_cast<uint64_t>(e.StaticInfo.Ped);
				break;
			}
		}
		// Get target's last vehicle to skip it
		uint64_t targetVehicle = 0;
		if (targetPed)
			targetVehicle = FrameWork::Memory::ReadMemory<uint64_t>(targetPed + Offsets::LastVehicle);

		for (size_t vi = 0; vi < vehs.size(); vi++)
		{
			const auto& v = vehs[vi];
			if (!v.Vehicle)
				continue;

			uint64_t vehAddr = reinterpret_cast<uint64_t>(v.Vehicle);
			if (vehAddr == targetVehicle)
				continue; // skip the vehicle the target is in

			Vector3D vehPos = v.Vehicle->GetCoordinate();

			Vector3D toTarget;
			toTarget.x = to.x - vehPos.x;
			toTarget.y = to.y - vehPos.y;
			toTarget.z = to.z - vehPos.z;
			if (toTarget.LengthSqr() < 4.0f)
				continue;

			// Fixed radius for all vehicles (model dimension offsets vary by version)
			float radius = 2.8f;

			Vector3D toEnt;
			toEnt.x = vehPos.x - from.x;
			toEnt.y = vehPos.y - from.y;
			toEnt.z = vehPos.z - from.z;
			float t = (toEnt.x * lineDir.x + toEnt.y * lineDir.y + toEnt.z * lineDir.z) / lenSqr;
			if (t < 0.0f || t > 1.0f)
				continue;

			Vector3D closest;
			closest.x = from.x + lineDir.x * t;
			closest.y = from.y + lineDir.y * t;
			closest.z = from.z + lineDir.z * t;

			Vector3D offset;
			offset.x = vehPos.x - closest.x;
			offset.y = vehPos.y - closest.y;
			offset.z = vehPos.z - closest.z;
			if (offset.LengthSqr() < radius * radius)
				return false;
		}

		return true;
	}

	bool FivemSDK::HasLineOfSight(Vector3D from, Vector3D to)
	{
		if (!m_ShellcodeReady)
			return true;

		using namespace FrameWork::Memory;

		WriteMemory<float>(m_ParamsAddr + 0x00, from.x);
		WriteMemory<float>(m_ParamsAddr + 0x04, from.y);
		WriteMemory<float>(m_ParamsAddr + 0x08, from.z);
		WriteMemory<float>(m_ParamsAddr + 0x0C, to.x);
		WriteMemory<float>(m_ParamsAddr + 0x10, to.y);
		WriteMemory<float>(m_ParamsAddr + 0x14, to.z);
		WriteMemory<uint64_t>(m_ParamsAddr + 0x18, WorldProbeAddr);
		WriteMemory<uint64_t>(m_ParamsAddr + 0x20, GetShapeTestResultAddr);
		WriteMemory<int32_t>(m_ParamsAddr + 0x28, 0);

		HANDLE hThread = CreateRemoteThread(
			ProcHandle, NULL, 0,
			(LPTHREAD_START_ROUTINE)m_ShellcodeAddr,
			(LPVOID)m_ParamsAddr, 0, NULL
		);

		if (!hThread)
			return true;

		WaitForSingleObject(hThread, 100);
		CloseHandle(hThread);

		int32_t result = ReadMemory<int32_t>(m_ParamsAddr + 0x28);
		return result != 0;
	}

	uint8_t get_owner_id(uint64_t net_object) {
		return FrameWork::Memory::ReadMemory<uint8_t>(net_object + 0x49);
	}
	bool set_network_id_can_migrate(uint64_t net_object, bool toggle) {
		{
			auto word_004E = FrameWork::Memory::ReadMemory<uint16_t>(net_object + 0x4E);
			if (!toggle)
				FrameWork::Memory::WriteMemory<uint16_t>(net_object + 0x4E, word_004E |= 1);
			else
				FrameWork::Memory::WriteMemory<uint16_t>(net_object + 0x4E, word_004E &= ~1);
		}
		return true;
	}
	bool set_network_id_exists_on_all_machines(uint64_t net_object, bool toggle) {
		auto word_004E = FrameWork::Memory::ReadMemory<uint16_t>(net_object + 0x4E);
		if (toggle)
			FrameWork::Memory::WriteMemory<uint16_t>(net_object + 0x4E, word_004E |= 8);
		else
			FrameWork::Memory::WriteMemory<uint16_t>(net_object + 0x4E, word_004E &= ~8);
		return true;
	}
	bool network_request_control_of_network_id(uint64_t net_object, uint64_t local_net_object) {
		FrameWork::Memory::WriteMemory<uint8_t>(net_object + 0x49, get_owner_id(local_net_object));
		FrameWork::Memory::WriteMemory<uint8_t>(net_object + 0x4A, 0xFF);
		FrameWork::Memory::WriteMemory<uint8_t>(net_object + 0x4B, 0);
		return true;
	}
	void FivemSDK::network_request_control_of_entity(uint64_t entity, uint64_t localplayer) {
		std::cout << std::hex << entity << " " << localplayer << std::endl;
		uint64_t entity_net_object = FrameWork::Memory::ReadMemory<uint64_t>(entity + 0xD0);
		uint64_t local_net_object = FrameWork::Memory::ReadMemory<uint64_t>(localplayer + 0xD0);
		set_network_id_can_migrate(entity_net_object, true);
		set_network_id_exists_on_all_machines(entity_net_object, true);
		network_request_control_of_network_id(entity_net_object, local_net_object);
	}

	bool FivemSDK::UpdateEntities()
	{
		if (!pWorld)
		{
			pWorld = reinterpret_cast<CWorld*>(FrameWork::Memory::ReadMemory<uint64_t>(World));
			if (!pWorld)
				return false;
		}

		pLocalPlayer = pWorld->LocalPlayer();

		if (pReplayInterface && !pLocalPlayer)
		{
			g_Options.General.ShutDown = true;
			return false;
		}

		if (!pLocalPlayer)
			return false;

		static bool Logged = false;
		if (!Logged)
			Logged = true;

		if (!pReplayInterface)
		{
			pReplayInterface = reinterpret_cast<CReplayInterface*>(FrameWork::Memory::ReadMemory<uint64_t>(ReplayInterface));
			if (!pReplayInterface)
				return false;
		}

		if (!pPedInterface)
		{
			pPedInterface = pReplayInterface->PedInterface();
			if (!pPedInterface)
				return false;
		}

		if (!pCamGameplayDirector)
		{
			pCamGameplayDirector = reinterpret_cast<CCamGameplayDirector*>(FrameWork::Memory::ReadMemory<uint64_t>(Camera));
			if (!pCamGameplayDirector)
				return false;
		}

		std::scoped_lock lock(LockLists);

		EntityList.clear();
		EntityList.reserve(pPedInterface->PedMaximum()); // reserva mem�ria -> evita realoca��o em loop

		for (size_t i = 0; i < pPedInterface->PedMaximum(); i++)
		{
			CPed* Ped = pPedInterface->PedList()->Ped(i);
			if (!Ped)
				continue;

			if (reinterpret_cast<uint64_t>(Ped) > 0xCCCCCCCCCCCCC)
				continue;

			if (Ped->IsAnimal())
				continue;

			if (!g_Options.Visuals.ESP.Players.ExcludeDeads && Ped->GetHealth() <= 101.f)
				continue;

			PedStaticInfo StaticInfo{};
			StaticInfo.Ped = Ped;
			StaticInfo.iIndex = static_cast<int>(i);
			StaticInfo.bIsLocalPlayer = (Ped == pLocalPlayer);
			StaticInfo.bIsNPC = Ped->IsNPC();

			int netId = Ped->GetPlayerInfo()->GetPlayerID();
			StaticInfo.bIsFriend = (Cheat::g_Fivem.FriendList.find(netId) != Cheat::g_Fivem.FriendList.end());

			if (!StaticInfo.bIsNPC)
			{
				StaticInfo.NetId = netId;
				StaticInfo.Name = g_Fivem.GetPlayerName(reinterpret_cast<uint64_t>(Ped->GetPlayerInfo()) + 0x20, StaticInfo.NetId);
			}
			else
			{
				StaticInfo.NetId = -1;
				StaticInfo.Name = "NPC";
			}

			uint64_t fragInsNm = FrameWork::Memory::ReadMemory<uint64_t>(Ped + Offsets::FragInsNmGTA);
			if (fragInsNm)
			{
				uint64_t fragPtr = FrameWork::Memory::ReadMemory<uint64_t>(fragInsNm + 0x68);
				if (fragPtr)
					StaticInfo.crSkeletonData = FrameWork::Memory::ReadMemory<uint64_t>(fragPtr + 0x178);
			}

			Entity CurrentEntity{};
			CurrentEntity.StaticInfo = StaticInfo;

			if (CurrentEntity.StaticInfo.Name.empty() && !PlayersInfo.empty() && !PlayerIdToName.empty())
			{
				auto it = PlayerIdToName.find(CurrentEntity.StaticInfo.NetId);
				if (it != PlayerIdToName.end() && CurrentEntity.StaticInfo.NetId != -1)
					CurrentEntity.StaticInfo.Name = it->second;
				else
					CurrentEntity.StaticInfo.Name = XorStr("username");
			}

			CurrentEntity.Cordinates = Ped->GetCoordinate();
			CurrentEntity.Visible = Ped->IsVisible();

			if (CurrentEntity.Visible && !CurrentEntity.StaticInfo.bIsLocalPlayer
				&& g_Options.Visuals.ESP.Players.VisibleOnly)
			{
				auto* dir = GetCamGameplayDirector();
				if (!dir) continue;
				auto cam = dir->GetFollowPedCamera();
				if (cam)
				{
					Vector3D camOrigin = cam->GetCameraPosition();
					if (camOrigin.LengthSqr() > 0.1f && !HasLineOfSight(camOrigin, CurrentEntity.Cordinates))
						CurrentEntity.Visible = false;
				}
			}

			if (CurrentEntity.StaticInfo.bIsLocalPlayer)
			{
				LocalPlayerInfo.Ped = Ped;
				LanGame = StaticInfo.bIsNPC;
				LocalPlayerInfo.iIndex = static_cast<int>(i);
				LocalPlayerInfo.WorldPos = CurrentEntity.Cordinates;
				LocalPlayerInfo.ScreenPos = WorldToScreen(CurrentEntity.Cordinates);
			}

			EntityList.push_back(std::move(CurrentEntity));
		}

		return true;
	}

	bool FivemSDK::UpdateVehicles()
	{
		if (!pReplayInterface || !pCamGameplayDirector)
			return false;

		if (!pVehicleInterface)
		{
			pVehicleInterface = pReplayInterface->VehicleInterface();
			if (!pVehicleInterface) return false;
		}

		std::vector<VehicleInfo> newVehs;
		newVehs.reserve(128);

		auto localPos = LocalPlayerInfo.WorldPos;
		float maxDist = g_Options.Visuals.ESP.Vehicles.RenderDistance;

		for (size_t i = 0; i < pVehicleInterface->VehicleMaximum(); i++)
		{
			CVehicle* Vehicle = pVehicleInterface->VehicleList()->Vehicle(i);
			if (!Vehicle) continue;

			if (Vehicle->GetCoordinate().DistTo(localPos) > maxDist)
				continue;

			VehicleInfo v;
			v.Vehicle = Vehicle;
			v.iIndex = i;

			v.ModelInfo = Vehicle->GetModelInfo();
			if (v.ModelInfo)
				v.Name = FrameWork::Memory::ReadProcessMemoryString(v.ModelInfo + 0x298, 24);

			newVehs.push_back(std::move(v));
		}

		{
			std::scoped_lock lock(LockLists2);
			VehicleList.swap(newVehs);
		}

		return true;
	}



	Vector3D GetBonePosByInstFragAndID(uint64_t crSkeletonData, unsigned int BoneID)
	{
		Matrix4x4 v4 = FrameWork::Memory::ReadMemory<Matrix4x4>(FrameWork::Memory::ReadMemory<uint64_t>(crSkeletonData + 0x8));
		Matrix4x4 Result = FrameWork::Memory::ReadMemory<Matrix4x4>(FrameWork::Memory::ReadMemory<uint64_t>(crSkeletonData + 0x18) + (BoneID << 6));

		Vector3D vec1(v4._11, v4._12, v4._13);
		Vector3D vec2(v4._21, v4._22, v4._23);
		Vector3D vec3(v4._31, v4._32, v4._33);
		Vector3D vec4(v4._41, v4._42, v4._43);
		Vector3D vec5(Result._41, Result._42, Result._43);

		return Vector3D(
			vec1.x * vec5.x + vec4.x + vec2.x * vec5.y + vec3.x * vec5.z,
			vec1.y * vec5.x + vec4.y + vec2.y * vec5.y + vec3.y * vec5.z,
			vec1.z * vec5.x + vec4.z + vec2.z * vec5.y + vec3.z * vec5.z
		);
	}

	Vector3D FivemSDK::GetBonePosVec3(Entity& Ped, unsigned int Mask)
	{
		uint64_t FragInstNMGta = FrameWork::Memory::ReadMemory<uint64_t>(Ped.StaticInfo.Ped + Offsets::FragInsNmGTA);
		if (FragInstNMGta)
		{
			Ped.StaticInfo.crSkeletonData = FrameWork::Memory::ReadMemory<uint64_t>(FrameWork::Memory::ReadMemory<uint64_t>(FragInstNMGta + 0x68) + 0x178);

			auto it = Ped.StaticInfo.MaskToBoneId.find(Mask);
			if (it == Ped.StaticInfo.MaskToBoneId.end())
			{
				unsigned int BoneId = 0;
				if (GetPedBoneIndex(Ped, Mask, BoneId))
				{
					if (BoneId)
					{
						Ped.StaticInfo.MaskToBoneId[Mask] = BoneId;
						return GetBonePosByInstFragAndID(Ped.StaticInfo.crSkeletonData, BoneId);
					}
				}
			}
			else
			{
				return GetBonePosByInstFragAndID(Ped.StaticInfo.crSkeletonData, it->second);
			}
		}

		return Ped.Cordinates;
	}

	Vector3D FivemSDK::GetBonePosFromCPed(CPed* ped, int boneIndex)
	{
		if (!ped) return Vector3D(0, 0, 0);

		uintptr_t pedAddr = (uintptr_t)ped;
		Matrix4x4 modelMatrix = FrameWork::Memory::ReadMemory<Matrix4x4>(pedAddr + 0x60);

		uintptr_t boneAddr = pedAddr + Offsets::BoneManager;
		uintptr_t readAddr = (GameVersion >= 2802) ? boneAddr : boneAddr + boneIndex * 0x10;

		Vector3D localPos = FrameWork::Memory::ReadMemory<Vector3D>(readAddr);

		return Vector3D(
			modelMatrix._11 * localPos.x + modelMatrix._21 * localPos.y + modelMatrix._31 * localPos.z + modelMatrix._41,
			modelMatrix._12 * localPos.x + modelMatrix._22 * localPos.y + modelMatrix._32 * localPos.z + modelMatrix._42,
			modelMatrix._13 * localPos.x + modelMatrix._23 * localPos.y + modelMatrix._33 * localPos.z + modelMatrix._43
		);
	}

	CPed* FivemSDK::GetAimingEntity()
	{
		return (CPed*)FrameWork::Memory::ReadMemory<uint64_t>(PlayerAimingAt);
	}

	bool FivemSDK::IsPlayerAiming()
	{
		return FrameWork::Memory::ReadMemory<bool>(bIsPlayerAiming);
	}

	bool FivemSDK::GetPedBoneIndex(Entity& Ped, unsigned int Mask, unsigned int& newIdx)
	{
		uint64_t crSkeletonData = FrameWork::Memory::ReadMemory<uint64_t>(Ped.StaticInfo.crSkeletonData);

		if (FrameWork::Memory::ReadMemory<int16_t>(crSkeletonData + 0x1A))
		{
			uint16_t v1 = FrameWork::Memory::ReadMemory<uint16_t>(crSkeletonData + 0x18);
			if (v1)
			{
				int64_t v2 = FrameWork::Memory::ReadMemory<int64_t>(crSkeletonData + 0x10);
				int Count = 0;
				for (int64_t i = FrameWork::Memory::ReadMemory<int64_t>(v2 + 0x8 * (Mask % v1)); ; i = FrameWork::Memory::ReadMemory<int64_t>(i + 0x8))
				{
					Count++;
					if (!i || i >= 0xCCCCCCCCCCCCCC || Count > 3)
						return false;

					int v5 = FrameWork::Memory::ReadMemory<int>(i);
					if (Mask == v5)
					{
						int v6 = FrameWork::Memory::ReadMemory<int>(i + 0x4);
						newIdx = v6;
						return true;
					}
				}
			}
		}
		else if (Mask < FrameWork::Memory::ReadMemory<uint64_t>(crSkeletonData + 0x5E))
		{
			newIdx = Mask;
			return true;
		}

		return false;
	}

	ImVec2 FivemSDK::GetClosestHitBox(Entity Ped)
	{
		ImVec2 Result = ImVec2(0, 0);

		ImVec2 Head = Ped.HeadPos;
		if (!g_Fivem.IsOnScreen(Head))
			return Result;

		ImVec2 Neck = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Ped, SKEL_Neck_1));
		if (!g_Fivem.IsOnScreen(Neck))
			return Result;

		ImVec2 Chest = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Ped, SKEL_Spine3));
		if (!g_Fivem.IsOnScreen(Chest))
			return Result;

		ImVec2 Center = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);

		float HeadDistance = Head.DistTo(Center);
		float NeckDistance = Neck.DistTo(Center);
		float ChestDistance = Chest.DistTo(Center);

		float DistancesArray[] = { HeadDistance, NeckDistance, ChestDistance };

		float Closest = DistancesArray[0];
		int ClosestBone = 0;

		for (int i = 0; i < 5; ++i)
		{
			if (DistancesArray[i] < Closest)
			{
				Closest = DistancesArray[i];
				ClosestBone = i;
			}
		}

		switch (ClosestBone)
		{
		case 0:
			return Head;
			break;
		case 1:
			return Neck;
			break;
		case 2:
			return Chest;
			break;
		default:
			return Head;
			break;
		}

	}

	bool FivemSDK::FindClosestEntity(float Fov, int MaxDistance, bool NPC, bool ClosestFov, Entity* Output)
	{
		Entity Closest;
		float ClosestWorldDistance = FLT_MAX;
		float ClosestScreenDistance = FLT_MAX;

		std::vector<Entity> ClosestPeds;
		auto localPlayerInfo = GetLocalPlayerInfo();

		bool Found = false;

		Matrix4x4 currentViewMatrix = GetViewMatrix();

		LockLists.lock();
		for (Entity Current : EntityList)
		{
			if (Current.StaticInfo.bIsLocalPlayer)
				continue;

			if (Current.StaticInfo.bIsNPC && !NPC)
				continue;

			if (!g_Options.LegitBot.TargetDead && Current.StaticInfo.Ped->GetHealth() <= 101.f)
				continue;

			float WorldDistance = Current.Cordinates.DistTo(localPlayerInfo.WorldPos);

			if (WorldDistance > MaxDistance)
				continue;

			// Usamos a versão com matrix passada para ser thread-safe e precisa
			ImVec2 Head = WorldToScreen(GetBonePosVec3(Current, SKEL_Head), currentViewMatrix);
			if (Head.x == 0 && Head.y == 0)
				continue;

			ImVec2 Center = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
			float ScreenDistance = Head.DistTo(Center);

			if (ScreenDistance > Fov)
				continue;

			if (ScreenDistance < ClosestScreenDistance)
			{
				ClosestScreenDistance = ScreenDistance;
				Closest = Current;
			}
		}
		LockLists.unlock();

		if (Closest.StaticInfo.iIndex != -1)
		{
			*Output = Closest;
			Found = true;
		}

		return Found;
	}

	void FivemSDK::ProcessCameraMovement(Vector3D WorldPosition, int SmoothHorizontal, int SmoothVertical)
	{
		if (!pCamGameplayDirector)
			return;

		auto FollowPedCamera = pCamGameplayDirector->GetFollowPedCamera();
		uint64_t CamFollowVehicle = FrameWork::Memory::ReadMemory<uint64_t>(FollowPedCamera + 0x10);

		if (g_Fivem.GetLocalPlayerInfo().Ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle))
		{
			if (FrameWork::Memory::ReadMemory<float>(CamFollowVehicle + 0x2AC) == -2.f)
			{
				FrameWork::Memory::WriteMemory<float>(CamFollowVehicle + 0x2AC, 0.f);
				FrameWork::Memory::WriteMemory<float>(CamFollowVehicle + 0x2C0, 111.f);
				FrameWork::Memory::WriteMemory<float>(CamFollowVehicle + 0x2C4, 111.f);
			}
		}

		Vector3D CrosshairPosition = FollowPedCamera->GetCrosshairPosition();
		Vector3D ViewAngles = FollowPedCamera->GetViewAngles();

		float Distance = CrosshairPosition.DistTo(WorldPosition);

		Vector3D AimAngles = Vector3D((WorldPosition.x - CrosshairPosition.x) / Distance, (WorldPosition.y - CrosshairPosition.y) / Distance, (WorldPosition.z - CrosshairPosition.z) / Distance);

		Vector3D FinalAngles = AimAngles;

		Vector3D CameraDelta = Vector3D(AimAngles.x - ViewAngles.x, AimAngles.y - ViewAngles.y, AimAngles.z - ViewAngles.z);

		if (SmoothHorizontal > 1)
		{
			FinalAngles.x = ViewAngles.x + CameraDelta.x / (float)SmoothHorizontal;
			FinalAngles.y = ViewAngles.y + CameraDelta.y / (float)SmoothHorizontal;
		}

		if (SmoothVertical > 1)
			FinalAngles.z = ViewAngles.z + CameraDelta.z / (float)SmoothVertical;

		float AimbotFixZ = ViewAngles.z - FollowPedCamera->GetThirdpersonViewAngles().z;

		Vector3D ThirdPersonAngles = FinalAngles;
		ThirdPersonAngles.z = ThirdPersonAngles.z - AimbotFixZ;

		FollowPedCamera->SetThirdpersonViewAngles(ThirdPersonAngles);
		FollowPedCamera->SetViewAngles(FinalAngles);
	}

	void FivemSDK::TeleportToObject(uintptr_t Object, uintptr_t Navigation, uintptr_t ModelInfo, Vector3D Position, Vector3D VisualPosition, bool Stop)
	{
		float BackupMagic = 0.f;
		if (Stop)
		{
			BackupMagic = FrameWork::Memory::ReadMemory<float>(ModelInfo + 0x2C);
			FrameWork::Memory::WriteMemory(ModelInfo + 0x2C, 0.f);
		}

		FrameWork::Memory::WriteMemory(Object + 0x90, VisualPosition);
		FrameWork::Memory::WriteMemory(Navigation + 0x50, Position);

		if (Stop)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
			FrameWork::Memory::WriteMemory(ModelInfo + 0x2C, BackupMagic);
		}
	}

	void FivemSDK::SpectatePed(uint64_t Ped, bool Toggle)
	{
	}

	ImVec2 FivemSDK::WorldToScreen(Vector3D Pos, Matrix4x4 ViewMatrix)
	{
		ViewMatrix.TransposeThisMatrix();

		Vector4D VecX(ViewMatrix._21, ViewMatrix._22, ViewMatrix._23, ViewMatrix._24);
		Vector4D VecY(ViewMatrix._31, ViewMatrix._32, ViewMatrix._33, ViewMatrix._34);
		Vector4D VecZ(ViewMatrix._41, ViewMatrix._42, ViewMatrix._43, ViewMatrix._44);

		Vector3D ScreenPos;
		ScreenPos.x = (VecX.x * Pos.x) + (VecX.y * Pos.y) + (VecX.z * Pos.z) + VecX.w;
		ScreenPos.y = (VecY.x * Pos.x) + (VecY.y * Pos.y) + (VecY.z * Pos.z) + VecY.w;
		ScreenPos.z = (VecZ.x * Pos.x) + (VecZ.y * Pos.y) + (VecZ.z * Pos.z) + VecZ.w;

		if (ScreenPos.z <= 0.1f)
			return ImVec2(0, 0);

		ScreenPos.z = 1.0f / ScreenPos.z;
		ScreenPos.x *= ScreenPos.z;
		ScreenPos.y *= ScreenPos.z;

		ScreenPos.x += ImGui::GetIO().DisplaySize.x / 2 + float(0.5f * ScreenPos.x * ImGui::GetIO().DisplaySize.x + 0.5f);
		ScreenPos.y = ImGui::GetIO().DisplaySize.y / 2 - float(0.5f * ScreenPos.y * ImGui::GetIO().DisplaySize.y + 0.5f);

		return ImVec2(ScreenPos.x, ScreenPos.y);
	}

	ImVec2 FivemSDK::WorldToScreen(Vector3D Pos)
	{
		return WorldToScreen(Pos, CachedViewMatrix);
	}

	void FivemSDK::UpdateViewMatrix()
	{
		if (!pViewPort)
		{
			pViewPort = FrameWork::Memory::ReadMemory<uint64_t>(ViewPort);
			if (!pViewPort)
				return;
		}

		CachedViewMatrix = FrameWork::Memory::ReadMemory<Matrix4x4>(pViewPort + 0x24C);
	}

	Matrix4x4 FivemSDK::GetViewMatrix()
	{
		if (!pViewPort)
		{
			pViewPort = FrameWork::Memory::ReadMemory<uint64_t>(ViewPort);
			if (!pViewPort)
				return Matrix4x4();
		}

		return FrameWork::Memory::ReadMemory<Matrix4x4>(pViewPort + 0x24C);
	}


	uint32_t GetRelayIpAddress(uint64_t PeerAddress)
	{
		if (g_Fivem.GetGameVersion() >= 2944)
			return FrameWork::Memory::ReadMemory<uint32_t>(PeerAddress + 72);

		if (g_Fivem.GetGameVersion() >= 2372)
			return FrameWork::Memory::ReadMemory<uint32_t>(PeerAddress + 60);

		return FrameWork::Memory::ReadMemory<uint32_t>(PeerAddress + 20);
	}

	uint32_t GetPublicIpAddress(uint64_t PeerAddress)
	{
		if (g_Fivem.GetGameVersion() >= 2944)
			return FrameWork::Memory::ReadMemory<uint32_t>(PeerAddress + 168);

		if (g_Fivem.GetGameVersion() >= 2372)
			return FrameWork::Memory::ReadMemory<uint32_t>(PeerAddress + 76);

		return FrameWork::Memory::ReadMemory<uint32_t>(PeerAddress + 28);
	}

	uint32_t GetLocalIpAddress(uint64_t PeerAddress)
	{
		if (g_Fivem.GetGameVersion() >= 2944)
			return FrameWork::Memory::ReadMemory<uint32_t>(PeerAddress + 176);

		if (g_Fivem.GetGameVersion() >= 2372)
			return FrameWork::Memory::ReadMemory<uint32_t>(PeerAddress + 84);

		return FrameWork::Memory::ReadMemory<uint32_t>(PeerAddress + 36);
	}

	// ---- Sistema de nomes via API HTTP (fallback do Barry mods) ----
	namespace
	{
		static std::unordered_map<int, std::string> g_HttpNameMap;
		static std::mutex g_HttpNameMutex;
		static bool g_HttpThreadStarted = false;

		static std::string GetServerIpFromCrashometry()
		{
			char value[255] = {};
			DWORD BufferSize = sizeof(value);
			if (RegGetValueA(HKEY_CURRENT_USER,
				XorStr("Software\\CitizenFX\\FiveM"),
				XorStr("Last Run Location"),
				RRF_RT_REG_SZ, NULL, (PVOID)value, &BufferSize) != ERROR_SUCCESS)
				return "";

			std::string DirFiveM = value;
			std::string CrashoMetryDir = DirFiveM + XorStr("data\\cache\\crashometry");
			std::ifstream File(CrashoMetryDir, std::ios::binary);
			if (!File) return "";

			std::string line;
			while (std::getline(File, line))
			{
				size_t pos = line.rfind(XorStr("last_server_url"));
				if (pos == std::string::npos) continue;
				size_t start = line.find(XorStr("last_server"), pos + 15);
				if (start == std::string::npos) continue;
				size_t ip_start = start + 11;
				size_t colon = line.find(':', ip_start);
				if (colon == std::string::npos) continue;
				size_t ip_end = colon + 6;
				if (ip_end > line.size()) ip_end = line.size();
				return line.substr(ip_start, ip_end - ip_start);
			}
			return "";
		}

		static void FetchPlayerNamesHttp()
		{
			std::string ip = GetServerIpFromCrashometry();
			if (ip.empty()) return;

			std::wstring wHost;
			int port = 30120;
			size_t colonPos = ip.rfind(':');
			if (colonPos != std::string::npos)
			{
				try { port = std::stoi(ip.substr(colonPos + 1)); } catch (...) {}
				wHost = std::wstring(ip.begin(), ip.begin() + colonPos);
			}
			else
			{
				wHost = std::wstring(ip.begin(), ip.end());
			}

			std::string response = FrameWork::Misc::DownloadServerInfo(wHost, port);
			if (response.empty()) return;

			try
			{
				auto playersArr = nlohmann::json::parse(response);
				std::unordered_map<int, std::string> newMap;
				for (auto& player : playersArr)
				{
					if (!player.contains(XorStr("id")) || !player.contains(XorStr("name"))) continue;
					int id = player[XorStr("id")].get<int>();
					std::string name = player[XorStr("name")].get<std::string>();
					if (!name.empty()) newMap[id] = name;
				}
				std::lock_guard<std::mutex> lock(g_HttpNameMutex);
				g_HttpNameMap = std::move(newMap);
			}
			catch (...) {}
		}

		static void StartHttpNameThread()
		{
			if (g_HttpThreadStarted) return;
			g_HttpThreadStarted = true;
			std::thread([]() {
				while (true) {
					FetchPlayerNamesHttp();
					std::this_thread::sleep_for(std::chrono::milliseconds(6000));
				}
			}).detach();
		}
	}
	// ---- Fim sistema HTTP ----

	std::string FivemSDK::GetPlayerName(uint64_t PeerAddress, int GameNetId)
	{
		StartHttpNameThread();

		// Primeiro tenta o mapa HTTP (mais confiável)
		if (GameNetId >= 0)
		{
			std::lock_guard<std::mutex> lock(g_HttpNameMutex);
			auto httpIt = g_HttpNameMap.find(GameNetId);
			if (httpIt != g_HttpNameMap.end() && !httpIt->second.empty())
				return httpIt->second;
		}

		// Fallback: tenta leitura de memória
		std::string Result = XorStr("** Invalid **");

		if (PeerAddress == 0x20)
			return Result;

		uint64_t HashMask = FrameWork::Memory::ReadMemory<uint64_t>(g_Fivem.NetIdToNamesPtr + 0x28);
		uint64_t HashPosition = (uint64_t)GameNetId & HashMask;

		uint64_t TableBase = FrameWork::Memory::ReadMemory<uint64_t>(g_Fivem.NetIdToNamesPtr + 0x10);
		uint64_t IndexAddress = TableBase + 0x10 * HashPosition;

		uint64_t Index = FrameWork::Memory::ReadMemory<uint64_t>(IndexAddress + sizeof(uint64_t));
		if (!Index)
			return Result;

		uint64_t EndAddress = FrameWork::Memory::ReadMemory<uint64_t>(g_Fivem.NetIdToNamesPtr);
		if (Index == EndAddress)
			return Result;

		uint64_t InitialValue = FrameWork::Memory::ReadMemory<uint64_t>(Index);

		if (GameNetId != FrameWork::Memory::ReadMemory<DWORD>(Index + 0x10))
		{
			while (Index != InitialValue)
			{
				Index = FrameWork::Memory::ReadMemory<uint64_t>(Index + 8);
				if (GameNetId == FrameWork::Memory::ReadMemory<DWORD>(Index + 0x10))
					break;
			}
		}

		if (Index != EndAddress)
		{
			std::string memName = FrameWork::Memory::ReadProcessMemoryString(Index + 0x18, 32);
			if (!memName.empty() && memName != XorStr("** Invalid **"))
				Result = memName;
		}

		return Result;
	}

	bool FivemSDK::IsOnScreen(ImVec2 Pos)
	{
		if (Pos.x < 0.1f || Pos.y < 0.1 || Pos.x > ImGui::GetIO().DisplaySize.x || Pos.y > ImGui::GetIO().DisplaySize.y)
			return false;

		return true;
	}
}
