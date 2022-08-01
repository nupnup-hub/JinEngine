#include"ImGuiManager.h"
#include"../../JWindows.h"
#include"../../JWindowEventType.h"
#include"../../../Graphic/JGraphic.h"
#include"../../../Utility/JCommonUtility.h"
#include "../../Icon/IconFontCppHeaders-master/IconsFontAwesome5.h" 
#include<Windows.h> 

namespace JinEngine
{
    ImFont* ImGuiManager::fontVector[3];

    ImVec4 ImGuiManager::colors[ImGuiCol_COUNT];
    bool ImGuiManager::optFullScreen = true;
    bool ImGuiManager::optWindowPadding = false;
    int ImGuiManager::minWindowWidth;
    int ImGuiManager::minWindowHeight;

    ImGuiManager::ImGuiManager()
        :guid(JCommonUtility::CalculateGuid(typeid(ImGuiManager).name()))
    {
    }
    ImGuiManager::~ImGuiManager()
    {
        // ImGui_ImplDX12_Shutdown();
        // ImGui_ImplWin32_Shutdown();
        // ImGui::DestroyContext();
    }
    void ImGuiManager::PushFont(int index)noexcept
    {
        ImGui::PushFont(fontVector[index]);
    }
    void ImGuiManager::PopFont()noexcept
    {
        ImGui::PopFont();
    }

    ImVec4 ImGuiManager::GetColor(ImGuiCol_ flag)noexcept
    {
        ImGuiStyle& style = ImGui::GetStyle();
        return style.Colors[flag];
    }
    void ImGuiManager::SetColorToDeep(ImGuiCol_ flag, float factor)noexcept
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[flag] = ImVec4(colors[flag].x - factor, colors[flag].y - factor,
            colors[flag].z - factor, colors[flag].w - factor);
    }
    void ImGuiManager::SetColor(const ImVec4& color, ImGuiCol_ flag)noexcept
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[flag] = color;
    }
    void ImGuiManager::ReturnDefaultColor(ImGuiCol_ flag)noexcept
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[flag] = colors[flag];
    }
    bool ImGuiManager::IsFullScreen()noexcept
    {
        return optFullScreen;
    }
    bool ImGuiManager::IsWindowPadding()noexcept
    {
        return optWindowPadding;
    }
    void ImGuiManager::SetImGuiBackend()
	{
        this->AddEventListener(*JWindow::Instance().EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_RESIZE);
         
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigWindowsResizeFromEdges = true;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigWindowsResizeFromEdges = true;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;        // Enable Multi-Viewport / Platform Windows
		SetDefaultColor();
		LoadFontFile();
		//PushFont();
		OnResize();

		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameBorderSize = 1;
         
		ImGui_ImplWin32_Init(JWindow::Instance().HandleInterface()->GetHandle());
        JGraphic::Instance().EditorInterface()->SetImGuiBackEnd();
    }
	void ImGuiManager::OnResize()
	{
		int clientWidth = JWindow::Instance().GetClientWidth();
		int clientHeight = JWindow::Instance().GetClientHeight();
		minWindowWidth = (int)((float)clientWidth / minRate);
		minWindowHeight = (int)((float)clientHeight / minRate);

		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(250, 250));
	}
    void ImGuiManager::OnEvent(const size_t& senderGuid, const Window::J_WINDOW_EVENT& eventType)
    {
        if (senderGuid == guid)
            return;
        if (eventType == Window::J_WINDOW_EVENT::WINDOW_RESIZE)
            OnResize();
    }
    void ImGuiManager::SetDefaultColor()noexcept
    {
        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.1f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
        colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
        colors[ImGuiCol_TabActive] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
        colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
        colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
        colors[ImGuiCol_DockingPreview] = ImVec4(colors[ImGuiCol_HeaderActive].x, colors[ImGuiCol_HeaderActive].y, colors[ImGuiCol_HeaderActive].z, colors[ImGuiCol_HeaderActive].w * 0.7f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
        colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

        ImGuiStyle& style = ImGui::GetStyle();
        for (int i = 0; i < (int)ImGuiCol_COUNT; ++i)
            style.Colors[i] = colors[i];
         
    }
    void ImGuiManager::LoadFontFile()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImFontConfig config;

        fontVector[0] = io.Fonts->AddFontDefault();
        fontVector[1] = io.Fonts->AddFontFromFileTTF("D:\\Visual_Studio_src\\GameEngine\\JinEngine\\Font\\NotoSerifKR-Bold.otf",
            48,
            nullptr,
            io.Fonts->GetGlyphRangesKorean());
        fontVector[2] = io.Fonts->AddFontFromFileTTF("D:\\Visual_Studio_src\\GameEngine\\JinEngine\\Font\\NotoSerifKR-Bold.otf",
            18,
            nullptr,
            io.Fonts->GetGlyphRangesKorean());
    }
}
