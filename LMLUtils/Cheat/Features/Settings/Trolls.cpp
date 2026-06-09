#include "Trolls.hpp"
#include "../../Options.hpp"
#include "../../FivemSDK/GTADefines.hpp"
#include <FrameWork/Utilities/Notify.hpp>
#include <cmath>
#include <thread>

namespace
{
	static bool g_HoldingVehicle = false;
	static Cheat::CVehicle* g_HeldVehicle = nullptr;
	static Vector3D g_ThrowDirection = { 0.f, 0.f, 0.f };
	static int g_GrabPhase = 0;
	static Vector3D g_GrabStartPos = { 0.f, 0.f, 0.f };
}

namespace Cheat
{
	static CVehicle* FindClosestVehicleToCrosshair()
	{
		auto vehicles = g_Fivem.GetVehicleList();
		if (vehicles.empty())
			return nullptr;

		auto localPos = g_Fivem.GetLocalPlayerInfo().WorldPos;
		float closestDist = FLT_MAX;
		CVehicle* closestVeh = nullptr;

		ImVec2 screenCenter(
			ImGui::GetIO().DisplaySize.x * 0.5f,
			ImGui::GetIO().DisplaySize.y * 0.5f
		);

		Matrix4x4 vm = g_Fivem.GetViewMatrix();

		for (auto& v : vehicles)
		{
			if (!v.Vehicle)
				continue;

			float worldDist = v.Vehicle->GetCoordinate().DistTo(localPos);
			if (worldDist > 250.f)
				continue;

			if (g_HoldingVehicle && v.Vehicle == g_HeldVehicle)
				continue;

			ImVec2 screenPos = g_Fivem.WorldToScreen(v.Vehicle->GetCoordinate(), vm);
			if (screenPos.x == 0.f && screenPos.y == 0.f)
				continue;

			float screenDist = std::sqrt(
				(screenPos.x - screenCenter.x) * (screenPos.x - screenCenter.x) +
				(screenPos.y - screenCenter.y) * (screenPos.y - screenCenter.y)
			);

			float fov = 150.f;
			if (screenDist > fov)
				continue;

			if (screenDist < closestDist)
			{
				closestDist = screenDist;
				closestVeh = v.Vehicle;
			}
		}

		return closestVeh;
	}

	void Trolls::GrabVehicle()
	{
		auto ped = g_Fivem.GetLocalPlayerInfo().Ped;
		if (!ped)
		{
			return;
		}

		CVehicle* target = FindClosestVehicleToCrosshair();
		if (!target)
		{
			return;
		}

		g_Fivem.network_request_control_of_entity((uint64_t)target, (uint64_t)ped);

		g_HeldVehicle = target;
		g_HoldingVehicle = true;

		g_GrabStartPos = target->GetCoordinate();
		g_GrabPhase = 50;

		target->SetLockState(CARLOCK_UNLOCKED);
		target->StartVehicleEngine();

	}

	void Trolls::ReleaseVehicle(bool throwVehicle)
	{
		if (!g_HeldVehicle || !g_HoldingVehicle)
			return;

		CPed* localPed = g_Fivem.GetLocalPlayerInfo().Ped;

		// Clear animation flags
		if (localPed)
		{
			localPed->SetConfigFlag(CPED_CONFIG_FLAG_IsHoldingProp, false);
			localPed->SetConfigFlag(CPED_CONFIG_FLAG_DontActivateRagdollForVehicleGrab, false);
		}

		if (throwVehicle && g_HeldVehicle)
		{
			uint64_t nav = g_HeldVehicle->GetNavigation();
			uint64_t model = g_HeldVehicle->GetModelInfo();
			if (!nav || !model)
			{
				goto cleanup;
			}

			// Ensure direction is valid; fallback to forward if zero
			Vector3D dir = g_ThrowDirection;
			float dirLen = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
			if (dirLen < 0.001f)
				dir = Vector3D(0.f, 1.f, 0.f);
			else
			{
				dir.x /= dirLen;
				dir.y /= dirLen;
				dir.z /= dirLen;
			}

			// Request network control with time to take effect
			if (localPed)
				g_Fivem.network_request_control_of_entity((uint64_t)g_HeldVehicle, (uint64_t)localPed);
			std::this_thread::sleep_for(std::chrono::milliseconds(50));

			Vector3D currentPos = g_HeldVehicle->GetCoordinate();
			float step = g_Options.Misc.Trolls.ThrowForce / 20.0f;

			for (int i = 0; i < 20; i++)
			{
				if (!g_HeldVehicle)
					break;

				currentPos.x += dir.x * step;
				currentPos.y += dir.y * step;
				currentPos.z += dir.z * step;

				if (localPed)
					g_Fivem.network_request_control_of_entity((uint64_t)g_HeldVehicle, (uint64_t)localPed);

				g_Fivem.TeleportToObject(
					(uintptr_t)g_HeldVehicle,
					nav, model,
					currentPos, currentPos, true
				);
			}

		}
		else
		{
		}

cleanup:
		g_HeldVehicle = nullptr;
		g_HoldingVehicle = false;
	}

	bool Trolls::IsHoldingVehicle()
	{
		return g_HoldingVehicle;
	}

	void Trolls::RunThread()
	{
		static bool lastKeyState = false;

		while (!g_Options.General.ShutDown)
		{
			auto& opts = g_Options.Misc.Trolls;

			if (!opts.VehicleGrabEnabled)
			{
				if (g_HoldingVehicle)
					ReleaseVehicle(false);
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			CPed* ped = g_Fivem.GetLocalPlayerInfo().Ped;
			if (!ped)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			// Update throw direction from camera
			auto* dir = g_Fivem.GetCamGameplayDirector();
			if (!dir) continue;
			auto cam = dir->GetFollowPedCamera();
			if (cam)
			{
				Vector3D camPos = cam->GetCameraPosition();
				Vector3D crosshairPos = cam->GetCrosshairPosition();
				Vector3D dir;
				dir.x = crosshairPos.x - camPos.x;
				dir.y = crosshairPos.y - camPos.y;
				dir.z = crosshairPos.z - camPos.z;
				float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
				if (len > 0.0001f)
				{
					g_ThrowDirection.x = dir.x / len;
					g_ThrowDirection.y = dir.y / len;
					g_ThrowDirection.z = dir.z / len;
				}
			}

			if (opts.GrabKey != 0)
			{
				bool keyDown = (GetAsyncKeyState(opts.GrabKey) & 0x8000) != 0;

				if (opts.GrabKeyState == 0) // Toggle mode
				{
					if (keyDown && !lastKeyState)
					{
						if (g_HoldingVehicle)
							ReleaseVehicle(true);
						else
							GrabVehicle();
					}
					lastKeyState = keyDown;
				}
				else // Hold mode
				{
					if (keyDown && !g_HoldingVehicle)
					{
						GrabVehicle();
						if (!g_HoldingVehicle)
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
							continue;
						}
					}
					else if (!keyDown && g_HoldingVehicle)
					{
						ReleaseVehicle(true);
					}
				}
			}

			// While holding vehicle, teleport it above the player's head
			if (g_HoldingVehicle && g_HeldVehicle)
			{
				// Force holding animation every frame
				ped->SetConfigFlag(CPED_CONFIG_FLAG_IsHoldingProp, true);
				ped->SetConfigFlag(CPED_CONFIG_FLAG_DontActivateRagdollForVehicleGrab, true);

				uint64_t nav = g_HeldVehicle->GetNavigation();
				uint64_t model = g_HeldVehicle->GetModelInfo();
				if (!nav || !model)
				{
				ReleaseVehicle(false);
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					continue;
				}

				Vector3D pedPos = ped->GetCoordinate();

				Vector3D holdPos;
				holdPos.x = pedPos.x;
				holdPos.y = pedPos.y;
				holdPos.z = pedPos.z + opts.HoldHeight;

				// Grab phase: smoothly lift vehicle straight up from where it was
				if (g_GrabPhase > 0)
				{
					float t = 1.0f - (g_GrabPhase / 50.0f);
					holdPos.x = g_GrabStartPos.x;
					holdPos.y = g_GrabStartPos.y;
					holdPos.z = g_GrabStartPos.z + (holdPos.z - g_GrabStartPos.z) * t;
					g_GrabPhase--;
				}

				g_Fivem.network_request_control_of_entity((uint64_t)g_HeldVehicle, (uint64_t)ped);

				g_Fivem.TeleportToObject(
					(uintptr_t)g_HeldVehicle,
					nav, model,
					holdPos, holdPos, false
				);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
}
