#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <Windows.h>
#include <functional>

#include <d3d11.h>
#include <d3dx11.h>

#include <FrameWork/FrameWork.hpp>

namespace FrameWork
{
	namespace Overlay
	{
		void Setup(DWORD TargetPid);
		void Initialize();
		void ShutDown();

		void UpdateWindowPos();
		void SetupWindowProcHook(std::function<void(HWND, UINT, WPARAM, LPARAM)> Funtion);

		bool IsSettuped();
		bool IsInitialized();
		HWND GetOverlayWindow();
		HWND GetTargetWindow();

		static ID3D11Device* g_pd3dDevice = nullptr;
		static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
		static IDXGISwapChain* g_pSwapChain;


		void dxInitialize();
		void dxRefresh();
		void dxShutDown();

		void dxCreateRenderTarget();
		void dxCleanupRenderTarget();
		void dxCleanupDeviceD3D();

		ID3D11Device* dxGetDevice();
		ID3D11DeviceContext* dxGetDeviceContext();
		IDXGISwapChain* dxGetSwapChain();
		ID3D11RenderTargetView* dxGetRenderTarget();

	}
}