#pragma once
#include <Windows.h>
#include <string_view>
#include <functional>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <math.h>
#include <d3d11.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_internal.h"

HRESULT __stdcall present_hook(IDXGISwapChain* swapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT(WINAPI* PresentFunc)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
HRESULT(*present_original)(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);


float DrawOutlinedText(ImFont* pFont, const ImVec2& pos, float size, ImU32 color, bool center, const char* text, ...);


ID3D11Device* device = nullptr;
ID3D11DeviceContext* immediateContext = nullptr;
ID3D11RenderTargetView* renderTargetView = nullptr;

VOID EndScene(ImGuiWindow& window) {
	window.DrawList->PushClipRectFullScreen();
	ImGui::End();
	ImGui::Render();
}

WNDPROC oriWndProc = NULL;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool dllLoaded = true;
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
ImGuiWindow& BeginScene();
ImFont* gameFont = NULL;




VOID SteamHook();
VOID DiscordHook();