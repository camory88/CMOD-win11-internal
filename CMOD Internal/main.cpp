#include <Windows.h>
#include "render.h"
#include <map>
#include "mem.h"
#include "game.hpp"

HRESULT __stdcall present_hook(IDXGISwapChain* swapChain, UINT SyncInterval, UINT Flags)
{
	if (dllLoaded == false)
	{
		return present_original(swapChain, SyncInterval, Flags);
	}

	if (!device) {
		swapChain->GetDevice(__uuidof(device), reinterpret_cast<PVOID*>(&device));
		device->GetImmediateContext(&immediateContext);
		ID3D11Texture2D* renderTarget = nullptr;
		swapChain->GetBuffer(0, __uuidof(renderTarget), reinterpret_cast<PVOID*>(&renderTarget));
		device->CreateRenderTargetView(renderTarget, nullptr, &renderTargetView);
		renderTarget->Release();
		DXGI_SWAP_CHAIN_DESC desc;
		swapChain->GetDesc(&desc);
		oriWndProc = (WNDPROC)SetWindowLongPtr(desc.OutputWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);

		ImGui::GetIO().Fonts->AddFontDefault();
		ImFontConfig font_cfg;
		font_cfg.GlyphExtraSpacing.x = 1.2;
		//gameFont = ImGui::GetIO( ).Fonts->AddFontFromMemoryTTF( TTSquaresCondensedBold, 14, 14, &font_cfg );
		gameFont = ImGui::GetIO().Fonts->AddFontDefault();
		ImGui::GetIO().Fonts->AddFontDefault();
		ImGui_ImplDX11_Init(desc.OutputWindow, device, immediateContext);
		ImGui_ImplDX11_CreateDeviceObjects();
		ImGuiStyle* style = &ImGui::GetStyle();

		style->WindowPadding = ImVec2(15, 15);
		style->WindowRounding = 5.0f;
		style->FramePadding = ImVec2(5, 5);
		style->FrameRounding = 4.0f;
		style->ItemSpacing = ImVec2(12, 8);
		style->ItemInnerSpacing = ImVec2(8, 6);
		style->IndentSpacing = 25.0f;
		style->ScrollbarSize = 15.0f;
		style->ScrollbarRounding = 9.0f;
		style->GrabMinSize = 5.0f;
		style->GrabRounding = 3.0f;

		style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		//style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
		style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
		style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
		style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
		style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
		style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
		style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
		style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
		style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

	}
	immediateContext->OMSetRenderTargets(1, &renderTargetView, nullptr);

	auto& window = BeginScene();
	DrawOutlinedText(gameFont, ImVec2(5, 5), 13.0f, ImColor(255, 255, 255), false, E("%.8f"), ImGui::GetIO().Framerate);

	


	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	ImGui::Begin(E("CMOD"), &MainMenu);
	ImGui::Text(E("Hello World!"));

	static bool test = false;
	ImGui::Checkbox(E("BRUH"), &test);
	
	uint64_t* Local_PlayerPtr = *(uint64_t**)(dwbase + LocalPlayer);
	Vector camAngle = *(Vector*)(Local_PlayerPtr + origin);

	ImGui::Text(E("X: %i"), camAngle.x);
	ImGui::Text(E("Y: %i"), camAngle.y);
	ImGui::Text(E("Z: %i"), camAngle.z);

	



	if(is_lobby)
		ImGui::Text(E("in lobby!"));
	else
		ImGui::Text(E("not in lobby!"));



	ImGui::End();
	


	EndScene(window);
	return present_original(swapChain, SyncInterval, Flags);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
	DisableThreadLibraryCalls(hModule);
	Memory::UnlinkModuleFromPEB(hModule);
	Memory::FakePeHeader(hModule);
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		DiscordHook();
		break;
	}

	return TRUE;
}