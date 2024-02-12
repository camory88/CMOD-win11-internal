#include "render.h"
#include "GolbalVars.h"



float DrawOutlinedText(ImFont* pFont, const ImVec2& pos, float size, ImU32 color, bool center, const char* text, ...)
{
	va_list(args);
	va_start(args, text);

	CHAR wbuffer[256] = { };
	vsprintf_s(wbuffer, text, args);

	va_end(args);

	ImGuiWindow* window = ImGui::GetCurrentWindow();

	std::stringstream stream(text);
	std::string line;

	float y = 0.0f;
	int i = 0;

	while (std::getline(stream, line))
	{
		ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, wbuffer);

		if (center)
		{
			window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);

			window->DrawList->AddText(pFont, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y + textSize.y * i), ImGui::GetColorU32(color), wbuffer);
		}
		else
		{
			window->DrawList->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			window->DrawList->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			window->DrawList->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			window->DrawList->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);

			window->DrawList->AddText(pFont, size, ImVec2(pos.x, pos.y + textSize.y * i), ImGui::GetColorU32(color), wbuffer);
		}

		y = pos.y + textSize.y * (i + 1);
		i++;
	}
	return y;
}

ImGuiWindow& BeginScene() {
	ImGui_ImplDX11_NewFrame();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
	ImGui::Begin(E("##scene"), nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);

	auto& io = ImGui::GetIO();
	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);

	return *ImGui::GetCurrentWindow();
}
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (getKeyDown(VK_INSERT))
		MainMenu = !MainMenu;
	if (getKeyDown(VK_DELETE))
	{
		//unload
	}
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam) && MainMenu == true)
	{
		return true;
	}
	return CallWindowProc(oriWndProc, hWnd, msg, wParam, lParam);
}



#define SteamdllBase (DWORD64)GetModuleHandleA(E("GameOverlayRenderer64.dll"))
VOID SteamHook()//calles creat hook function
{
	const char* Present_Hook_sig = E("48 89 6C 24 ?? 48 89 74 24 ?? 41 56 48 83 EC ?? 41 8B E8");
	const char* Create_Hook_sig = E("48 89 5C 24 ?? 57 48 83 EC ?? 33 C0 48 89 44 24");

	auto Present_Hook = NULL;
	Present_Hook = util::ida_signature(SteamdllBase, Present_Hook_sig);

	auto Create_Hook = NULL;
	Create_Hook = util::ida_signature(SteamdllBase, Create_Hook_sig);

	__int64(__fastcall * CreatHook)(unsigned __int64 a1, __int64 a2, unsigned __int64* a3, int a4);

	CreatHook = (decltype(CreatHook))Create_Hook;
	CreatHook(Present_Hook, (__int64)&present_hook, (unsigned __int64*)&present_original, 1);
}

#define DiscordDllBase (DWORD64)GetModuleHandleA(E("DiscordHook64.dll"))
VOID DiscordHook()// Vtable swap
{
	//AllocConsole();
	//FILE* f;
	//freopen_s(&f, "CONOUT$", "w", stdout);

	uint64_t addr = (uint64_t)((uintptr_t)GetModuleHandleA("DiscordHook64.dll") + 0xE9090);
	if (addr == NULL)
		return MessageBoxA(0, E("Discord Overlay not found!"), E("ERROR"), 0);
	PresentFunc* discord_present = (PresentFunc*)addr;

	present_original = *discord_present;

	_InterlockedExchangePointer((volatile PVOID*)addr, present_hook);
}