#pragma once 
#include<string> 
#include"../../../Core/Event/JEventListener.h" 
#include"../../JWindowEventType.h"
#include"../../../../Lib/imgui/imgui_internal.h"
#include"../../../../Lib/imgui/imgui_impl_dx12.h"
#include"../../../../Lib/imgui/imgui_impl_win32.h"
#include"../../../../Lib/imgui/imgui_internal.h"

namespace JinEngine
{
    class ImGuiManager : public Core::JEventListener<size_t, Window::J_WINDOW_EVENT>
    {
    private:
        const size_t guid; 
        static ImFont* fontVector[3];
        static bool optFullScreen;
        static bool optWindowPadding;
        static int minWindowWidth;
        static int minWindowHeight;
        static constexpr float minRate = 7.5f;
    private:
        static ImVec4 colors[ImGuiCol_COUNT];
    public:
        ImGuiManager();
        ~ImGuiManager();
        ImGuiManager(const ImGuiManager& rhs) = delete;
        ImGuiManager& operator=(const ImGuiManager& rhs) = delete;

        //0 = default, 1 = bigRegular, 2 = smallRegular
        static void PushFont(int index)noexcept;
        static void PopFont()noexcept;
        static ImVec4 GetColor(ImGuiCol_ flag)noexcept;
        
        static void SetColorToDeep(ImGuiCol_ flag, float factor)noexcept;
        static void SetColor(const ImVec4& color, ImGuiCol_ flag)noexcept;
        static void ReturnDefaultColor(ImGuiCol_ flag)noexcept;
        static bool IsFullScreen()noexcept;
        static bool IsWindowPadding()noexcept;

        void SetImGuiBackend();
        void OnResize();
        virtual void OnEvent(const size_t& senderGuid, const Window::J_WINDOW_EVENT& eventType)final;
    private: 
        static void SetDefaultColor()noexcept;
        static void LoadFontFile();
    };
}
