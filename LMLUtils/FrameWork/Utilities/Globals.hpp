#pragma once
#include <Framework/FrameWork.hpp>

#include <d3dx11.h>
#include <d3d11.h>
#include <D3DX11tex.h>
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "D3DX11.lib" )

class cColors {
public:
	ImVec4 Base = ImColor(255, 255, 255); // antes: ImColor(198, 0, 255)
	ImVec4 PrimaryText = ImColor(255, 255, 255);
	ImVec4 SecundaryText = ImColor(255, 255, 255);

	ImVec4 FeaturesText = ImColor(255, 255, 255);
	ImVec4 SecundaryFeaturesText = ImColor(255, 255, 255);

	ImVec4 BorderCol = ImColor(0, 0, 0);
	ImVec4 LinesCol = ImColor(0, 0, 0);
	ImVec4 BackgroundCol = ImColor(0, 0, 0);

	ImVec4 ChildCol = ImColor(10, 10, 10);
	ImVec4 ChildBorderCol = ImColor(255, 255, 255); // antes: ImColor(198, 0, 255)

	ImVec4 TitleBar = ImColor(10, 10, 10);
	ImVec4 TitleBarBorder = ImColor(10, 10, 10);

	ImVec4 SideBar = ImColor(0, 0, 0);
	ImVec4 SideBarBorder = ImColor(255, 255, 255); // antes: ImColor(198, 0, 255)

	ImVec4 ButtonHovered = ImColor(255, 255, 255); // antes: ImColor(198, 0, 255)

	ImVec4 InputBackground = ImColor(10, 10, 10, 10);
	ImVec4 InputBorder = ImColor(255, 255, 255); // antes: ImColor(198, 0, 255
};

inline cColors g_Col;

class c_globals {
public:
	std::string id;
	std::string version;

	bool g_bPassedByThisVerify;
	uintptr_t g_VerifyLogin;
	bool done;
	bool IsOpen;

	DWORD ProcIdFiveM = 0;

	std::string ServerIp;
	std::string UserName;
	std::string Role;

	ImVec2 TestePos;

	char m_Config[6000];
	HWND g_hCheatWindow;
	HWND g_hGameWindow;
	ImVec2 g_vGameWindowSize;
	ImVec2 g_vGameWindowPos;
	ImVec2 g_vGameWindowCenter;

	ImFont* m_FontBig;
	ImFont* m_FontBigSmall;
	ImFont* m_FontNormal;
	ImFont* m_FontSecundary;
	ImFont* m_FontSmaller;
	ImFont* m_DrawFont;
	ImFont* m_Expand;

	ImFont* FontAwesomeSolid;
	ImFont* FontAwesomeSolidSmall;
	ImFont* FontAwesomeRegular;
	ImFont* FontAwesomeBrands;


	ID3D11ShaderResourceView* Logo = nullptr;
	ID3D11ShaderResourceView* decor_background = nullptr;


};

inline c_globals g_Variables;

class c_menu {
public:

	enum PAGES {
		Combat,
		Visuals,
		Local,
		World,
		Exploits,
		Settings,
		Login
	};

	bool IsOpen;

	int iTabCount = 0;
	float TabAlpha = 0.f;
	int iCurrentPage = 6;
	float TabAdd = 0.f;
	bool IsLogged = false;

	char cDiscordId[200];

	ImVec2 MenuSize{ 650, 450 };
};

inline c_menu g_MenuInfo;