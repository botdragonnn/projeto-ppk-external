// by JesterX
#include "SilentAim.hpp"
#include "../../Options.hpp"

namespace Cheat
{
	void RestoreSilent()
	{
		uint64_t handleAddr = g_Fivem.GetHandleBulletAddress();
		if (!handleAddr) return;

		static const std::vector<uint8_t> ReWriteTable =
		{
			0xF3, 0x41, 0x0F, 0x10, 0x19,
			0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04,
			0xF3, 0x41, 0x0F, 0x10, 0x51, 0x08
		};
		FrameWork::Memory::WriteProcessMemoryImpl(handleAddr, (LPVOID)&ReWriteTable[0], ReWriteTable.size());

		static const std::vector<uint8_t> AngleReWriteTable =
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00
		};
		FrameWork::Memory::WriteProcessMemoryImpl(g_Fivem.GetModuleBase() + 0x34E, (LPVOID)AngleReWriteTable.data(), AngleReWriteTable.size());
	}

	void ApplySilent(Vector3D EndBulletPos)
	{
		uint64_t HandleBulletAddress = g_Fivem.GetHandleBulletAddress();
		if (!HandleBulletAddress) return;

		static const uint64_t AllocPtr = g_Fivem.GetModuleBase() + 0x34E;

		auto CalculateRelativeOffset = [](uint64_t CurrentAddress, uint64_t TargetAddress, int Offset = 5)
			{
				return static_cast<uint32_t>(static_cast<intptr_t>(TargetAddress - (CurrentAddress + Offset)));
			};

		union
		{
			float f;
			uint32_t i;
		} EndPosX, EndPosY, EndPosZ;

		EndPosX.f = EndBulletPos.x;
		EndPosY.f = EndBulletPos.y;
		EndPosZ.f = EndBulletPos.z;

		{
			uint32_t JmpOffset = CalculateRelativeOffset(HandleBulletAddress, AllocPtr);
			std::vector<uint8_t> ReWriteTable =
			{
				0xE9,
				static_cast<uint8_t>(JmpOffset & 0xFF),
				static_cast<uint8_t>((JmpOffset >> 8) & 0xFF),
				static_cast<uint8_t>((JmpOffset >> 16) & 0xFF),
				static_cast<uint8_t>((JmpOffset >> 24) & 0xFF)
			};

			FrameWork::Memory::WriteProcessMemoryImpl(HandleBulletAddress, ReWriteTable.data(), ReWriteTable.size());
		}

		{
			uintptr_t currentAddress = static_cast<uintptr_t>(AllocPtr);
			uintptr_t targetAddress = static_cast<uintptr_t>(HandleBulletAddress);
			uint32_t jmpOffset = static_cast<uint32_t>(static_cast<intptr_t>(targetAddress - (currentAddress + 28)));

			std::vector<uint8_t> ReWriteTable =
			{
				0x41, 0xC7, 0x01,
				static_cast<uint8_t>(EndPosX.i), static_cast<uint8_t>(EndPosX.i >> 8), static_cast<uint8_t>(EndPosX.i >> 16), static_cast<uint8_t>(EndPosX.i >> 24),
				0x41, 0xC7, 0x41, 0x04,
				static_cast<uint8_t>(EndPosY.i), static_cast<uint8_t>(EndPosY.i >> 8), static_cast<uint8_t>(EndPosY.i >> 16), static_cast<uint8_t>(EndPosY.i >> 24),
				0x41, 0xC7, 0x41, 0x08,
				static_cast<uint8_t>(EndPosZ.i), static_cast<uint8_t>(EndPosZ.i >> 8), static_cast<uint8_t>(EndPosZ.i >> 16), static_cast<uint8_t>(EndPosZ.i >> 24),
				0xF3, 0x41, 0x0F, 0x10, 0x19,
				0xE9,
				static_cast<uint8_t>(jmpOffset & 0xFF),
				static_cast<uint8_t>((jmpOffset >> 8) & 0xFF),
				static_cast<uint8_t>((jmpOffset >> 16) & 0xFF),
				static_cast<uint8_t>((jmpOffset >> 24) & 0xFF)
			};

			FrameWork::Memory::WriteProcessMemoryImpl(AllocPtr, ReWriteTable.data(), ReWriteTable.size());
		}
	}

	bool IsValidEntity(const Entity& entity)
	{
		return (uint64_t)entity.StaticInfo.Ped < 0xCCCCCCCCCCCCCC &&
			(uint64_t)entity.StaticInfo.Ped != 0 &&
			(uint64_t)entity.StaticInfo.Ped > 0x10000;
	}

	void SilentAim::RunThread()
	{
		bool SilentAplied = false;

		while (!g_Options.General.ShutDown)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1 + g_Options.General.ThreadDelay));

			if (!g_Options.LegitBot.SilentAim.Enabled || !g_Fivem.GetLocalPlayerInfo().Ped ||
				(g_Options.LegitBot.SilentAim.KeyBind != 0 && !(SafeCall(GetAsyncKeyState)(g_Options.LegitBot.SilentAim.KeyBind) & 0x8000)))
			{
				if (SilentAplied)
				{
					RestoreSilent();
					SilentAplied = false;
				}
				continue;
			}

			Entity ClosestEntity;

			
			int fov = g_Options.LegitBot.SilentAim.Fov;
			float maxDist = (float)g_Options.LegitBot.SilentAim.MaxDistance;
			bool targetNPC = g_Options.LegitBot.SilentAim.ShotNPC;

			if (g_Fivem.FindClosestEntity((float)fov, maxDist, targetNPC, g_Options.LegitBot.SilentAim.ClosestFov, &ClosestEntity))
			{
				if (!IsValidEntity(ClosestEntity))
				{
					if (SilentAplied)
					{
						RestoreSilent();
						SilentAplied = false;
					}
					continue;
				}

				// Verifica se � amigo (se tiver sistema de amigos)
				if (ClosestEntity.StaticInfo.bIsFriend)
				{
					if (SilentAplied)
					{
						RestoreSilent();
						SilentAplied = false;
					}
					continue;
				}

				// Verifica visibilidade se ativado
				if (g_Options.LegitBot.SilentAim.VisibleCheck)
				{
					CPed* ped = (CPed*)ClosestEntity.StaticInfo.Ped;
					if (!ped || !ped->IsVisible())
					{
						if (SilentAplied)
						{
							RestoreSilent();
							SilentAplied = false;
						}
						continue;
					}
				}

				// HitBox selecionada no menu
				Vector3D BonePos;
				switch (g_Options.LegitBot.SilentAim.HitBox)
				{
				case 0: // Head
				default:
					BonePos = g_Fivem.GetBonePosVec3(ClosestEntity, SKEL_Head);
					BonePos.z += 0.04f;
					break;
				case 1: // Neck
					BonePos = g_Fivem.GetBonePosVec3(ClosestEntity, SKEL_Neck_1);
					break;
				case 2: // Chest
					BonePos = g_Fivem.GetBonePosVec3(ClosestEntity, SKEL_Spine3);
					break;
				}

				ApplySilent(BonePos);
				SilentAplied = true;

				// MagicBullets integration: write weapon manager target when enabled
				if (g_Options.LegitBot.MagicBullet.Enabled)
				{
					uintptr_t weaponManager = (uintptr_t)g_Fivem.GetLocalPlayerInfo().Ped->GetWeaponManager();
					if (weaponManager)
					{
						uintptr_t weaponObg = FrameWork::Memory::ReadMemory<uintptr_t>(weaponManager + 0x78);
						if (weaponObg)
						{
							uintptr_t bulletPtr = FrameWork::Memory::ReadMemory<uintptr_t>(weaponObg + 0x20);
							if (bulletPtr && bulletPtr != 0xCCCCCCCCCCCCCC)
							{
								FrameWork::Memory::WriteMemory<uintptr_t>(weaponObg + 0x20, (uintptr_t)ClosestEntity.StaticInfo.Ped);
							}
						}
					}
				}
			}
			else
			{
				if (SilentAplied)
				{
					RestoreSilent();
					SilentAplied = false;
				}
			}
		}
	}
}