// by HyperX
#pragma once
#include "VehicleESP.hpp"

#include "../../FivemSDK/Fivem.hpp"
#include "../../Options.hpp"

namespace Cheat
{
	void ESP::Vehicles()
	{
		if (!g_Fivem.GetLocalPlayerInfo().Ped)
			return;

		ImVec2 localScreen = g_Fivem.GetLocalPlayerInfo().ScreenPos;
		auto& vehOpts = g_Options.Visuals.ESP.Vehicles;

		int processed = 0;
		const int maxVehicles = 100;
		for (VehicleInfo Current : g_Fivem.GetVehicleList())
		{
			if (processed++ > maxVehicles)
				break;

			ImVec2 Position = g_Fivem.WorldToScreen(Current.Vehicle->GetCoordinate());
			if (!g_Fivem.IsOnScreen(Position))
				continue;

			float OffsetY = 0;
			float dist = Current.Vehicle->GetCoordinate().DistTo(g_Fivem.GetLocalPlayerInfo().WorldPos);
			bool isClose = dist < vehOpts.RenderDistance;

			if (vehOpts.Marker)
			{
				ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(Position.x, Position.y + OffsetY), isClose ? 4 : 2, ImColor(0.f, 0.f, 0.f, vehOpts.MarkerColor[3] * (isClose ? 1.0f : 0.5f)));
				ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(Position.x, Position.y + OffsetY), isClose ? 3 : 1.5f, FrameWork::Misc::Float4ToImColor(vehOpts.MarkerColor));
				OffsetY += isClose ? 12 : 6;
			}

			if (!isClose)
				continue;

			ImGui::PushFont(FrameWork::Assets::InterBold12);

			if (vehOpts.Model)
			{
				std::string ModelText = " " + Current.Name;
				ImVec2 TextSize = ImGui::CalcTextSize(ModelText.c_str());
				ImGui::GetBackgroundDrawList()->AddText(
					ImVec2(Position.x + 1 - TextSize.x / 2, Position.y + OffsetY + 1),
					ImColor(0.f, 0.f, 0.f, vehOpts.TextColor[3]),
					ModelText.c_str()
				);
				ImGui::GetBackgroundDrawList()->AddText(
					ImVec2(Position.x - TextSize.x / 2, Position.y + OffsetY),
					FrameWork::Misc::Float4ToImColor(vehOpts.VehicleNameColor),
					ModelText.c_str()
				);
				OffsetY += 12;
			}

			if (vehOpts.Door)
			{
				int lockState = Current.Vehicle->GetLockState();
				bool isUnlocked = (lockState == CARLOCK_UNLOCKED || lockState == CARLOCK_NONE);
				std::string VehicleStatusText = isUnlocked ? "Unlocked" : "Locked";

				ImVec2 TextSize = ImGui::CalcTextSize(VehicleStatusText.c_str());
				ImGui::GetBackgroundDrawList()->AddText(
					ImVec2(Position.x + 1 - TextSize.x / 2, Position.y + OffsetY + 1),
					ImColor(0.f, 0.f, 0.f, vehOpts.TextColor[3]),
					VehicleStatusText.c_str()
				);
				ImGui::GetBackgroundDrawList()->AddText(
					ImVec2(Position.x - TextSize.x / 2, Position.y + OffsetY),
					FrameWork::Misc::Float4ToImColor(vehOpts.TextColor),
					VehicleStatusText.c_str()
				);
				OffsetY += 12;
			}

			if (vehOpts.Distance)
			{
				char bfr[48];
				sprintf(
					bfr,
					XorStr(" %dm"),
					(int)Current.Vehicle->GetCoordinate().DistTo(g_Fivem.GetLocalPlayerInfo().WorldPos)
				);
				ImVec2 TextSize = ImGui::CalcTextSize(bfr);
				ImGui::GetBackgroundDrawList()->AddText(
					ImVec2(Position.x + 1 - TextSize.x / 2, Position.y + OffsetY + 1),
					ImColor(0.f, 0.f, 0.f, vehOpts.TextColor[3]),
					bfr
				);
				ImGui::GetBackgroundDrawList()->AddText(
					ImVec2(Position.x - TextSize.x / 2, Position.y + OffsetY),
					FrameWork::Misc::Float4ToImColor(vehOpts.VehicleDistanceColor),
					bfr
				);
				OffsetY += 12;
			}

			// LOCK STATUS ICON
			if (vehOpts.LockStatus)
			{
				int lockState = Current.Vehicle->GetLockState();
				bool isLocked = (lockState != CARLOCK_UNLOCKED && lockState != CARLOCK_NONE);
				ImColor lockCol = isLocked ? FrameWork::Misc::Float4ToImColor(vehOpts.LockedColor) : FrameWork::Misc::Float4ToImColor(vehOpts.UnlockedColor);
				const char* lockIcon = isLocked ? "X" : "O";
				ImVec2 iconSize = ImGui::CalcTextSize(lockIcon);
				ImGui::GetBackgroundDrawList()->AddText(
					ImVec2(Position.x + 1 - iconSize.x / 2, Position.y + OffsetY + 1),
					ImColor(0.f, 0.f, 0.f, 1.f),
					lockIcon
				);
				ImGui::GetBackgroundDrawList()->AddText(
					ImVec2(Position.x - iconSize.x / 2, Position.y + OffsetY),
					lockCol,
					lockIcon
				);
				OffsetY += 12;
			}

			ImGui::PopFont();

			// SNAP LINES (drawn after popfont so no font state issues)
			if (vehOpts.SnapLines)
			{
				ImGui::GetBackgroundDrawList()->AddLine(
					localScreen,
					Position,
					FrameWork::Misc::Float4ToImColor(vehOpts.SnapLinesColor)
				);
			}
		}
	}
}