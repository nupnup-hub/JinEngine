#include"JImGuiAdaptee.h" 
#include"../../Data/ImGui/JImGuiInitData.h"
#include"../../Data/ImGui/JImGuiPrivateData.h" 
#include"../../../Align/JEditorAlignCalculator.h"
#include"../../../Page/Docking/ImGui/JImGuiDockUpdateHelper.h"
#include"../../../../Core/Math/JVectorExtend.h"
#include"../../../../Core/File/JFileConstant.h"
#include"../../../../Core/File/JFileIOHelper.h"
#include"../../../../Object/Resource/JResourceManager.h"
#include"../../../../Object/Resource/Texture/JTexture.h"
#include"../../../../Application/JApplicationEngine.h" 
#include"../../../../Application/JApplicationProject.h" 
#include"../../../../Graphic/Gui/JGuiBackendInterface.h" 
#include"../../../../Graphic/GraphicResource/JGraphicResourceInterface.h"  
#include"../../../../Graphic/GraphicResource/JGraphicResourceUserAccess.h"   
#include"../../../../../ThirdParty/imgui/imgui.h"
#include"../../../../../ThirdParty/imgui/imgui_internal.h" 

template<typename T>
ImVec2::ImVec2(const JinEngine::JVector2<T>& jVec2)
{
	x = static_cast<float>(jVec2.x);
	y = static_cast<float>(jVec2.y);
}
template<typename T>
ImVec2& ImVec2::operator=(const JinEngine::JVector2<T>& jVec2)
{
	x = static_cast<float>(jVec2.x);
	y = static_cast<float>(jVec2.y);
	return *this;
}
template<typename T>
ImVec4::ImVec4(const JinEngine::JVector4<T>& jVec4)
{
	x = static_cast<float>(jVec4.x);
	y = static_cast<float>(jVec4.y);
	z = static_cast<float>(jVec4.z);
	w = static_cast<float>(jVec4.w);
}
template<typename T>
ImVec4::ImVec4(const JinEngine::JVector3<T>& jVec3, const T _w)
{
	x = static_cast<float>(jVec3.x);
	y = static_cast<float>(jVec3.y);
	z = static_cast<float>(jVec3.z);
	w = _w;
}
template<typename T>
ImVec4& ImVec4::operator=(const JinEngine::JVector4<T>& jVec4)
{
	x = jVec4.x;
	y = jVec4.y;
	z = jVec4.z;
	w = jVec4.w;
	return *this;
}

namespace JinEngine::Editor
{
	namespace
	{
		static std::wstring GetSrcImGuiSaveDataPath()noexcept
		{
			return Core::JFileConstant::MakeFilePath(JApplicationEngine::ProjectPath(), L"imgui.ini");
		}
		static std::wstring GetCopiedImGuiSaveDataPath()noexcept
		{
			return Core::JFileConstant::MakeFilePath(JApplicationProject::EditoConfigPath(), L"imgui.ini");;
		}
	}
	namespace
	{
		static void SetGuiStyle(JImGuiPrivateData* data)
		{
			auto& colors = data->colors;
			colors[ImGuiCol_Text] = JVector4F(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_TextDisabled] = JVector4F(0.50f, 0.50f, 0.50f, 1.00f);
			colors[ImGuiCol_WindowBg] = JVector4F(0.1f, 0.1f, 0.12f, 1.00f);
			colors[ImGuiCol_ChildBg] = JVector4F(0.1f, 0.1f, 0.12f, 0.00f);
			colors[ImGuiCol_PopupBg] = JVector4F(0.1f, 0.1f, 0.12f, 1.00f);
			colors[ImGuiCol_Border] = JVector4F(0.5f, 0.5f, 0.5f, 0.7f);
			colors[ImGuiCol_BorderShadow] = JVector4F(0.00f, 0.00f, 0.00f, 0.24f);
			colors[ImGuiCol_FrameBg] = JVector4F(0.15f, 0.15f, 0.17f, 0.54f);
			colors[ImGuiCol_FrameBgHovered] = JVector4F(0.4f, 0.4f, 0.4532f, 0.8f);
			colors[ImGuiCol_FrameBgActive] = JVector4F(0.55f, 0.55f, 0.62f, 1.00f);
			colors[ImGuiCol_TitleBg] = JVector4F(0.15f, 0.15f, 0.17f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = JVector4F(JVector3F(Graphic::Constants::backBufferClearColor.xyz) * 0.75f, 1.0f);//JVector4F(0.2f, 0.2f, 0.225f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = JVector4F(JVector3F(Graphic::Constants::backBufferClearColor.xyz) * 0.325f, 1.0f);	 //JVector4F(0.125f, 0.125f, 0.125f, 1.00f);							   //JVector4F(0.125f, 0.125f, 0.125f, 1.00f);
			colors[ImGuiCol_MenuBarBg] = JVector4F(0.15f, 0.15f, 0.17f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = JVector4F(0.05f, 0.05f, 0.05f, 0.54f);
			colors[ImGuiCol_ScrollbarGrab] = JVector4F(0.34f, 0.34f, 0.34f, 0.74f);
			colors[ImGuiCol_ScrollbarGrabHovered] = JVector4F(0.40f, 0.40f, 0.40f, 0.74f);
			colors[ImGuiCol_ScrollbarGrabActive] = JVector4F(0.56f, 0.56f, 0.56f, 0.74f);
			colors[ImGuiCol_CheckMark] = JVector4F(0.33f, 0.67f, 0.86f, 1.00f);
			colors[ImGuiCol_SliderGrab] = JVector4F(0.34f, 0.34f, 0.34f, 0.74f);
			colors[ImGuiCol_SliderGrabActive] = JVector4F(0.56f, 0.56f, 0.56f, 0.74f);
			colors[ImGuiCol_Button] = JVector4F(0.05f, 0.05f, 0.05f, 0.54f);
			colors[ImGuiCol_ButtonHovered] = JVector4F(0.19f, 0.19f, 0.19f, 0.74f);
			colors[ImGuiCol_ButtonActive] = JVector4F(0.20f, 0.22f, 0.23f, 1.00f);
			colors[ImGuiCol_Header] = JVector4F(0.15f, 0.15f, 0.17f, 0.54f);
			colors[ImGuiCol_HeaderHovered] = JVector4F(0.4f, 0.4f, 0.4532f, 0.8f);
			colors[ImGuiCol_HeaderActive] = JVector4F(0.55f, 0.55f, 0.62f, 1.00f);
			colors[ImGuiCol_Separator] = JVector4F(0.28f, 0.28f, 0.28f, 1.00f);
			colors[ImGuiCol_SeparatorHovered] = JVector4F(0.44f, 0.44f, 0.44f, 1.00f);
			colors[ImGuiCol_SeparatorActive] = JVector4F(0.40f, 0.44f, 0.47f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = JVector4F(0.28f, 0.28f, 0.28f, 0.29f);
			colors[ImGuiCol_ResizeGripHovered] = JVector4F(0.44f, 0.44f, 0.44f, 0.29f);
			colors[ImGuiCol_ResizeGripActive] = JVector4F(0.40f, 0.44f, 0.47f, 1.00f);
			colors[ImGuiCol_Tab] = JVector4F(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_TabHovered] = Graphic::Constants::backBufferClearColor; //JVector4F(0.14f, 0.14f, 0.25f, 1.00f);				
			colors[ImGuiCol_TabActive] = Graphic::Constants::backBufferClearColor;	//JVector4F(0.14f, 0.14f, 0.25f, 1.00f);
			colors[ImGuiCol_TabUnfocused] = JVector4F(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_TabUnfocusedActive] = JVector4F(0.25f, 0.25f, 0.25f, 1.00f);
			colors[ImGuiCol_DockingPreview] = JVector4F(0.33f, 0.67f, 0.86f, 1.00f);
			colors[ImGuiCol_DockingEmptyBg] = JVector4F(1.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotLines] = JVector4F(1.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = JVector4F(1.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = JVector4F(1.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = JVector4F(1.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_TableHeaderBg] = JVector4F(0.00f, 0.00f, 0.00f, 0.52f);
			colors[ImGuiCol_TableBorderStrong] = JVector4F(0.00f, 0.00f, 0.00f, 0.52f);
			colors[ImGuiCol_TableBorderLight] = JVector4F(0.28f, 0.28f, 0.28f, 0.29f);
			//colors[ImGuiCol_TableRowBg] = JVector4F(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_TableRowBg] = JVector4F(1.00f, 1.00f, 1.00f, 0.12f);
			colors[ImGuiCol_TableRowBgAlt] = JVector4F(1.00f, 1.00f, 1.00f, 0.12f);
			colors[ImGuiCol_TextSelectedBg] = JVector4F(0.20f, 0.22f, 0.23f, 1.00f);
			colors[ImGuiCol_DragDropTarget] = JVector4F(0.33f, 0.67f, 0.86f, 1.00f);
			colors[ImGuiCol_NavHighlight] = JVector4F(1.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = JVector4F(1.00f, 0.00f, 0.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = JVector4F(1.00f, 0.00f, 0.00f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = JVector4F(1.00f, 0.00f, 0.00f, 0.35f);

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
			style.WindowRounding = 12;
			style.ChildRounding = 8;
			style.FrameRounding = 8;
			style.PopupRounding = 8;
			style.ScrollbarRounding = 14;
			style.GrabRounding = 6;
			style.LogSliderDeadzone = 4;
			style.TabRounding = 8;

			for (int i = 0; i < (int)ImGuiCol_COUNT; ++i)
				style.Colors[i] = colors[i];
		}
		static void LoadFontFile(JImGuiPrivateData* data)
		{
			ImGuiIO& io = ImGui::GetIO();
			ImFontConfig config;
			config.OversampleH = 2;

			std::wstring mediumFontPath = JApplicationEngine::ResourcePath() + L"\\Font\\NotoSansKR-Medium.otf";
			ImFont* defaultFont = io.Fonts->AddFontDefault();
			ImFont* mediumFont = io.Fonts->AddFontFromFileTTF(JCUtil::WstrToU8Str(mediumFontPath).c_str(),
				24,
				&config,
				io.Fonts->GetGlyphRangesKorean());
			data->fontMap.clear();

			data->fontMap.emplace(J_GUI_FONT_TYPE::DEFAULT, std::unordered_map<Core::J_LANGUAGE_TYPE, ImFont*>());
			data->fontMap.emplace(J_GUI_FONT_TYPE::MEDIUM, std::unordered_map<Core::J_LANGUAGE_TYPE, ImFont*>());

			data->fontMap.find(J_GUI_FONT_TYPE::DEFAULT)->second.emplace(Core::J_LANGUAGE_TYPE::KOREAN, defaultFont);
			data->fontMap.find(J_GUI_FONT_TYPE::DEFAULT)->second.emplace(Core::J_LANGUAGE_TYPE::ENGLISH, defaultFont);

			data->fontMap.find(J_GUI_FONT_TYPE::MEDIUM)->second.emplace(Core::J_LANGUAGE_TYPE::KOREAN, mediumFont);
			data->fontMap.find(J_GUI_FONT_TYPE::MEDIUM)->second.emplace(Core::J_LANGUAGE_TYPE::ENGLISH, mediumFont);
		}
		static ImGuiCol_ ConvertColorType(const J_GUI_COLOR type)noexcept
		{
			//same
			return (ImGuiCol_)type;
		}
		static ImGuiWindowFlags_ ConvertWindowFlag(J_GUI_WINDOW_FLAG flag)noexcept
		{
			ImGuiWindowFlags imguiFlag = ImGuiWindowFlags_None;
			if (Core::HasSQValueEnum(flag, J_GUI_WINDOW_FLAG_NO_DOCKING))
				imguiFlag |= ImGuiWindowFlags_NoDocking;
			if (Core::HasSQValueEnum(flag, J_GUI_WINDOW_FLAG_NO_MOVE))
				imguiFlag |= ImGuiWindowFlags_NoMove;
			if (Core::HasSQValueEnum(flag, J_GUI_WINDOW_FLAG_NO_TITLE_BAR))
				imguiFlag |= ImGuiWindowFlags_NoTitleBar;
			if (Core::HasSQValueEnum(flag, J_GUI_WINDOW_FLAG_NO_SCROLL_BAR))
				imguiFlag |= ImGuiWindowFlags_NoScrollbar;
			if (Core::HasSQValueEnum(flag, J_GUI_WINDOW_FLAG_NO_SCROLL_WITH_MOUSE))
				imguiFlag |= ImGuiWindowFlags_NoScrollWithMouse;
			if (Core::HasSQValueEnum(flag, J_GUI_WINDOW_FLAG_NO_RESIZE))
				imguiFlag |= ImGuiWindowFlags_NoResize;
			if (Core::HasSQValueEnum(flag, J_GUI_WINDOW_FLAG_NO_COLLAPSE))
				imguiFlag |= ImGuiWindowFlags_NoCollapse;
			if (Core::HasSQValueEnum(flag, J_GUI_WINDOW_FLAG_NO_SAVE))
				imguiFlag |= ImGuiWindowFlags_NoSavedSettings;
			if (Core::HasSQValueEnum(flag, J_GUI_WINDOW_FLAG_NO_FOCUS_ON_APPEARING))
				imguiFlag |= ImGuiWindowFlags_NoFocusOnAppearing;
			if (Core::HasSQValueEnum(flag, J_GUI_WINDOW_FLAG_NO_BRING_TO_FRONT_ON_FOCUS))
				imguiFlag |= ImGuiWindowFlags_NoBringToFrontOnFocus;
			if (Core::HasSQValueEnum(flag, J_GUI_WINDOW_FLAG_AUTO_RESIZE))
				imguiFlag |= ImGuiWindowFlags_AlwaysAutoResize;
			if (Core::HasSQValueEnum(flag, J_GUI_WINDOW_FLAG_MENU_BAR))
				imguiFlag |= ImGuiWindowFlags_MenuBar;
			if (Core::HasSQValueEnum(flag, J_GUI_WINDOW_FLAG_NO_BACKGROUND))
				imguiFlag |= ImGuiWindowFlags_NoBackground;
			if (Core::HasSQValueEnum(flag, J_GUI_WINDOW_FLAG_NO_NAV_INPUT))
				imguiFlag |= ImGuiWindowFlags_NoNavInputs;
			if (Core::HasSQValueEnum(flag, J_GUI_WINDOW_FLAG_NO_NAV_FOCUS))
				imguiFlag |= ImGuiWindowFlags_NoNavFocus;
			return (ImGuiWindowFlags_)imguiFlag;
		}
		static ImGuiDockNodeFlags_ ConvertDockFlag(const J_GUI_DOCK_NODE_FLAG flag)noexcept
		{ 
			ImGuiDockNodeFlags imguiFlag = ImGuiDockNodeFlags_None;
			if (Core::HasSQValueEnum(flag, J_GUI_DOCK_NODE_FLAG_NO_WINDOW_MENU_BUTTON))
				imguiFlag |= ImGuiDockNodeFlags_NoWindowMenuButton;
			if (Core::HasSQValueEnum(flag, J_GUI_DOCK_NODE_FLAG_NO_SPLIT_ME))
				imguiFlag |= ImGuiDockNodeFlags_NoDockingSplitMe;
			if (Core::HasSQValueEnum(flag, J_GUI_DOCK_NODE_FLAG_NO_SPLIT_OTHER))
				imguiFlag |= ImGuiDockNodeFlags_NoDockingSplitOther;
			if (Core::HasSQValueEnum(flag, J_GUI_DOCK_NODE_FLAG_NO_OVER_ME))
				imguiFlag |= ImGuiDockNodeFlags_NoDockingOverMe;
			if (Core::HasSQValueEnum(flag, J_GUI_DOCK_NODE_FLAG_NO_OVER_OTHER))
				imguiFlag |= ImGuiDockNodeFlags_NoDockingOverOther;
			if (Core::HasSQValueEnum(flag, J_GUI_DOCK_NODE_FLAG_NO_OVER_EMPTY))
				imguiFlag |= ImGuiDockNodeFlags_NoDockingOverEmpty;

			return (ImGuiDockNodeFlags_)imguiFlag;
		}
		static ImGuiPopupFlags_ ConvertPopupFlag(const J_GUI_POPUP_FLAG flag)noexcept
		{
			ImGuiPopupFlags_ imguiFlag = ImGuiPopupFlags_None;
			return (ImGuiPopupFlags_)imguiFlag;
		}
		static ImGuiTreeNodeFlags_ ConvertTreeNodeFlag(const J_GUI_TREE_NODE_FLAG flag)noexcept
		{
			ImGuiTreeNodeFlags imguiFlag = ImGuiTreeNodeFlags_None;
			if (Core::HasSQValueEnum(flag, J_GUI_TREE_NODE_FLAG_DEFAULT_OPEN))
				imguiFlag |= ImGuiTreeNodeFlags_DefaultOpen;
			if (Core::HasSQValueEnum(flag, J_GUI_TREE_NODE_FLAG_EXTEND_HIT_BOX_WIDTH))
				imguiFlag |= ImGuiTreeNodeFlags_SpanAvailWidth;
			if (Core::HasSQValueEnum(flag, J_GUI_TREE_NODE_FLAG_EXTEND_FULL_WIDTH))
				imguiFlag |= ImGuiTreeNodeFlags_SpanFullWidth;
			if (Core::HasSQValueEnum(flag, J_GUI_TREE_NODE_FLAG_FRAMED))
				imguiFlag |= ImGuiTreeNodeFlags_Framed;
			if (Core::HasSQValueEnum(flag, J_GUI_TREE_NODE_FLAG_OPEN_ON_ARROW))
				imguiFlag |= ImGuiTreeNodeFlags_OpenOnArrow;

			return (ImGuiTreeNodeFlags_)imguiFlag;
		}
		static ImGuiButtonFlags_ ConvertButtonFlag(const J_GUI_BUTTON_FLAG flag)noexcept
		{
			ImGuiButtonFlags imguiFlag = ImGuiButtonFlags_None;
			if (Core::HasSQValueEnum(flag, J_GUI_BUTTON_FLAG_MOUSE_BUTTION_LEFT))
				imguiFlag |= ImGuiButtonFlags_MouseButtonLeft;
			if (Core::HasSQValueEnum(flag, J_GUI_BUTTON_FLAG_MOUSE_BUTTION_RIGHT))
				imguiFlag |= ImGuiButtonFlags_MouseButtonRight;
			if (Core::HasSQValueEnum(flag, J_GUI_BUTTON_FLAG_MOUSE_BUTTION_MIDDLE))
				imguiFlag |= ImGuiButtonFlags_MouseButtonMiddle;

			return (ImGuiButtonFlags_)imguiFlag;
		}
		static ImGuiSelectableFlags_ ConvertSelectableFlag(const J_GUI_SELECTABLE_FLAG flag)noexcept
		{
			ImGuiSelectableFlags imguiFlag = ImGuiSelectableFlags_None;
			if (Core::HasSQValueEnum(flag, J_GUI_SELECTABLE_FLAG_ALLOW_ITEM_OVERLAP))
				imguiFlag |= ImGuiSelectableFlags_AllowItemOverlap;
			if (Core::HasSQValueEnum(flag, J_GUI_SELECTABLE_FLAG_ALLOW_DOUBLE_CLICK))
				imguiFlag |= ImGuiSelectableFlags_AllowDoubleClick;
			if (Core::HasSQValueEnum(flag, J_GUI_SELECTABLE_FLAG_SPAN_ALL_COLUMNS))
				imguiFlag |= ImGuiSelectableFlags_SpanAllColumns;
			return (ImGuiSelectableFlags_)imguiFlag;
		}
		static ImGuiInputTextFlags_ ConvertInputTextFlag(const J_GUI_INPUT_TEXT_FLAG flag)noexcept
		{
			ImGuiInputTextFlags imguiFlag = ImGuiInputTextFlags_None;
			if (Core::HasSQValueEnum(flag, J_GUI_INPUT_TEXT_FLAG_READ_ONLY))
				imguiFlag |= ImGuiInputTextFlags_ReadOnly;
			if (Core::HasSQValueEnum(flag, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE))
				imguiFlag |= ImGuiInputTextFlags_EnterReturnsTrue;
			if (Core::HasSQValueEnum(flag, J_GUI_INPUT_TEXT_FLAG_NO_HORIZONTAL_SCROLL))
				imguiFlag |= ImGuiInputTextFlags_NoHorizontalScroll;
			return (ImGuiInputTextFlags_)imguiFlag;
		}
		static ImGuiSliderFlags_ ConvertSliderFlag(const J_GUI_SLIDER_FLAG flag)noexcept
		{
			ImGuiSliderFlags imguiFlag = ImGuiSliderFlags_None;
			if (Core::HasSQValueEnum(flag, J_GUI_SLIDER_FLAG_ALWAYS_CLAMP))
				imguiFlag |= ImGuiSliderFlags_AlwaysClamp;
			if (Core::HasSQValueEnum(flag, J_GUI_SLIDER_FLAG_NO_INPUT))
				imguiFlag |= ImGuiSliderFlags_NoInput;
			return (ImGuiSliderFlags_)imguiFlag;
		}
		static ImGuiTabBarFlags_ ConvertTabBarFlag(const J_GUI_TAB_BAR_FLAG flag)noexcept
		{
			ImGuiTabBarFlags imguiFlag = ImGuiTabBarFlags_None;
			return (ImGuiTabBarFlags_)imguiFlag;
		}
		static ImGuiTabItemFlags_ ConvertTabItemFlag(const J_GUI_TAB_ITEM_FLAG flag)noexcept
		{
			ImGuiTabItemFlags imguiFlag = ImGuiTabItemFlags_None;
			return (ImGuiTabItemFlags_)imguiFlag;
		}
		static ImGuiTableFlags_ ConvertTableFlag(const J_GUI_TABLE_FLAG flag)noexcept
		{
			ImGuiTableFlags imguiFlag = ImGuiTableFlags_None;

			if (Core::HasSQValueEnum(flag, J_GUI_TABLE_FLAG_SIZING_FIXED_FIT))
				imguiFlag |= ImGuiTableFlags_SizingFixedFit;
			if (Core::HasSQValueEnum(flag, J_GUI_TABLE_FLAG_ROW_BG))
				imguiFlag |= ImGuiTableFlags_RowBg;
			if (Core::HasSQValueEnum(flag, J_GUI_TABLE_FLAG_CONTEXT_MENU_IN_BODY))
				imguiFlag |= ImGuiTableFlags_ContextMenuInBody;
			if (Core::HasSQValueEnum(flag, J_GUI_TABLE_FLAG_REORDERABLE))
				imguiFlag |= ImGuiTableFlags_Reorderable;
			if (Core::HasSQValueEnum(flag, J_GUI_TABLE_FLAG_HIDEABLE))
				imguiFlag |= ImGuiTableFlags_Hideable;
			if (Core::HasSQValueEnum(flag, J_GUI_TABLE_FLAG_RESIZABLE))
				imguiFlag |= ImGuiTableFlags_Resizable;
			if (Core::HasSQValueEnum(flag, J_GUI_TABLE_FLAG_BORDER_INNER_H))
				imguiFlag |= ImGuiTableFlags_BordersInnerH;
			if (Core::HasSQValueEnum(flag, J_GUI_TABLE_FLAG_BORDER_OUTHER_H))
				imguiFlag |= ImGuiTableFlags_BordersOuterH;
			if (Core::HasSQValueEnum(flag, J_GUI_TABLE_FLAG_BORDER_INNER_V))
				imguiFlag |= ImGuiTableFlags_BordersInnerV;
			if (Core::HasSQValueEnum(flag, J_GUI_TABLE_FLAG_BORDER_OUTHER_V))
				imguiFlag |= ImGuiTableFlags_BordersOuterV;

			return (ImGuiTableFlags_)imguiFlag;
		}
		static ImGuiTableColumnFlags_ ConvertTableColumnFlag(const J_GUI_TABLE_COLUMN_FLAG flag)noexcept
		{
			ImGuiTableColumnFlags imguiFlag = ImGuiTableColumnFlags_None;
			if (Core::HasSQValueEnum(flag, J_GUI_TABLE_COLUMN_FLAG_WIDTH_STRETCH))
				imguiFlag |= ImGuiTableColumnFlags_WidthStretch;
			if (Core::HasSQValueEnum(flag, J_GUI_TABLE_COLUMN_FLAG_WIDTH_FIXED))
				imguiFlag |= ImGuiTableColumnFlags_WidthFixed;
			if (Core::HasSQValueEnum(flag, J_GUI_TABLE_COLUMN_FLAG_NO_RESIZE))
				imguiFlag |= ImGuiTableColumnFlags_NoResize;
			if (Core::HasSQValueEnum(flag, J_GUI_TABLE_COLUMN_FLAG_NO_REORDER))
				imguiFlag |= ImGuiTableColumnFlags_NoReorder;
			if (Core::HasSQValueEnum(flag, J_GUI_TABLE_COLUMN_FLAG_NO_HIDE))
				imguiFlag |= ImGuiTableColumnFlags_NoHide;
			return (ImGuiTableColumnFlags_)imguiFlag;
		}
		static ImGuiComboFlags_ ConvertComboFlag(const J_GUI_COMBO_FLAG flag)noexcept
		{
			ImGuiComboFlags imguiFlag = ImGuiComboFlags_None;
			if (Core::HasSQValueEnum(flag, J_GUI_COMBO_FLAG_POPUP_ALIGN_LEFT))
				imguiFlag |= ImGuiComboFlags_PopupAlignLeft;
			if (Core::HasSQValueEnum(flag, J_GUI_COMBO_FLAG_HEIGHT_SMALL))
				imguiFlag |= ImGuiComboFlags_HeightSmall;
			if (Core::HasSQValueEnum(flag, J_GUI_COMBO_FLAG_HEIGHT_REGULAR))
				imguiFlag |= ImGuiComboFlags_HeightRegular;
			if (Core::HasSQValueEnum(flag, J_GUI_COMBO_FLAG_HEIGHT_LARGE))
				imguiFlag |= ImGuiComboFlags_HeightLarge;
			if (Core::HasSQValueEnum(flag, J_GUI_COMBO_FLAG_HEIGHT_LARGEST))
				imguiFlag |= ImGuiComboFlags_HeightLargest;
			if (Core::HasSQValueEnum(flag, J_GUI_COMBO_FLAG_HEIGHT_NO_ARROW_BUTTON))
				imguiFlag |= ImGuiComboFlags_NoArrowButton;
			if (Core::HasSQValueEnum(flag, J_GUI_COMBO_FLAG_HEIGHT_NO_PREVIEW))
				imguiFlag |= ImGuiComboFlags_NoPreview;
			return (ImGuiComboFlags_)imguiFlag;
		}
		static ImGuiColorEditFlags_ ConvertColorEditFlag(const J_GUI_COLOR_EDIT_FALG flag)noexcept
		{
			ImGuiColorEditFlags imguiFlag = ImGuiColorEditFlags_None;
			if (Core::HasSQValueEnum(flag, J_GUI_COLOR_EDIT_FALG_ALPHA_BAR))
				imguiFlag |= ImGuiColorEditFlags_AlphaBar;
			if (Core::HasSQValueEnum(flag, J_GUI_COLOR_EDIT_FALG_ALPHA_PREVIEW))
				imguiFlag |= ImGuiColorEditFlags_AlphaPreview;
			if (Core::HasSQValueEnum(flag, J_GUI_COLOR_EDIT_FALG_DISPLAY_RGB))
				imguiFlag |= ImGuiColorEditFlags_DisplayRGB;
			if (Core::HasSQValueEnum(flag, J_GUI_COLOR_EDIT_FALG_INPUT_RGB))
				imguiFlag |= ImGuiColorEditFlags_InputRGB;
			return (ImGuiColorEditFlags_)imguiFlag;
		}
		static ImGuiFocusedFlags_ ConvertFocusFlag(const J_GUI_FOCUS_FLAG flag)noexcept
		{
			ImGuiFocusedFlags imguiFlag = ImGuiFocusedFlags_None;
			if (Core::HasSQValueEnum(flag, J_GUI_FOCUS_FLAG_CHILD_WINDOW))
				imguiFlag |= ImGuiFocusedFlags_ChildWindows;
			if (Core::HasSQValueEnum(flag, J_GUI_FOCUS_FLAG_DOCK_HIERARCHY))
				imguiFlag |= ImGuiFocusedFlags_DockHierarchy;
			if (Core::HasSQValueEnum(flag, J_GUI_FOCUS_FLAG_NO_POPUP_HIERARCHY))
				imguiFlag |= ImGuiFocusedFlags_NoPopupHierarchy;
			return (ImGuiFocusedFlags_)imguiFlag;
		}
		static ImGuiHoveredFlags_ ConvertHoveredFlag(const J_GUI_HOVERED_FLAG flag)noexcept
		{
			ImGuiHoveredFlags imguiFlag = ImGuiHoveredFlags_None;
			if (Core::HasSQValueEnum(flag, J_GUI_HOVERED_FLAG_CHILD_WINDOW))
				imguiFlag |= ImGuiHoveredFlags_ChildWindows;
			if (Core::HasSQValueEnum(flag, J_GUI_HOVERED_FLAG_DOCK_HIERARCHY))
				imguiFlag |= ImGuiHoveredFlags_DockHierarchy;

			return (ImGuiHoveredFlags_)imguiFlag;
		}
		static ImGuiItemFlags_ ConvertItemFlag(const J_GUI_ITEM_FLAG flag)
		{
			ImGuiItemFlags imguiFlag = ImGuiItemFlags_None;
			if (Core::HasSQValueEnum(flag, J_GUI_ITEM_FLAG_DISABLED))
				imguiFlag |= ImGuiItemFlags_Disabled;
			if (Core::HasSQValueEnum(flag, J_GUI_ITEM_FLAG_INPUTABLE))
				imguiFlag |= ImGuiItemFlags_Inputable;
			if (Core::HasSQValueEnum(flag, J_GUI_ITEM_FLAG_READ_ONLY))
				imguiFlag |= ImGuiItemFlags_ReadOnly;
			if (Core::HasSQValueEnum(flag, J_GUI_ITEM_FLAG_NO_NAV))
				imguiFlag |= ImGuiItemFlags_NoNav;
			if (Core::HasSQValueEnum(flag, J_GUI_ITEM_FLAG_NO_NAV_DEFAULT_FOCUS))
				imguiFlag |= ImGuiItemFlags_NoNavDefaultFocus;
			return (ImGuiItemFlags_)imguiFlag;
		}
		static ImGuiDragDropFlags_ ConvertDragFlag(const J_GUI_DRAG_DROP_FLAG flag)
		{
			ImGuiDragDropFlags imguiFlag = ImGuiDragDropFlags_None;
			return (ImGuiDragDropFlags_)imguiFlag;
		}
		static ImDrawFlags_ ConvertDrawFlag(const J_GUI_DRAW_FLAG flag)
		{
			ImDrawFlags imguiFlag = ImDrawFlags_None;
			if (Core::HasSQValueEnum(flag, J_GUI_DRAW_FLAG_ROUND_UP_LEFT))
				imguiFlag |= ImDrawFlags_RoundCornersTopLeft;
			if (Core::HasSQValueEnum(flag, J_GUI_DRAW_FLAG_ROUND_UP_RIGHT))
				imguiFlag |= ImDrawFlags_RoundCornersTopRight;
			if (Core::HasSQValueEnum(flag, J_GUI_DRAW_FLAG_ROUND_DOWN_LEFT))
				imguiFlag |= ImDrawFlags_RoundCornersBottomLeft;
			if (Core::HasSQValueEnum(flag, J_GUI_DRAW_FLAG_ROUND_DOWN_RIGHT))
				imguiFlag |= ImDrawFlags_RoundCornersBottomRight;
			return (ImDrawFlags_)imguiFlag;
		}
		static ImGuiMouseButton_ ConvertMouseBtnType(const Core::J_MOUSE_BUTTON type)
		{
			switch (type)
			{
			case Core::J_MOUSE_BUTTON::LEFT:
				return ImGuiMouseButton_Left;
			case Core::J_MOUSE_BUTTON::RIGHT:
				return ImGuiMouseButton_Right;
			case Core::J_MOUSE_BUTTON::MIDDLE:
				return ImGuiMouseButton_Middle;
			default:
				break;
			}
			return (ImGuiMouseButton_)0;
		}
		static ImGuiStyleVar_ ConvertStyleType(const J_GUI_STYLE type)
		{
			switch (type)
			{
			case JinEngine::Editor::J_GUI_STYLE::WINDOW_PADDING:
				return ImGuiStyleVar_WindowPadding;
			case JinEngine::Editor::J_GUI_STYLE::WINDOW_ROUNDING:
				return ImGuiStyleVar_WindowRounding;
			case JinEngine::Editor::J_GUI_STYLE::WINDOW_BORDER_SIZE:
				return ImGuiStyleVar_WindowBorderSize;
			case JinEngine::Editor::J_GUI_STYLE::WINDOW_MIN_SIZE:
				return ImGuiStyleVar_WindowMinSize;
			case JinEngine::Editor::J_GUI_STYLE::CHILD_ROUNDING:
				return ImGuiStyleVar_ChildRounding;
			case JinEngine::Editor::J_GUI_STYLE::CHILD_BORDER_SIZE:
				return ImGuiStyleVar_ChildBorderSize;
			case JinEngine::Editor::J_GUI_STYLE::POPUP_ROUNDING:
				return ImGuiStyleVar_PopupRounding;
			case JinEngine::Editor::J_GUI_STYLE::POPUP_BORDER_SIZE:
				return ImGuiStyleVar_PopupBorderSize;
			case JinEngine::Editor::J_GUI_STYLE::FRAME_PADDING:
				return ImGuiStyleVar_FramePadding;
			case JinEngine::Editor::J_GUI_STYLE::FRAME_ROUNDING:
				return ImGuiStyleVar_FrameRounding;
			case JinEngine::Editor::J_GUI_STYLE::FRAME_BORDER_SIZE:
				return ImGuiStyleVar_FrameBorderSize;
			case JinEngine::Editor::J_GUI_STYLE::ITEM_SPACING:
				return ImGuiStyleVar_ItemSpacing;
			case JinEngine::Editor::J_GUI_STYLE::ITEM_INNER_SPACING:
				return ImGuiStyleVar_ItemInnerSpacing;
			case JinEngine::Editor::J_GUI_STYLE::INDENT_SPACING:
				return ImGuiStyleVar_IndentSpacing;
			case JinEngine::Editor::J_GUI_STYLE::CELL_PADDING:
				return ImGuiStyleVar_CellPadding;
			case JinEngine::Editor::J_GUI_STYLE::SCROLL_BAR_SIZE:
				return ImGuiStyleVar_ScrollbarSize;
			case JinEngine::Editor::J_GUI_STYLE::SCROLL_BAR_ROUNDING:
				return ImGuiStyleVar_ScrollbarRounding;
			case JinEngine::Editor::J_GUI_STYLE::GRAB_MIN_SIZE:
				return ImGuiStyleVar_GrabMinSize;
			case JinEngine::Editor::J_GUI_STYLE::GRAB_ROUNDING:
				return ImGuiStyleVar_GrabRounding;
			case JinEngine::Editor::J_GUI_STYLE::TAB_ROUNDING:
				return ImGuiStyleVar_TabRounding;
			case JinEngine::Editor::J_GUI_STYLE::SELECTABLE_TEXT_ALIGN:
				return ImGuiStyleVar_SelectableTextAlign;
			default:
				break;
			}
			return (ImGuiStyleVar_)-1;
		}
		static ImGuiDir_ ConvertDirType(const J_GUI_CARDINAL_DIR type)
		{
			switch (type)
			{
			case JinEngine::Editor::J_GUI_CARDINAL_DIR::NONE:
				return ImGuiDir_None;
			case JinEngine::Editor::J_GUI_CARDINAL_DIR::LEFT:
				return ImGuiDir_Left;
			case JinEngine::Editor::J_GUI_CARDINAL_DIR::RIGHT:
				return ImGuiDir_Right;
			case JinEngine::Editor::J_GUI_CARDINAL_DIR::UP:
				return ImGuiDir_Up;
			case JinEngine::Editor::J_GUI_CARDINAL_DIR::DOWN:
				return ImGuiDir_Down;
			default:
				break;
			}
			return ImGuiDir_None;
		}
		static void* ConvertGraphicGpuHandle(const JGuiImageInfo& info)
		{ 
			if (info.useFirstHandle)
			{
				if (info.handle != nullptr)
					return info.handle;
				else
					return info.gUserAccess->GraphicResourceUserInterface().GetFirstGpuHandle(info.bType);
			}
			else
				return info.gUserAccess->GraphicResourceUserInterface().GetGpuHandle(info.rType, info.bType, info.bIndex, info.dataIndex);
		}
		static bool StuffWindowInfo(ImGuiWindow* wnd, _Out_ JGuiWindowInfo& info)
		{
			if (wnd == nullptr)
				return false;

			const bool hasDockNode = wnd->DockNode != nullptr;
			info.windowID = wnd->ID;
			info.dockID = hasDockNode ? wnd->DockNode->ID : 0;
			info.lastDockID = wnd->DockId;
			info.windowName = wnd->Name;
			info.pos = wnd->Pos;
			info.size = wnd->Size;
			info.focusOrder = wnd->FocusOrder;
			info.dockOrder = hasDockNode ? wnd->DockOrder : invalidIndex;
			info.isLastFrameFocused = wnd->LastFrameJustFocused;
			info.isLastFrameActive = wnd->LastFrameActive; 
			info.hasDockNode = hasDockNode;
			return true;
		}
		static bool StuffDockNodeInfo(ImGuiDockNode* dockNode, _Out_ JGuiDockNodeInfo& info)
		{ 
			if (dockNode == nullptr)
				return false;

			bool hasParent = dockNode->ParentNode != nullptr;
			info.dockID = dockNode->ID;
			info.parentDockID = hasParent ? dockNode->ParentNode->ID : 0;
			info.selectedTabID = dockNode->TabBar != NULL ? dockNode->SelectedTabId : 0;
			info.visibleWindowID = dockNode->VisibleWindow != nullptr ? dockNode->VisibleWindow->ID : 0;
			info.isRootNode = dockNode->IsRootNode();
			info.isFocused = dockNode->IsFocused;
			info.isVisible = dockNode->IsVisible;

			if (dockNode->TabBar != NULL)
			{
				const uint tabItemCount = (uint)dockNode->TabBar->Tabs.size();
				info.tabItemID.resize(tabItemCount);
				for (uint i = 0; i < tabItemCount; ++i)
					info.tabItemID[i] = dockNode->TabBar->Tabs[i].ID;
			}

			return true;
		}
		static ImGuiDockNode* FindDockNodeRoot(ImGuiDockNode* dock)noexcept
		{
			if (dock->IsRootNode())
				return dock;

			return FindDockNodeRoot(dock->ParentNode);
		}
		static const char* ConvertFloattingFormat(const uint formatDigit)
		{
			switch (formatDigit)
			{
			case 0:
				return " ";
			case 1:
				return "%.1f";
			case 2:
				return "%.2f";
			case 3:
				return "%.3f";
			case 4:
				return "%.4f";
			case 5:
				return "%.5f";
			case 6:
				return "%.6f";
			case 7:
				return "%.7f";
			default:
				return "%.5f";
			}
		}
	}

	void JImGuiAdaptee::Initialize(std::unique_ptr<Graphic::JGuiInitData>&& initData)
	{
		if (!IsSameGui(initData->GetGuiIdentification()))
			return;

		JImGuiInitData* imguiInitData = static_cast<JImGuiInitData*>(initData.get());
		data = std::make_unique<JImGuiPrivateData>();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigWindowsResizeFromEdges = true;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		//io.ConfigWindowsResizeFromEdges = true;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;        // Enable Multi-Viewport / Platform Windows

		SetGuiStyle(data.get());
		LoadFontFile(data.get());
		IntiailizeBackend(imguiInitData);
	}
	void JImGuiAdaptee::Clear()
	{
		ClearBackend();
		ImGui::DestroyContext();
		data.reset();
	}
	void JImGuiAdaptee::LoadGuiData()
	{ 
		const bool hasImguiTxt = (_waccess(GetCopiedImGuiSaveDataPath().c_str(), 00) != -1);
		_wremove(GetSrcImGuiSaveDataPath().c_str());
		if (hasImguiTxt)
			JFileIOHelper::CopyFile(GetCopiedImGuiSaveDataPath(), GetSrcImGuiSaveDataPath());
	}
	void JImGuiAdaptee::StoreGuiData()
	{
		if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
			JFileIOHelper::CopyFile(GetSrcImGuiSaveDataPath(), GetCopiedImGuiSaveDataPath());
	}
	J_GUI_TYPE JImGuiAdaptee::GetGuiType()const noexcept
	{
		return J_GUI_TYPE::IM_GUI;
	}
#pragma region Color
	JVector4<float> JImGuiAdaptee::GetColor(const J_GUI_COLOR flag)noexcept
	{
		ImGuiStyle& style = ImGui::GetStyle();
		return style.Colors[ConvertColorType(flag)];
	}
	uint JImGuiAdaptee::GetUColor(const J_GUI_COLOR flag)const noexcept
	{
		ImGuiStyle& style = ImGui::GetStyle();
		return ConvertUColor(style.Colors[ConvertColorType(flag)]);
	}
	void JImGuiAdaptee::SetColor(const J_GUI_COLOR flag, const JVector4<float>& color)noexcept
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ConvertColorType(flag)] = color;
	}
	void JImGuiAdaptee::SetColorToDefault(const J_GUI_COLOR flag)noexcept
	{
		ImGuiStyle& style = ImGui::GetStyle();
		auto colorFlag = ConvertColorType(flag);
		style.Colors[colorFlag] = data->colors[colorFlag];
	}
	void JImGuiAdaptee::SetAllColorToSoft(const JVector4<float>& factor)noexcept
	{
		ImGuiStyle& style = ImGui::GetStyle();
		for (uint i = 0; i < ImGuiCol_COUNT; ++i)
			style.Colors[i] = (JVector4<float>)style.Colors[i] + factor;
	}
	void JImGuiAdaptee::SetAllColorToDefault()noexcept
	{
		auto& colors = data->colors;
		ImGuiStyle& style = ImGui::GetStyle();
		for (uint i = 0; i < ImGuiCol_COUNT; ++i)
			style.Colors[i] = colors[i];
	}
	void JImGuiAdaptee::PushColor(const J_GUI_COLOR colType, const JVector4<float>& color)
	{
		ImGui::PushStyleColor(ConvertColorType(colType), color);
	}
	void JImGuiAdaptee::PopColor(const uint count)
	{ 
		ImGui::PopStyleColor(count);
	}
#pragma endregion
#pragma region Style
	JVector2<float> JImGuiAdaptee::GetWindowPadding()const noexcept
	{
		return ImGui::GetStyle().WindowPadding;
	}
	float JImGuiAdaptee::GetWindowBorderSize()const noexcept
	{
		return ImGui::GetStyle().WindowBorderSize;
	}
	float JImGuiAdaptee::GetWindowRounding()const noexcept
	{
		return ImGui::GetStyle().WindowRounding;
	}
	JVector2<float> JImGuiAdaptee::GetFramePadding()const noexcept
	{
		return ImGui::GetStyle().FramePadding;
	}
	float JImGuiAdaptee::GetFrameBorderSize()const noexcept
	{
		return ImGui::GetStyle().FrameBorderSize;
	}
	JVector2<float> JImGuiAdaptee::GetItemSpacing()const noexcept
	{
		return ImGui::GetStyle().ItemSpacing;
	}
	JVector2<float> JImGuiAdaptee::GetItemInnerSpacing()const noexcept
	{
		return ImGui::GetStyle().ItemInnerSpacing;
	}
	float JImGuiAdaptee::GetScrollBarSize()const noexcept
	{
		return ImGui::GetStyle().ScrollbarSize;
	}
	float JImGuiAdaptee::GetStyleValueF(const J_GUI_STYLE style)const noexcept
	{
		switch (style)
		{
		case JinEngine::Editor::J_GUI_STYLE::WINDOW_ROUNDING:
			return ImGui::GetStyle().WindowRounding;
		case JinEngine::Editor::J_GUI_STYLE::WINDOW_BORDER_SIZE:
			return ImGui::GetStyle().WindowBorderSize;
		case JinEngine::Editor::J_GUI_STYLE::CHILD_ROUNDING:
			return ImGui::GetStyle().ChildRounding;
		case JinEngine::Editor::J_GUI_STYLE::CHILD_BORDER_SIZE:
			return ImGui::GetStyle().ChildBorderSize;
		case JinEngine::Editor::J_GUI_STYLE::POPUP_ROUNDING:
			return ImGui::GetStyle().PopupRounding;
		case JinEngine::Editor::J_GUI_STYLE::POPUP_BORDER_SIZE:
			return ImGui::GetStyle().PopupBorderSize;
		case JinEngine::Editor::J_GUI_STYLE::FRAME_ROUNDING:
			return ImGui::GetStyle().FrameRounding;
		case JinEngine::Editor::J_GUI_STYLE::FRAME_BORDER_SIZE:
			return ImGui::GetStyle().FrameBorderSize;
		case JinEngine::Editor::J_GUI_STYLE::INDENT_SPACING:
			return ImGui::GetStyle().IndentSpacing;
		case JinEngine::Editor::J_GUI_STYLE::SCROLL_BAR_SIZE:
			return ImGui::GetStyle().ScrollbarSize;
		case JinEngine::Editor::J_GUI_STYLE::SCROLL_BAR_ROUNDING:
			return ImGui::GetStyle().ScrollbarRounding;
		case JinEngine::Editor::J_GUI_STYLE::GRAB_MIN_SIZE:
			return ImGui::GetStyle().GrabMinSize;
		case JinEngine::Editor::J_GUI_STYLE::GRAB_ROUNDING:
			return ImGui::GetStyle().GrabRounding;
		case JinEngine::Editor::J_GUI_STYLE::TAB_ROUNDING:
			return ImGui::GetStyle().TabRounding;
		default:
			return 0;
		}
	}
	JVector2<float> JImGuiAdaptee::GetStyleValueV2(const J_GUI_STYLE style)const noexcept
	{
		switch (style)
		{
		case JinEngine::Editor::J_GUI_STYLE::WINDOW_PADDING:
			return ImGui::GetStyle().WindowPadding;
		case JinEngine::Editor::J_GUI_STYLE::WINDOW_MIN_SIZE:
			return ImGui::GetStyle().WindowMinSize;
		case JinEngine::Editor::J_GUI_STYLE::FRAME_PADDING:
			return ImGui::GetStyle().FramePadding;
		case JinEngine::Editor::J_GUI_STYLE::ITEM_SPACING:
			return ImGui::GetStyle().ItemSpacing;
		case JinEngine::Editor::J_GUI_STYLE::ITEM_INNER_SPACING:
			return ImGui::GetStyle().ItemInnerSpacing;
		case JinEngine::Editor::J_GUI_STYLE::CELL_PADDING:
			return ImGui::GetStyle().CellPadding;
		case JinEngine::Editor::J_GUI_STYLE::SELECTABLE_TEXT_ALIGN:
			return ImGui::GetStyle().SelectableTextAlign;
		default:
			return JVector2F::Zero();
		}
	}
	void JImGuiAdaptee::SetWindowPadding(const JVector2<float>& padding)noexcept
	{
		ImGui::GetStyle().WindowPadding = padding;
	}
	void JImGuiAdaptee::SetWindowBorderSize(const float size)noexcept
	{
		ImGui::GetStyle().WindowBorderSize = size;
	}
	void JImGuiAdaptee::SetWindowRounding(const float factor)noexcept
	{
		ImGui::GetStyle().WindowRounding = factor;
	}
	void JImGuiAdaptee::SetFramePadding(const JVector2<float>& padding)noexcept
	{
		ImGui::GetStyle().FramePadding = padding;
	}
	void JImGuiAdaptee::SetFrameBorderSize(const float size)noexcept
	{
		ImGui::GetStyle().FrameBorderSize = size;
	}
	void JImGuiAdaptee::SetItemSpacing(const JVector2<float>& spacing)noexcept
	{
		ImGui::GetStyle().ItemSpacing = spacing;
	}
	void JImGuiAdaptee::SetItemInnerSpacing(const JVector2<float>& spacing)noexcept
	{
		ImGui::GetStyle().ItemInnerSpacing = spacing;
	}
	void JImGuiAdaptee::SetScrollBarSize(const float size)noexcept
	{
		ImGui::GetStyle().ScrollbarSize = size;
	}
	void JImGuiAdaptee::PushStyle(const J_GUI_STYLE style, const float value)
	{
		ImGui::PushStyleVar(ConvertStyleType(style), value);
	}
	void JImGuiAdaptee::PushStyle(const J_GUI_STYLE style, const JVector2<float>& value)
	{
		ImGui::PushStyleVar(ConvertStyleType(style), value);
	}
	void JImGuiAdaptee::PopStyle(const uint count)
	{
		ImGui::PopStyleVar(count);
	}
#pragma endregion
#pragma region IO 
	JVector2<float> JImGuiAdaptee::GetAlphabetSize()const noexcept
	{
		return ImGui::CalcTextSize("0");
	}
	float JImGuiAdaptee::GetFontSize()const noexcept
	{
		return GImGui->FontSize;
	}
	JVector2<float> JImGuiAdaptee::CalTextSize(const std::string& str)const noexcept
	{
		return ImGui::CalcTextSize(str.c_str());
	}
	float JImGuiAdaptee::GetGlobalFontScale()const noexcept
	{
		return ImGui::GetIO().FontGlobalScale;
	}
	float JImGuiAdaptee::GetCurrentWindowFontScale()const noexcept
	{
		return ImGui::GetCurrentWindow()->FontWindowScale;
	}
	void JImGuiAdaptee::SetGlobalFontScale(const float scale)noexcept
	{
		ImGui::GetIO().FontGlobalScale = scale;
	}
	void JImGuiAdaptee::SetCurrentWindowFontScale(const float scale)noexcept
	{
		ImGui::SetWindowFontScale(scale);
	}
	void JImGuiAdaptee::SetFont(const J_GUI_FONT_TYPE fontType)
	{
		data->fontType = fontType;
	}
	void JImGuiAdaptee::PushFont()const noexcept
	{
		ImGui::PushFont(data->fontMap.find(data->fontType)->second.find(JApplicationEngine::GetLanguageType())->second);
	}
	void JImGuiAdaptee::PopFont()const noexcept
	{
		ImGui::PopFont();
	}
	uint JImGuiAdaptee::GetMouseClickedCount(const Core::J_MOUSE_BUTTON btn)const noexcept
	{
		return ImGui::GetMouseClickedCount(ConvertMouseBtnType(btn));
	}
	float JImGuiAdaptee::GetMouseWheel()const noexcept
	{
		return ImGui::GetIO().MouseWheel;
	}
	JVector2<float> JImGuiAdaptee::GetMousePos() const noexcept
	{
		return ImGui::GetMousePos();
	}
	JVector2<float> JImGuiAdaptee::GetMouseDragDelta() const noexcept
	{
		return ImGui::GetMouseDragDelta();
	}
	JVector2<float> JImGuiAdaptee::GetCursorPos()const noexcept
	{
		return ImGui::GetCursorPos();
	}
	JVector2<float> JImGuiAdaptee::GetCursorScreenPos()const noexcept
	{
		return ImGui::GetCursorScreenPos();
	}
	void JImGuiAdaptee::SetCursorPos(const JVector2<float>& pos)noexcept
	{
		ImGui::SetCursorPos(pos);
	}
	void JImGuiAdaptee::SetCursorScreenPos(const JVector2<float>& pos)noexcept
	{
		ImGui::SetCursorScreenPos(pos);
	}
	bool JImGuiAdaptee::IsMouseClicked(const Core::J_MOUSE_BUTTON btn)const noexcept
	{
		return ImGui::IsMouseClicked(ConvertMouseBtnType(btn));
	}
	bool JImGuiAdaptee::IsMouseReleased(const Core::J_MOUSE_BUTTON btn)const noexcept
	{
		return ImGui::IsMouseReleased(ConvertMouseBtnType(btn));
	}
	bool JImGuiAdaptee::IsMouseDown(const Core::J_MOUSE_BUTTON btn)const noexcept
	{ 
		return ImGui::IsMouseDown(ConvertMouseBtnType(btn));
	}
	bool JImGuiAdaptee::IsMouseDragging(const Core::J_MOUSE_BUTTON btn)const noexcept
	{
		return ImGui::IsMouseDragging(ConvertMouseBtnType(btn));
	}
	bool JImGuiAdaptee::IsMouseInRect(const JVector2<float>& min, const JVector2<float>& max)const noexcept
	{
		return ImGui::IsMouseHoveringRect(min, max, false);
		/*
			ImVec2 mousePos = ImGui::GetMousePos();
		if (mousePos.x >= position.x && mousePos.x <= position.x + size.x &&
			mousePos.y >= position.y && mousePos.y <= position.y + size.y)
			return true;
		else
			return false;
		*/
	}
	bool JImGuiAdaptee::IsMouseHoveringRect(const JVector2<float>& min, const JVector2<float>& max, const bool clip)const noexcept
	{
		return ImGui::IsMouseHoveringRect(min, max, clip);
	}
	bool JImGuiAdaptee::IsMouseInLine(JVector2<float> st, JVector2<float> ed, const float thickness)const noexcept
	{
		const ImVec2 mousePos = ImGui::GetMousePos();

		const float halfThickness = thickness / 2.0f;
		const float minX = min(st.x, ed.x);
		const float maxX = max(st.x, ed.x);
		const float minY = min(st.y, ed.y);
		const float maxY = max(st.y, ed.y);

		if (minX - halfThickness <= mousePos.x && mousePos.x <= maxX + halfThickness
			&& minY <= mousePos.y && mousePos.y <= maxY)
		{
			const float dy = (maxY - minY) / (maxX - minX);
			float yRange00 = (mousePos.x - (minX - halfThickness)) * dy + minY;
			float yRange01 = (mousePos.x - (minX + halfThickness)) * dy + minY;

			float yRange02 = ((maxX - halfThickness) - mousePos.x) * dy + minY;
			float yRange03 = ((maxX + halfThickness) - mousePos.x) * dy + minY;

			float minYRange00 = min(yRange00, yRange01);
			float maxYRange00 = max(yRange00, yRange01);

			float minYRange01 = min(yRange02, yRange03);
			float maxYRange01 = max(yRange02, yRange03);

			return (minYRange00 <= mousePos.y && mousePos.y <= maxYRange00) ||
				(minYRange01 <= mousePos.y && mousePos.y <= maxYRange01);
		}
		else
			return false;
	}
	bool JImGuiAdaptee::IsKeyPressed(const Core::J_KEYCODE key)const noexcept
	{
		return ImGui::IsKeyPressed((int)key);
	}
	bool JImGuiAdaptee::IsKeyReleased(const Core::J_KEYCODE key)const noexcept
	{
		return ImGui::IsKeyReleased((int)key);
	}
	bool JImGuiAdaptee::IsKeyDown(const Core::J_KEYCODE key)const noexcept
	{
		return ImGui::IsKeyDown((int)key);
	}
	bool JImGuiAdaptee::BeginDragDropSource(J_GUI_DRAG_DROP_FLAG_ flag)
	{
		return ImGui::BeginDragDropSource(ConvertDragFlag((J_GUI_DRAG_DROP_FLAG)flag));
	}
	bool JImGuiAdaptee::SetDragDropPayload(const std::string& typeName, Core::JTypeInstanceSearchHint* draggingHint, J_GUI_CONDIITON cond)
	{
		return ImGui::SetDragDropPayload(typeName.c_str(), draggingHint, sizeof(Core::JTypeInstanceSearchHint));
	}
	void JImGuiAdaptee::EndDragDropSource()
	{
		ImGui::EndDragDropSource();
	}
	bool JImGuiAdaptee::BeginDragDropTarget()
	{
		return ImGui::BeginDragDropTarget();
	}
	Core::JTypeInstanceSearchHint* JImGuiAdaptee::TryGetTypeHintDragDropPayload(const std::string& typeName, J_GUI_DRAG_DROP_FLAG_ flag)
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(typeName.c_str(), ImGuiDragDropFlags_None);
		if (payload == nullptr)
			return nullptr;

		if (payload->DataSize != sizeof(Core::JTypeInstanceSearchHint))
			return nullptr;

		return static_cast<Core::JTypeInstanceSearchHint*>(payload->Data);
	}
	void JImGuiAdaptee::EndDragDropTarget()
	{

	}
#pragma endregion
#pragma region Widget
	bool JImGuiAdaptee::BeginWindow(const std::string& name, bool* pOpen, J_GUI_WINDOW_FLAG flags)
	{ 
		if (pOpen)
			return ImGui::Begin(name.c_str(), pOpen, ConvertWindowFlag(flags));
		else
			return ImGui::Begin(name.c_str(), 0, ConvertWindowFlag(flags));
	}
	void JImGuiAdaptee::EndWindow()
	{
		ImGui::End();
	}
	bool JImGuiAdaptee::BeginChildWindow(const std::string& name, const JVector2<float>& windowSize, bool border, J_GUI_WINDOW_FLAG flags)
	{
		return ImGui::BeginChild(name.c_str(), windowSize, border, ConvertWindowFlag(flags));
	}
	void JImGuiAdaptee::EndChildWindow()
	{
		ImGui::EndChild();
	}
	void JImGuiAdaptee::OpenPopup(const std::string& name, J_GUI_POPUP_FLAG flags)
	{
		ImGui::OpenPopup(name.c_str(), ConvertPopupFlag(flags));
	}
	void JImGuiAdaptee::CloseCurrentPopup()
	{
		ImGui::CloseCurrentPopup();
	}
	bool JImGuiAdaptee::BeginPopup(const std::string& name, J_GUI_WINDOW_FLAG flags)
	{
		return ImGui::BeginPopup(name.c_str(), ConvertWindowFlag(flags));
	}
	void JImGuiAdaptee::EndPopup()
	{
		ImGui::EndPopup();
	}
	void JImGuiAdaptee::BeginGroup()
	{
		ImGui::BeginGroup();
	}
	void JImGuiAdaptee::EndGroup()
	{
		ImGui::EndGroup();
	}
	void JImGuiAdaptee::Text(const std::string& text)const noexcept
	{
		ImGui::Text(text.c_str());
	}
	bool JImGuiAdaptee::CheckBox(const std::string& checkName, bool& v)
	{
		return ImGui::Checkbox(checkName.c_str(), &v);
	}
	bool JImGuiAdaptee::Button(const std::string& btnName, const JVector2<float>& jVec2, J_GUI_BUTTON_FLAG flag)
	{
		return ImGui::ButtonEx(btnName.c_str(), jVec2, ConvertButtonFlag(flag));
	} 
	bool JImGuiAdaptee::ArrowButton(const std::string& name, const JVector2<float>& jVec2, const float arrowScale, J_GUI_BUTTON_FLAG flag, J_GUI_CARDINAL_DIR dir)
	{
		if (jVec2 == JVector2F::Zero())
		{
			float sz = ImGui::GetFrameHeight();
			return ImGui::CustomArrowButton(name.c_str(), ConvertDirType(dir), JVector2F(sz, sz) * arrowScale, ConvertButtonFlag(flag), arrowScale);
		}
		else
			return ImGui::CustomArrowButton(name.c_str(), ConvertDirType(dir), jVec2 * arrowScale, ConvertButtonFlag(flag), arrowScale);
	}
	bool JImGuiAdaptee::IsTreeNodeOpend(const std::string& name, J_GUI_TREE_NODE_FLAG_ flags)
	{
		return ImGui::TreeNodeBehaviorIsOpen(ImGui::GetID((name + "##TreeNode").c_str()), flags);
	}
	bool JImGuiAdaptee::TreeNodeEx(const std::string& nodeName, J_GUI_TREE_NODE_FLAG flags)
	{ 
		return ImGui::TreeNodeEx(nodeName.c_str(), ConvertTreeNodeFlag(flags));
	}
	void JImGuiAdaptee::TreePop()
	{
		ImGui::TreePop();
	}
	bool JImGuiAdaptee::Selectable(const std::string& name, bool* pSelected, J_GUI_SELECTABLE_FLAG flags, const JVector2<float>& sizeArg)
	{
		if (pSelected == nullptr)
			return ImGui::Selectable(name.c_str(), false, ConvertSelectableFlag(flags), sizeArg);
		else
			return ImGui::Selectable(name.c_str(), pSelected, ConvertSelectableFlag(flags), sizeArg);
	}
	bool JImGuiAdaptee::Selectable(const std::string& name, bool selected, J_GUI_SELECTABLE_FLAG flags, const JVector2<float>& sizeArg)
	{
		return ImGui::Selectable(name.c_str(), selected, ConvertSelectableFlag(flags), sizeArg);
	}
	bool JImGuiAdaptee::InputText(const std::string& name, std::string& buff, J_GUI_INPUT_TEXT_FLAG flags)
	{
		return ImGui::InputText(name.c_str(), &buff[0], buff.size(), ConvertInputTextFlag(flags));
	}
	bool JImGuiAdaptee::InputText(const std::string& name, std::string& buff, const size_t size, J_GUI_INPUT_TEXT_FLAG flags)
	{
		return ImGui::InputText(name.c_str(), &buff[0], size, ConvertInputTextFlag(flags));
	}
	bool JImGuiAdaptee::InputText(const std::string& name, std::string& buff, std::string& result, const std::string& hint, J_GUI_INPUT_TEXT_FLAG flags)
	{
		bool isInputEnd = false;
		if (result.size() == 0)
			isInputEnd = ImGui::InputTextWithHint(name.c_str(), hint.c_str(), &buff[0], buff.size(), ConvertInputTextFlag(flags));
		else
			isInputEnd = ImGui::InputText(name.c_str(), &buff[0], buff.size(), ConvertInputTextFlag(flags));
		if (isInputEnd)
			result = JCUtil::EraseSideChar(buff, '\0');
		return isInputEnd;
	}
	bool JImGuiAdaptee::InputMultilineText(const std::string& name, std::string& buff, std::string& result, const JVector2<float>& size, J_GUI_INPUT_TEXT_FLAG flags)
	{
		//InputMultilineText는 Enter가 input에 종료가 아닌 개행으로 취급하므로 
		//ImGuiInputTextFlags_EnterReturnsTrue를 적용했을시 따로 ImGui::IsKeyDown(ImGuiKey_Enter)을 호출해줄 필요가있다.
		ImGuiInputTextState* state = ImGui::GetInputTextState(ImGui::GetCurrentWindow()->GetID(name.c_str()));
		if (state != nullptr)
		{
			int nowBuffEnd = buff.find_first_of('\0');
			int preBuffEnd = state->CurLenA - 1;
			if (nowBuffEnd != preBuffEnd)
			{
				if (state->Stb.cursor == preBuffEnd)
					state->Stb.cursor = nowBuffEnd;
				if (state->Stb.select_start == preBuffEnd)
					state->Stb.select_start = nowBuffEnd;
				if (state->Stb.select_end == preBuffEnd)
					state->Stb.select_end = nowBuffEnd;
			}
			const char* buf_end = NULL;
			state->CurLenW = ImTextStrFromUtf8(state->TextW.Data, buff.size(), &buff[0], NULL, &buf_end);
			state->TextAIsValid = true;
			state->TextA.resize(state->TextW.Size * 4 + 1);
			state->CurLenA = ImTextStrToUtf8(state->TextA.Data, state->TextA.Size, state->TextW.Data, NULL);
		}

		auto imguiFlag = ConvertInputTextFlag(flags);
		const bool hasEnterRetTrueFlag = (imguiFlag & ImGuiInputTextFlags_EnterReturnsTrue) > 0;
		bool isInputEnd = ImGui::InputTextMultiline(name.c_str(), &buff[0], buff.size(), size, imguiFlag)
			|| (hasEnterRetTrueFlag && ImGui::IsKeyDown(ImGuiKey_Enter));
		if (isInputEnd)
			result = JCUtil::EraseSideChar(buff, '\0');	//+ '\0' 
		return isInputEnd;
	}
	bool JImGuiAdaptee::InputInt(const std::string& name, int* value, J_GUI_INPUT_TEXT_FLAG flags, int step)
	{  
		return ImGui::InputInt(name.c_str(), value, step, 100, ConvertInputTextFlag(flags));
	}
	bool JImGuiAdaptee::InputFloat(const std::string& name, float* value, J_GUI_INPUT_TEXT_FLAG flags, const uint formatDigit, float step)
	{ 
		return ImGui::InputFloat(name.c_str(), value, step, 100, ConvertFloattingFormat(formatDigit), ConvertInputTextFlag(flags));
	}
	bool JImGuiAdaptee::SliderInt(const std::string& name, int* value, int vMin, int vMax, J_GUI_SLIDER_FLAG flags)
	{
		return ImGui::SliderInt(name.c_str(), value, vMin, vMax, "%d", ConvertSliderFlag(flags));
	}
	bool JImGuiAdaptee::SliderFloat(const std::string& name, float* value, float vMin, float vMax, const uint formatDigit, J_GUI_SLIDER_FLAG flags)
	{
		return ImGui::SliderFloat(name.c_str(), value, vMin, vMax, ConvertFloattingFormat(formatDigit), ConvertSliderFlag(flags));
	}
	bool JImGuiAdaptee::VSliderInt(const std::string& name, JVector2<float> size, int* value, int vMin, int vMax, J_GUI_SLIDER_FLAG flags)
	{
		return ImGui::VSliderInt(name.c_str(), size, value, vMin, vMax, "%d", ConvertSliderFlag(flags));
	}
	bool JImGuiAdaptee::VSliderFloat(const std::string& name, JVector2<float> size, float* value, float vMin, float vMax, const uint formatDigit, J_GUI_SLIDER_FLAG flags)
	{
		return ImGui::VSliderFloat(name.c_str(), size, value, vMin, vMax, ConvertFloattingFormat(formatDigit), ConvertSliderFlag(flags));
	}
	bool JImGuiAdaptee::BeginTabBar(const std::string& name, const J_GUI_TAB_BAR_FLAG flags)
	{
		return ImGui::BeginTabBar(name.c_str(), ConvertTabBarFlag(flags));
	}
	void JImGuiAdaptee::EndTabBar()
	{
		ImGui::EndTabBar();
	}
	bool JImGuiAdaptee::BeginTabItem(const std::string& name, bool* pOpen, J_GUI_TAB_ITEM_FLAG flags)
	{
		return ImGui::BeginTabItem(name.c_str(), pOpen, ConvertTabItemFlag(flags));
	}
	void JImGuiAdaptee::EndTabItem()
	{
		ImGui::EndTabItem();
	}
	bool JImGuiAdaptee::TabItemButton(const std::string& name, const J_GUI_TAB_ITEM_FLAG flags)
	{
		return ImGui::TabItemButton(name.c_str(), ConvertTabItemFlag(flags));
	}
	bool JImGuiAdaptee::BeginTable(const std::string& name, const int columnCount, J_GUI_TABLE_FLAG flags, const JVector2<float> outerSize, const float innerWidth)
	{
		return ImGui::BeginTable(name.c_str(), columnCount, ConvertTableFlag(flags), outerSize, innerWidth);
	}
	void JImGuiAdaptee::EndTable()
	{
		ImGui::EndTable();
	}
	void JImGuiAdaptee::TableSetupColumn(const std::string& name, J_GUI_TABLE_COLUMN_FLAG flags, float initWeight)
	{
		ImGui::TableSetupColumn(name.c_str(), ConvertTableColumnFlag(flags), initWeight);
	}
	void JImGuiAdaptee::TableHeadersRow()
	{
		ImGui::TableHeadersRow();
	}
	void JImGuiAdaptee::TableNextRow()
	{
		ImGui::TableNextRow();
	}
	void JImGuiAdaptee::TableSetColumnIndex(const int index)
	{ 
		ImGui::TableSetColumnIndex(index);
	}
	bool JImGuiAdaptee::BeginMainMenuBar()
	{
		return ImGui::BeginMainMenuBar();
	}
	void JImGuiAdaptee::EndMainMenuBar()
	{
		ImGui::EndMainMenuBar();
	}
	bool JImGuiAdaptee::BeginMenuBar()
	{
		return ImGui::BeginMenuBar();
	}
	void JImGuiAdaptee::EndMenuBar()
	{
		ImGui::EndMenuBar();
	}
	bool JImGuiAdaptee::BeginMenu(const std::string& name, bool enable)
	{
		return ImGui::BeginMenu(name.c_str(), enable);
	}
	void JImGuiAdaptee::EndMenu()
	{
		ImGui::EndMenu();
	}
	bool JImGuiAdaptee::MenuItem(const std::string& name, bool selected, bool enabled)
	{
		return ImGui::MenuItem(name.c_str(), NULL, selected, enabled);
	}
	bool JImGuiAdaptee::MenuItem(const std::string& name, const std::string& shortcut, bool selected, bool enabled)
	{
		return ImGui::MenuItem(name.c_str(), shortcut.c_str(), selected, enabled);
	}
	bool JImGuiAdaptee::BeginCombo(const std::string& name, const std::string& preview, J_GUI_COMBO_FLAG flags)
	{
		return ImGui::BeginCombo(name.c_str(), preview.c_str(), ConvertComboFlag(flags));
	}
	void JImGuiAdaptee::EndCombo()
	{
		ImGui::EndCombo();
	}
	bool JImGuiAdaptee::BeginListBox(const std::string& name, const JVector2<float> size)
	{
		return ImGui::BeginListBox(name.c_str(), size);
	}
	void JImGuiAdaptee::EndListBox()
	{
		ImGui::EndListBox();
	}
	bool JImGuiAdaptee::ColorPicker(const std::string& name, JVector3<float>& color, J_GUI_COLOR_EDIT_FALG_ flags)
	{
		return ImGui::ColorPicker3(name.c_str(), color.Data(), ConvertColorEditFlag((J_GUI_COLOR_EDIT_FALG)flags));
	}
	bool JImGuiAdaptee::ColorPicker(const std::string& name, JVector4<float>& color, J_GUI_COLOR_EDIT_FALG_ flags)
	{
		return ImGui::ColorPicker4(name.c_str(), color.Data(), ConvertColorEditFlag((J_GUI_COLOR_EDIT_FALG)flags));
	}
	void JImGuiAdaptee::Tooltip(const std::string& message)noexcept
	{
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip(message.c_str());
	}
	void JImGuiAdaptee::InvalidImage(const JVector2<float>& size,
		const JVector2<float>& uv0,
		const JVector2<float>& uv1,
		const JVector4<float>& tintCol,
		const JVector4<float>& borderCol)
	{
		auto missing = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::MISSING).Get();
		JGuiImageInfo info(missing);
		ImGui::Image((ImTextureID)ConvertGraphicGpuHandle(info), size, uv0, uv1, tintCol, borderCol);
	}
	void JImGuiAdaptee::Image(const JGuiImageInfo& info,
		const JVector2<float>& size,
		const JVector2<float>& uv0,
		const JVector2<float>& uv1,
		const JVector4<float>& tintCol,
		const JVector4<float>& borderCol)
	{ 
		if (!info.IsValid())
			InvalidImage(size, uv0, uv1);
		else
			ImGui::Image((ImTextureID)ConvertGraphicGpuHandle(info), size, uv0, uv1, tintCol, borderCol);
	}
	bool JImGuiAdaptee::ImageButton(const std::string name,
		const JGuiImageInfo& info,
		const JVector2<float>& size,
		const JVector2<float>& uv0,
		const JVector2<float>& uv1,
		float framePadding,
		const JVector4<float>& bgCol,
		const JVector4<float>& tintCol)
	{
		ImTextureID txtId = ConvertGraphicGpuHandle(info);
		const ImVec2 padding = (framePadding >= 0) ? ImVec2((float)framePadding, (float)framePadding) : ImGui::GetStyle().FramePadding;
		return ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID(name.c_str()),
			txtId,
			size,
			uv0, uv1,
			padding,
			bgCol, tintCol);
	}
	void JImGuiAdaptee::AddImage(const JGuiImageInfo& info,
		const JVector2<float>& pMin,
		const JVector2<float>& pMax,
		bool isFront,
		const JVector4<float>& color,
		const JVector2<float>& uvMin,
		const JVector2<float>& uvMax)
	{
		if (isFront)
			ImGui::GetForegroundDrawList()->AddImage((ImTextureID)ConvertGraphicGpuHandle(info), pMin, pMax, uvMin, uvMax, ConvertUColor(color));
		else
			ImGui::GetCurrentWindow()->DrawList->AddImage((ImTextureID)ConvertGraphicGpuHandle(info), pMin, pMax, uvMin, uvMax, ConvertUColor(color));
	}
	void JImGuiAdaptee::AddRoundedImage(const JGuiImageInfo& info,
		const JVector2<float>& pMin,
		const JVector2<float>& pMax,
		const bool isFront,
		const float rounding,
		J_GUI_DRAW_FLAG_ flag,
		const JVector4<float>& color,
		const JVector2<float>& uvMin,
		const JVector2<float>& uvMax)
	{
		if (isFront)
			ImGui::GetForegroundDrawList()->AddImageRounded((ImTextureID)ConvertGraphicGpuHandle(info), pMin, pMax, uvMin, uvMax, ConvertUColor(color), rounding, ConvertDrawFlag((J_GUI_DRAW_FLAG)flag));
		else
			ImGui::GetCurrentWindow()->DrawList->AddImageRounded((ImTextureID)ConvertGraphicGpuHandle(info), pMin, pMax, uvMin, uvMax, ConvertUColor(color), rounding, ConvertDrawFlag((J_GUI_DRAW_FLAG)flag));
	}
	bool JImGuiAdaptee::ImageSelectable(const std::string name,
		JGuiImageInfo info,
		bool& pressed,
		bool changeValueIfPreesd,
		const JVector2<float>& size,
		const bool useRestoreCursorPos)
	{
		const JVector2<float> preCursor = ImGui::GetCursorPos();
		const bool isPress = Selectable("##" + name, false, J_GUI_SELECTABLE_FLAG_ALLOW_ITEM_OVERLAP, size);
		ImGui::SetCursorPos(preCursor);

		info.displayAllType = false;
		Image(info, size);

		if (isPress && changeValueIfPreesd)
			pressed = !pressed;

		if (useRestoreCursorPos)
			ImGui::SetCursorPos(preCursor);
		return isPress;
	}
	bool JImGuiAdaptee::Switch(const std::string& name,
		bool& pressed,
		bool changeValueIfPreesd,
		const JVector2<float>& size)
	{
		if (pressed)
			PushColor(J_GUI_COLOR::HEADER, GetColor(J_GUI_COLOR::HEADER_ACTIVE));
		const bool isPress = ImGui::Button(name.c_str(), size);
		if (pressed)
			PopColor(1);
		if (isPress && changeValueIfPreesd)
			pressed = !pressed;
		return isPress;
	}
	bool JImGuiAdaptee::ImageSwitch(const std::string name,
		JGuiImageInfo info,
		bool& pressed,
		bool changeValueIfPreesd,
		const JVector2<float>& size,
		const JVector4<float>& bgColor, 
		const JVector4<float>& frameColor,
		const float frameThickness)
	{  
		const JVector2<float> pos = ImGui::GetCurrentWindow()->DC.CursorPos;
		DrawRectFilledColor(pos, size, pressed ? GetColor(J_GUI_COLOR::HEADER_ACTIVE) : bgColor, ImGui::GetStyle().FrameRounding, J_GUI_DRAW_FLAG_ROUND_ALL, true);
		if (frameThickness > 0)
			DrawRectFrame(pos, size, frameColor, frameThickness, ImGui::GetStyle().FrameRounding, J_GUI_DRAW_FLAG_ROUND_ALL, true);

		const JVector2<float> preCursor = ImGui::GetCursorPos(); 
		const bool isPress = Selectable(name, false, J_GUI_SELECTABLE_FLAG_ALLOW_ITEM_OVERLAP, size);
 
		ImGui::SetCursorPos(preCursor);
		info.displayAllType = false;
		Image(info, size);

		if (isPress && changeValueIfPreesd)
			pressed = !pressed;
		return isPress;
	}
	bool JImGuiAdaptee::ImageButton(const std::string name,
		JGuiImageInfo info,
		const JVector2<float>& size,
		const JVector4<float>& bgColor, 
		const JVector4<float>& frameColor,
		const float frameThickness)
	{ 
		const JVector2<float> pos = ImGui::GetCurrentWindow()->DC.CursorPos; 
		DrawRectFilledColor(pos, size, bgColor, ImGui::GetStyle().FrameRounding, J_GUI_DRAW_FLAG_ROUND_ALL, true);
		if (frameThickness > 0)
			DrawRectFrame(pos, size, frameColor, frameThickness, ImGui::GetStyle().FrameRounding, J_GUI_DRAW_FLAG_ROUND_ALL, true);

		const JVector2<float> preCursor = ImGui::GetCursorPos();
		const bool isPress = JImGuiAdaptee::Selectable(name, false, J_GUI_SELECTABLE_FLAG_ALLOW_ITEM_OVERLAP, size);
		ImGui::SetCursorPos(preCursor);

		info.displayAllType = false;
		Image(info, size);

		return isPress;
	}
	bool JImGuiAdaptee::MaximizeButton(const bool isLocatedCloseBtnLeftSide)
	{
		ImGuiWindow* wnd = ImGui::GetCurrentWindow();
		ImGuiDockNode* dockNode = wnd->DockNode;

		const ImRect titleBarRect = ImGui::GetCurrentWindow()->TitleBarRect();
		const JVector2<float> titleBarSize = titleBarRect.GetSize();

		float xOffset = 0;
		if (isLocatedCloseBtnLeftSide)
			xOffset = CalCloseButtionSize();	//close btn size *  3.5f

		const JVector2<float> selectablePos = ImGui::GetWindowPos() + JVector2<float>(ImGui::GetWindowSize().x - titleBarSize.y - xOffset, 0);
		const JVector2<float> selectableSize = JVector2<float>(titleBarSize.y, titleBarSize.y);

		const JVector2<float> rectPos = selectablePos + JVector2<float>(titleBarSize.y * 0.15f, titleBarSize.y * 0.15f);
		const JVector2<float> rectSize = selectableSize * 0.65f;

		if (dockNode != nullptr && dockNode->TabBar != nullptr)
		{
			ImRect dockRect = dockNode->TabBar->Tabs[dockNode->TabBar->Tabs.size() - 1].Window->DockTabItemRect;
			const float tabRectX = dockRect.GetCenter().x + dockRect.GetSize().x * 0.6f;
			if (tabRectX > selectablePos.x)
				return false;
		}

		ImGui::PushClipRect(selectablePos, selectablePos + selectableSize, false);
		DrawRectFrame(rectPos, rectSize, GetColor(J_GUI_COLOR::TEXT), 1.0f, 1.0f, J_GUI_DRAW_FLAG_ROUND_ALL, true);
		ImGui::PopClipRect();
		return ImGui::IsWindowFocused() && ImGui::IsMouseClicked(0) && IsMouseInRect(selectablePos, selectablePos + selectableSize);
	}
	bool JImGuiAdaptee::MinimizeButton(const bool isLocatedCloseBtnLeftSide, const bool isLocatedMaximizeBtnLeftSize)
	{
		ImGuiWindow* wnd = ImGui::GetCurrentWindow();
		ImGuiDockNode* dockNode = wnd->DockNode;

		const ImRect titleBarRect = ImGui::GetCurrentWindow()->TitleBarRect();
		const JVector2<float> titleBarSize = titleBarRect.GetSize();
		 
		float xOffset = 0;
		if (isLocatedCloseBtnLeftSide)
			xOffset = CalCloseButtionSize();	//close btn size *  3.5f
		if (isLocatedMaximizeBtnLeftSize)
			xOffset += CalCloseButtionSize();	//close btn size *  3.5f

		const JVector2<float> selectablePos = ImGui::GetWindowPos() + JVector2<float>(ImGui::GetWindowSize().x - titleBarSize.y - xOffset, 0);
		const JVector2<float> selectableSize = JVector2<float>(titleBarSize.y, titleBarSize.y);

		const JVector2<float> linePos0 = selectablePos + JVector2<float>(titleBarSize.y * 0.15f, titleBarSize.y * 0.5f);
		const JVector2<float> linePos1 = linePos0 + JVector2F(selectableSize.x * 0.65f, .0);

		if (dockNode != nullptr && dockNode->TabBar != nullptr)
		{
			ImRect dockRect = dockNode->TabBar->Tabs[dockNode->TabBar->Tabs.size() - 1].Window->DockTabItemRect;
			const float tabRectX = dockRect.GetCenter().x + dockRect.GetSize().x * 0.6f;
			if (tabRectX > selectablePos.x)
				return false;
		}

		JVector2F preCursor = ImGui::GetCursorPos();
		ImGui::PushClipRect(selectablePos, selectablePos + selectableSize, false);
		AddLine(linePos0, linePos1, GetColor(J_GUI_COLOR::TEXT), 2.5f);
		ImGui::PopClipRect();
		ImGui::SetCursorPos(preCursor);
		return ImGui::IsWindowFocused() && ImGui::IsMouseClicked(0) && IsMouseInRect(selectablePos, selectablePos + selectableSize);
	}
	bool JImGuiAdaptee::PreviousSizeButton(const bool isLocatedCloseBtnLeftSide, const bool isLocatedMaximizeBtnLeftSize)
	{
		ImGuiWindow* wnd = ImGui::GetCurrentWindow();
		ImGuiDockNode* dockNode = wnd->DockNode;

		const ImRect titleBarRect = ImGui::GetCurrentWindow()->TitleBarRect();
		const JVector2<float> titleBarSize = titleBarRect.GetSize();

		float xOffset = 0;
		if (isLocatedCloseBtnLeftSide)
			xOffset += CalCloseButtionSize();	//close btn size *  3.5f
		if (isLocatedMaximizeBtnLeftSize)
			xOffset += CalCloseButtionSize();	//close btn size *  3.5f

		const JVector2<float> selectablePos = ImGui::GetWindowPos() + JVector2<float>(ImGui::GetWindowSize().x - titleBarSize.y - xOffset, 0);
		const JVector2<float> selectableSize = JVector2<float>(titleBarSize.y, titleBarSize.y);

		const JVector2<float> backRectPos = selectablePos + JVector2<float>(titleBarSize.y * 0.3f, titleBarSize.y * 0.15f);
		const JVector2<float> backRectSize = selectableSize * 0.5f;

		const JVector2<float> frontRectPos = selectablePos + JVector2<float>(titleBarSize.y * 0.15f, titleBarSize.y * 0.3f);
		const JVector2<float> frontRectSize = selectableSize * 0.5f;

		if (dockNode != nullptr && dockNode->TabBar != nullptr)
		{
			ImRect dockRect = dockNode->TabBar->Tabs[dockNode->TabBar->Tabs.size() - 1].Window->DockTabItemRect;
			const float tabRectX = dockRect.GetCenter().x + dockRect.GetSize().x * 0.6f;
			if (tabRectX > selectablePos.x)
				return false;
		}

		ImGui::PushClipRect(selectablePos, selectablePos + selectableSize, false);
		DrawRectFrame(backRectPos, backRectSize, GetColor(J_GUI_COLOR::TEXT), 1.0f, 1.0f, J_GUI_DRAW_FLAG_ROUND_ALL, true);
		DrawRectFilledColor(frontRectPos, frontRectSize, GetColor(J_GUI_COLOR::TITLE_BG), 1.0f, J_GUI_DRAW_FLAG_ROUND_ALL, true);
		DrawRectFrame(frontRectPos, frontRectSize, GetColor(J_GUI_COLOR::TEXT), 1.0f, 1.0f, J_GUI_DRAW_FLAG_ROUND_ALL, true);
		ImGui::PopClipRect();
		return ImGui::IsWindowFocused() && ImGui::IsMouseClicked(0) && IsMouseInRect(selectablePos, selectablePos + selectableSize);
	}
	void JImGuiAdaptee::DrawRectFilledMultiColor(const JVector2<float>& pos,
		const JVector2<float>& size,
		const JVector4<float>& upLeftCol,
		const JVector4<float>& upRightCol,
		const JVector4<float>& downLeftCol,
		const JVector4<float>& downRightCol,
		const float rounding,
		const J_GUI_DRAW_FLAG_ drawFlag,
		const bool useRestoreCursorPos)noexcept
	{
		JVector2<float> nowCursor = ImGui::GetCursorPos();
		JVector2<float> bboxMin = JVector2<float>(pos.x, pos.y);
		JVector2<float> bboxMax = pos + size;

		if ((drawFlag & J_GUI_DRAW_FLAG_ROUND_ALL) > 0 && rounding > 0)
		{
			ImGui::GetWindowDrawList()->AddRectFilledMultiColor(bboxMin, bboxMax,
				ConvertUColor(upLeftCol),
				ConvertUColor(upRightCol),
				ConvertUColor(downRightCol),
				ConvertUColor(downLeftCol),
				rounding,
				ConvertDrawFlag((J_GUI_DRAW_FLAG)drawFlag));
		}
		else
		{
			ImGui::GetWindowDrawList()->AddRectFilledMultiColor(bboxMin, bboxMax,
				ConvertUColor(upLeftCol),
				ConvertUColor(upRightCol),
				ConvertUColor(downRightCol),
				ConvertUColor(downLeftCol));
		}

		if (useRestoreCursorPos)
			ImGui::SetCursorPos(nowCursor);
	}
	void JImGuiAdaptee::DrawRectFilledColor(const JVector2<float>& pos,
		const JVector2<float>& size,
		const JVector4<float>& color,
		const float rounding,
		const J_GUI_DRAW_FLAG_ drawFlag,
		const bool useRestoreCursorPos)noexcept
	{
		JVector2<float> nowCursor = ImGui::GetCursorPos();
		JVector2<float> bboxMin = JVector2<float>(pos.x, pos.y);
		JVector2<float> bboxMax = pos + size;

		ImGui::GetWindowDrawList()->AddRectFilled(bboxMin, bboxMax,
			ConvertUColor(color),
			rounding, ConvertDrawFlag((J_GUI_DRAW_FLAG)drawFlag));

		if (useRestoreCursorPos)
			ImGui::SetCursorPos(nowCursor);
	}
	void JImGuiAdaptee::DrawRectFrame(const JVector2<float>& pos,
		const JVector2<float>& size,
		const JVector4<float>& color,
		const float thickness,
		const float rounding,
		const J_GUI_DRAW_FLAG_ drawFlag,
		const bool useRestoreCursorPos)noexcept
	{
		JVector2<float> nowCursor = ImGui::GetCursorPos();
		JVector2<float> bboxMin = JVector2<float>(pos.x, pos.y);
		JVector2<float> bboxMax = pos + size;

		ImGui::GetWindowDrawList()->AddRect(bboxMin, bboxMax,
			ConvertUColor(color),
			rounding, ConvertDrawFlag((J_GUI_DRAW_FLAG)drawFlag),
			thickness);

		if (useRestoreCursorPos)
			ImGui::SetCursorPos(nowCursor);
	}
	void JImGuiAdaptee::DrawToolTipBox(const std::string& uniqueLabel,
		const std::string& tooltip,
		const JVector2<float>& pos,
		const JVector2<float>& padding,
		const bool useRestoreCursorPos)
	{
		const JVector2<float> nowCursor = ImGui::GetCursorPos();
		JVector2<float> boxSize = ImGui::CalcTextSize(tooltip.c_str()) + (padding * 2);
		boxSize.x += GetAlphabetSize().x * 3;

		JEditorTextAlignCalculator textCal;
		textCal.Update(tooltip, boxSize, false);
		 
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(boxSize);
		J_GUI_WINDOW_FLAG_ flag = J_GUI_WINDOW_FLAG_NO_TITLE_BAR |
			J_GUI_WINDOW_FLAG_AUTO_RESIZE |
			J_GUI_WINDOW_FLAG_NO_NAV |
			J_GUI_WINDOW_FLAG_NO_SAVE |
			J_GUI_WINDOW_FLAG_NO_BRING_TO_FRONT_ON_FOCUS |
			J_GUI_WINDOW_FLAG_NO_FOCUS_ON_APPEARING;

		JImGuiAdaptee::BeginWindow(uniqueLabel, 0, (J_GUI_WINDOW_FLAG)flag);
		ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

		ImGui::SetCursorScreenPos(pos + padding);
		JImGuiAdaptee::Text(textCal.MiddleAligned());
		JImGuiAdaptee::EndWindow();
		if (useRestoreCursorPos)
			ImGui::SetCursorPos(nowCursor);
	}
	void JImGuiAdaptee::DrawToolTipBox(const std::string& uniqueLabel,
		const std::string& tooltip,
		const JVector2<float>& pos,
		const float maxWidth,
		const JVector2<float>& padding,
		const J_EDITOR_ALIGN_TYPE alignType,
		const bool useRestoreCursorPos)
	{
		const JVector2<float> nowCursor = ImGui::GetCursorPos();
		JEditorTextAlignCalculator textAlignCal;
		textAlignCal.Update(tooltip, JVector2<float>(maxWidth, 0), false);

		const std::string alignedTooltip = textAlignCal.Aligned(alignType);
		const JVector2<float> boxSize = ImGui::CalcTextSize(alignedTooltip.c_str()) + (padding * 2);
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(boxSize);
		BeginWindow(uniqueLabel, 0, (J_GUI_WINDOW_FLAG)(J_GUI_WINDOW_FLAG_NO_TITLE_BAR | J_GUI_WINDOW_FLAG_AUTO_RESIZE));

		ImGui::SetCursorScreenPos(pos + padding);
		Text(alignedTooltip);
		EndWindow();
		if (useRestoreCursorPos)
			ImGui::SetCursorPos(nowCursor);
	}
	void JImGuiAdaptee::DrawCircleFilledColor(const JVector2<float>& centerPos,
		const float radius,
		const JVector4<float>& color,
		const bool useRestoreCursorPos)
	{
		JVector2<float> nowCursor = ImGui::GetCursorPos();

		ImGui::GetWindowDrawList()->AddCircleFilled(centerPos, radius, ConvertUColor(color));
		if (useRestoreCursorPos)
			ImGui::SetCursorPos(nowCursor);
	}
	void JImGuiAdaptee::DrawCircle(const JVector2<float>& centerPos,
		const float radius,
		const JVector4<float>& color,
		const bool useRestoreCursorPos,
		const float thickness)
	{
		JVector2<float> nowCursor = ImGui::GetCursorPos();
		ImGui::GetWindowDrawList()->AddCircle(centerPos, radius, ConvertUColor(color), 0, thickness);
		if (useRestoreCursorPos)
			ImGui::SetCursorPos(nowCursor);
	}
	void JImGuiAdaptee::PushClipRect(const JVector2<float>& rectMinP, const JVector2<float>& rectMaxP, const bool intersectWithCurrentClipRect)
	{
		ImGui::GetCurrentWindow()->DrawList->PushClipRect(rectMinP, rectMaxP, intersectWithCurrentClipRect);
	}
	void JImGuiAdaptee::AddLine(const JVector2<float>& p1, const JVector2<float>& p2, const JVector4<float>& color, const float thickness)
	{
		ImGui::GetCurrentWindow()->DrawList->AddLine(p1, p2, ConvertUColor(color), thickness);
	}
	void JImGuiAdaptee::AddTriangle(const JVector2<float>& p1, const JVector2<float>& p2, const JVector2<float>& p3, const JVector4<float>& color, const float thickness)
	{
		ImGui::GetCurrentWindow()->DrawList->AddTriangle(p1, p2, p3, ConvertUColor(color), thickness);
	}
	void JImGuiAdaptee::AddTriangleFilled(const JVector2<float>& p1, const JVector2<float>& p2, const JVector2<float>& p3, const JVector4<float>& color)
	{
		ImGui::GetCurrentWindow()->DrawList->AddTriangleFilled(p1, p2, p3, ConvertUColor(color));
	}
	void JImGuiAdaptee::NewLine()noexcept
	{
		ImGui::NewLine();
	}
	void JImGuiAdaptee::SameLine()noexcept
	{
		ImGui::SameLine();
	}
	void JImGuiAdaptee::Separator()noexcept
	{
		ImGui::Separator();
	}
	void JImGuiAdaptee::Indent(const float width)noexcept
	{
		ImGui::Indent(width);
	}
	void JImGuiAdaptee::UnIndent(const float width)noexcept
	{
		ImGui::Unindent(width);
	}
	float JImGuiAdaptee::IndentMovementPixel(const float width)const noexcept
	{ 
		//Is same as Indent formula
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		float indentX = (width != 0.0f) ? width : g.Style.IndentSpacing;
		return  window->DC.Indent.x +  indentX + window->DC.ColumnsOffset.x;
	}

	JVector2<float> JImGuiAdaptee::GetMainWorkPos()const noexcept
	{
		return ImGui::GetMainViewport()->WorkPos;
	}
	JVector2<float> JImGuiAdaptee::GetMainWorkSize()const noexcept
	{
		return ImGui::GetMainViewport()->WorkSize;
	}
	JVector2<float> JImGuiAdaptee::GetWindowPos()const noexcept
	{
		return ImGui::GetWindowPos();
	}
	JVector2<float> JImGuiAdaptee::GetWindowSize()const noexcept
	{
		return ImGui::GetWindowSize();
	}
	JVector2<float> JImGuiAdaptee::GetRestWindowSpace()const noexcept
	{
		return ImGui::GetWindowSize() - (ImGui::GetCursorScreenPos() - ImGui::GetWindowPos());
	}
	JVector2<float> JImGuiAdaptee::GetWindowContentsSize()const noexcept
	{
		const JVector2<float> windowPaddig = ImGui::GetStyle().WindowPadding;
		const float windowBorder = ImGui::GetStyle().WindowBorderSize;
		return ImGui::GetWindowSize() - (windowPaddig * 2) - CreateVec2(windowBorder * 2);
	}
	JVector2<float> JImGuiAdaptee::GetRestWindowContentsSize()const noexcept
	{
		return ImGui::GetContentRegionAvail();
		/*
		const JVector2<uint> wndPos = ImGui::GetWindowPos();
		const JVector2<uint> cursorPos = ImGui::GetCursorScreenPos();
		if (wndPos.x > cursorPos.x || wndPos.y > cursorPos.y)
			return GetGuiWindowSize();

		const JVector2<uint> localCursorPos = cursorPos - wndPos;
		const JVector2<uint> windowPaddig = ImGui::GetStyle().WindowPadding;
		const float windowBorder = ImGui::GetStyle().WindowBorderSize;

		const bool isOverPaddingBorderX = localCursorPos.x > (windowPaddig.x + windowBorder);
		const bool isOverPaddingBorderY = localCursorPos.y > (windowPaddig.y + windowBorder);

		const JVector2<uint> subtract = JVector2<uint>(isOverPaddingBorderX ? windowPaddig.x : 0,
			isOverPaddingBorderY ? windowPaddig.y : 0);

		return ImGui::GetWindowSize() - subtract - (ImGui::GetCursorScreenPos() - ImGui::GetWindowPos());
		*/
	}
	JVector2<float> JImGuiAdaptee::GetWindowMenuBarSize()const noexcept
	{
		return ImGui::GetCurrentWindow()->MenuBarRect().GetSize();
	}
	JVector2<float> JImGuiAdaptee::GetWindowTitleBarSize()const noexcept
	{
		return ImGui::GetCurrentWindow()->TitleBarRect().GetSize();
	}
	int JImGuiAdaptee::GetWindowOrder(const GuiID windowID)const noexcept
	{
		ImGuiContext* cont = ImGui::GetCurrentContext();
		const int wndCount = (int)cont->Windows.size();
		for (int i = 0; i < wndCount; ++i)
		{
			if (cont->Windows[i]->ID == windowID)
				return i;
		}
		return invalidIndex;
	}
	bool JImGuiAdaptee::GetWindowInfo(const std::string& wndName, _Inout_ JGuiWindowInfo& info)const noexcept
	{
		return StuffWindowInfo(ImGui::FindWindowByName(wndName.c_str()), info);
	}
	bool JImGuiAdaptee::GetWindowInfo(const GuiID windowID, _Inout_ JGuiWindowInfo& info)const noexcept
	{ 
		return StuffWindowInfo(ImGui::FindWindowByID(windowID), info);
	}
	bool JImGuiAdaptee::GetCurrentWindowInfo(_Inout_ JGuiWindowInfo& info)const noexcept
	{ 
		return StuffWindowInfo(ImGui::GetCurrentWindow(), info);
	}
	std::vector<JGuiWindowInfo> JImGuiAdaptee::GetDisplayedWindowInfo(const bool isSortedBackToFront)const noexcept
	{   
		ImVector<ImGuiWindow*>& guiWindowVec = ImGui::GetCurrentContext()->Windows;
		const uint count = (uint)guiWindowVec.size();
		std::vector<JGuiWindowInfo> result(count);
		for (uint i = 0; i < count; ++i)
			StuffWindowInfo(guiWindowVec[i], result[i]);
		return result;
	}
	std::set<GuiID>JImGuiAdaptee::GetWindowOpendTreeNodeID(const GuiID windowID)const noexcept
	{
		auto wnd = ImGui::FindWindowByID(windowID);
		if (wnd == nullptr)
			return std::set<GuiID>{};

		auto storage = wnd->DC.StateStorage;
		std::set<GuiID> res;
		for (const auto& data : storage->Data)
		{
			if(data.val_i > 0)
				res.emplace(data.key);
		}
		return res;
	}
	void JImGuiAdaptee::SetNextWindowPos(const JVector2<float>& pos, J_GUI_CONDIITON flag)noexcept
	{
		ImGui::SetNextWindowPos(pos, flag);
	}
	void JImGuiAdaptee::SetNextWindowSize(const JVector2<float>& size, J_GUI_CONDIITON flag)noexcept
	{
		ImGui::SetNextWindowSize(size, flag);
	}
	void JImGuiAdaptee::SetNextWindowFocus()noexcept
	{ 
		ImGui::SetNextWindowFocus();
	} 
	bool JImGuiAdaptee::IsCurrentWindowFocused(J_GUI_FOCUS_FLAG flag)const noexcept
	{ 
		return ImGui::IsWindowFocused(ConvertFocusFlag(flag));
	} 
	void JImGuiAdaptee::FocusWindow(const GuiID windowID)
	{
		ImGui::FocusWindow(ImGui::FindWindowByID(windowID));
	}
	void JImGuiAdaptee::FocusCurrentWindow()
	{
		ImGui::FocusWindow(ImGui::GetCurrentWindow());
	}
	void JImGuiAdaptee::RestoreFromMaximize(const GuiID windowID, const std::vector<GuiID>& preTabItemID)
	{
		ImGuiWindow* window = ImGui::FindWindowByID(windowID);
		if (window == nullptr ||
			window->DockNode == nullptr ||
			window->DockNode->TabBar == nullptr ||
			preTabItemID.size() == 0)
			return;

		ImGuiTabBar* tabBar = window->DockNode->TabBar;
		const uint lastTabItemCount = (uint)preTabItemID.size();
		if (lastTabItemCount != tabBar->Tabs.size())
			return;

		ImVector<ImGuiTabItem> copiedTab = tabBar->Tabs;
		for (uint i = 0; i < lastTabItemCount; ++i)
		{
			for (uint j = i; j < lastTabItemCount; ++j)
			{
				if (preTabItemID[i] == copiedTab[j].ID)
				{
					std::swap(copiedTab[i], copiedTab[j]);
					break;
				}
			}
		}
		tabBar->Tabs = copiedTab;
	}
	GuiID JImGuiAdaptee::CalCurrentWindowItemID(const std::string& label)const noexcept
	{
		return ImGui::GetCurrentWindow()->GetID(label.c_str());
	}
	JVector2<float> JImGuiAdaptee::GetLastItemRectMin()const noexcept
	{
		return ImGui::GetItemRectMin();
	}
	JVector2<float> JImGuiAdaptee::GetLastItemRectMax()const noexcept
	{
		return ImGui::GetItemRectMax();
	}
	JVector2<float> JImGuiAdaptee::GetLastItemRectSize()const noexcept
	{
		return ImGui::GetItemRectSize();
	}
	void JImGuiAdaptee::SetNextItemWidth(const float width)noexcept
	{
		ImGui::SetNextItemWidth(width);
	}
	void JImGuiAdaptee::SetNextItemOpen(const bool value, J_GUI_CONDIITON flag)noexcept
	{
		ImGui::SetNextItemOpen(value, flag);
	}
	void JImGuiAdaptee::SetLastItemDefaultFocus()
	{
		ImGui::SetItemDefaultFocus();
	}
	bool JImGuiAdaptee::IsLastItemActivated()const noexcept
	{
		return ImGui::IsItemActivated();
	}
	bool JImGuiAdaptee::IsLastItemHovered(J_GUI_HOVERED_FLAG flag)const noexcept
	{
		return ImGui::IsItemHovered(ConvertHoveredFlag(flag));
	}
	bool JImGuiAdaptee::IsLastItemClicked(const Core::J_MOUSE_BUTTON btn)const noexcept
	{
		return ImGui::IsItemClicked(ConvertMouseBtnType(btn));
	}
	bool JImGuiAdaptee::IsTreeNodeArrowHovered(const std::string& nodeName)const noexcept
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiContext& g = *GImGui;

		ImGuiStyle& style = ImGui::GetStyle();
		const JVector2<float> padding = style.FramePadding;
		const JVector2<float> label_size = ImGui::CalcTextSize(nodeName.c_str(), NULL, false);

		const float text_offset_x = g.FontSize + padding.x * 3;
		const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);
		const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);
		JVector2<float> text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
		const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
		const float arrow_hit_x2 = (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;

		return (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);
	}
	bool JImGuiAdaptee::CanUseWidget(const Core::J_GUI_WIDGET_TYPE type)const noexcept
	{
		return true;
	}
	JVector2<float> JImGuiAdaptee::CalDefaultButtonSize(const std::string& label)const noexcept
	{
		ImGuiStyle& style = ImGui::GetStyle();
		const JVector2<float> labelSize = ImGui::CalcTextSize(label.c_str(), NULL, true);
		return 	JVector2<float>(labelSize.x + style.FramePadding.x * 2.0f, labelSize.y + style.FramePadding.y * 2.0f);
	}
	JVector2<float> JImGuiAdaptee::CalButtionSize(const std::string& label, const JVector2<float>& size)const noexcept
	{
		const JVector2<float> defaultSize = CalDefaultButtonSize(label);
		return ImGui::CalcItemSize(size, defaultSize.x, defaultSize.y);
	}
	float JImGuiAdaptee::CalCloseButtionSize()const noexcept
	{
		return ImMax(2.0f, GImGui->FontSize * 0.5f + 1.0f) * 3.5f;
	}
	void JImGuiAdaptee::PushItemFlag(J_GUI_ITEM_FLAG_ flag, const bool value)noexcept
	{
		ImGui::PushItemFlag(ConvertItemFlag((J_GUI_ITEM_FLAG)flag), value);
	}
	void JImGuiAdaptee::PopItemFlag()noexcept
	{
		ImGui::PopItemFlag();
	}
	void JImGuiAdaptee::PushItemWidth(const float width)noexcept
	{
		ImGui::PushItemWidth(width);
	}
	void JImGuiAdaptee::PopItemWidth()noexcept
	{
		ImGui::PopItemWidth();
	}
#pragma endregion
#pragma region Docking
	bool JImGuiAdaptee::GetDockNodeInfoByWindowName(const std::string& windowName, _Inout_ JGuiDockNodeInfo& info)const noexcept
	{
		auto window = ImGui::FindWindowByName(windowName.c_str());
		if (window == nullptr)
			return false;
		return StuffDockNodeInfo(window->DockNode, info);
	}
	bool JImGuiAdaptee::GetDockNodeInfo(const std::string& dockNodeName, _Inout_ JGuiDockNodeInfo& info)const noexcept
	{
		return GetDockNodeInfo(ImGui::GetID(dockNodeName.c_str()), info);
	}
	bool JImGuiAdaptee::GetDockNodeInfo(const GuiID dockID, _Inout_ JGuiDockNodeInfo& info)const noexcept
	{
		ImGuiDockNode* dockNode = static_cast<ImGuiDockNode*>(ImGui::GetCurrentContext()->DockContext.Nodes.GetVoidPtr(dockID));
		if (dockNode == nullptr)
			return false;

		return StuffDockNodeInfo(dockNode, info);
	}
	bool JImGuiAdaptee::GetDockNodeHostWindowInfo(const GuiID childDockID, _Out_ JGuiWindowInfo& info) const noexcept
	{
		ImGuiDockNode* dockNode = static_cast<ImGuiDockNode*>(ImGui::GetCurrentContext()->DockContext.Nodes.GetVoidPtr(childDockID));
		if (dockNode == nullptr)
			return false;
		 
		if (dockNode->HostWindow == nullptr)
			return false;

		return GetWindowInfo(dockNode->HostWindow->ID, info);
	}
	bool JImGuiAdaptee::GetCurrentDockNodeInfo(_Out_ JGuiDockNodeInfo& info)const noexcept
	{  
		return StuffDockNodeInfo(ImGui::GetWindowDockNode(), info);
	}
	bool JImGuiAdaptee::HasDockNode(const std::string& dockNodeName)const noexcept
	{
		ImGuiDockNode* dockNode = static_cast<ImGuiDockNode*>(ImGui::GetCurrentContext()->DockContext.Nodes.GetVoidPtr(ImGui::GetID(dockNodeName.c_str())));
		return dockNode != nullptr;
	}
	bool JImGuiAdaptee::CanUseDockHirechary()noexcept
	{
		return true;
	}
	void JImGuiAdaptee::BuildDockHirechary(const std::vector<std::unique_ptr<JGuiDockBuildNode>>& nodeVec)
	{
		/*
		* //imgui dock node tree is binary
		* //imgui split dir
		*/
		const uint nodeCount = (uint)nodeVec.size();
		if (nodeCount < 2)
			return;

		for (const auto& data : nodeVec)
		{
			if (data->parentNumber != invalidIndex)
			{
				ImGui::Begin(data->windowName.c_str());
				ImGui::End();
			}
		}

		const JGuiDockBuildNode* root = nodeVec[0].get();
		ImGuiID dockspaceId = ImGui::GetID(root->dockSpaceName.c_str());
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGui::DockBuilderRemoveNode(dockspaceId);
		ImGui::DockBuilderAddNode(dockspaceId);
		ImGui::DockBuilderSetNodePos(dockspaceId, viewport->WorkPos);
		ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);

		//ImGuiID dock_main = dockspaceId;
		std::vector<ImGuiID> dockID(nodeCount);
		dockID[0] = dockspaceId;

		ImGui::DockBuilderSplitNode(dockID[0],
			ConvertDirType(nodeVec[1]->splitDir),
			nodeVec[1]->splitRate,
			&dockID[1],
			&dockID[2]);
		int nextNodeIndex = 2;

		for (uint i = nextNodeIndex; i < nodeCount; ++i)
		{
			int paerntNumber = nodeVec[i]->parentNumber;
			ImGuiDir dir = ConvertDirType(nodeVec[i]->splitDir);
			if (dir == ImGuiDir_None)
			{
				dockID[i] = ImGui::DockBuilderSplitNode(dockID[paerntNumber],
					ImGuiDir_Up,
					nodeVec[i]->splitRate,
					&dockID[paerntNumber],
					nullptr);
			}
			else
			{
				dockID[i] = ImGui::DockBuilderSplitNode(dockID[paerntNumber],
					ConvertDirType(nodeVec[i]->splitDir),
					nodeVec[i]->splitRate,
					nullptr,
					&dockID[paerntNumber]);
			}

			//MessageBox(0, std::to_wstring(dockID[i]).c_str(), std::to_wstring(paerntNumber).c_str(), 0);
		}
		for (uint i = 0; i < nodeCount; ++i)
			ImGui::DockBuilderDockWindow(nodeVec[i]->windowName.c_str(), dockID[i]);
		ImGui::DockBuilderFinish(dockspaceId);
	}
	void JImGuiAdaptee::UpdateDockSpace(const std::string& name, const JVector2<float>& size, J_GUI_DOCK_NODE_FLAG_ dockspaceFlag)
	{
		ImGui::DockSpace(ImGui::GetID(name.c_str()), size, ConvertDockFlag((J_GUI_DOCK_NODE_FLAG)dockspaceFlag));
	}
	void JImGuiAdaptee::OverrideNextDockNodeFlag(J_GUI_DOCK_NODE_FLAG_ flag)
	{
		ImGuiWindowClass window_class;
		ImGuiDockNodeFlags_ oldFlag = (ImGuiDockNodeFlags_)window_class.DockNodeFlagsOverrideSet;
		ImGuiDockNodeFlags_ newFlag = ConvertDockFlag((J_GUI_DOCK_NODE_FLAG)flag);
		window_class.DockNodeFlagsOverrideSet = Core::AddSQValueEnum(oldFlag, newFlag);
		ImGui::SetNextWindowClass(&window_class);

		//ImGuiWindowClass window_class;
		//window_class.DockNodeFlagsOverrideSet = ConvertDockFlag((J_GUI_DOCK_NODE_FLAG)flag);
		//ImGui::SetNextWindowClass(&window_class);
	}
	void JImGuiAdaptee::CloseTabItem(const GuiID windowID)
	{
		ImGuiWindow* window = nullptr;
		ImGuiContext* cont = ImGui::GetCurrentContext();
		const int wndCount = (int)cont->Windows.size();
		for (int i = 0; i < wndCount; ++i)
		{
			if (cont->Windows[i]->ID == windowID)
			{
				window = cont->Windows[i];
				break;
			}
		}
		if (window->DockNode != nullptr)
			window->DockNode->WantCloseTabId = window->ID;
	}
	std::unique_ptr<JDockUpdateHelper> JImGuiAdaptee::CreateDockUpdateHelper(const J_EDITOR_PAGE_TYPE pageType)
	{
		return std::make_unique<JImGuiDockUpdateHelper>(pageType);
	}
#pragma endregion
}