	#pragma once

	#include <cstdint>

	namespace Cheat
	{
		namespace Offsets
		{
			extern uint64_t Word;
			extern uint64_t m_DamageHandler;
			extern uint64_t FrameFlag;
			extern uint64_t m_InfiniteAmmo0;
			extern uint64_t m_InfiniteAmmo1;
			extern uint64_t EntityType;
			extern uint64_t Armor;
			extern uint64_t BoneManager;
			extern uint64_t PlayerInfo;
			extern uint64_t PlayerNetID;
			extern uint64_t WeaponManager;
			extern uint64_t FragInsNmGTA;
			extern uint64_t ConfigFlags;
			extern uint64_t ResetFlags;
			extern uint64_t MaxHealth;
			extern uint64_t LastVehicle;
			extern uint64_t VisibleFlag;
			extern uint64_t SpeedModifier;
			extern uint64_t VehicleDriver;
			extern uint64_t DoorLock;
			extern uint64_t SteeringAngle;
			extern uint64_t AcceleratorForce;
			extern uint64_t BrakeForce;
			extern uint64_t SeatBelt;
			extern uint64_t SeatBeltWindShield;
			extern uint64_t SetVehicleCustomPrimaryColour;
			extern uint64_t Handling;
			extern uint64_t VehicleState;
			extern uint64_t Stamina;
			extern uint64_t Clothes;
			extern uint64_t ArmsKinematics;
			extern uint64_t LegsKinematics;
			extern uint64_t VehicleBodyHealth;
			extern uint64_t VehicleTankHealth;
			extern uint64_t MagicPatch;
			inline extern uint64_t m_Handling = 0;
			inline extern uint64_t m_VehicleBodyHealth = 0;
			inline extern uint64_t m_VehicleEngineHealth = 0;
			inline extern uint64_t m_SeatBealt = 0;
			inline extern int PathSeatBealt = 0;
			inline extern uint64_t m_NoReload = 0;
			inline extern uint64_t m_WeaponAmmo = 0x60;
			inline extern uint64_t m_Spread = 0;
			inline extern uint64_t m_RaycastTable = 0;
		}
	}