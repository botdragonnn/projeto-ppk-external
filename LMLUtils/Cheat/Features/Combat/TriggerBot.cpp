// by HyperX
#include "TriggerBot.hpp"

#include <thread>

#include "../../Options.hpp"
#include <FrameWork/FrameWork.hpp>

namespace Cheat
{
	void TriggerBot::RunThread()
	{
		while (!g_Options.General.ShutDown)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10 + g_Options.General.ThreadDelay));

			if (!g_Options.LegitBot.Trigger.Enabled)
				continue;

			if (!g_Fivem.GetLocalPlayerInfo().Ped)
				continue;

			static bool Shooting = false;
			bool CanShoot = false;

			if (SafeCall(GetAsyncKeyState)(g_Options.LegitBot.Trigger.KeyBind) & 0x8000)
			{
				float triggerFov = (float)g_Options.LegitBot.Trigger.Fov;

				Entity Target;
				if (g_Fivem.FindClosestEntity(
					triggerFov,
					(float)g_Options.LegitBot.Trigger.MaxDistance,
					g_Options.LegitBot.Trigger.ShotNPC,
					true,
					&Target))
				{
					if ((uint64_t)Target.StaticInfo.Ped < 0xCCCCCCCCCCCCCC &&
						(uint64_t)Target.StaticInfo.Ped > 0x10000 &&
						!Target.StaticInfo.bIsFriend)
					{
						if (g_Options.LegitBot.Trigger.VisibleCheck)
					{
						CPed* ped = (CPed*)Target.StaticInfo.Ped;
						CanShoot = ped && ped->IsVisible();
					}
						else
							CanShoot = true;
					}
				}
			}

			if (CanShoot && !Shooting)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(g_Options.LegitBot.Trigger.ReactionTime));

				INPUT input = { 0 };
				input.type = INPUT_MOUSE;
				input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
				SafeCall(SendInput)(1, &input, sizeof(INPUT));

				Shooting = true;
			}
			else if (!CanShoot && Shooting)
			{
				INPUT input = { 0 };
				input.type = INPUT_MOUSE;
				input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
				SafeCall(SendInput)(1, &input, sizeof(INPUT));

				Shooting = false;
			}
		}
	}
}
