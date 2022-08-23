#include"JImGuiImpl.h" 
#include"../../../Core/JDataType.h"  

#include"../../../Window/JWindows.h"
#include"../../../Window/JWindowEventType.h" 
#include"../../../Graphic/JGraphic.h"
#include"../../../Graphic/JGraphicTexture.h"
#include"../../../Graphic/JGraphicTextureUserInterface.h"

#include"../../../Utility/JCommonUtility.h"
#include "../../Icon/IconFontCppHeaders-master/IconsFontAwesome5.h" 
#include"../../../../Lib/imgui/imgui_impl_dx12.h"
#include"../../../../Lib/imgui/imgui_impl_win32.h" 

#include<Windows.h> 
#include<memory>
#include<deque>

template<typename T>
ImVec2::ImVec2(const JinEngine::JVector2<T>& jVec2)
{
	x = jVec2.x;
	y = jVec2.y;
}

template<typename T>
ImVec2& ImVec2::operator=(const JinEngine::JVector2<T>& jVec2)
{
	x = jVec2.x;
	y = jVec2.y;
	return *this;
}

template<typename T>
ImVec4::ImVec4(const JinEngine::JVector4<T>& jVec4)
{
	x = jVec4.x;
	y = jVec4.y;
	z = jVec4.z;
	w = jVec4.a;
}
template<typename T>
ImVec4& ImVec4::operator=(const JinEngine::JVector4<T>& jVec4)
{
	x = jVec4.x;
	y = jVec4.y;
	z = jVec4.z;
	w = jVec4.a;
	return *this;
}

namespace JinEngine
{
	class JObject;
	namespace Editor
	{
		class JImGui : public Core::JEventListener<size_t, Window::J_WINDOW_EVENT>, public Graphic::JGraphicTextureUserInterface
		{
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
			ImFont* fontVector[3];
			int fontIndex = 0;
		public:
			ImVec4 colors[ImGuiCol_COUNT];
		public:
			bool optFullScreen;
			bool optWindowPadding;
		public:
			std::bitset<3> mouseClick;
			bool isDrag;
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
		public:
			JImGui()
				:guid(JCommonUtility::CalculateGuid(typeid(JImGui).name()))
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
				LoadDefaultColor();
				LoadFontFile();
				//PushFont();
				OnResize();

				ImGuiStyle& style = ImGui::GetStyle();
				style.FrameBorderSize = 1;

				ImGui_ImplWin32_Init(JWindow::Instance().HandleInterface()->GetHandle());
				JGraphic::Instance().EditorInterface()->SetImGuiBackEnd();
			}
			~JImGui()
			{
				this->RemoveListener(*JWindow::Instance().EvInterface(), guid);
			}
		public:
			void ClearWidgetCount()
			{
				windowCount = childWindowCount = textCount = checkBoxCount = buttonCount =
					imageButtonCount = treeNodeCount = selectableCount = inputDataCount = sliderCount =
					tabBarCount = tabItemCount = tableCount = menuBarCount = menuCount =
					menuItemCount = comboCount = 0;
			}
			uint WidgetSum()
			{
				return windowCount + childWindowCount + textCount + checkBoxCount + buttonCount +
					imageButtonCount + treeNodeCount + selectableCount + inputDataCount + sliderCount +
					tabBarCount + tabItemCount + tableCount + menuBarCount + menuCount +
					menuItemCount + comboCount;
			}
		public:
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetGraphicCpuHandle(Graphic::JGraphicTexture& graphicTexture)
			{
				return GetCpuHandle(graphicTexture);
			}
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGraphicGpuHandle(Graphic::JGraphicTexture& graphicTexture)
			{
				return GetGpuHandle(graphicTexture);
			}
		private:
			void OnResize()
			{
				displaySize = { JWindow::Instance().GetDisplayWidth() ,  JWindow::Instance().GetDisplayHeight() };
				windowSize = { JWindow::Instance().GetWindowWidth() ,  JWindow::Instance().GetWindowHeight() };
				clientSize = { JWindow::Instance().GetClientWidth() ,  JWindow::Instance().GetClientHeight() };
				clientPos = { JWindow::Instance().GetClientPositionX() ,  JWindow::Instance().GetClientPositionY() };
				minClientSize = { (int)(clientSize.x / minRate),  (int)(clientSize.y / minRate) };

				ImGuiStyle& style = ImGui::GetStyle();
				ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(250, 250));
			}
			void OnEvent(const size_t& senderGuid, const Window::J_WINDOW_EVENT& eventType)
			{
				if (senderGuid == guid)
					return;
				if (eventType == Window::J_WINDOW_EVENT::WINDOW_RESIZE)
					OnResize();
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
		};
		class JImGuiEventManager : public Core::JEventManager<size_t, J_EDITOR_EVENT, JEditorEventStruct*>
		{
		public:
			std::deque<std::unique_ptr<JEditorEventStruct>> evQueue;
		public:
			JImGuiEventManager()
			{
				RegistEvCallable();
			}
			~JImGuiEventManager()
			{
				ClearEvent();
			}
		public:
			JEventInterface* EvInterface()final
			{
				return this;
			}
		public:
			void OnEvnet()
			{
				SendEventNotification();
			}
		private:
			void RegistEvCallable()final
			{
				auto lam = [](const size_t& a, const size_t& b) {return a == b; };
				RegistIdenCompareCallable(lam);
			}
		};
		class JEditorPageData
		{
		public:
			std::string openInitObjTypeName;
			size_t openInitObjGuid;
			bool hasOpenInitObjType;
			JObject* selectObject;
			const bool IsRequiredInitData;
		public:
			JEditorPageData(const bool IsRequiredInitData)
				:IsRequiredInitData(IsRequiredInitData)
			{}
		};

		namespace
		{
			static std::unique_ptr<JImGui> jImgui;
			static std::unique_ptr<JImGuiEventManager> jImguiEvM;
			static std::unordered_map<J_EDITOR_PAGE_TYPE, JEditorPageData> pageData;
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
		void JImGuiImpl::SetFont(int index)noexcept
		{
			jImgui->fontIndex = index;
		}
		void JImGuiImpl::PushFont()noexcept
		{
			ImGui::PushFont(jImgui->fontVector[jImgui->fontIndex]);
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
		bool JImGuiImpl::InputFloat(const std::string& name, float* value, float step , float stepFast, const char* format, ImGuiInputTextFlags flags)
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
		bool JImGuiImpl::EndTable()
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
		bool JImGuiImpl::IsLeftMouseClick()noexcept
		{
			return jImgui->mouseClick[0];
		}
		bool JImGuiImpl::IsRightMouseClick()noexcept
		{
			return jImgui->mouseClick[1];
		}
		bool JImGuiImpl::IsMiddleMouseClick()noexcept
		{
			return jImgui->mouseClick[2];
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
		uint JImGuiImpl::GetSliderWidth()noexcept
		{
			int sliderOffset = ImGui::GetStyle().ScrollbarSize + (ImGui::GetStyle().ItemSpacing.x * 2) + (JImGuiImpl::GetTextSize().x * 4);
			return ImGui::GetWindowWidth() - sliderOffset;
		}
		bool JImGuiImpl::IsFullScreen()noexcept
		{
			return jImgui->optFullScreen;
		}
		bool JImGuiImpl::IsWindowPadding()noexcept
		{
			return jImgui->optWindowPadding;
		}
		JImGuiImpl::JEvInterface* JImGuiImpl::EvInterface()
		{
			return jImguiEvM.get();
		}
		JEditorEventStruct* JImGuiImpl::RegisterEvStruct(std::unique_ptr<JEditorEventStruct> evStruct)
		{
			if (evStruct != nullptr)
			{
				JEditorEventStruct* ptr = evStruct.get();
				jImguiEvM->evQueue.push_back(std::move(evStruct));
				return ptr;
			}
			else
				return nullptr;
		}
		void JImGuiImpl::ClearEvStructQueue()
		{
			jImguiEvM->evQueue.clear();
		}
		void JImGuiImpl::RegisterPage(const J_EDITOR_PAGE_TYPE pageType, bool hasOpenInitObjType)noexcept
		{
			pageData.emplace(pageType, JEditorPageData(hasOpenInitObjType));
		}
		void JImGuiImpl::UnRegisterPage(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			pageData.erase(pageType);
		}
		void JImGuiImpl::ClearPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			pageData.find(pageType)->second.selectObject = nullptr;
		}
		bool JImGuiImpl::HasValidOpenPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			auto data = pageData.find(pageType);
			return data->second.IsRequiredInitData ? data->second.IsRequiredInitData && data->second.hasOpenInitObjType : true;
		}
		JEditorOpenPageEvStruct JImGuiImpl::GetOpendPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			auto data = pageData.find(pageType);
			if (data->second.hasOpenInitObjType)
				return JEditorOpenPageEvStruct{ data->second.openInitObjTypeName, data->second.openInitObjGuid, pageType };
			else
				return JEditorOpenPageEvStruct{ pageType };
		}
		JObject* JImGuiImpl::GetSelectedObj(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return pageData.find(pageType)->second.selectObject;
		}
		void JImGuiImpl::SetPageOpenData(JEditorOpenPageEvStruct* evStruct)noexcept
		{
			auto data = pageData.find(evStruct->pageType);
			data->second.openInitObjTypeName = evStruct->objTypeName;
			data->second.openInitObjGuid = evStruct->objGuid;
			data->second.hasOpenInitObjType = evStruct->hasOpenInitObjType;
		}
		void JImGuiImpl::SetSelectedObj(const J_EDITOR_PAGE_TYPE pageType, JObject* obj)noexcept
		{
			pageData.find(pageType)->second.selectObject = obj;
		}
		void JImGuiImpl::StartEditorUpdate()
		{
			jImguiEvM->OnEvnet();
			ClearEvStructQueue();
		}
		void JImGuiImpl::EndEditorUpdate()
		{
			jImgui->ClearWidgetCount();
		}
		void JImGuiImpl::Initialize()
		{
			if (jImgui == nullptr)
				jImgui = std::make_unique<JImGui>();

			if (jImguiEvM == nullptr)
				jImguiEvM = std::make_unique<JImGuiEventManager>();
		}
		void JImGuiImpl::Clear()
		{
			jImgui.reset();
			jImguiEvM.reset();
		}
	}
}