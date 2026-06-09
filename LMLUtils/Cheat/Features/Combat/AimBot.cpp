// by jesterX
#include "AimBot.hpp"
#include "../../Options.hpp"
#include <FrameWork/Utilities/Memory.hpp>

namespace Cheat
{

    void AimBot::RunThread()
    {
        auto lastTime = std::chrono::steady_clock::now();

        while (!g_Options.General.ShutDown)
        {
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<float> elapsed = now - lastTime;

            if (elapsed.count() < 0.01f + g_Options.General.ThreadDelay / 1000.0f)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            lastTime = now;

            if (!g_Options.LegitBot.AimBot.Enabled)
                continue;

            if (!g_Fivem.GetLocalPlayerInfo().Ped)
                continue;

            // Resetar alvo se nenhum v�lido
            AimbotTargetPed = 0;

            Entity ClosestEntity;

            if (g_Fivem.FindClosestEntity(
                g_Options.LegitBot.AimBot.FOV,
                g_Options.LegitBot.AimBot.MaxDistance,
                g_Options.LegitBot.AimBot.TargetNPC,
                g_Options.LegitBot.AimBot.ClosestFov,
                &ClosestEntity))
            {
                if ((uint64_t)ClosestEntity.StaticInfo.Ped >= 0xCCCCCCCCCCCCCC ||
                    ClosestEntity.StaticInfo.bIsFriend)
                    continue;

                if (g_Options.LegitBot.AimBot.VisibleCheck)
                {
                    CPed* ped = (CPed*)ClosestEntity.StaticInfo.Ped;
                    if (!ped || !ped->IsVisible())
                        continue;
                }

                if (!SafeCall(GetAsyncKeyState)(g_Options.LegitBot.AimBot.KeyBind))
                    continue;

                // Alvo validado: define como Ped na mira
                AimbotTargetPed = (DWORD_PTR)ClosestEntity.StaticInfo.Ped;

                Vector3D BonePos;

                switch (g_Options.LegitBot.AimBot.HitBox)
                {
                case 0: // Head
                    BonePos = g_Fivem.GetBonePosVec3(ClosestEntity, SKEL_Head);
                    BonePos.z += 0.04f;
                    break;
                case 1: // Neck
                    BonePos = g_Fivem.GetBonePosVec3(ClosestEntity, SKEL_Neck_1);
                    break;
                case 2: // Chest
                    BonePos = g_Fivem.GetBonePosVec3(ClosestEntity, SKEL_Spine3);
                    break;
                default:
                    continue;
                }

                // PREDICTION
                if (g_Options.LegitBot.AimBot.Prediction)
                {
                    CPed* targetPed = (CPed*)ClosestEntity.StaticInfo.Ped;
                    if (targetPed)
                    {
                        Vector3D targetVel = FrameWork::Memory::ReadMemory<Vector3D>((uintptr_t)targetPed + 0x50);
                        if (targetVel.x == targetVel.x && targetVel.y == targetVel.y && targetVel.z == targetVel.z)
                        {
                            float dist = ClosestEntity.Cordinates.DistTo(g_Fivem.GetLocalPlayerInfo().WorldPos);
                            if (dist == dist && dist > 0.1f)
                            {
                                float bulletSpeed = 800.0f;
                                float travelTime = dist / bulletSpeed;
                                BonePos.x += targetVel.x * travelTime;
                                BonePos.y += targetVel.y * travelTime;
                                BonePos.z += targetVel.z * travelTime;
                            }
                        }
                    }
                }

                g_Fivem.ProcessCameraMovement(
                    BonePos,
                    g_Options.LegitBot.AimBot.SmoothHorizontal,
                    g_Options.LegitBot.AimBot.SmoothVertical);
            }
        }
    }
}
