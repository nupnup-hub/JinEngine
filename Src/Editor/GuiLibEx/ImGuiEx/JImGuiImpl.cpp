#include"JImGuiImpl.h" 
#include"../../../Application/JApplicationVariable.h"

#include"../../../Core/Identity/JIdentifier.h"
#include"../../../Core/JDataType.h"  
#include"../../../Core/Platform/JLanguageType.h"

#include"../../../Window/JWindows.h"
#include"../../../Window/JWindowEventType.h" 
#include"../../../Graphic/JGraphic.h"
#include"../../../Graphic/JGraphicTexture.h"
#include"../../../Graphic/JGraphicTextureUserInterface.h"

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
		class JImGui : public Core::JEventListener<size_t, Window::J_WINDOW_EVENT>,
			public Graphic::JGraphicTextureUserInterface
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
			JVector2<int> textSize;
			JVector2<int> textSizeOffset;
		public:
			FontMap fontMap;
			J_EDITOR_FONT_TYPE fontType;
		public:
			ImVec4 colors[ImGuiCol_COUNT];
		public:
			bool optWindowPadding;
		public:
			std::bitset<3> mouseClick;
			bool isDrag;
		public:
			bool enablePopup = true;
			bool enableSelector = true;
		public:
			static constexpr float minRate = 7.5f;
		public:
			//widget Count
			uint windowCount;
			uint childWindowCount;
			uint textCount;
			uint checkBoxCount;
			uint buttonCount;
			uint imageButtonCount;
			uint treeNodeCount;
			uint selectableCount;
			uint inputDataCount;
			uint sliderCount;
			uint tabBarCount;
			uint tabItemCount;
			uint tableCount;
			uint menuBarCount;
			uint menuCount;
			uint menuItemCount;
			uint comboCount;
			uint textureCount;
		public:
			JImGui()
				:guid(JCUtil::CalculateGuid(typeid(JImGui).name()))
			{
				this->AddEventListener(*JWindow::Instance().EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_RESIZE);
				this->AddEventListener(*JWindow::Instance().EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_CLOSE);

				IMGUI_CHECKVERSION();
				ImGui::CreateContext();
				ImGuiIO& io = ImGui::GetIO(); (void)io;
				//io.ConfigWindowsResizeFromEdges = true;
				//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
				//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
				//io.ConfigWindowsResizeFromEdges = true;
				io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
				//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;        // Enable Multi-Viewport / Platform Windows
				LoadDefaultColor();
				LoadFontFile();
				//PushFont();

				ImGuiStyle& style = ImGui::GetStyle();
				//ImGuiStyle& style = ImGui::GetStyle();
				//style.FrameBorderSize = 1; 
				ImGui_ImplWin32_Init(JWindow::Instance().HandleInterface()->GetHandle());
				JGraphic::Instance().EditorInterface()->SetImGuiBackEnd();

				OnResize();
			}
			~JImGui()
			{
				RemoveListener(*JWindow::Instance().EvInterface(), guid);
			}
		public:
			void ClearWidgetCount()
			{
				windowCount = childWindowCount = textCount = checkBoxCount = buttonCount =
					imageButtonCount = treeNodeCount = selectableCount = inputDataCount = sliderCount =
					tabBarCount = tabItemCount = tableCount = menuBarCount = menuCount =
					menuItemCount = comboCount = textureCount = 0;
			}
			uint WidgetSum()
			{
				return windowCount + childWindowCount + textCount + checkBoxCount + buttonCount +
					imageButtonCount + treeNodeCount + selectableCount + inputDataCount + sliderCount +
					tabBarCount + tabItemCount + tableCount + menuBarCount + menuCount +
					menuItemCount + comboCount + textureCount;
			}
		public:
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetGraphicCpuHandle(Graphic::JGraphicTexture& graphicTexture)
			{
				return CallGetCpuHandle(graphicTexture);
			}
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGraphicGpuHandle(Graphic::JGraphicTexture& graphicTexture)
			{
				return CallGetGpuHandle(graphicTexture);
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
				else if (eventType == Window::J_WINDOW_EVENT::WINDOW_CLOSE)
				{
					ImGui_ImplDX12_Shutdown();
					ImGui_ImplWin32_Shutdown();
				}
			}
		private:
			void LoadDefaultColor()
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
			void LoadFontFile()
			{
				ImGuiIO& io = ImGui::GetIO();
				ImFontConfig config;
				 
				ImFont* defaultFont = io.Fonts->AddFontDefault();
				ImFont* boldFont = io.Fonts->AddFontFromFileTTF("D:\\Visual_Studio_src\\GameEngine\\JinEngine\\Font\\NotoSerifKR-Bold.otf",
					48,
					nullptr,
					io.Fonts->GetGlyphRangesKorean());
				ImFont* semiBoldFont = io.Fonts->AddFontFromFileTTF("D:\\Visual_Studio_src\\GameEngine\\JinEngine\\Font\\NotoSerifKR-SemiBold.otf",
					20,
					nullptr,
					io.Fonts->GetGlyphRangesKorean());
				ImFont* regularFont = io.Fonts->AddFontFromFileTTF("D:\\Visual_Studio_src\\GameEngine\\JinEngine\\Font\\NotoSerifKR-Regular.otf",
					20,
					nullptr,
					io.Fonts->GetGlyphRangesKorean());
				ImFont* mediumFont = io.Fonts->AddFontFromFileTTF("D:\\Visual_Studio_src\\GameEngine\\JinEngine\\Font\\NotoSerifKR-Medium.otf",
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
		namespace
		{
			static std::unique_ptr<JImGui> jImgui;
			static JVector2<float> Convert(const ImVec2 imV2)
			{
				return JVector2<float>(imV2.x, imV2.y);
			}
		}
		JVector2<int> JImGuiImpl::GetTextSize()noexcept
		{
			return jImgui->textSize;
		}
		JVector2<int> JImGuiImpl::GetTextSizeOffset()noexcept
		{
			return jImgui->textSizeOffset;
		}
		void JImGuiImpl::SetTextSize()noexcept
		{
			jImgui->textSize.x = ImGui::CalcTextSize("d").x;
			jImgui->textSize.y = ImGui::CalcTextSize("d").y;
		}
		void JImGuiImpl::SetTextSizeOffset(JVector2<int> offset)noexcept
		{
			jImgui->textSizeOffset = offset;
		}
		void JImGuiImpl::SetFont(const J_EDITOR_FONT_TYPE fontType)noexcept
		{
			jImgui->fontType = fontType;
		}
		void JImGuiImpl::PushFont()noexcept
		{
			ImGui::PushFont(jImgui->fontMap.find(jImgui->fontType)->second.find(JApplicationVariable::GetEngineLanguageType())->second);
		}
		void JImGuiImpl::PopFont()noexcept
		{
			ImGui::PopFont();
		}

		ImVec4 JImGuiImpl::GetColor(ImGuiCol_ flag)noexcept
		{
			ImGuiStyle& style = ImGui::GetStyle();
			return style.Colors[flag];
		}
		void JImGuiImpl::SetColorToDeep(ImGuiCol_ flag, float factor)noexcept
		{
			ImGuiStyle& style = ImGui::GetStyle();
			style.Colors[flag] = ImVec4(jImgui->colors[flag].x - factor, jImgui->colors[flag].y - factor,
				jImgui->colors[flag].z - factor, jImgui->colors[flag].w - factor);
		}
		void JImGuiImpl::SetColor(const ImVec4& color, ImGuiCol_ flag)noexcept
		{
			ImGuiStyle& style = ImGui::GetStyle();
			style.Colors[flag] = color;
		}
		void JImGuiImpl::SetColorToDefault(ImGuiCol_ flag)noexcept
		{
			ImGuiStyle& style = ImGui::GetStyle();
			style.Colors[flag] = jImgui->colors[flag];
		}
		void JImGuiImpl::SetAllColorToDefault()noexcept
		{
			ImGuiStyle& style = ImGui::GetStyle();
			for (uint i = 0; i < ImGuiCol_COUNT; ++i)
				style.Colors[i] = jImgui->colors[i];
		}
		JVector2<int> JImGuiImpl::GetDisplaySize()noexcept
		{
			return jImgui->displaySize;
		}
		JVector2<int> JImGuiImpl::GetWindowSize()noexcept
		{
			return jImgui->windowSize;
		}
		JVector2<int> JImGuiImpl::GetClientWindowPos()noexcept
		{
			return jImgui->clientPos;
		}
		JVector2<int> JImGuiImpl::GetClientWindowSize()noexcept
		{
			return jImgui->clientSize;
		}
		JVector2<float> JImGuiImpl::GetGuiWindowPos()noexcept
		{
			return Convert(ImGui::GetWindowPos());
		}
		JVector2<float> JImGuiImpl::GetGuiWindowSize()noexcept
		{
			return Convert(ImGui::GetWindowSize());
		}
		JVector2<float> JImGuiImpl::GetCursorPos()noexcept
		{
			return JVector2<float>(ImGui::GetCursorPosX(), ImGui::GetCursorPosY());
		}
		bool JImGuiImpl::BeginWindow(const std::string& name, bool* p_open, ImGuiWindowFlags flags)
		{
			++jImgui->windowCount;
			return ImGui::Begin(name.c_str(), p_open, flags);
		}
		void JImGuiImpl::EndWindow()
		{
			ImGui::End();
		}
		bool JImGuiImpl::BeginChildWindow(const std::string& name, const JVector2<float>& windowSize, bool border, ImGuiWindowFlags extra_flags)
		{
			++jImgui->childWindowCount;
			return ImGui::BeginChild(name.c_str(), windowSize, border, extra_flags);
		}
		void JImGuiImpl::EndChildWindow()
		{
			ImGui::EndChild();
		}
		void JImGuiImpl::Text(const std::string& text)
		{
			++jImgui->textCount;
			ImGui::Text(text.c_str());
		}
		bool JImGuiImpl::CheckBox(const std::string& checkName, bool& v)
		{
			++jImgui->checkBoxCount;
			return ImGui::Checkbox(checkName.c_str(), &v);
		}
		bool JImGuiImpl::Button(const std::string& btnName, const JVector2<float>& jVec2)
		{
			++jImgui->buttonCount;
			return ImGui::Button(btnName.c_str(), jVec2);
		}
		bool JImGuiImpl::TreeNodeEx(const std::string& nodeName, ImGuiTreeNodeFlags flags)
		{
			++jImgui->treeNodeCount;
			return ImGui::TreeNodeEx(nodeName.c_str(), flags);
		}
		void JImGuiImpl::TreePop()
		{
			ImGui::TreePop();
		}
		bool JImGuiImpl::Selectable(const std::string& name, bool* pSelected, ImGuiSelectableFlags flags, const JVector2<float>& sizeArg)
		{
			++jImgui->selectableCount;
			if (pSelected == nullptr)
				return ImGui::Selectable(name.c_str(), false, flags, sizeArg);
			else
				return ImGui::Selectable(name.c_str(), pSelected, flags, sizeArg);
		}
		bool JImGuiImpl::InputText(const std::string& name, char* buf, size_t bufSize, ImGuiInputTextFlags flags, ImGuiInputTextCallback txtCallback, void* userData)
		{
			++jImgui->inputDataCount;
			return ImGui::InputText(name.c_str(), buf, bufSize, flags, txtCallback, userData);
		}
		bool JImGuiImpl::InputInt(const std::string& name, int* value, int step, int stepFast, ImGuiInputTextFlags flags)
		{
			++jImgui->inputDataCount;
			return ImGui::InputInt(name.c_str(), value, step, stepFast, flags);
		}
		bool JImGuiImpl::InputFloat(const std::string& name, float* value, float step, float stepFast, const char* format, ImGuiInputTextFlags flags)
		{
			++jImgui->inputDataCount;
			return ImGui::InputFloat(name.c_str(), value, step, stepFast, format, flags);
		}
		bool JImGuiImpl::SliderFloat(const std::string& name, float* value, float vMin, float vMax, const char* format, ImGuiSliderFlags flags)
		{
			++jImgui->sliderCount;
			return ImGui::SliderFloat(name.c_str(), value, vMin, vMax, format, flags);
		}
		bool JImGuiImpl::BeginTabBar(const std::string& name, const ImGuiTabBarFlags flags)
		{
			++jImgui->tabBarCount;
			return ImGui::BeginTabBar(name.c_str(), flags);
		}
		void JImGuiImpl::EndTabBar()
		{
			ImGui::EndTabBar();
		}
		bool JImGuiImpl::BeginTable(const std::string& name, int columnCount, ImGuiTableFlags flags, const JVector2<float> outerSize, float innerWidth)
		{
			++jImgui->tableCount;
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
			++jImgui->tabItemCount;
			return ImGui::TabItemButton(name.c_str(), flags);
		}
		bool JImGuiImpl::BeginMainMenuBar()
		{
			++jImgui->menuBarCount;
			return ImGui::BeginMainMenuBar();
		}
		void JImGuiImpl::EndMainMenuBar()
		{
			ImGui::EndMainMenuBar();
		}
		bool JImGuiImpl::BeginMenuBar()
		{
			++jImgui->menuBarCount;
			return ImGui::BeginMenuBar();
		}
		void JImGuiImpl::EndMenuBar()
		{
			ImGui::EndMenuBar();
		}
		bool JImGuiImpl::BeginMenu(const std::string& name, bool enable)
		{
			++jImgui->menuCount;
			return ImGui::BeginMenu(name.c_str(), enable);
		}
		void JImGuiImpl::EndMenu()
		{
			ImGui::EndMenu();
		}
		bool JImGuiImpl::MenuItem(const std::string& name, bool selected, bool enabled)
		{
			++jImgui->menuItemCount;
			return ImGui::MenuItem(name.c_str(), NULL, selected, enabled);
		}
		bool JImGuiImpl::MenuItem(const std::string& name, const std::string& shortcut, bool selected, bool enabled)
		{
			++jImgui->menuItemCount;
			return ImGui::MenuItem(name.c_str(), shortcut.c_str(), selected, enabled);
		}
		bool JImGuiImpl::BeginCombo(const std::string& name, const char* previewValue, ImGuiComboFlags flags)
		{
			++jImgui->comboCount;
			return ImGui::BeginCombo(name.c_str(), previewValue, flags);
		}
		void JImGuiImpl::EndCombo()
		{
			ImGui::EndCombo();
		}
		void JImGuiImpl::Image(Graphic::JGraphicTexture& graphicTexture,
			const JVector2<float>& size,
			const JVector2<float>& uv0,
			const JVector2<float>& uv1,
			const JVector4<float>& tintCol,
			const JVector4<float>& borderCol)
		{
			ImGui::Image((ImTextureID)jImgui->GetGraphicGpuHandle(graphicTexture).ptr, size, uv0, uv1, tintCol, borderCol);
		}
		bool JImGuiImpl::ImageButton(Graphic::JGraphicTexture& graphicTexture,
			const JVector2<float>& size,
			const JVector2<float>& uv0,
			const JVector2<float>& uv1,
			int framePadding,
			const JVector4<float>& bgCol,
			const JVector4<float>& tintCol)
		{
			return ImGui::ImageButton((ImTextureID)jImgui->GetGraphicGpuHandle(graphicTexture).ptr, size, uv0, uv1, framePadding, bgCol, tintCol);
		}
		void JImGuiImpl::AddImage(Graphic::JGraphicTexture& graphicTexture,
			const JVector2<float>& pMin,
			const JVector2<float>& pMax,
			bool isBack,
			ImU32 color,
			const JVector2<float>& uvMin,
			const JVector2<float>& uvMax)
		{
			if (isBack)
				ImGui::GetBackgroundDrawList()->AddImage((ImTextureID)jImgui->GetGraphicGpuHandle(graphicTexture).ptr, pMin, pMax, uvMin, uvMax, color);
			else
				ImGui::GetForegroundDrawList()->AddImage((ImTextureID)jImgui->GetGraphicGpuHandle(graphicTexture).ptr, pMin, pMax, uvMin, uvMax, color);
		}
		bool JImGuiImpl::IsLeftMouseClicked()noexcept
		{
			return jImgui->mouseClick[0];
		}
		bool JImGuiImpl::IsRightMouseClicked()noexcept
		{
			return jImgui->mouseClick[1];
		}
		bool JImGuiImpl::IsMiddleMouseClicked()noexcept
		{
			return jImgui->mouseClick[2];
		}
		bool JImGuiImpl::IsDraggingMouse()noexcept
		{
			return jImgui->isDrag;
		}
		bool JImGuiImpl::IsMouseInWindow(const JVector2<float>& position, const JVector2<float>& size)noexcept
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
			jImgui->mouseClick[btn] = value;
		}
		void JImGuiImpl::SetMouseDrag(bool value)noexcept
		{
			jImgui->isDrag = value;
		}
		float JImGuiImpl::GetSliderWidth()noexcept
		{
			float sliderOffset = ImGui::GetStyle().ScrollbarSize + (ImGui::GetStyle().ItemSpacing.x * 2) + (JImGuiImpl::GetTextSize().x * 4);
			return ImGui::GetWindowWidth() - sliderOffset;
		}
		bool JImGuiImpl::IsFullScreen()noexcept
		{
			return JWindow::Instance().IsFullScreen();
		}
		bool JImGuiImpl::IsWindowPadding()noexcept
		{
			return jImgui->optWindowPadding;
		}
		bool JImGuiImpl::IsEnablePopup()noexcept
		{
			return jImgui->enablePopup;
		}
		bool JImGuiImpl::IsEnableSelector()noexcept
		{
			return jImgui->enableSelector;
		}
		void JImGuiImpl::StartEditorUpdate()
		{ }
		void JImGuiImpl::MouseUpdate()
		{
			JImGuiImpl::SetMouseClick(0, ImGui::IsMouseClicked(0));
			JImGuiImpl::SetMouseClick(1, ImGui::IsMouseClicked(1));
			JImGuiImpl::SetMouseClick(2, ImGui::IsMouseClicked(2));
		}
		void JImGuiImpl::EndEditorUpdate()
		{
			jImgui->ClearWidgetCount();
		}
		void JImGuiImpl::Initialize()
		{
			if (jImgui == nullptr)
				jImgui = std::make_unique<JImGui>();
		}
		void JImGuiImpl::Clear()
		{
			jImgui.reset();
		}
	}
}