// by HyperX
#include "MagicBullet.hpp"
#include "../../Options.hpp"

namespace Cheat {
    void MagicBullet::RunThread()
    {
        if (!g_Options.LegitBot.MagicBullet.Enabled) {
            return;
        }

        Entity closestEntity;

        // Use maximum FOV and distance for "strongest" effect
        if (!g_Fivem.FindClosestEntity(
            180.0f, // 180 FOV (everything in front/around)
            1000.0f, // 1000m range
            true,   // Hit NPCs too
            true,   // Closest by FOV
            &closestEntity)) {
            return;
        }

        // Target head for maximum damage
        auto bonePosition = g_Fivem.GetBonePosVec3(closestEntity, SKEL_Head);
        bonePosition.z += 0.04f;

        uintptr_t weaponManager = (uintptr_t)g_Fivem.GetLocalPlayerInfo().Ped->GetWeaponManager();
        if (!weaponManager) return;

        uintptr_t weaponObg = FrameWork::Memory::ReadMemory<uintptr_t>(weaponManager + 0x78);
        if (!weaponObg) return;

        uintptr_t CWeapon = FrameWork::Memory::ReadMemory<uintptr_t>(weaponObg + 0x320);
        if (!CWeapon) return;

        // Magic Bullet: Force all projectiles to spawn directly on the target's head
        // This effectively "wallbangs" because the bullet never has to travel through walls
        FrameWork::Memory::WriteMemory<Vector3D>(CWeapon + 0x20, bonePosition);
    }
}