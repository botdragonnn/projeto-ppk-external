// Magic Bullet — standalone quando Silent Aim OFF, delegado ao Silent Aim quando ON
#include "MagicBullet.hpp"
#include "../../Options.hpp"
#include "../../FivemSDK/Fivem.hpp"
#include <limits>

namespace Cheat {
    extern void ApplySilent(Vector3D EndBulletPos);
    extern void RestoreSilent();

    void MagicBullet::Restore()
    {
        if (!g_Options.LegitBot.SilentAim.Enabled)
            RestoreSilent();
    }

    void MagicBullet::RunThread()
    {
        if (!g_Options.LegitBot.MagicBullet.Enabled || !g_Fivem.GetLocalPlayerInfo().Ped)
        {
            if (!g_Options.LegitBot.SilentAim.Enabled)
                RestoreSilent();
            return;
        }

        // Silent Aim ON → ele ja escreve CWeapon+0x20 com o mesmo BonePos
        if (g_Options.LegitBot.SilentAim.Enabled)
            return;

        // Silent Aim OFF → modo standalone
        int key = g_Options.LegitBot.MagicBullet.KeyBind;
        if (key != 0 && !(SafeCall(GetAsyncKeyState)(key) & 0x8000))
        {
            RestoreSilent();
            return;
        }

        Entity target;
        if (!g_Fivem.FindClosestEntity(
            (float)g_Options.LegitBot.SilentAim.Fov,
            g_Options.LegitBot.SilentAim.MaxDistance,
            g_Options.LegitBot.SilentAim.ShotNPC,
            g_Options.LegitBot.SilentAim.ClosestFov, &target))
        {
            RestoreSilent();
            return;
        }

        if (!(uint64_t)target.StaticInfo.Ped ||
            (uint64_t)target.StaticInfo.Ped > 0x7FFFFFFFFFFF ||
            target.StaticInfo.bIsFriend)
        {
            RestoreSilent();
            return;
        }

        Vector3D BonePos = g_Fivem.GetBonePosVec3(target, SKEL_Head);
        BonePos.z += 0.04f;

        ApplySilent(BonePos);

        auto* ped = g_Fivem.GetLocalPlayerInfo().Ped;
        if (!ped) return;
        auto* wpnMgr = ped->GetWeaponManager();
        if (!wpnMgr || !Offsets::m_CObject || !Offsets::m_CWeapon) return;

        uint64_t cObj = FrameWork::Memory::ReadMemory<uint64_t>((uint64_t)wpnMgr + Offsets::m_CObject);
        if (!cObj || cObj > 0x7FFFFFFFFFFF) return;

        uint64_t cWeapon = FrameWork::Memory::ReadMemory<uint64_t>(cObj + Offsets::m_CWeapon);
        if (!cWeapon || cWeapon > 0x7FFFFFFFFFFF) return;

        FrameWork::Memory::WriteProcessMemoryImpl(cWeapon + 0x20, &BonePos, sizeof(Vector3D));
    }
}
