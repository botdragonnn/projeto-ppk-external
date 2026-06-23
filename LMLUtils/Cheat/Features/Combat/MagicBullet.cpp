// Magic Bullet: sempre escreve CWeapon+0x20; ApplySilent só se Silent Aim estiver OFF
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

    static void WriteCWeapon(const Vector3D& BonePos)
    {
        auto* ped = g_Fivem.GetLocalPlayerInfo().Ped;
        if (!ped) return;
        auto* wpnMgr = ped->GetWeaponManager();
        if (!wpnMgr || !Offsets::m_CObject || !Offsets::m_CWeapon) return;

        uint64_t cObj = FrameWork::Memory::ReadMemory<uint64_t>((uint64_t)wpnMgr + Offsets::m_CObject);
        if (!cObj || cObj > 0x7FFFFFFFFFFF) return;

        uint64_t cWeapon = FrameWork::Memory::ReadMemory<uint64_t>(cObj + Offsets::m_CWeapon);
        if (!cWeapon || cWeapon > 0x7FFFFFFFFFFF) return;

        FrameWork::Memory::WriteProcessMemoryImpl(cWeapon + 0x20, (LPVOID)&BonePos, sizeof(Vector3D));
    }

    static bool FindTarget(Vector3D& outBonePos)
    {
        Entity target;
        if (!g_Fivem.FindClosestEntity(
            (float)g_Options.LegitBot.SilentAim.Fov,
            g_Options.LegitBot.SilentAim.MaxDistance,
            g_Options.LegitBot.SilentAim.ShotNPC,
            g_Options.LegitBot.SilentAim.ClosestFov, &target))
            return false;

        if (!(uint64_t)target.StaticInfo.Ped ||
            (uint64_t)target.StaticInfo.Ped > 0x7FFFFFFFFFFF ||
            target.StaticInfo.bIsFriend)
            return false;

        outBonePos = g_Fivem.GetBonePosVec3(target, SKEL_Head);
        outBonePos.z += 0.04f;
        return true;
    }

    void MagicBullet::RunThread()
    {
        if (!g_Options.LegitBot.MagicBullet.Enabled || !g_Fivem.GetLocalPlayerInfo().Ped)
        {
            if (!g_Options.LegitBot.SilentAim.Enabled)
                RestoreSilent();
            return;
        }

        int key = g_Options.LegitBot.MagicBullet.KeyBind;
        if (key != 0 && !(SafeCall(GetAsyncKeyState)(key) & 0x8000))
        {
            if (!g_Options.LegitBot.SilentAim.Enabled)
                RestoreSilent();
            return;
        }

        Vector3D BonePos;
        if (!FindTarget(BonePos))
        {
            if (!g_Options.LegitBot.SilentAim.Enabled)
                RestoreSilent();
            return;
        }

        // Sempre escreve CWeapon + 0x20 (auto-aim)
        WriteCWeapon(BonePos);

        // ApplySilent só se Silent Aim estiver OFF (se ON, ele mesmo cuida)
        if (!g_Options.LegitBot.SilentAim.Enabled)
            ApplySilent(BonePos);
    }
}
