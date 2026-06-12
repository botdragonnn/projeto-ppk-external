// by HyperX
#include "PlayerESP.hpp"

#include <cmath>
#include <unordered_map>

#include "../../FivemSDK/Fivem.hpp"
#include "../../Options.hpp"
#include <Cheat/Features/Combat/AimBot.hpp>

struct Position
{
	ImVec2 Pos;
};

namespace Cheat
{
	void ESP::Players()
	{
		if (!g_Fivem.GetLocalPlayerInfo().Ped)
			return;

		static std::unordered_map<CPed*, EspAnim> EspAnimations;

	// Cache de opções locais para reduzir acessos globais repetidos
	auto& opts = g_Options.Visuals.ESP.Players;

	auto DrawList = ImGui::GetBackgroundDrawList();

	// ── Radar / Minimap ──
	if (opts.Minimap)
	{
		const float radarSize = 200.0f;
		const ImVec2 radarPos(50.0f, 50.0f);
		const float scale = radarSize / (2.0f * static_cast<float>(opts.RenderDistance));

		DrawList->AddRectFilled(radarPos, ImVec2(radarPos.x + radarSize, radarPos.y + radarSize), ImColor(0, 0, 0, 150), 5.0f);
		DrawList->AddRect(radarPos, ImVec2(radarPos.x + radarSize, radarPos.y + radarSize), ImColor(255, 255, 255, 200), 5.0f, 0, 1.5f);

		ImVec2 radarCenter(radarPos.x + radarSize * 0.5f, radarPos.y + radarSize * 0.5f);
		DrawList->AddCircleFilled(radarCenter, 3.0f, ImColor(255, 255, 0, 255), 12);

		Vector3D localPos = g_Fivem.GetLocalPlayerInfo().WorldPos;

		for (auto& ent : g_Fivem.GetEntitiyList())
		{
			CPed* ped = ent.StaticInfo.Ped;
			if (!ped) continue;

			bool isLocalPlayer = ent.StaticInfo.bIsLocalPlayer;
			bool isNPC = ent.StaticInfo.bIsNPC;

			if (!opts.ShowLocalPlayer && isLocalPlayer)
				continue;

			if (!opts.ShowNPCs && isNPC && !isLocalPlayer)
				continue;

			if (opts.VisibleOnly && !ent.Visible)
				continue;

			Vector3D entityPos = ped->GetCoordinate();
			float distance = entityPos.DistTo(localPos);
			if (distance > static_cast<float>(opts.RenderDistance))
				continue;

			ImVec2 radarEntityPos(
				radarCenter.x + (entityPos.x - localPos.x) * scale,
				radarCenter.y - (entityPos.y - localPos.y) * scale
			);

			radarEntityPos.x = (std::max)(radarPos.x, (std::min)(radarEntityPos.x, radarPos.x + radarSize));
			radarEntityPos.y = (std::max)(radarPos.y, (std::min)(radarEntityPos.y, radarPos.y + radarSize));

			DrawList->AddCircleFilled(radarEntityPos, 3.0f, ImColor(255, 255, 255, 255), 12);
		}
	}

		for (Entity Current : g_Fivem.GetEntitiyList())
		{
			if (Current.StaticInfo.bIsLocalPlayer && !opts.ShowLocalPlayer)
				continue;

			if (Current.StaticInfo.bIsNPC && !opts.ShowNPCs)
				continue;

			if (opts.VisibleOnly && !Current.Visible)
				continue;

			Vector3D PedCoordinates = Current.Cordinates;

			auto& CurrentESPAnim = EspAnimations[Current.StaticInfo.Ped];
			CurrentESPAnim.CanFadeOut = false;

			float Distance = PedCoordinates.DistTo(g_Fivem.GetLocalPlayerInfo().WorldPos);
			if (Distance > opts.RenderDistance)
				continue;

			ImVec2 PedLocation = g_Fivem.WorldToScreen(PedCoordinates);

			ImVec2 Head = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_Head));
			if (!g_Fivem.IsOnScreen(Head))
				continue;

			ImVec2 LeftFoot = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_L_Foot));
			if (!g_Fivem.IsOnScreen(LeftFoot))
				continue;

			ImVec2 RightFoot = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_R_Foot));
			if (!g_Fivem.IsOnScreen(RightFoot))
				continue;

			// Calcula altura baseada em head -> foot
			float maxFootY = LeftFoot.y > RightFoot.y ? LeftFoot.y : RightFoot.y;
			float Height = maxFootY - Head.y; // Diferença direta, sem fabsf

			float Width = Height / 1.8f;

			// Ajusta a altura para cobrir todo o personagem
			Height *= 1.37f; // Fator reduzido para não ficar muito grande

			float BoxLeft = Head.x - (Width / 2.f);
			float BoxRight = Head.x + (Width / 2.f);
			float BoxTop = Head.y - Height * 0.11f; // Apenas 5% acima da cabeça
			float BoxBottom = BoxTop + Height;

			ImVec2 BoxMin(BoxLeft, BoxTop);
			ImVec2 BoxMax(BoxRight, BoxBottom);

			ImVec2 BoxCenter = ImVec2((BoxMin.x + BoxMax.x) * 0.5f, (BoxMin.y + BoxMax.y) * 0.5f);

			auto RgbColor = [&]() -> ImColor {
				float speed = opts.RGBSpeed * 66.0f;
				return ImColor(
					(sinf(ImGui::GetTime() * speed + 0.0f) * 0.5f + 0.5f),
					(sinf(ImGui::GetTime() * speed + 2.0f) * 0.5f + 0.5f),
					(sinf(ImGui::GetTime() * speed + 4.0f) * 0.5f + 0.5f));
			};

			ImColor boxColor = opts.RGB ? RgbColor() : FrameWork::Misc::Float4ToImColor(opts.BoxColor);
			if (opts.HighlightVisible && Current.Visible)
			{
				float r = boxColor.Value.x; float g = boxColor.Value.y; float b = boxColor.Value.z;
				boxColor = ImColor(min(1.f, r * 1.5f), min(1.f, g * 1.5f), min(1.f, b * 1.5f), boxColor.Value.w);
			}

			Position Positions[] =
			{
				{ ImVec2(PedLocation.x, BoxTop - 5.0f) }, // Top (acima da box)
				{ ImVec2(PedLocation.x + Width / 2, BoxTop) }, // Right
				{ ImVec2(PedLocation.x, BoxBottom + 5.0f) }, // Bottom (abaixo da box)
				{ ImVec2(PedLocation.x - Width / 2, BoxTop) } // Left
			};

			ImVec2 Padding[4] = { ImVec2(0, 0), ImVec2(0, 0), ImVec2(0, 0), ImVec2(0, 0) };

			// Posições para textos (ajustadas)
			float TextTopY = BoxTop - 20.0f; // Reduzido de 25 para 20
			float TextBottomY = BoxBottom + 5.0f; // Reduzido de 8 para 5

			if (opts.Box)
			{
				DrawList->AddRect(ImVec2(BoxLeft - 1, BoxTop - 1), ImVec2(BoxRight + 1, BoxBottom + 1), ImColor(0, 0, 0, 200), 0, 0, 1.5f);
				DrawList->AddRect(ImVec2(BoxLeft + 1, BoxTop + 1), ImVec2(BoxRight - 1, BoxBottom - 1), ImColor(0, 0, 0, 200), 0, 0, 1.5f);
				DrawList->AddRect(ImVec2(BoxLeft, BoxTop), ImVec2(BoxRight, BoxBottom), opts.RGB ? RgbColor() : FrameWork::Misc::Float4ToImColor(opts.BoxColor), 0, 0, 1.5f);
			}

			// ── Head Circle ──
			if (opts.HeadCircle)
			{
				Vector3D HeadPos3D = g_Fivem.GetBonePosVec3(Current, SKEL_Head);
				if (HeadPos3D.x != 0.0f || HeadPos3D.y != 0.0f || HeadPos3D.z != 0.0f)
				{
					Vector3D headCenter = HeadPos3D + Vector3D(0.f, 0.f, 0.04f);
					ImVec2 headScreen = g_Fivem.WorldToScreen(headCenter);
					if (headScreen.x != 0 && headScreen.y != 0)
					{
						int circleRadius = (int)(Height / 15.f);
						if (circleRadius <= 10)
							circleRadius = (int)(Height / 20.f);
						ImColor headCircleColor = opts.RGB ? RgbColor() : FrameWork::Misc::Float4ToImColor(opts.HeadCircleColor);
						DrawList->AddCircle(headScreen, (float)circleRadius, headCircleColor, 48, 0.5f);
					}
				}
			}

			// ── NoClip visual indicator ──

			if (opts.Skeleton)
			{
				// ... código do skeleton mantido igual ...
				do
				{
					ImVec2 Pelvis = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_Pelvis));
					if (!g_Fivem.IsOnScreen(Pelvis)) break;

					ImVec2 Neck = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_Neck_1));
					if (!g_Fivem.IsOnScreen(Neck)) break;

					ImVec2 LeftClavicle = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_L_Clavicle));
					if (!g_Fivem.IsOnScreen(LeftClavicle)) break;

					ImVec2 RightClavicle = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_R_Clavicle));
					if (!g_Fivem.IsOnScreen(RightClavicle)) break;

					ImVec2 LeftUperarm = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_L_UpperArm));
					if (!g_Fivem.IsOnScreen(LeftUperarm)) break;

					ImVec2 RightUperarm = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_R_UpperArm));
					if (!g_Fivem.IsOnScreen(RightUperarm)) break;

					ImVec2 RightFormArm = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_R_Forearm));
					if (!g_Fivem.IsOnScreen(RightFormArm)) break;

					ImVec2 LeftFormArm = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_L_Forearm));
					if (!g_Fivem.IsOnScreen(LeftFormArm)) break;

					ImVec2 RightHand = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_R_Hand));
					if (!g_Fivem.IsOnScreen(RightHand)) break;

					ImVec2 LeftHand = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_L_Hand));
					if (!g_Fivem.IsOnScreen(LeftHand)) break;

					ImVec2 LeftThigh = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_L_Thigh));
					if (!g_Fivem.IsOnScreen(LeftThigh)) break;

					ImVec2 LeftCalf = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_L_Calf));
					if (!g_Fivem.IsOnScreen(LeftCalf)) break;

					ImVec2 RightThigh = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_R_Thigh));
					if (!g_Fivem.IsOnScreen(RightThigh)) break;

					ImVec2 RightCalf = g_Fivem.WorldToScreen(g_Fivem.GetBonePosVec3(Current, SKEL_R_Calf));
					if (!g_Fivem.IsOnScreen(RightCalf)) break;

					ImColor Color = opts.RGB ?
						RgbColor() :
						(Current.StaticInfo.bIsFriend ? FrameWork::Misc::Float4ToImColor(opts.FriendSkeletonColor)
							: FrameWork::Misc::Float4ToImColor(opts.SkeletonColor));

DrawList->AddLine(Neck, RightClavicle, Color, 0.5f);
				DrawList->AddLine(Neck, LeftClavicle, Color, 0.5f);

					DrawList->AddLine(RightClavicle, RightUperarm, Color, 0.5f);
					DrawList->AddLine(LeftClavicle, LeftUperarm, Color, 0.5f);

					DrawList->AddLine(RightUperarm, RightFormArm, Color, 0.5f);
					DrawList->AddLine(LeftUperarm, LeftFormArm, Color, 0.5f);

					DrawList->AddLine(RightFormArm, RightHand, Color, 0.5f);
					DrawList->AddLine(LeftFormArm, LeftHand, Color, 0.5f);

					DrawList->AddLine(Neck, Pelvis, Color, 0.5f);

					DrawList->AddLine(Pelvis, LeftThigh, Color, 0.5f);
					DrawList->AddLine(Pelvis, RightThigh, Color, 0.5f);

					DrawList->AddLine(LeftThigh, LeftCalf, Color, 0.5f);
					DrawList->AddLine(RightThigh, RightCalf, Color, 0.5f);

					DrawList->AddLine(LeftCalf, LeftFoot, Color, 0.5f);
					DrawList->AddLine(RightCalf, RightFoot, Color, 0.5f);
				} while (false);
			}

			// ... código das health bars mantido igual (mas ajustar para nova box) ...
			const auto DrawHealthBarV = [&DrawList](ImVec2 pos, ImVec2 dim, ImColor col, int background) {
				if (background == 1) { // Bar
					DrawList->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + dim.x, pos.y - (dim.y + 1)), col);
				}
				else { // Background
					DrawList->AddRectFilled(ImVec2(pos.x - 1, pos.y + 1), ImVec2(pos.x + dim.x + 1, pos.y - (dim.y + 2)), ImColor(0, 0, 0, 255));
					DrawList->AddRectFilled(ImVec2(pos.x, pos.y - 1), ImVec2(pos.x + dim.x, pos.y - (dim.y + 2)), ImColor(80, 80, 80, 125));
				}
				};
			const auto DrawHealthBarH = [&DrawList](ImVec2 pos, ImVec2 dim, ImColor col, int background) {
				if (background == 1) { // Bar
					DrawList->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + dim.y, pos.y + dim.x), col);
				}
				else { // Background
					DrawList->AddRectFilled(ImVec2(pos.x - 1, pos.y - 1), ImVec2(pos.x + dim.y + 1, pos.y + dim.x + 1), ImColor(0, 0, 0, 255));
					DrawList->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + dim.y, pos.y + dim.x), ImColor(80, 80, 80, 125));
				}
				};

			if (opts.HealthBar)
			{
				float Health = Current.StaticInfo.Ped->GetHealth();
				float MaxHealth = Current.StaticInfo.Ped->GetMaxHealth();

				CurrentESPAnim.Health = ImLerp(CurrentESPAnim.Health, Health, ImGui::GetIO().DeltaTime * 4.0f);
				float AnimHealth = CurrentESPAnim.Health;

				float FullHealthBar = Height; // Usa a altura total da box
				float DecreaseHealthBar = FullHealthBar * (AnimHealth / MaxHealth);

				float FullHealthBarH = Width; // Usa a largura total da box
				float DecreaseHealthBarH = FullHealthBarH * (AnimHealth / MaxHealth);

				if (DecreaseHealthBarH > FullHealthBarH)
				{
					DecreaseHealthBarH = FullHealthBarH;
				}

				if (DecreaseHealthBar > FullHealthBar)
				{
					DecreaseHealthBar = FullHealthBar;
				}

				ImColor FullHealth = (ImVec4(ImColor(80, 80, 80, 200)));
				ImColor BarColor = FrameWork::Misc::Float4ToImColor(opts.HealthBarColor);

				switch (opts.HealthBarState)
				{
				case 0: // Top
					DrawHealthBarH(ImVec2(BoxMin.x, BoxMin.y - 6), ImVec2(3, FullHealthBarH), (ImVec4(FullHealth)), 0);
					DrawHealthBarH(ImVec2(BoxMin.x, BoxMin.y - 6), ImVec2(3, DecreaseHealthBarH), (ImVec4(BarColor)), 1);
					break;
				case 1: // Right
					DrawHealthBarV(ImVec2(BoxMax.x + 6, BoxMax.y), ImVec2(3, FullHealthBar), (ImVec4(FullHealth)), 0);
					DrawHealthBarV(ImVec2(BoxMax.x + 6, BoxMax.y), ImVec2(3, DecreaseHealthBar), (ImVec4(BarColor)), 1);
					break;
				case 2: // Bottom
					DrawHealthBarH(ImVec2(BoxMin.x, BoxMax.y + 6), ImVec2(3, FullHealthBarH), (ImVec4(FullHealth)), 0);
					DrawHealthBarH(ImVec2(BoxMin.x, BoxMax.y + 6), ImVec2(3, DecreaseHealthBarH), (ImVec4(BarColor)), 1);
					break;
				case 3: // Left
					DrawHealthBarV(ImVec2(BoxMin.x - 6, BoxMax.y), ImVec2(3, FullHealthBar), (ImVec4(FullHealth)), 0);
					DrawHealthBarV(ImVec2(BoxMin.x - 6, BoxMax.y), ImVec2(3, DecreaseHealthBar), (ImVec4(BarColor)), 1);
					break;

				default:
					DrawHealthBarV(ImVec2(BoxMin.x - 6, BoxMax.y), ImVec2(3, FullHealthBar), (ImVec4(FullHealth)), 0);
					DrawHealthBarV(ImVec2(BoxMin.x - 6, BoxMax.y), ImVec2(3, DecreaseHealthBar), (ImVec4(BarColor)), 1);
					break;
				}
			}

			if (opts.ArmorBar)
			{
				float Armor = Current.StaticInfo.Ped->GetArmor();
				float MaxArmor = 100.0f;

				CurrentESPAnim.Armor = ImLerp(CurrentESPAnim.Armor, Armor, ImGui::GetIO().DeltaTime * 4.0f);
				float AnimArmor = CurrentESPAnim.Armor;

				float FullArmorBar = Height;
				float DecreaseArmorBar = FullArmorBar * (AnimArmor / MaxArmor);

				float FullArmorBarH = Width;
				float DecreaseArmorBarH = FullArmorBarH * (AnimArmor / MaxArmor);

				if (DecreaseArmorBarH > FullArmorBarH)
					DecreaseArmorBarH = FullArmorBarH;

				if (DecreaseArmorBar > FullArmorBar)
					DecreaseArmorBar = FullArmorBar;

				ImColor FullArmor = ImColor(80, 80, 80, 200);
				ImColor ArmorColor = FrameWork::Misc::Float4ToImColor(opts.ArmorColor);
				if (Armor <= 0.f) ArmorColor = ImColor(0, 0, 0, 0);

				// Calcula offset extra se HealthBar estiver no mesmo lado
				int sideOffset = 0;
				if (opts.HealthBarState == opts.ArmorBarState)
					sideOffset = 9; // gap extra quando ambas no mesmo lado

				switch (opts.ArmorBarState)
				{
				case 0:
					DrawHealthBarH(ImVec2(BoxMin.x, BoxMin.y - 6 - sideOffset), ImVec2(3, FullArmorBarH), FullArmor, 0);
					DrawHealthBarH(ImVec2(BoxMin.x, BoxMin.y - 6 - sideOffset), ImVec2(3, DecreaseArmorBarH), ArmorColor, 1);
					break;
				case 1:
					DrawHealthBarV(ImVec2(BoxMax.x + 6 + sideOffset, BoxMax.y), ImVec2(3, FullArmorBar), FullArmor, 0);
					DrawHealthBarV(ImVec2(BoxMax.x + 6 + sideOffset, BoxMax.y), ImVec2(3, DecreaseArmorBar), ArmorColor, 1);
					break;
				case 2:
					DrawHealthBarH(ImVec2(BoxMin.x, BoxMax.y + 6 + sideOffset), ImVec2(3, FullArmorBarH), FullArmor, 0);
					DrawHealthBarH(ImVec2(BoxMin.x, BoxMax.y + 6 + sideOffset), ImVec2(3, DecreaseArmorBarH), ArmorColor, 1);
					break;
				case 3:
					DrawHealthBarV(ImVec2(BoxMin.x - 6 - sideOffset, BoxMax.y), ImVec2(3, FullArmorBar), FullArmor, 0);
					DrawHealthBarV(ImVec2(BoxMin.x - 6 - sideOffset, BoxMax.y), ImVec2(3, DecreaseArmorBar), ArmorColor, 1);
					break;
				default:
					DrawHealthBarV(ImVec2(BoxMin.x - 6 - sideOffset, BoxMax.y), ImVec2(3, FullArmorBar), FullArmor, 0);
					DrawHealthBarV(ImVec2(BoxMin.x - 6 - sideOffset, BoxMax.y), ImVec2(3, DecreaseArmorBar), ArmorColor, 1);
					break;
				}
			}

			// LISTA DE TEXTOS PARA DESENHAR (mantendo a ordem: Name, Distance, Weapon)
			struct TextInfo {
				std::string text;
				bool enabled;
				int positionState; // 0 = Top, 2 = Bottom (segundo o preview)
			};

			std::vector<TextInfo> textsToDraw;

			// NAME
			if (opts.Name)
			{
				std::string Name = Current.StaticInfo.Name;
				if (Name.empty() || Current.StaticInfo.bIsNPC)
					Name = XorStr("NPC");

				textsToDraw.push_back({ Name, true, opts.NameState });
			}

			// DISTANCE
			if (opts.Distance && !Current.StaticInfo.bIsLocalPlayer)
			{
				std::string PlayerDistance = std::to_string((int)Distance) + XorStr("m");
				textsToDraw.push_back({ PlayerDistance, true, opts.DistanceState });
			}

			// WEAPON NAME
			if (opts.WeaponName)
			{
				CWeaponManager* WeaponManager = Current.StaticInfo.Ped->GetWeaponManager();
				if (WeaponManager)
				{
					CWeaponInfo* WeaponInfo = WeaponManager->GetWeaponInfo();
					if (WeaponInfo)
					{
						std::string WeaponName = WeaponInfo->GetWeaponName();
						if (!WeaponName.empty())
						{
							textsToDraw.push_back({ WeaponName, true, opts.WeaponNameState });
						}
					}
				}
			}

			// DESENHA TEXTOS EM CIMA (POSIÇÃO 0 = TOP)
			float currentTextYTop = TextTopY;

			for (const auto& textInfo : textsToDraw)
			{
				if (textInfo.positionState == 0) // Top
				{
					ImVec2 TextSize = ImGui::CalcTextSize(textInfo.text.c_str());
					ImVec2 TextPos = ImVec2(BoxCenter.x - (TextSize.x / 2), currentTextYTop);

					ImGui::PushFont(FrameWork::Assets::InterBold12);
					DrawList->AddText(ImVec2(TextPos.x + 1, TextPos.y + 1), ImColor(0.f, 0.f, 0.f, 1.f), textInfo.text.c_str());
					ImColor textCol = FrameWork::Misc::Float4ToImColor(opts.TextColor);
					DrawList->AddText(TextPos, textCol, textInfo.text.c_str());
					ImGui::PopFont();

					currentTextYTop -= 12.0f; // Espaçamento entre textos
				}
			}

			// DESENHA TEXTOS EMBAIXO (POSIÇÃO 2 = BOTTOM)
			float currentTextYBottom = TextBottomY;

			for (const auto& textInfo : textsToDraw)
			{
				if (textInfo.positionState == 2) // Bottom
				{
					ImVec2 TextSize = ImGui::CalcTextSize(textInfo.text.c_str());
					ImVec2 TextPos = ImVec2(BoxCenter.x - (TextSize.x / 2), currentTextYBottom);

					ImGui::PushFont(FrameWork::Assets::InterBold12);
					DrawList->AddText(ImVec2(TextPos.x + 1, TextPos.y + 1), ImColor(0.f, 0.f, 0.f, 1.f), textInfo.text.c_str());
					ImColor textCol = FrameWork::Misc::Float4ToImColor(opts.TextColor);
					DrawList->AddText(TextPos, textCol, textInfo.text.c_str());
					ImGui::PopFont();

					currentTextYBottom += 12.0f; // Espaçamento entre textos
				}
			}

			// SNAP LINES
			if (opts.SnapLines)
			{
				if (!Current.StaticInfo.bIsLocalPlayer)
				{
					ImColor snapColor = opts.RGB ?
						RgbColor() :
						FrameWork::Misc::Float4ToImColor(opts.SnapLinesColor);
					DrawList->AddLine(g_Fivem.GetLocalPlayerInfo().ScreenPos, PedLocation, snapColor);
				}
			}
		}
	}
}