#pragma once

#include <FrameWork/FrameWork.hpp>
#include <FrameWork/Dependencies/ImGui/imgui.h>

#include "Assets/FontInter.hpp"
#include "Assets/FontAwesome.hpp"

#include <d3d11.h>
#include <d3dx11.h>

namespace FrameWork
{
	namespace Assets
	{
		inline ImFont* InterBlack14 = nullptr;
		inline ImFont* InterBold12 = nullptr;
		inline ImFont* InterMedium12 = nullptr;
		inline ImFont* InterMedium14 = nullptr;
		inline ImFont* InterMedium16 = nullptr;
		inline ImFont* FontAwesomeRegular = nullptr;
		inline ImFont* FontAwesomeSolid = nullptr;
		inline ImFont* FontAwesomeSolid14 = nullptr;
		inline ImFont* FontAwesomeBrands = nullptr;

		inline ImFont* InterBlack = nullptr;
		inline ImFont* InterBold = nullptr;
		inline ImFont* InterExtraBold = nullptr;
		inline ImFont* InterExtraLight = nullptr;
		inline ImFont* InterLight = nullptr;
		inline ImFont* InterMedium = nullptr;
		inline ImFont* InterRegular = nullptr;
		inline ImFont* InterSemiBold = nullptr;
		inline ImFont* LexendFont = nullptr;
		inline ImFont* InterThin = nullptr;

		inline ImFont* DefaultESPFont = nullptr;
		inline ImFont* SmallFonts = nullptr;
		inline ImFont* EspFont = nullptr;


		inline ID3D11ShaderResourceView* LogoAimlock;

		void Initialize(ID3D11Device* Device);
	}
}