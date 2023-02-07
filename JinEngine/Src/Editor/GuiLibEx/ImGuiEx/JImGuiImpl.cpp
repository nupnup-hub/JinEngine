#include"JImGuiImpl.h" 
#include"../../Align/JEditorAlignCalculator.h"
#include"../../../Application/JApplicationVariable.h"

#include"../../../Core/Identity/JIdentifier.h"
#include"../../../Core/JDataType.h"  
#include"../../../Core/Platform/JLanguageType.h"

#include"../../../Window/JWindows.h"
#include"../../../Window/JWindowEventType.h" 
#include"../../../Graphic/JGraphic.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceHandleInterface.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceUserInterface.h"

#include"../../../Utility/JCommonUtility.h"
#include"../../../../Lib/imgui/imgui_impl_dx12.h"
#include"../../../../Lib/imgui/imgui_impl_win32.h" 
#include "../../Icon/IconFontCppHeaders-master/IconsFontAwesome5.h" 

#include<Windows.h> 
#include<memory>
#include<deque>

namespace JinEngine
{
	namespace Editor
	{
		namespace Private
		{
			enum class IMGUI_WIDGET
			{
				WINDOW = 0,
				CHILD_WINDOW,
				POPUP,
				TEXT,
				CHECKBOX,
				BUTTON,
				IMAGE,
				DRAW,
				TREE,
				SELECTABLE,
				INPUT_DATA,
				SLIDER,
				TAB_BAR,
				TAB_ITEM,
				TABLE,
				MENU_BAR,
				MENU,
				MENU_ITEM,
				COMBOBOX,
				SWITCH,
				COUNT
			};
		}

		class JImGui : public Core::JEventListener<size_t, Window::J_WINDOW_EVENT>,
			public Graphic::JGraphicResourceUserInterface
		{
		private:
			using FontMap = std::unordered_map<J_EDITOR_FONT_TYPE, std::unordered_map<Core::J_LANGUAGE_TYPE, ImFont*>>;
		public:
			const size_t guid;
		public:
			JVector2<int> displaySize;
			JVector2<int> windowSize;
			JVector2<int> clientSize;
			JVector2<int> clientPos;
			JVector2<int> minClientSize;
			JVector2<int> alphabetSize;
			JVector2<int> textSizeOffset;
		public:
			uint textBufRange = 100;
		public:
			FontMap fontMap;
			J_EDITOR_FONT_TYPE fontType;
		public:
			ImVec4 colors[ImGuiCol_COUNT];
		public:
			std::bitset<3> mouseClick;
			bool isDrag = false;
		public:
			bool enablePopup = true;
			bool enableSelector = true;
			bool optWindowPadding = false;
		public:
			static constexpr float minRate = 7.5f;
		public:
			uint actWidgetCount[(uint)Private::IMGUI_WIDGET::COUNT];
		public:
			JImGui()
				:guid(JCUtil::CalculateGuid(typeid(JImGui).name()))
			{
				this->AddEventListener(*JWindow::Instance().EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_RESIZE);

				IMGUI_CHECKVERSION();
				ImGui::CreateContext();
				ImGuiIO& io = ImGui::GetIO(); (void)io;
				//io.ConfigWindowsResizeFromEdges = true;
				//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
				//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
				//io.ConfigWindowsResizeFromEdges = true;
				io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
				//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;        // Enable Multi-Viewport / Platform Windows
				//LoadDefaultColor(); 
				SetStyle();
				LoadFontFile();
				//PushFont();

				/*
				ImGuiStyle& style = ImGui::GetStyle();
				style.WindowBorderSize = 1;
				style.ChildBorderSize = 1;
				style.FrameBorderSize = 1;
				style.PopupBorderSize = 1;
				style.TabBorderSize = 1;

				style.FrameRounding = 6;
				style.GrabRounding = 6;
				*/
				//style.FrameBorderSize = 1; 
				ImGui_ImplWin32_Init(JWindow::Instance().HandleInterface()->GetHandle());
				JGraphic::Instance().EditorInterface()->SetImGuiBackEnd();

				OnResize();
				ClearActWidgetCount();
			}
			~JImGui()
			{
				ImGui_ImplDX12_Shutdown();
				ImGui_ImplWin32_Shutdown();
				ImGui::DestroyContext();
				RemoveListener(*JWindow::Instance().EvInterface(), guid);
			}
		public:
			void ClearActWidgetCount()
			{
				for (uint i = 0; i < (uint)Private::IMGUI_WIDGET::COUNT; ++i)
					actWidgetCount[i] = 0;
			}
			uint TotalActWidgetCount()
			{
				uint sum = 0;
				for (uint i = 0; i < (uint)Private::IMGUI_WIDGET::COUNT; ++i)
					sum += actWidgetCount[i];
				return sum;
			}
			void AddActWidgetCount(const Private::IMGUI_WIDGET type)
			{
				++actWidgetCount[(uint)type];
			}
		public:
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGraphicGpuSrvHandle(Graphic::JGraphicResourceHandleInterface& handle)
			{
				return CallGetGpuSrvHandle(handle, Graphic::J_GRAPHIC_BIND_TYPE::SRV, 0);
			}
		private:
			void OnResize()
			{
				displaySize = { JWindow::Instance().GetDisplayWidth() ,  JWindow::Instance().GetDisplayHeight() };
				windowSize = { JWindow::Instance().GetWindowWidth() ,  JWindow::Instance().GetWindowHeight() };
				clientSize = { JWindow::Instance().GetClientWidth() ,  JWindow::Instance().GetClientHeight() };
				clientPos = { JWindow::Instance().GetClientPositionX() ,  JWindow::Instance().GetClientPositionY() };
				minClientSize = { (int)(clientSize.x / minRate),  (int)(clientSize.y / minRate) };
			}
			void OnEvent(const size_t& senderGuid, const Window::J_WINDOW_EVENT& eventType)
			{
				if (senderGuid == guid)
					return;
				if (eventType == Window::J_WINDOW_EVENT::WINDOW_RESIZE)
					OnResize();
			}
		private:
			void SetStyle()
			{
				colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
				colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
				colors[ImGuiCol_WindowBg] = ImVec4(0.185f, 0.185f, 0.185f, 1.00f);
				colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
				colors[ImGuiCol_PopupBg] = ImVec4(0.145f, 0.145f, 0.145f, 0.92f);
				colors[ImGuiCol_Border] = ImVec4(0.5f, 0.5f, 0.5f, 0.7f);
				colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
				colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
				colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
				colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
				colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
				colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
				colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
				colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
				colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
				colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
				colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
				colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
				colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
				colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
				colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
				colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
				colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
				colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
				colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
				colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
				colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
				colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
				colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
				colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
				colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
				colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
				colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
				colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
				colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
				colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
				colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
				colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
				colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
				colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
				colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
				colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
				colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
				colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
				colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
				colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
				colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
				colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
				colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
				colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
				colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
				colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
				colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
				colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
				colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

				ImGuiStyle& style = ImGui::GetStyle();
				style.WindowPadding = ImVec2(8.00f, 8.00f);
				style.FramePadding = ImVec2(5.00f, 2.00f);
				style.CellPadding = ImVec2(6.00f, 6.00f);
				style.ItemSpacing = ImVec2(6.00f, 6.00f);
				style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
				style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
				style.IndentSpacing = 25;
				style.ScrollbarSize = 15;
				style.GrabMinSize = 10;
				style.WindowBorderSize = 1;
				style.ChildBorderSize = 1;
				style.PopupBorderSize = 1;
				style.FrameBorderSize = 1;
				style.TabBorderSize = 1;
				style.WindowRounding = 7;
				style.ChildRounding = 4;
				style.FrameRounding = 3;
				style.PopupRounding = 4;
				style.ScrollbarRounding = 9;
				style.GrabRounding = 3;
				style.LogSliderDeadzone = 4;
				style.TabRounding = 4;

				for (int i = 0; i < (int)ImGuiCol_COUNT; ++i)
					style.Colors[i] = colors[i];
			}
			void LoadFontFile()
			{
				ImGuiIO& io = ImGui::GetIO();
				ImFontConfig config;

				ImFont* defaultFont = io.Fonts->AddFontDefault();
				ImFont* boldFont = io.Fonts->AddFontFromFileTTF("D:\\Visual_Studio_src\\JinEngine\\JinEngine\\EngineResource\\Font\\NotoSerifKR-Bold.otf",
					48,
					nullptr,
					io.Fonts->GetGlyphRangesKorean());
				ImFont* semiBoldFont = io.Fonts->AddFontFromFileTTF("D:\\Visual_Studio_src\\JinEngine\\JinEngine\\EngineResource\\Font\\NotoSerifKR-SemiBold.otf",
					20,
					nullptr,
					io.Fonts->GetGlyphRangesKorean());
				ImFont* regularFont = io.Fonts->AddFontFromFileTTF("D:\\Visual_Studio_src\\JinEngine\\JinEngine\\EngineResource\\Font\\NotoSerifKR-Regular.otf",
					20,
					nullptr,
					io.Fonts->GetGlyphRangesKorean());
				ImFont* mediumFont = io.Fonts->AddFontFromFileTTF("D:\\Visual_Studio_src\\JinEngine\\JinEngine\\EngineResource\\Font\\NotoSerifKR-Medium.otf",
					20,
					nullptr,
					io.Fonts->GetGlyphRangesKorean());

				fontMap.clear();

				fontMap.emplace(J_EDITOR_FONT_TYPE::DEFAULT, std::unordered_map<Core::J_LANGUAGE_TYPE, ImFont*>());
				fontMap.emplace(J_EDITOR_FONT_TYPE::BOLD, std::unordered_map<Core::J_LANGUAGE_TYPE, ImFont*>());
				fontMap.emplace(J_EDITOR_FONT_TYPE::SEMI_BOLD, std::unordered_map<Core::J_LANGUAGE_TYPE, ImFont*>());
				fontMap.emplace(J_EDITOR_FONT_TYPE::REGULAR, std::unordered_map<Core::J_LANGUAGE_TYPE, ImFont*>());
				fontMap.emplace(J_EDITOR_FONT_TYPE::MEDIUM, std::unordered_map<Core::J_LANGUAGE_TYPE, ImFont*>());

				fontMap.find(J_EDITOR_FONT_TYPE::DEFAULT)->second.emplace(Core::J_LANGUAGE_TYPE::KOREAN, defaultFont);
				fontMap.find(J_EDITOR_FONT_TYPE::DEFAULT)->second.emplace(Core::J_LANGUAGE_TYPE::ENGLISH, defaultFont);

				fontMap.find(J_EDITOR_FONT_TYPE::BOLD)->second.emplace(Core::J_LANGUAGE_TYPE::KOREAN, boldFont);
				fontMap.find(J_EDITOR_FONT_TYPE::BOLD)->second.emplace(Core::J_LANGUAGE_TYPE::ENGLISH, boldFont);

				fontMap.find(J_EDITOR_FONT_TYPE::SEMI_BOLD)->second.emplace(Core::J_LANGUAGE_TYPE::KOREAN, semiBoldFont);
				fontMap.find(J_EDITOR_FONT_TYPE::SEMI_BOLD)->second.emplace(Core::J_LANGUAGE_TYPE::ENGLISH, semiBoldFont);

				fontMap.find(J_EDITOR_FONT_TYPE::REGULAR)->second.emplace(Core::J_LANGUAGE_TYPE::KOREAN, regularFont);
				fontMap.find(J_EDITOR_FONT_TYPE::REGULAR)->second.emplace(Core::J_LANGUAGE_TYPE::ENGLISH, regularFont);

				fontMap.find(J_EDITOR_FONT_TYPE::MEDIUM)->second.emplace(Core::J_LANGUAGE_TYPE::KOREAN, mediumFont);
				fontMap.find(J_EDITOR_FONT_TYPE::MEDIUM)->second.emplace(Core::J_LANGUAGE_TYPE::ENGLISH, mediumFont);
			}
		};
		namespace Private
		{
			static std::unique_ptr<JImGui> jImgui;
		}

		JVector2<int> JImGuiImpl::GetAlphabetSize()noexcept
		{
			return Private::jImgui->alphabetSize;
		}
		uint JImGuiImpl::GetTextBuffRange()noexcept
		{
			return Private::jImgui->textBufRange;
		}
		void JImGuiImpl::SetAlphabetSize()noexcept
		{
			Private::jImgui->alphabetSize = ImGui::CalcTextSize("0");
		}
		void JImGuiImpl::SetFont(const J_EDITOR_FONT_TYPE fontType)noexcept
		{
			Private::jImgui->fontType = fontType;
		}
		void JImGuiImpl::PushFont()noexcept
		{
			ImGui::PushFont(Private::jImgui->fontMap.find(Private::jImgui->fontType)->second.find(JApplicationVariable::GetEngineLanguageType())->second);
		}
		void JImGuiImpl::PopFont()noexcept
		{
			ImGui::PopFont();
		}
		float JImGuiImpl::GetButtonDeepFactor()noexcept
		{
			return 0.1f;
		}
		float JImGuiImpl::GetTreeDeepFactor()noexcept
		{
			return 0.1f;
		}
		ImVec4 JImGuiImpl::GetColor(ImGuiCol_ flag)noexcept
		{
			ImGuiStyle& style = ImGui::GetStyle();
			return style.Colors[flag];
		}
		ImU32 JImGuiImpl::GetUColor(ImGuiCol_ flag)noexcept
		{
			ImGuiStyle& style = ImGui::GetStyle();
			return IM_COL32(style.Colors[flag].x * 255,
				style.Colors[flag].y * 255,
				style.Colors[flag].z * 255,
				style.Colors[flag].w * 255);
		}
		void JImGuiImpl::SetColorToDeep(ImGuiCol_ flag, float factor)noexcept
		{
			ImGuiStyle& style = ImGui::GetStyle();
			style.Colors[flag] = ImVec4(style.Colors[flag].x - factor, style.Colors[flag].y - factor,
				style.Colors[flag].z - factor, style.Colors[flag].w - factor);
		}
		void JImGuiImpl::SetColor(const ImVec4& color, ImGuiCol_ flag)noexcept
		{
			ImGuiStyle& style = ImGui::GetStyle();
			style.Colors[flag] = color;
		}
		void JImGuiImpl::SetColorToDefault(ImGuiCol_ flag)noexcept
		{
			ImGuiStyle& style = ImGui::GetStyle();
			style.Colors[flag] = Private::jImgui->colors[flag];
		}
		void JImGuiImpl::SetAllColorToDeep(float factor)noexcept
		{
			ImGuiStyle& style = ImGui::GetStyle();
			for (uint i = 0; i < ImGuiCol_COUNT; ++i)
			{
				style.Colors[i] = ImVec4(style.Colors[i].x - factor, style.Colors[i].y - factor,
					style.Colors[i].z - factor, style.Colors[i].w - factor);
			}
		}
		void JImGuiImpl::SetAllColorToDefault()noexcept
		{
			ImGuiStyle& style = ImGui::GetStyle();
			for (uint i = 0; i < ImGuiCol_COUNT; ++i)
				style.Colors[i] = Private::jImgui->colors[i];
		}
		void JImGuiImpl::ActivateButtonColor()noexcept
		{
			JImGuiImpl::SetColorToDefault(ImGuiCol_Text);
			JImGuiImpl::SetColorToDefault(ImGuiCol_Button);
			JImGuiImpl::SetColorToDefault(ImGuiCol_ButtonHovered);
			JImGuiImpl::SetColorToDefault(ImGuiCol_ButtonActive);
		}
		void JImGuiImpl::DeActivateButtonColor()noexcept
		{
			JImGuiImpl::SetColor(JVector4<float>(0.5f, 0.5f, 0.5f, 1.0f), ImGuiCol_Text);
			JImGuiImpl::SetColorToDeep(ImGuiCol_Button, 0.2f);
			JImGuiImpl::SetColor(JVector4<float>(0, 0, 0, 0), ImGuiCol_ButtonHovered);
			JImGuiImpl::SetColor(JVector4<float>(0, 0, 0, 0), ImGuiCol_ButtonActive);
		}
		JVector2<int> JImGuiImpl::GetDisplaySize()noexcept
		{
			return Private::jImgui->displaySize;
		}
		JVector2<int> JImGuiImpl::GetWindowSize()noexcept
		{
			return Private::jImgui->windowSize;
		}
		JVector2<int> JImGuiImpl::GetClientWindowPos()noexcept
		{
			return Private::jImgui->clientPos;
		}
		JVector2<int> JImGuiImpl::GetClientWindowSize()noexcept
		{
			return Private::jImgui->clientSize;
		}
		JVector2<float> JImGuiImpl::GetGuiWindowPos()noexcept
		{
			return ImGui::GetWindowPos();
		}
		JVector2<float> JImGuiImpl::GetGuiWindowSize()noexcept
		{
			return ImGui::GetWindowSize();
		}
		JVector2<int> JImGuiImpl::GetGuiWidnowContentsSize()noexcept
		{
			const JVector2<float> windowPaddig = ImGui::GetStyle().WindowPadding;
			const float windowBorder = ImGui::GetStyle().WindowBorderSize;
			return ImGui::GetWindowSize() - (windowPaddig * 2) - JVector2<float>(windowBorder * 2);
		} 
		ImGuiWindow* JImGuiImpl::GetGuiWindow(const ImGuiID id)noexcept
		{
			ImGuiContext* cont = ImGui::GetCurrentContext();
			const int wndCount = (int)cont->Windows.size();
			for (int i = 0; i < wndCount; ++i)
			{
				if (cont->Windows[i]->ID == id)
					return cont->Windows[i];
			}
			return nullptr;
		}
		ImGuiWindow* JImGuiImpl::GetGuiWindow(const ImGuiID id, _Out_ int& order)noexcept
		{
			order = -1;
			ImGuiContext* cont = ImGui::GetCurrentContext();
			const int wndCount = (int)cont->Windows.size();
			for (int i = 0; i < wndCount; ++i)
			{
				if (cont->Windows[i]->ID == id)
				{
					order = i;
					return cont->Windows[i];
				}
			}
			return nullptr;
		}
		bool JImGuiImpl::BeginWindow(const std::string& name, bool* p_open, ImGuiWindowFlags flags)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::WINDOW);
			if (p_open)
				return ImGui::Begin(name.c_str(), p_open, flags);
			else
				return ImGui::Begin(name.c_str(), 0, flags);
		}
		void JImGuiImpl::EndWindow()
		{
			ImGui::End();
		}
		bool JImGuiImpl::BeginChildWindow(const std::string& name, const JVector2<float>& windowSize, bool border, ImGuiWindowFlags extra_flags)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::CHILD_WINDOW);
			return ImGui::BeginChild(name.c_str(), windowSize, border, extra_flags);
		}
		void JImGuiImpl::EndChildWindow()
		{
			ImGui::EndChild();
		}
		bool JImGuiImpl::BeginPopup(const std::string& name, ImGuiPopupFlags flags)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::POPUP);
			return ImGui::BeginPopup(name.c_str(), flags);
		}
		void JImGuiImpl::EndPopup()
		{
			ImGui::EndPopup();
		}
		void JImGuiImpl::Text(const std::string& text)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::TEXT);
			ImGui::Text(text.c_str());
		}
		bool JImGuiImpl::CheckBox(const std::string& checkName, bool& v)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::CHECKBOX);
			return ImGui::Checkbox(checkName.c_str(), &v);
		}
		bool JImGuiImpl::Button(const std::string& btnName, const JVector2<float>& jVec2)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::BUTTON);
			return ImGui::Button(btnName.c_str(), jVec2);
		}
		bool JImGuiImpl::TreeNodeEx(const std::string& nodeName, ImGuiTreeNodeFlags flags)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::TREE);
			return ImGui::TreeNodeEx(nodeName.c_str(), flags);
		}
		void JImGuiImpl::TreePop()
		{
			ImGui::TreePop();
		}
		bool JImGuiImpl::Selectable(const std::string& name, bool* pSelected, ImGuiSelectableFlags flags, const JVector2<float>& sizeArg)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::SELECTABLE);
			if (pSelected == nullptr)
				return ImGui::Selectable(name.c_str(), false, flags, sizeArg);
			else
				return ImGui::Selectable(name.c_str(), pSelected, flags, sizeArg);
		}
		bool JImGuiImpl::Selectable(const std::string& name, bool selected, ImGuiSelectableFlags flags, const JVector2<float>& sizeArg)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::SELECTABLE);
			return ImGui::Selectable(name.c_str(), selected, flags, sizeArg);
		}
		bool JImGuiImpl::InputText(const std::string& name, std::string& buff, ImGuiInputTextFlags flags, ImGuiInputTextCallback txtCallback, void* userData)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::INPUT_DATA);
			return ImGui::InputText(name.c_str(), &buff[0], buff.size(), flags, txtCallback, userData);
		}
		bool JImGuiImpl::InputText(const std::string& name, std::string& buff, std::string& result, const std::string& hint, ImGuiInputTextFlags flags, ImGuiInputTextCallback txtCallback, void* userData)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::INPUT_DATA);
			bool isInputEnd = false;
			if (result.size() == 0)
				isInputEnd = ImGui::InputTextWithHint(name.c_str(), hint.c_str(), &buff[0], buff.size(), flags, txtCallback, userData);
			else
				isInputEnd = ImGui::InputText(name.c_str(), &buff[0], buff.size(), flags, txtCallback, userData);
			if (isInputEnd)
				result = JCUtil::EraseSideChar(buff, '\0');
			return isInputEnd;
		}
		bool JImGuiImpl::InputMultilineText(const std::string& name, std::string& buff, std::string& result, const JVector2<float>& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback txtCallback, void* userData)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::INPUT_DATA);
			bool isInputEnd = ImGui::InputTextMultiline(name.c_str(), &buff[0], buff.size(), size, flags, txtCallback, userData);
			if (isInputEnd)
				result = JCUtil::EraseSideChar(buff, '\0');
			return isInputEnd;
		}
		bool JImGuiImpl::InputInt(const std::string& name, int* value, ImGuiInputTextFlags flags, int step, int stepFast)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::INPUT_DATA);
			return ImGui::InputInt(name.c_str(), value, step, stepFast, flags);
		}
		bool JImGuiImpl::InputFloat(const std::string& name, float* value, ImGuiInputTextFlags flags, const char* format, float step, float stepFast)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::INPUT_DATA);
			return ImGui::InputFloat(name.c_str(), value, step, stepFast, format, flags);
		}
		bool JImGuiImpl::SliderInt(const std::string& name, int* value, int vMin, int vMax, const char* format, ImGuiSliderFlags flags)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::SLIDER);
			return ImGui::SliderInt(name.c_str(), value, vMin, vMax, format, flags);
		}
		bool JImGuiImpl::SliderFloat(const std::string& name, float* value, float vMin, float vMax, const char* format, ImGuiSliderFlags flags)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::SLIDER);
			return ImGui::SliderFloat(name.c_str(), value, vMin, vMax, format, flags);
		}
		bool JImGuiImpl::VSliderInt(const std::string& name, JVector2<float> size, int* value, int vMin, int vMax, const char* format, ImGuiSliderFlags flags)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::SLIDER);
			return ImGui::VSliderInt(name.c_str(), size, value, vMin, vMax, format, flags);
		}
		bool JImGuiImpl::VSliderFloat(const std::string& name, JVector2<float> size, float* value, float vMin, float vMax, const char* format, ImGuiSliderFlags flags)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::SLIDER);
			return ImGui::VSliderFloat(name.c_str(), size, value, vMin, vMax, format, flags);
		}
		bool JImGuiImpl::BeginTabBar(const std::string& name, const ImGuiTabBarFlags flags)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::TAB_BAR);
			return ImGui::BeginTabBar(name.c_str(), flags);
		}
		void JImGuiImpl::EndTabBar()
		{
			ImGui::EndTabBar();
		}
		bool JImGuiImpl::BeginTable(const std::string& name, int columnCount, ImGuiTableFlags flags, const JVector2<float> outerSize, float innerWidth)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::TABLE);
			return ImGui::BeginTable(name.c_str(), columnCount, flags, outerSize, innerWidth);
		}
		void JImGuiImpl::EndTable()
		{
			ImGui::EndTable();
		}
		void JImGuiImpl::TableSetupColumn(const std::string& name, ImGuiTableColumnFlags flags, float init_Width_or_Weight, ImGuiID userId)
		{
			ImGui::TableSetupColumn(name.c_str(), flags, init_Width_or_Weight, userId);
		}
		void JImGuiImpl::TableHeadersRow()
		{
			ImGui::TableHeadersRow();
		}
		void JImGuiImpl::TableNextRow()
		{
			ImGui::TableNextRow();
		}
		void JImGuiImpl::TableSetColumnIndex(int index)
		{
			ImGui::TableSetColumnIndex(index);
		}
		bool JImGuiImpl::TabItemButton(const std::string& name, const ImGuiTabItemFlags flags)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::TAB_ITEM);
			return ImGui::TabItemButton(name.c_str(), flags);
		}
		bool JImGuiImpl::BeginMainMenuBar()
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::MENU_BAR);
			return ImGui::BeginMainMenuBar();
		}
		void JImGuiImpl::EndMainMenuBar()
		{
			ImGui::EndMainMenuBar();
		}
		bool JImGuiImpl::BeginMenuBar()
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::MENU_BAR);
			return ImGui::BeginMenuBar();
		}
		void JImGuiImpl::EndMenuBar()
		{
			ImGui::EndMenuBar();
		}
		bool JImGuiImpl::BeginMenu(const std::string& name, bool enable)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::MENU);
			return ImGui::BeginMenu(name.c_str(), enable);
		}
		void JImGuiImpl::EndMenu()
		{
			ImGui::EndMenu();
		}
		bool JImGuiImpl::MenuItem(const std::string& name, bool selected, bool enabled)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::MENU_ITEM);
			return ImGui::MenuItem(name.c_str(), NULL, selected, enabled);
		}
		bool JImGuiImpl::MenuItem(const std::string& name, const std::string& shortcut, bool selected, bool enabled)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::MENU_ITEM);
			return ImGui::MenuItem(name.c_str(), shortcut.c_str(), selected, enabled);
		}
		bool JImGuiImpl::BeginCombo(const std::string& name, const char* previewValue, ImGuiComboFlags flags)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::COMBOBOX);
			return ImGui::BeginCombo(name.c_str(), previewValue, flags);
		}
		void JImGuiImpl::EndCombo()
		{
			ImGui::EndCombo();
		}
		void JImGuiImpl::Image(Graphic::JGraphicResourceHandleInterface& handle,
			const JVector2<float>& size,
			const JVector2<float>& uv0,
			const JVector2<float>& uv1,
			const JVector4<float>& tintCol,
			const JVector4<float>& borderCol)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::IMAGE);
			ImGui::Image((ImTextureID)Private::jImgui->GetGraphicGpuSrvHandle(handle).ptr, size, uv0, uv1, tintCol, borderCol);
		}
		bool JImGuiImpl::ImageButton(const std::string name,
			Graphic::JGraphicResourceHandleInterface& handle,
			const JVector2<float>& size,
			const JVector2<float>& uv0,
			const JVector2<float>& uv1,
			float framePadding,
			const JVector4<float>& bgCol,
			const JVector4<float>& tintCol)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::IMAGE);
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::BUTTON);

			const ImVec2 padding = (framePadding >= 0) ? ImVec2((float)framePadding, (float)framePadding) : ImGui::GetStyle().FramePadding;
			return ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID(name.c_str()),
				(ImTextureID)Private::jImgui->GetGraphicGpuSrvHandle(handle).ptr,
				size,
				uv0, uv1,
				padding,
				bgCol, tintCol);
		}
		void JImGuiImpl::AddImage(Graphic::JGraphicResourceHandleInterface& handle,
			const JVector2<float>& pMin,
			const JVector2<float>& pMax,
			bool isBack,
			ImU32 color,
			const JVector2<float>& uvMin,
			const JVector2<float>& uvMax)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::IMAGE);
			if (isBack)
				ImGui::GetBackgroundDrawList()->AddImage((ImTextureID)Private::jImgui->GetGraphicGpuSrvHandle(handle).ptr, pMin, pMax, uvMin, uvMax, color);
			else
				ImGui::GetForegroundDrawList()->AddImage((ImTextureID)Private::jImgui->GetGraphicGpuSrvHandle(handle).ptr, pMin, pMax, uvMin, uvMax, color);
		}
		bool JImGuiImpl::ImageSelectable(const std::string name,
			Graphic::JGraphicResourceHandleInterface& handle,
			bool& pressed,
			bool changeValueIfPreesd,
			const JVector2<float>& size)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::IMAGE);
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::SELECTABLE);

			const JVector2<float> preCursor = ImGui::GetCursorPos();
			const bool isPress = JImGuiImpl::Selectable(name, false, ImGuiSelectableFlags_AllowItemOverlap, size);
			ImGui::SetCursorPos(preCursor);
			ImGui::Image((ImTextureID)Private::jImgui->GetGraphicGpuSrvHandle(handle).ptr, size);

			if (isPress && changeValueIfPreesd)
				pressed = !pressed;

			return isPress;
		}
		bool JImGuiImpl::Switch(const std::string& name,
			bool& pressed,
			bool changeValueIfPreesd,
			const JVector2<float>& sizeArg)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::SWITCH);
			if (pressed)
				JImGuiImpl::SetColorToDeep(ImGuiCol_Header, -JImGuiImpl::GetButtonDeepFactor());
			const bool isPress = ImGui::Button(name.c_str(), sizeArg);
			if (pressed)
				JImGuiImpl::SetColorToDeep(ImGuiCol_Header, JImGuiImpl::GetButtonDeepFactor());
			if (isPress && changeValueIfPreesd)
				pressed = !pressed;
			return isPress;
		}
		bool JImGuiImpl::ImageSwitch(const std::string name,
			Graphic::JGraphicResourceHandleInterface& handle,
			bool& pressed,
			bool changeValueIfPreesd,
			const JVector2<float>& size,
			const ImU32 bgColor,
			const ImU32 bgDelta,
			const ImU32 frameColor,
			const float frameThickness)
		{
			ImU32 finalBgColor = bgColor;
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::IMAGE);
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::SWITCH);
			if (pressed)
				finalBgColor -= bgDelta;

			const JVector2<float> pos = ImGui::GetCurrentWindow()->DC.CursorPos;
			DrawRectFilledColor(pos, size, finalBgColor, true);
			if (frameThickness > 0)
				DrawRectFrame(pos, size, frameColor, frameThickness, true);

			const JVector2<float> preCursor = ImGui::GetCursorPos();
			const bool isPress = JImGuiImpl::Selectable(name, false, ImGuiSelectableFlags_AllowItemOverlap, size);
			ImGui::SetCursorPos(preCursor);
			ImGui::Image((ImTextureID)Private::jImgui->GetGraphicGpuSrvHandle(handle).ptr, size);

			if (isPress && changeValueIfPreesd)
				pressed = !pressed;

			return isPress;
		}
		void JImGuiImpl::DrawRectFilledMultiColor(const JVector2<float>& pos, const JVector2<float>& size, const ImU32 color, const ImU32 colorDelta, const bool useRestoreCursorPos)noexcept
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::DRAW);
			JVector2<float> nowCursor = ImGui::GetCursorPos();
			JVector2<float> bboxMin = JVector2<float>(pos.x, pos.y);
			JVector2<float> bboxMax = pos + size;
			ImGui::GetWindowDrawList()->AddRectFilledMultiColor(bboxMin, bboxMax,
				color + colorDelta,
				color - colorDelta,
				color + colorDelta,
				color - colorDelta);
			if (useRestoreCursorPos)
				ImGui::SetCursorPos(nowCursor);
		}
		void JImGuiImpl::DrawRectFilledColor(const JVector2<float>& pos, const JVector2<float>& size, const ImU32 color, const bool useRestoreCursorPos)noexcept
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::DRAW);
			JVector2<float> nowCursor = ImGui::GetCursorPos();
			JVector2<float> bboxMin = JVector2<float>(pos.x, pos.y);
			JVector2<float> bboxMax = pos + size;
			ImGui::GetWindowDrawList()->AddRectFilled(bboxMin, bboxMax, color);
			if (useRestoreCursorPos)
				ImGui::SetCursorPos(nowCursor);
		}
		void JImGuiImpl::DrawRectFrame(const JVector2<float>& pos,
			const JVector2<float>& size,
			const float thickness,
			const ImU32 color,
			const bool useRestoreCursorPos)noexcept
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::DRAW);
			JVector2<float> nowCursor = ImGui::GetCursorPos();
			JVector2<float> bboxMin = JVector2<float>(pos.x, pos.y);
			JVector2<float> bboxMax = pos + size;

			ImGui::GetWindowDrawList()->AddRect(bboxMin, bboxMax, color, 0, 0, thickness);
			if (useRestoreCursorPos)
				ImGui::SetCursorPos(nowCursor);
		}
		void JImGuiImpl::DrawToolTipBox(const std::string& uniqueLabel,
			const std::string& tooltip,
			const JVector2<float>& pos,
			const JVector2<float>& padding,
			const bool useRestoreCursorPos)
		{
			const JVector2<float> nowCursor = ImGui::GetCursorPos();
			const JVector2<float> boxSize = ImGui::CalcTextSize(tooltip.c_str()) + (padding * 2);
			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(boxSize);
			JImGuiImpl::BeginWindow(uniqueLabel, 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

			ImGui::SetCursorScreenPos(pos + padding);
			JImGuiImpl::Text(tooltip);
			JImGuiImpl::EndWindow();
			if (useRestoreCursorPos)
				ImGui::SetCursorPos(nowCursor);
		}
		void JImGuiImpl::DrawToolTipBox(const std::string& uniqueLabel,
			const std::string& tooltip,
			const JVector2<float>& pos,
			const float maxWidth,
			const JVector2<float>& padding,
			const J_EDITOR_ALIGN_TYPE alignType,
			const bool useRestoreCursorPos)
		{
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::DRAW);
			Private::jImgui->AddActWidgetCount(Private::IMGUI_WIDGET::TEXT);

			const JVector2<float> nowCursor = ImGui::GetCursorPos();
			JEditorTextAlignCalculator textAlignCal;
			textAlignCal.Update(tooltip, JVector2<float>(maxWidth, 0), false);

			const std::string alignedTooltip = textAlignCal.Aligned(alignType);
			const JVector2<float> boxSize = ImGui::CalcTextSize(alignedTooltip.c_str()) + (padding * 2);
			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(boxSize);
			JImGuiImpl::BeginWindow(uniqueLabel, 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

			ImGui::SetCursorScreenPos(pos + padding);
			JImGuiImpl::Text(alignedTooltip);
			JImGuiImpl::EndWindow();
			if (useRestoreCursorPos)
				ImGui::SetCursorPos(nowCursor);
		}
		bool JImGuiImpl::IsLeftMouseClicked()noexcept
		{
			return Private::jImgui->mouseClick[0];
		}
		bool JImGuiImpl::IsRightMouseClicked()noexcept
		{
			return Private::jImgui->mouseClick[1];
		}
		bool JImGuiImpl::IsMiddleMouseClicked()noexcept
		{
			return Private::jImgui->mouseClick[2];
		}
		bool JImGuiImpl::IsMouseInRect(const JVector2<float>& position, const JVector2<float>& size)noexcept
		{
			ImVec2 mousePos = ImGui::GetMousePos();
			if (mousePos.x >= position.x && mousePos.x <= position.x + size.x &&
				mousePos.y >= position.y && mousePos.y <= position.y + size.y)
				return true;
			else
				return false;
		}
		void JImGuiImpl::SetMouseClick(const ImGuiMouseButton btn, const bool value)noexcept
		{
			Private::jImgui->mouseClick[btn] = value;
		}
		float JImGuiImpl::GetSliderPosX(bool hasScrollbar)noexcept
		{
			float posX = ImGui::GetWindowSize().x - JImGuiImpl::GetSliderWidth() -
				ImGui::GetStyle().FramePadding.x - ImGui::GetStyle().ItemSpacing.x;

			if (hasScrollbar)
				return posX - ImGui::GetStyle().ScrollbarSize;
			else
				return posX;
		}
		float JImGuiImpl::GetSliderWidth()noexcept
		{
			//float sliderOffset = ImGui::GetStyle().ScrollbarSize + (ImGui::GetStyle().ItemSpacing.x * 2) + (JImGuiImpl::GetAlphabetSize().x * 4);
			return 	GetClientWindowSize().x * 0.05f;
		}
		void JImGuiImpl::SetTooltip(const std::string& message)noexcept
		{
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
				ImGui::SetTooltip(message.c_str());
		}
		float JImGuiImpl::GetFrameRounding()noexcept
		{
			return ImGui::GetStyle().FrameRounding;
		}
		void JImGuiImpl::SetFrameRounding(float value)noexcept
		{
			ImGui::GetStyle().FrameRounding = value;
		}
		bool JImGuiImpl::IsFullScreen()noexcept
		{
			return JWindow::Instance().IsFullScreen();
		}
		bool JImGuiImpl::IsWindowPadding()noexcept
		{
			return Private::jImgui->optWindowPadding;
		}
		bool JImGuiImpl::IsEnablePopup()noexcept
		{
			return Private::jImgui->enablePopup;
		}
		bool JImGuiImpl::IsEnableSelector()noexcept
		{
			return Private::jImgui->enableSelector;
		}
		void JImGuiImpl::StartEditorUpdate()
		{
		}
		void JImGuiImpl::MouseUpdate()
		{
			JImGuiImpl::SetMouseClick(0, ImGui::IsMouseClicked(0));
			JImGuiImpl::SetMouseClick(1, ImGui::IsMouseClicked(1));
			JImGuiImpl::SetMouseClick(2, ImGui::IsMouseClicked(2));
		}
		void JImGuiImpl::EndEditorUpdate()
		{
			Private::jImgui->ClearActWidgetCount();
		}
		void JImGuiImpl::Initialize()
		{
			if (Private::jImgui == nullptr)
				Private::jImgui = std::make_unique<JImGui>();
		}
		void JImGuiImpl::Clear()
		{
			Private::jImgui.reset();
		}
	}
}