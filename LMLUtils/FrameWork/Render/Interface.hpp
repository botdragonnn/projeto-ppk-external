#pragma once

#include <FrameWork/FrameWork.hpp>

#include <d3d11.h>
#include <d3dx11.h>

namespace FrameWork
{
	class Interface
	{
	public:
		Interface() { }
		Interface(HWND Window, HWND TargetWindow, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) { Initialize(Window, TargetWindow, Device, DeviceContext); }
		~Interface() { ShutDown(); }
		void RenderActiveFeaturesOverlay();
		void RenderAdminListOverlay();
		void Initialize(HWND Window, HWND TargetWindow, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		void UpdateStyle();

		void RenderGui();

		void WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void HandleMenuKey();

		void ShutDown();

		bool GetMenuOpen() { return bIsMenuOpen; }

		ID3D11Device* m_pDevice = nullptr;
		ID3D11DeviceContext* m_pDeviceCtx = nullptr;
	private:
		HWND hWindow;
		HWND hTargetWindow;
		ID3D11Device* IDevice;

		bool bIsMenuOpen = true;

	public:
		UINT ResizeWidht = 0;
		UINT ResizeHeight = 0;
	};
}
