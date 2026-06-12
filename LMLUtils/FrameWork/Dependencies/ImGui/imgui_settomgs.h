#pragma once
#pragma warning(disable: 4244 4267 4242 4312 4311 4100 4189 4456 4457 4458 4696)

#include "imgui.h"

#include <D3DX11.h>

namespace font
{
	inline ImFont* inter_medium_widget = nullptr;
	inline ImFont* inter_medium_small = nullptr;
	inline ImFont* inter_medium_big = nullptr;
	inline ImFont* inter_bold = nullptr;
	inline ImFont* icomoon = nullptr;
	inline ImFont* icomoon_default = nullptr;
}

namespace texture
{
	inline ID3D11ShaderResourceView* preview = nullptr;

	inline ID3D11ShaderResourceView* ragebot = nullptr;
	inline ID3D11ShaderResourceView* legitbot = nullptr;
	inline ID3D11ShaderResourceView* visuals = nullptr;
	inline ID3D11ShaderResourceView* misc = nullptr;
	inline ID3D11ShaderResourceView* world = nullptr;
	inline ID3D11ShaderResourceView* settings = nullptr;
}

namespace c
{

	inline ImVec4 accent = ImColor(0.04f, 0.04f, 0.04f, 0.0f);
	inline ImVec4 black = ImColor(0, 0, 0, 255);

	inline ImVec4 green = ImColor(0, 255, 0, 255);
	inline ImVec4 red = ImColor(255, 255, 255, 255);

	namespace bg
	{
		inline ImVec4 filling = ImColor(6, 7, 20, 255);
		inline ImVec4 stroke = ImColor(19, 21, 32, 255);
		inline ImVec4 topbar = ImColor(7, 7, 11, 255);
		inline ImVec4 container = ImColor(7, 8, 10, 255);
		inline ImVec2 size = ImVec2(770, 450);
		inline float rounding = 5;
	}

	namespace child
	{
		inline ImVec4 filling = ImColor(6, 9, 10, 100);
		inline ImVec4 stroke = ImColor(18, 20, 34, 255);
		//Color de tabs
		inline ImVec4 top = ImColor(9, 10, 15, 255);
		inline ImVec4 child_text = ImColor(33, 37, 62, 255);

		inline float rounding = 5;
	}

	namespace element
	{
		namespace info_bar
		{
			inline ImVec4 filling = ImColor(0, 0, 0, 255);
			inline ImVec4 stroke = ImColor(14, 15, 22, 255);

			inline ImVec4 box = ImColor(8, 9, 12, 255);
			inline ImVec4 box_outline = ImColor(12, 12, 20, 255);
		}
		//Botones de tabs
		inline ImVec4 page_active = ImColor(255, 255, 255, 30);
		inline ImVec4 selectable = ImColor(18, 20, 34, 255);

		namespace popup_elements
		{
			//Botones
			inline ImVec4 filling = ImColor(255, 255, 255, 30);
			inline ImVec4 cog = ImColor(64, 69, 104, 255);
		}

		namespace notify
		{
			inline ImVec4 red_status = ImColor(255, 255, 255, 255);
			inline ImVec4 blue_status = ImColor(255, 255, 255, 255);
			inline ImVec4 green_status = ImColor(74, 201, 126, 255);
			inline ImVec4 yellow_status = ImColor(226, 111, 32, 255);

		}

		namespace combo
		{
			inline ImVec4 filling = ImColor(20, 20, 24, 255);
		}

		inline ImVec4 filling = ImColor(25, 23, 48, 255);
		//checkmarks desactivados
		inline ImVec4 circle_mark = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
		inline ImVec4 text_active = ImColor(255, 255, 255, 255);
		inline ImVec4 text_hov = ImColor(111, 115, 148, 255);
		inline ImVec4 text = ImColor(255, 255, 255, 255);
		inline ImVec4 selector = ImColor(30, 31, 33, 80);

		inline float rounding = 5;
	}
}
