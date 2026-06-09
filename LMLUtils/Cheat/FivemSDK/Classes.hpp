#pragma once

#include "Offsets.hpp"
#include "GTADefines.hpp"

#include <FrameWork/FrameWork.hpp>
#include <thread>
#include <chrono>
#include <random>
#include <unordered_map>


namespace Cheat
{
	constexpr uintptr_t INVINCIBILITY_OFFSET = 0x188;
	constexpr uint8_t INVINCIBILITY_BIT = 8;

	class CVehicle
	{
	public:

		void ExplodeCar(bool Toggle)
		{
			uint64_t GetModelInfo = FrameWork::Memory::ReadMemory<uint64_t>(this + 0x20);
			FrameWork::Memory::WriteMemory<uint32_t>(GetModelInfo + Offsets::FrameFlag, 1 << 13);
		}

		uint64_t GetModelInfo()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<uint64_t>(this + 0x20);
		}


		uint64_t GetColor()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<uint64_t>(this + 0x48);
		}

		void SetBodyHealth(uint64_t c_vehicle, float value)
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory<float>(this + Offsets::VehicleBodyHealth, value);
		}

		void SetTankHealth(uint64_t c_vehicle, float value)
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory<float>(this + Offsets::VehicleTankHealth, value);
		}


		uint64_t GetNavigation()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<uint64_t>(this + 0x30);
		}

		Vector3D GetCoordinate()
		{
			if (!this)
				return Vector3D{ 0,0,0 };

			return FrameWork::Memory::ReadMemory<Vector3D>(this + 0x90);
		}

		void SetInvencible(bool Toggle)
		{
			if (!this)
				return;

			uint64_t OldFlag = FrameWork::Memory::ReadMemory<uint64_t>(this + 0x188);
			FrameWork::Memory::WriteMemory(this + 0x188, Toggle ? (OldFlag | (1 << 8)) : (OldFlag & ~(1 << 8)));
		}

		uint64_t GetDriver()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<uint64_t>(this + Offsets::VehicleDriver);
		}

		void StartVehicleEngine()
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory(this + Offsets::VehicleState, (BYTE)0x17);
		}

		void SetGravity(float Value)
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory<float>((uintptr_t)this + 0x0C3C, Value);
		}
		void StopVehicleEngine()
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory(this + Offsets::VehicleState, (BYTE)0x01);
		}

		void SetBrakeForce(float BrakeForce)
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory(this + Offsets::BrakeForce, (float)BrakeForce);
		}

		void SetAcceleration(float Acceleration)
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory(this + Offsets::AcceleratorForce, (float)Acceleration);
		}

		void SetSteeringWhellAngle(float Angle)
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory(this + Offsets::SteeringAngle, (float)Angle);
		}

		void SetDriver(uint64_t NewDriver)
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory(this + Offsets::VehicleDriver, NewDriver);
		}

		int GetLockState()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<unsigned int>(this + Offsets::DoorLock);
		}

		void FixVehicle()
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory(this + Offsets::VehicleState, (BYTE)0x17);
		}

		uint64_t GetHandling()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<uint64_t>(this + Offsets::m_Handling);
		}

		void SetLockState(eCarLockState NewState)
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory(this + Offsets::DoorLock, (unsigned int)NewState);
		}
	};

	class CPlayerInfo
	{
	public:
		int GetPlayerID()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<int>(this + Offsets::PlayerNetID);
		}

		float GetSpeed()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<float>(this + Offsets::SpeedModifier);
		}

		void SetSpeed(float NewSpeed)
		{
			if (!this)
				return;

			if (Offsets::SpeedModifier == 0)
				return;

			FrameWork::Memory::WriteMemory(this + Offsets::SpeedModifier, NewSpeed);
		}
	};

	class CWeaponInfo
	{
	public:
		std::string GetWeaponName()
		{
			if (!this)
				return NULL;

			return FrameWork::Memory::ReadProcessMemoryString(FrameWork::Memory::ReadMemory<uint64_t>(this + 0x05F0));
		}
	};

	class CWeaponObject
	{
	public:
		uintptr_t GetWeapon()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<uintptr_t>((uintptr_t)this + 0x320);
		}
	};

	class CWeaponManager
	{
	public:
		CWeaponInfo* GetWeaponInfo()
		{
			if (!this)
				return 0;

			return (CWeaponInfo*)FrameWork::Memory::ReadMemory<uint64_t>(this + 0x20);
		}

		CWeaponObject* GetWeaponObject()
		{
			if (!this)
				return 0;

			return (CWeaponObject*)FrameWork::Memory::ReadMemory<uint64_t>(this + 0x78);
		}

		uint64_t GetCPedInventory()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<uint64_t>(this - 0x8);
		}
	};

	class CPed
	{
	public:


		void invisible_on(bool toggle)
		{
			if (!this)
				return;

			if (toggle)
			{
				FrameWork::Memory::WriteMemory<BYTE>(this + Offsets::VisibleFlag, 0x0);
			}
		}

		void invisible_off(bool toggle)
		{
			if (!this)
				return;

			if (toggle)
			{
				FrameWork::Memory::WriteMemory<BYTE>(this + Offsets::VisibleFlag, 0x37);
			}
		}

		void SetNetObjectInvisible(bool toggle)
		{
			if (!this)
				return;

			// Write to the network object invisible flag (offset 0x40 from net object pointer at 0xD0)
			uint64_t netObject = FrameWork::Memory::ReadMemory<uint64_t>((uintptr_t)this + 0xD0);
			if (!netObject)
				return;

			FrameWork::Memory::WriteMemory<BYTE>(netObject + 0x40, toggle ? 1 : 0);
		}


		void SeatBealt(bool toggle) {
			if (!this) { return; }

			if (toggle) {
				FrameWork::Memory::WriteBytes(Offsets::PathSeatBealt, { 0x90,0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, });
				FrameWork::Memory::WriteMemory<BYTE>((uintptr_t)this + Offsets::m_SeatBealt, 0xC9);

			}
			else {
				FrameWork::Memory::WriteMemory<BYTE>((uintptr_t)this + Offsets::m_SeatBealt, 0xC8);
			}


		}
		CPlayerInfo* GetPlayerInfo()
		{
			if (!this)
				return 0;

			return (CPlayerInfo*)FrameWork::Memory::ReadMemory<uint64_t>(this + Offsets::PlayerInfo);
		}

		void GodMode(bool enable) {
			if (!this) { return; }

			auto playerInfo = (uintptr_t)GetPlayerInfo();
			FrameWork::Memory::WriteMemory(playerInfo + Offsets::m_DamageHandler, enable ? 0.f : 1.f);
			FrameWork::Memory::WriteMemory(playerInfo + Offsets::m_DamageHandler + 0x10, enable ? 0.f : 1.f);
		}

		CVehicle* GetLastVehicle()
		{
			if (!this)
				return 0;

			return (CVehicle*)FrameWork::Memory::ReadMemory<uint64_t>(this + Offsets::LastVehicle);
		}

		CWeaponManager* GetWeaponManager()
		{
			if (!this)
				return 0;

			return (CWeaponManager*)FrameWork::Memory::ReadMemory<uint64_t>(this + Offsets::WeaponManager);
		}

		uint64_t GetPedInventory()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<uint64_t>(this + Offsets::WeaponManager - 0x8);
		}

		uint64_t GetModelInfo()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<uint64_t>(this + 0x20);
		}

		uint64_t GetNavigation()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<uint64_t>(this + 0x30);
		}

		Vector3D GetCoordinate()
		{
			if (!this)
				return Vector3D{ 0,0,0 };

			return FrameWork::Memory::ReadMemory<Vector3D>(this + 0x90);
		}


		float GetHealth()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<float>(this + 0x280);
		}

		void boost(uintptr_t handling, float boostValue)
		{

			FrameWork::Memory::WriteMemory<float>(handling + 0x4C, boostValue);
		}

		void SetHealth(float NewHealth)
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory(this + 0x280, NewHealth);
		}

		void RemoveKinematics()
		{
			if (this)
				return;

			FrameWork::Memory::PatchFunc(Offsets::ArmsKinematics, 5);
			FrameWork::Memory::PatchFunc(Offsets::LegsKinematics, 5);
		}

		float GetMaxHealth()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<float>(this + Offsets::MaxHealth);
		}

		float GetArmor()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<float>(this + Offsets::Armor);
		}

		void SetArmor(float NewArmor)
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory(this + Offsets::Armor, NewArmor);
		}
		void EnableHealthRegeneration()
		{
			if (!this)
				return;

			float currentHealth = GetHealth();

			if (currentHealth < 101.0f) {
				SetHealth(101.0f);
				return;
			}

			if (currentHealth < 150.0f) {
				float newHealth = currentHealth + 2.0f;

				if (newHealth > 150.0f)
					newHealth = 150.0f;

				SetHealth(newHealth);
			}
		}

		uint32_t GetPedType()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<uint32_t>(this + Offsets::EntityType);
		}

		bool IsVisible()
		{
			if (!this) return false;

			uint64_t pedAddr = (uint64_t)this;

			// Ativa flag de tracking de visibilidade (bit 29 em ped + 0xC0)
			uint32_t entity_flags = FrameWork::Memory::ReadMemory<uint32_t>(pedAddr + 0xC0);
			if ((entity_flags & (1u << 29)) == 0u)
			{
				FrameWork::Memory::WriteMemory<uint32_t>(pedAddr + 0xC0, entity_flags | (1u << 29));
			}

			// Lê o ponteiro fx_draw_data (usado pelo sistema de oclusão do jogo)
			uint64_t fx_draw_data = FrameWork::Memory::ReadMemory<uint64_t>(pedAddr + 0x48);
			if (!fx_draw_data)
			{
				return HasConfigFlag(CPED_CONFIG_FLAG_VisibleOnScreen);
			}

			uint32_t query_id = static_cast<uint32_t>(FrameWork::Memory::ReadMemory<uint8_t>(fx_draw_data + 0x2B)) & 0xFF;

			if (query_id > 0 && query_id <= 1000)
			{
				uintptr_t raycast_table = Offsets::m_RaycastTable;
				if (!raycast_table) return HasConfigFlag(CPED_CONFIG_FLAG_VisibleOnScreen);

				int visibility_value = FrameWork::Memory::ReadMemory<int>(raycast_table + ((query_id - 1) * 0x80) + 0x78);
				return visibility_value > 100;
			}
			else
			{
				uint64_t last_time_seen = FrameWork::Memory::ReadMemory<uint64_t>(fx_draw_data + 0x18);
				static std::unordered_map<uint64_t, uint64_t> lastTimeSeenMap;
				auto it = lastTimeSeenMap.find(pedAddr);
				if (it == lastTimeSeenMap.end() || last_time_seen > it->second)
				{
					lastTimeSeenMap[pedAddr] = last_time_seen;
					return true;
				}
				lastTimeSeenMap[pedAddr] = last_time_seen;
				if (lastTimeSeenMap.size() > 4096) lastTimeSeenMap.clear();
				return false;
			}
		}



		void RandomDelay() {

			int delay = (GetTickCount() % 20) + 1;
			Sleep(delay);
		}


		uint64_t ReadWithVariation(uintptr_t address) {
			RandomDelay();


			if (GetTickCount() % 3 == 0) {

				return FrameWork::Memory::ReadMemory<uint64_t>(address);
			}
			else {

				uint32_t part1 = FrameWork::Memory::ReadMemory<uint32_t>(address);
				uint32_t part2 = FrameWork::Memory::ReadMemory<uint32_t>(address + 4);
				return (static_cast<uint64_t>(part2) << 32) | part1;
			}
		}


		void WriteWithVariation(uintptr_t address, uint64_t value) {
			RandomDelay();

			int method = GetTickCount() % 4;

			switch (method) {
			case 0:

				FrameWork::Memory::WriteMemory(address, value);
				break;
			case 1:

				FrameWork::Memory::WriteMemory<uint32_t>(address, value & 0xFFFFFFFF);
				FrameWork::Memory::WriteMemory<uint32_t>(address + 4, (value >> 32) & 0xFFFFFFFF);
				break;
			case 2:

				for (int i = 0; i < 8; i++) {
					uint8_t byte = (value >> (i * 8)) & 0xFF;
					FrameWork::Memory::WriteMemory<uint8_t>(address + i, byte);
					if (i % 2 == 0) Sleep(1);
				}
				break;
			case 3:

				uint64_t tempValue = value ^ 0x1234567890ABCDEF;
				FrameWork::Memory::WriteMemory(address, tempValue);
				FrameWork::Memory::WriteMemory(address, value);
				break;
			}
		}


		bool CheckCurrentState(uintptr_t entity, bool desiredState) {
			uint64_t flags = ReadWithVariation(entity + INVINCIBILITY_OFFSET);
			bool currentState = (flags & (1ULL << INVINCIBILITY_BIT)) != 0;
			return currentState == desiredState;
		}

		bool SetInvencible(uintptr_t entity, bool enable) {
			if (!entity) return false;


			if (CheckCurrentState(entity, enable)) {
				return true;
			}

			try {
				uint64_t currentFlags = ReadWithVariation(entity + INVINCIBILITY_OFFSET);
				uint64_t newFlags;

				if (enable) {
					newFlags = currentFlags | (1ULL << INVINCIBILITY_BIT);
				}
				else {
					newFlags = currentFlags & ~(1ULL << INVINCIBILITY_BIT);
				}


				if (currentFlags != newFlags) {
					WriteWithVariation(entity + INVINCIBILITY_OFFSET, newFlags);


					Sleep(15);
					return CheckCurrentState(entity, enable);
				}

				return true;

			}
			catch (...) {
				return false;
			}
		}



		bool IsNPC()
		{
			uint32_t PedType = GetPedType();

			if (!PedType)
				return false;

			PedType = PedType << 11 >> 25;

			if (PedType != 2)
				return true;

			return false;
		}

		bool IsAnimal()
		{
			uint32_t PedType = GetPedType();

			if (!PedType)
				return false;

			PedType = PedType << 11 >> 25;

			if (PedType > 27)
				return true;

			return false;
		}

		void SetConfigFlag(ePedConfigFlags Flag, bool Value)
		{
			int v1 = (int)Flag;
			if (!this || v1 >= 0x1CA)
				return;

			auto v2 = 1 << (v1 & 0x1F);
			auto v3 = v1 >> 5;
			auto v4 = (uint64_t)this + 4 * v3 + Offsets::ConfigFlags;
			auto v5 = FrameWork::Memory::ReadMemory<long>(v4);

			if (Value != ((v2 & v5) != 0))
			{
				auto v6 = v2 & (v5 ^ -(uint8_t)(Value ? 1 : 0));
				v5 ^= v6;

				FrameWork::Memory::WriteMemory(v4, v5);
			}
		}

		bool HasConfigFlag(ePedConfigFlags Flag)
		{
			int v1 = (int)Flag;
			if (!this || v1 >= 0x1CA)
				return false;

			auto v2 = 1 << (v1 & 0x1F);
			auto v3 = v1 >> 5;
			auto v4 = (uint64_t)this + 4 * v3 + Offsets::ConfigFlags;
			auto v5 = FrameWork::Memory::ReadMemory<long>(v4);

			return (v2 & v5) != 0;
		}

	}; // end class CPed

	class CPedList
	{
	public:
		CPed* Ped(int Index)
		{
			if (!this)
				return 0;

			return (CPed*)FrameWork::Memory::ReadMemory<uint64_t>(this + (Index * 0x10));
		}
	};

	class CPedInterface
	{
	public:
		CPedList* PedList()
		{
			if (!this)
				return 0;

			return (CPedList*)FrameWork::Memory::ReadMemory<uint64_t>(this + 0x100);
		}

		uint64_t PedMaximum()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<uint64_t>(this + 0x108);
		}
	};

	class CVehicleList
	{
	public:
		CVehicle* Vehicle(int Index)
		{
			if (!this)
				return 0;

			return (CVehicle*)FrameWork::Memory::ReadMemory<uint64_t>(this + (Index * 0x10));
		}
	};

	class CVehicleInterface
	{
	public:
		CVehicleList* VehicleList()
		{
			if (!this)
				return 0;

			return (CVehicleList*)FrameWork::Memory::ReadMemory<uint64_t>(this + 0x180);
		}

		uint64_t VehicleMaximum()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<uint64_t>(this + 0x188);
		}

		uint64_t VehiclesAtList()
		{
			if (!this)
				return 0;

			return FrameWork::Memory::ReadMemory<uint64_t>(this + 0x190);
		}
	};

	class CReplayInterface
	{
	public:
		CVehicleInterface* VehicleInterface()
		{
			if (!this)
				return 0;

			return (CVehicleInterface*)FrameWork::Memory::ReadMemory<uint64_t>(this + 0x10);
		}

		CPedInterface* PedInterface()
		{
			if (!this)
				return 0;

			return (CPedInterface*)FrameWork::Memory::ReadMemory<uint64_t>(this + 0x18);
		}
	};

	class CCamFollowPedCamera
	{
	public:
		Vector3D GetViewAngles()
		{
			if (!this)
				return Vector3D(0, 0, 0);

			return FrameWork::Memory::ReadMemory<Vector3D>(this + 0x40);
		}

		void SetViewAngles(Vector3D Angles)
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory(this + 0x40, Angles);
		}

		Vector3D GetCrosshairPosition()
		{
			if (!this)
				return Vector3D(0, 0, 0);

			return FrameWork::Memory::ReadMemory<Vector3D>(this + 0x60);
		}

		void SetCrosshairPosition(Vector3D Position)
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory(this + 0x60, Position);
		}

		Vector3D GetThirdpersonViewAngles()
		{
			if (!this)
				return Vector3D(0, 0, 0);

			return FrameWork::Memory::ReadMemory<Vector3D>(this + 0x3d0);
		}

		void SetThirdpersonViewAngles(Vector3D ViewAngles)
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory(this + 0x3d0, ViewAngles);
		}

		Vector3D GetCameraPosition()
		{
			if (!this)
				return Vector3D(0, 0, 0);

			return FrameWork::Memory::ReadMemory<Vector3D>(this + 0x3F0);
		}

		void SetCameraPosition(Vector3D NewPosition)
		{
			if (!this)
				return;

			FrameWork::Memory::WriteMemory(this + 0x3F0, NewPosition);
		}
	};

	class CCamGameplayDirector
	{
	public:
		CCamFollowPedCamera* GetFollowPedCamera()
		{
			if (!this)
				return 0;

			return (CCamFollowPedCamera*)FrameWork::Memory::ReadMemory<uint64_t>(this + 0x3C0);
		}
	};

	class CWorld
	{
	public:
		CPed* LocalPlayer()
		{
			if (!this)
				return 0;

			return (CPed*)FrameWork::Memory::ReadMemory<uint64_t>(this + 0x8);
		}
	};
}