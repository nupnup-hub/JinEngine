#pragma once
#include"JGuiType.h"  
#include"JGuiWindow.h"
#include"../Align/JEditorAlignType.h" 
#include"../EditTool/JEditorInputBuffHelper.h"
#include"../Page/JEditorPageEnum.h"
#include"../Interface/JEditorTransitionInterface.h"
#include"../../Core/Math/JVector.h"
#include"../../Core/Reflection/JGuiWidgetType.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Core/Reflection/JReflection.h"
#include"../../Core/Input/JMouse.h"
#include"../../Core/Input/JKeyboard.h"
#include"../../Graphic/Gui/JGuiData.h"  

namespace JinEngine
{
	namespace Graphic
	{
		class JGuiBackendInterface;
	} 
	namespace Editor
	{
		struct JGuiImageInfo;
		class JEditorManager;
		class JDockUpdateHelper;
		class JGuiBehaviorAdapter; 
		class JGui
		{
#pragma region Color
		public:
			//to soft factor
			static JVector4<float> GetSelectedWidgetColorFactor()noexcept; 
			static JVector4<float> GetUnFocusedWidgetColorFactor()noexcept;
			static JVector4<float> GetDeActivatedTextColorFactor()noexcept;
			static JVector4<float> GetColor(const J_GUI_COLOR flag)noexcept;
			static uint GetUColor(const J_GUI_COLOR flag)noexcept;
			static void SetColor(const J_GUI_COLOR flag, const JVector4<float>& color)noexcept;
			static void SetColorToSoft(const J_GUI_COLOR flag, const JVector4<float>& factor)noexcept;
			//Set widget color to default
			static void SetColorToDefault(const J_GUI_COLOR flag)noexcept;
			static void SetAllColorToSoft(const JVector4<float>& factor)noexcept;
			//Set all widget color to default
			static void SetAllColorToDefault()noexcept; 
			static uint32 ConvertUColor(const JVector4<float>& color)noexcept;
			static void PushColor(const J_GUI_COLOR colType, const JVector4<float>& color);
			static void PushColorToSoft(const J_GUI_COLOR colType, const JVector4<float>& factor);
			//Set node, node hover, node activate color 
			static void PushTreeNodeColorToSoft(const JVector4<float>& factor)noexcept;
			//Set button, button hover, button activate color 
			static void PushButtonColorToSoftSet(const JVector4<float>& factor)noexcept;
			//Set text button, button hover, button activate color
			static void PushButtonColorDeActSet()noexcept; 
			static void PopColor(const uint count = 1);
			static void PopTreeNodeColorToSoftSet();
			static void PopButtonColorToSoftSet();
			static void PopButtonColorDeActSet();
#pragma endregion
#pragma region Style
		public:
			static JVector2<float> GetWindowPadding()noexcept;
			static float GetWindowBorderSize()noexcept;
			static float GetWindowRounding()noexcept;
			static JVector2<float> GetFramePadding()noexcept;
			static float GetFrameBorderSize()noexcept;
			static JVector2<float> GetItemSpacing()noexcept;
			static JVector2<float> GetItemInnerSpacing()noexcept;
			static float GetScrollBarSize()noexcept;
			static float GetStyleValueF(const J_GUI_STYLE style);
			static JVector2<float> GetStyleValueV2(const J_GUI_STYLE style);
			static void SetWindowPadding(const JVector2<float>& padding)noexcept;
			static void SetWindowBorderSize(const float size)noexcept;
			static void SetWindowRounding(const float factor)noexcept;
			static void SetFramePadding(const JVector2<float>& padding)noexcept;
			static void SetFrameBorderSize(const float size)noexcept;
			static void SetItemSpacing(const JVector2<float>& spacing)noexcept;
			static void SetItemInnerSpacing(const JVector2<float>& spacing)noexcept;
			static void SetScrollBarSize(const float size)noexcept;
			static void PushStyle(const J_GUI_STYLE style, const float value);
			static void PushStyle(const J_GUI_STYLE style, const JVector2<float>& value);
			static void PopStyle(const uint count = 1);
#pragma endregion
#pragma region IO
		public:
			//Text
			static JVector2<float> GetAlphabetSize()noexcept;
			static float GetFontSize()noexcept;
			static JVector2<float> CalTextSize(const std::string& str)noexcept;
			static uint GetTextBuffRange()noexcept;
		public:
			static float GetGlobalFontScale()noexcept;
			static float GetCurrentWindowFontScale()noexcept;
			static void SetGlobalFontScale(const float scale)noexcept;
			static void SetCurrentWindowFontScale(const float scale)noexcept;
			static void SetFont(const J_GUI_FONT_TYPE fontType);
			static void PushFont()noexcept;
			static void PopFont()noexcept;
		public:
			//임시함수 gui lib에 input을 사용중이지만 추후에 keyboard, mouse class 설계 구현후 폐기
			static uint GetMouseClickedCount(const Core::J_MOUSE_BUTTON btn)noexcept;
			static float GetMouseWheel()noexcept;
			static JVector2<float> GetMousePos()noexcept;
			static JVector2<float> GetMouseDragDelta()noexcept;
			static JVector2<float> GetCursorPos()noexcept;						//local
			static JVector2<float> GetCursorScreenPos() noexcept;				//world
			static float GetCursorPosX()noexcept;		//local
			static float GetCursorPosY()noexcept;		//local
			static void SetCursorPos(const JVector2<float>& pos)noexcept;		//local
			static void SetCursorPosX(const float x)noexcept;		//local
			static void SetCursorPosY(const float y)noexcept;		//local
			static void SetCursorScreenPos(const JVector2<uint>& pos)noexcept; //world
			static bool IsMouseClicked(const Core::J_MOUSE_BUTTON btn)noexcept;
			static bool IsLastMouseClicked(const Core::J_MOUSE_BUTTON btn)noexcept;
			static bool IsMouseReleased(const Core::J_MOUSE_BUTTON btn)noexcept;
			static bool IsMouseDown(const Core::J_MOUSE_BUTTON btn)noexcept;
			static bool IsMouseDragging(const Core::J_MOUSE_BUTTON btn)noexcept;
			static bool AnyMouseDown(const bool containMiddle = true, const bool containRight = true, const bool containLeft = true)noexcept;
			static bool AnyMouseClicked(const bool containMiddle = true, const bool containRight = true, const bool containLeft = true)noexcept;
			//use world pos
			static bool IsMouseInRect(const JVector2<float>& position, const JVector2<float>& size)noexcept; 
			static bool IsMouseHoveringRect(const JVector2<float>& min, const JVector2<float>& max, const bool clip = true)noexcept;
			static bool IsMouseInLine(JVector2<float> st, JVector2<float> ed, const float thickness)noexcept;
			static bool CanFocusByMouseRightClick()noexcept;
		public:
			static bool IsKeyPressed(const Core::J_KEYCODE key)noexcept;
			static bool IsKeyReleased(const Core::J_KEYCODE key)noexcept;
			static bool IsKeyDown(const Core::J_KEYCODE key)noexcept;
		public:
			static bool BeginDragDropSource(J_GUI_DRAG_DROP_FLAG_ flag = J_GUI_DRAG_DROP_FLAG_NONE);
			static bool SetDragDropPayload(const std::string& typeName, Core::JTypeInstanceSearchHint* draggingHint, J_GUI_CONDIITON cond = J_GUI_CONDIITON::J_GUI_CONDIITON_NONE);
			static void EndDragDropSource();
			static bool BeginDragDropTarget();
			static Core::JTypeInstanceSearchHint* TryGetTypeHintDragDropPayload(const std::string& typeName, J_GUI_DRAG_DROP_FLAG_ flag = J_GUI_DRAG_DROP_FLAG_NONE);
			static void EndDragDropTarget();
#pragma endregion
#pragma region Widget
		public:
			static bool BeginWindow(const std::string& name, bool* p_open = nullptr, J_GUI_WINDOW_FLAG_ flags = J_GUI_WINDOW_FLAG_NONE);
			static void EndWindow();
			static bool BeginChildWindow(const std::string& name, const JVector2<float>& windowSize = JVector2<float>{ 0,0 }, bool border = false, J_GUI_WINDOW_FLAG_ flags = J_GUI_WINDOW_FLAG_NONE);
			static void EndChildWindow();
			//popup trigger
			static void OpenPopup(const std::string& name, J_GUI_POPUP_FLAG_ flags = J_GUI_POPUP_FLAG_NONE);
			static void CloseCurrentPopup();
			static bool BeginPopup(const std::string& name, J_GUI_WINDOW_FLAG_ flags = J_GUI_WINDOW_FLAG_NONE);
			static void EndPopup();
			static void BeginGroup();
			static void EndGroup();
			static void Text(const std::string& text);
			static void Text(const std::string& text, const float fontScale);
			static bool CheckBox(const std::string& name, bool& v);
			static bool Button(const std::string& name, const JVector2<float>& jVec2 = { 0,0 });
			static bool IsTreeNodeOpend(const std::string& name, J_GUI_TREE_NODE_FLAG_ flags);
			static bool TreeNodeEx(const std::string& name, J_GUI_TREE_NODE_FLAG_ flags = J_GUI_TREE_NODE_FLAG_NONE);
			static void TreePop();
			static bool Selectable(const std::string& name, bool* pSelected = nullptr, J_GUI_SELECTABLE_FLAG_ flags = J_GUI_SELECTABLE_FLAG_NONE, const JVector2<float>& sizeArg = { 0,0 });
			static bool Selectable(const std::string& name, bool selected, J_GUI_SELECTABLE_FLAG_ flags = J_GUI_SELECTABLE_FLAG_NONE, const JVector2<float>& sizeArg = { 0,0 });
			static bool InputText(const std::string& name, std::string& buff, J_GUI_INPUT_TEXT_FLAG_ flags = J_GUI_INPUT_TEXT_FLAG_NONE);
			static bool InputText(const std::string& name, std::string& buff, const size_t size, J_GUI_INPUT_TEXT_FLAG_ flags = J_GUI_INPUT_TEXT_FLAG_NONE);
			static bool InputText(const std::string& name, std::string& buff, std::string& result, const std::string& hint, J_GUI_INPUT_TEXT_FLAG_ flags = J_GUI_INPUT_TEXT_FLAG_NONE);
			static bool InputMultilineText(const std::string& name, std::string& buff, std::string& result, const JVector2<float>& size, J_GUI_INPUT_TEXT_FLAG_ flags = J_GUI_INPUT_TEXT_FLAG_NONE);
			static bool InputInt(const std::string& name, int* value, J_GUI_INPUT_TEXT_FLAG_ flags = J_GUI_INPUT_TEXT_FLAG_NONE, int step = 1);
			static bool InputFloat(const std::string& name, float* value, J_GUI_INPUT_TEXT_FLAG_ flags = J_GUI_INPUT_TEXT_FLAG_NONE, const char* format = "%.2f", float step = 0.0f);
		public:
			static bool SliderInt(const std::string& name, int* value, int vMin, int vMax, const char* format = "%d", J_GUI_SLIDER_FLAG_ flags = J_GUI_SLIDER_FLAG_NONE);
			static bool SliderFloat(const std::string& name, float* value, float vMin, float vMax, const char* format = "%.2f", J_GUI_SLIDER_FLAG_ flags = J_GUI_SLIDER_FLAG_NONE);
			static bool VSliderInt(const std::string& name, JVector2<float> size, int* value, int vMin, int vMax, const char* format = "%d", J_GUI_SLIDER_FLAG_ flags = J_GUI_SLIDER_FLAG_NONE);
			static bool VSliderFloat(const std::string& name, JVector2<float> size, float* value, float vMin, float vMax, const char* format = "%.2f", J_GUI_SLIDER_FLAG_ flags = J_GUI_SLIDER_FLAG_NONE);
		public:
			static bool BeginTabBar(const std::string& name, J_GUI_TAB_BAR_FLAG_ flags = J_GUI_TAB_BAR_FLAG_NONE);
			static void EndTabBar();
			static bool BeginTabItem(const std::string& name, bool* p_open = NULL, J_GUI_TAB_ITEM_FLAG_ flags = J_GUI_TAB_ITEM_FLAG_NONE);
			static void EndTabItem();
			static bool TabItemButton(const std::string& name, J_GUI_TAB_ITEM_FLAG_ flags = J_GUI_TAB_ITEM_FLAG_NONE);
		public:
			static bool BeginTable(const std::string& name, const int columnCount, J_GUI_TABLE_FLAG_ flags = J_GUI_TABLE_FLAG_NONE, const JVector2<float> outerSize = { 0,0 }, const float innerWidth = 0);
			static void EndTable();
			static void TableSetupColumn(const std::string& name, J_GUI_TABLE_COLUMN_FLAG_ flags = J_GUI_TABLE_COLUMN_FLAG_NONE, float initWeight = 0);
			static void TableHeadersRow();
			static void TableNextRow();
			static void TableSetColumnIndex(const int index);
		public:
			static bool BeginMainMenuBar();
			static void EndMainMenuBar();
			static bool BeginMenuBar();
			static void EndMenuBar();
			static bool BeginMenu(const std::string& name, bool enable = true);
			static void EndMenu();
			static bool MenuItem(const std::string& name, bool selected, bool enabled);
			static bool MenuItem(const std::string& name, const std::string& shortcut, bool selected, bool enabled);
		public:
			static bool BeginCombo(const std::string& name, const std::string& preview, J_GUI_COMBO_FLAG_ flags = J_GUI_COMBO_FLAG_NONE);
			static void EndCombo();
		public:
			static bool BeginListBox(const std::string& name, const JVector2<float> size = { 0,0 });
			static void EndListBox();
		public:
			static bool ColorPicker(const std::string& name, JVector3<float>& color, J_GUI_COLOR_EDIT_FALG_ flags);
			static bool ColorPicker(const std::string& name, JVector4<float>& color, J_GUI_COLOR_EDIT_FALG_ flags);
			static void Tooltip(const std::string& message, const float fontScale = 0.8f)noexcept;
			template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
			static void Tooltip(T value, const float fontScale = 0.8f)noexcept
			{
				if constexpr(std::is_integral_v<T> || std::is_floating_point_v<T>)
					Tooltip(std::to_string(value), fontScale);
			}
		public:
			//Image  
			static void InvalidImage(const JVector2<float>& size,
				const JVector2<float>& uv0 = JVector2<float>(0, 0),
				const JVector2<float>& uv1 = JVector2<float>(1, 1),
				const JVector4<float>& tintCol = JVector4<float>(1, 1, 1, 1),
				const JVector4<float>& borderCol = JVector4<float>(0, 0, 0, 0));
			static void Image(JGuiImageInfo info,
				const JVector2<float>& size,
				const JVector2<float>& uv0 = JVector2<float>(0, 0),
				const JVector2<float>& uv1 = JVector2<float>(1, 1),
				const JVector4<float>& tintCol = JVector4<float>(1, 1, 1, 1),
				const JVector4<float>& borderCol = JVector4<float>(0, 0, 0, 0));
			//Use ImGui::ImageButtonEx
			//display one image
			static bool ImageButton(const std::string name,
				JGuiImageInfo info,
				const JVector2<float>& size,
				const JVector2<float>& uv0 = JVector2<float>(0, 0),
				const JVector2<float>& uv1 = JVector2<float>(1, 1),
				float framePadding = -1,
				const JVector4<float>& bgCol = JVector4<float>(0, 0, 0, 0),
				const JVector4<float>& tintCol = JVector4<float>(1, 1, 1, 1));
			static void AddInvalidImage(const JVector2<float>& pMin,
				const JVector2<float>& pMax,
				const bool isFront = false, 
				const JVector4<float>& color = JVector4<float>(1, 1, 1, 1),
				const JVector2<float>& uvMin = JVector2<float>(0, 0),
				const JVector2<float>& uvMax = JVector2<float>(1, 1));
			static void AddRoundedInvalidImage(const JVector2<float>& pMin,
				const JVector2<float>& pMax,
				const float rounding,
				J_GUI_DRAW_FLAG_ flag,
				const bool isFront = false,
				const JVector4<float>& color = JVector4<float>(1, 1, 1, 1),
				const JVector2<float>& uvMin = JVector2<float>(0, 0),
				const JVector2<float>& uvMax = JVector2<float>(1, 1));
			static void AddImage(JGuiImageInfo info,
				const JVector2<float>& pMin,
				const JVector2<float>& pMax,
				const bool isFront = false,
				const JVector4<float>& color = JVector4<float>(1, 1, 1, 1),
				const JVector2<float>& uvMin = JVector2<float>(0, 0),
				const JVector2<float>& uvMax = JVector2<float>(1, 1));
			static void AddRoundedImage(JGuiImageInfo info,
				const JVector2<float>& pMin,
				const JVector2<float>& pMax,
				const bool isFront = false,
				const float rounding = GetStyleValueF(J_GUI_STYLE::FRAME_ROUNDING),
				J_GUI_DRAW_FLAG_ flag = J_GUI_DRAW_FLAG::J_GUI_DRAW_FLAG_ROUND_ALL,
				const JVector4<float>& color = JVector4<float>(1, 1, 1, 1),
				const JVector2<float>& uvMin = JVector2<float>(0, 0),
				const JVector2<float>& uvMax = JVector2<float>(1, 1));
		public:
			//Custom Widget 
			//display one image
			static bool ImageSelectable(const std::string name,
				JGuiImageInfo info,
				bool& pressed,
				bool changeValueIfPreesd,
				const JVector2<float>& size,
				const bool useRestoreCursorPos);
			static bool Switch(const std::string& name,
				bool& pSelected,
				bool changeValueIfPreesd,
				const JVector2<float>& size = { 0,0 });
			//display one image
			static bool ImageSwitch(const std::string name,
				JGuiImageInfo info,
				bool& pressed,
				bool changeValueIfPreesd,
				const JVector2<float>& size,
				const JVector4<float>& bgColor,
				const JVector4<float>& bgDelta,
				const JVector4<float>& frameColor = GetColor(J_GUI_COLOR::FRAME_BG),
				const float frameThickness = 0.0f); 
			//display one image
			static bool ImageButton(const std::string name,
				JGuiImageInfo info,
				const JVector2<float>& size,
				const JVector4<float>& bgColor,
				const JVector4<float>& bgDelta,
				const JVector4<float>& frameColor = GetColor(J_GUI_COLOR::FRAME_BG),
				const float frameThickness = 0.0f);
			static bool MaximizeButton(const bool isLocatedCloseBtnLeftSide = true);
			static bool MinimizeButton(const bool isLocatedCloseBtnLeftSide, const bool isLocatedMaximizeBtnLeftSize);
			static bool PreviousSizeButton(const bool isLocatedCloseBtnLeftSide, const bool isLocatedMaximizeBtnLeftSize);
		public:
			//use wolrd cursor pos
			//Use engine defined multi color pattern
			static void DrawRectFilledMultiColor(const JVector2<float>& pos,
				const JVector2<float>& size,
				const JVector4<float>& color,
				const JVector4<float>& colorDelta = JVector4<float>(0.15f, 0.15f, 0.15f, 0.1f),
				const bool useRestoreCursorPos = true, 
				const bool useFrameRounding = true)noexcept;
			//addDeltaLeftUpRightDown <-> addDeltaRightUpLeftDown
			static void DrawRectFilledMultiColor(const JVector2<float>& pos,
				const JVector2<float>& size,
				const JVector4<float>& color,
				const JVector4<float>& colorDelta,
				const bool useRestoreCursorPos,
				const J_GUI_ORDINAL_DIR_FLAG_ addedDeltaDir,
				const bool useFrameRounding = true)noexcept;
			static void DrawRectFilledMultiColor(const JVector2<float>& pos,
				const JVector2<float>& size,
				const JVector4<float>& upLeftCol,
				const JVector4<float>& upRightCol,
				const JVector4<float>& downLeftCol,
				const JVector4<float>& downRightCol,
				const bool useRestoreCursorPos,
				const bool useFrameRounding = true)noexcept;
			static void DrawRectFilledMultiColor(const JVector2<float>& pos,
				const JVector2<float>& size,
				const JVector4<float>& upLeftCol,
				const JVector4<float>& upRightCol,
				const JVector4<float>& downLeftCol,
				const JVector4<float>& downRightCol,
				const float rounding,
				const J_GUI_DRAW_FLAG_ drawFlag,
				const bool useRestoreCursorPos)noexcept;
			static void DrawRectFilledColor(const JVector2<float>& pos,
				const JVector2<float>& size,
				const JVector4<float>& color,
				const bool useRestoreCursorPos,
				const bool useFrameRounding = true)noexcept;
			static void DrawRectFilledColor(const JVector2<float>& pos,
				const JVector2<float>& size,
				const JVector4<float>& color,
				const float rounding,
				const J_GUI_DRAW_FLAG_ drawFlag,
				const bool useRestoreCursorPos)noexcept;
			static void DrawRectFrame(const JVector2<float>& pos,
				const JVector2<float>& size,
				const JVector4<float>& color,
				const float thickness,
				const bool useRestoreCursorPos,
				const bool useFrameRounding = true)noexcept;
			static void DrawRectFrame(const JVector2<float>& pos,
				const JVector2<float>& size,
				const JVector4<float>& color,
				const float thickness,
				const float rounding,
				const J_GUI_DRAW_FLAG_ drawFlag,
				const bool useRestoreCursorPos)noexcept;
			static void DrawToolTipBox(const std::string& uniqueLabel,
				const std::string& tooltip,
				const JVector2<float>& pos,
				const JVector2<float>& padding,
				const bool useRestoreCursorPos);
			static void DrawToolTipBox(const std::string& uniqueLabel,
				const std::string& tooltip,
				const JVector2<float>& pos,
				const float maxWidth,
				const JVector2<float>& padding,
				const J_EDITOR_ALIGN_TYPE alignType,
				const bool useRestoreCursorPos);
			static void DrawCircleFilledColor(const JVector2<float>& centerPos,
				const float radius,
				const JVector4<float>& color,
				const bool useRestoreCursorPos);
			static void DrawCircle(const JVector2<float>& centerPos,
				const float radius,
				const JVector4<float>& color,
				const bool useRestoreCursorPos,
				const float thickness = 1.0f);
		public:
			static void PushClipRect(const JVector2<float>& rectMinP, const JVector2<float>& rectMaxP, const bool intersectWithCurrentClipRect);
			static void AddLine(const JVector2<float>& p1, const JVector2<float>& p2, const JVector4<float>& color, const float thickness = 1.0f);
			static void AddTriangle(const JVector2<float>& p1, const JVector2<float>& p2, const JVector2<float>& p3, const JVector4<float>& color, const float thickness = 1.0f);
			static void AddTriangleFilled(const JVector2<float>& p1, const JVector2<float>& p2, const JVector2<float>& p3, const JVector4<float>& color);
		public:
			static void NewLine()noexcept;
			static void SameLine()noexcept;
			static void Separator()noexcept;
			static void Indent()noexcept;
			static void UnIndent()noexcept;
		public:
			//Window 
			// Work Area
			static JVector2<float> GetMainWorkPos()noexcept;
			// Work Area
			static JVector2<float> GetMainWorkSize()noexcept;
			static JVector2<float> GetDisplaySize()noexcept;
			static JVector2<float> GetAppWindowSize()noexcept;
			static JVector2<float> GetClientWindowPos()noexcept;
			static JVector2<float> GetClientWindowSize()noexcept;
			static JVector2<float> GetDefaultClientWindowMinSize()noexcept;
			static JVector2<float> GetWindowPos()noexcept;
			static JVector2<float> GetWindowSize()noexcept;
			static JVector2<float> GetRestWindowSpace()noexcept;
			//applied padding border
			static JVector2<float> GetWindowContentsSize()noexcept;
			//applied padding border cursor pos
			static JVector2<float> GetRestWindowContentsSize()noexcept;
			//current window menubar size
			static JVector2<float> GetWindowMenuBarSize()noexcept;
			//current window titlebar size
			static JVector2<float> GetWindowTitleBarSize()noexcept;
			//get displayed window order back to front
			//return -1 if didn't display
			static int GetWindowOrder(const GuiID windowID)noexcept;
			static bool GetWindowInfo(const std::string& wndName, _Out_ JGuiWindowInfo& info)noexcept;
			static bool GetWindowInfo(const GuiID windowID, _Out_ JGuiWindowInfo& info)noexcept; 
			static bool GetCurrentWindowInfo(_Out_ JGuiWindowInfo& info)noexcept; 
			static std::vector<JGuiWindowInfo> GetDisplayedWindowInfo()noexcept;
			static void SetNextWindowPos(const JVector2<float>& pos, J_GUI_CONDIITON flag = J_GUI_CONDIITON_NONE)noexcept;
			static void SetNextWindowSize(const JVector2<float>& size, J_GUI_CONDIITON flag = J_GUI_CONDIITON_NONE)noexcept;
			static void SetNextWindowFocus()noexcept;
			static bool IsCurrentWindowFocused(J_GUI_FOCUS_FLAG_ flag)noexcept;  
			static void FocusWindow(const GuiID windowID);
			static void FocusCurrentWindow();
			static void RestoreFromMaximize(const GuiID windowID, const std::vector<GuiID>& preTabItemID);
		public:
			//item : button, slider, input...
			static JVector2<float> GetLastItemRectMin()noexcept;
			static JVector2<float> GetLastItemRectMax()noexcept;
			static JVector2<float> GetLastItemRectSize()noexcept;
			static void SetNextItemWidth(const float width)noexcept;
			static void SetNextItemOpen(const bool value, J_GUI_CONDIITON flag = J_GUI_CONDIITON_NONE)noexcept;
			static void SetLastItemDefaultFocus();
			static bool IsLastItemActivated()noexcept;
			static bool IsLastItemHovered(J_GUI_HOVERED_FLAG_ flag = J_GUI_HOVERED_FLAG_NONE)noexcept;
			static bool IsLastItemClicked(const Core::J_MOUSE_BUTTON btn)noexcept;
			static bool IsLastItemAnyClicked(const bool containMiddle = true, const bool containRight = true, const bool containLeft = true)noexcept;
			static bool IsTreeNodeArrowHovered(const std::string& nodeName);
			static bool CanUseWidget(const Core::J_GUI_WIDGET_TYPE type);
			static JVector2<float> CalDefaultButtonSize(const std::string& label)noexcept;
			static JVector2<float> CalButtionSize(const std::string& label, const JVector2<float>& size)noexcept; 
			static float CalCloseButtionSize()noexcept;
			static void PushItemFlag(J_GUI_ITEM_FLAG_ flag, const bool value)noexcept;
			static void PopItemFlag();
			static void PushItemWidth(const float width)noexcept;
			static void PopItemWidth()noexcept;
		public:
			//util 
			static float GetSliderRightAlignPosX(bool hasScrollbar = false)noexcept;
			static float GetSliderWidth()noexcept;
		public:
			static bool IsFullScreen()noexcept;
			static bool IsEnablePopup()noexcept;
			static bool IsEnableSelector()noexcept;
		public:
			//Widget Set  
			static void ComboSet(const std::string& uniqueLabel, int& selectedIndex, const std::vector<std::string>& strVec);
			template<typename Object, std::enable_if_t<std::is_base_of_v<Core::JIdentifier, Object>, int> = 0>
			static void ComboSet(const std::string& uniqueLabel, int& selectedIndex, const std::vector<Object*>& objVec)
			{
				if (BeginCombo(uniqueLabel, JCUtil::WstrToU8Str(objVec[selectedIndex]->GetName()), J_GUI_COMBO_FLAG_HEIGHT_LARGE))
				{
					const uint count = (uint)objVec.size();
					for (uint i = 0; i < count; i++)
					{
						bool isSelected = (selectedIndex == i);
						if (Selectable(JCUtil::WstrToU8Str(objVec[i]->GetName()), &isSelected))
							selectedIndex = i;
						if (isSelected)
							SetLastItemDefaultFocus();
					}
					EndCombo();
				}
			}
			template<typename EnumType>
			static void ComboEnumSet(const std::string& uniqueLabel, int& selectedIndex)
			{
				Core::JEnumInfo* enumInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(EnumType).name());
				if (BeginCombo(uniqueLabel, enumInfo->ElementName(enumInfo->EnumValue(selectedIndex)), J_GUI_COMBO_FLAG_HEIGHT_LARGE))
				{
					const uint enumCount = enumInfo->GetEnumCount();
					for (uint i = 0; i < enumCount; i++)
					{
						bool isSelected = (selectedIndex == i);
						if (Selectable(enumInfo->ElementName(enumInfo->EnumValue(i)), &isSelected))
							selectedIndex = i;
						if (isSelected)
							SetLastItemDefaultFocus();
					}
					EndCombo();
				}
			}
			//Widget Set
			//Support Redo undo transition
			template<typename ...Param>
			static bool CheckBoxSetT(const std::string& objName, const std::string& uniqueLabel, const bool preValue, Core::JFunctor<void, const bool, Param...>& commitFunctor, Param... var)
			{
				bool nowValue = preValue;
				if (CheckBox("##CheckBox" + uniqueLabel, nowValue))
				{
					using Functor = Core::JFunctor<void, const bool, Param...>;
					using Binder = Core::JBindHandle<Functor, const bool, Param...>;

					bool dovalue = nowValue;
					bool undovalue = preValue;

					ExecuteWidgetSet<Binder>("Checkbox set value",
						"object name: " + objName + " value:" + std::to_string(preValue) + " To" + std::to_string(nowValue),
						commitFunctor,
						dovalue,
						undovalue,
						std::make_tuple(var...),
						std::make_tuple(var...),
						std::make_index_sequence<sizeof...(Param)>());
					return true;
				}
				return false;
			}
			template<typename ...Param>
			static bool InputIntSetT(const std::string& objName, const std::string& uniqueLabel, const int preValue, Core::JFunctor<void, const int, Param...>& commitFunctor, Param... var)
			{
				int nowValue = preValue;
				if (InputInt("##InputInt" + uniqueLabel, &nowValue))
				{
					if (nowValue != preValue)
					{
						using Functor = Core::JFunctor<void, const int, Param...>;
						using Binder = Core::JBindHandle<Functor, const int, Param...>;

						int dovalue = nowValue;
						int undovalue = preValue;

						ExecuteWidgetSet<Binder>("Input int set value",
							"object name: " + objName + " value:" + std::to_string(preValue) + " To" + std::to_string(nowValue),
							commitFunctor,
							dovalue,
							undovalue,
							std::make_tuple(var...),
							std::make_tuple(var...),
							std::make_index_sequence<sizeof...(Param)>());
						return true;
					}
				}
				return false;
			}
			template<typename ...Param>
			static bool InputFloatSetT(const std::string& objName, const std::string& uniqueLabel, const float preValue, Core::JFunctor<void, const float, Param...>& commitFunctor, Param... var)
			{
				float nowValue = preValue;
				if (InputFloat("##IntputFloat" + uniqueLabel, &nowValue))
				{
					if (nowValue != preValue)
					{
						using Functor = Core::JFunctor<void, const float, Param...>;
						using Binder = Core::JBindHandle<Functor, const float, Param...>;

						float dovalue = nowValue;
						float undovalue = preValue;

						ExecuteWidgetSet<Binder>("Input float set value:",
							"object name: " + objName + " value:" + std::to_string(preValue) + " To" + std::to_string(nowValue),
							commitFunctor,
							dovalue,
							undovalue,
							std::make_tuple(var...),
							std::make_tuple(var...),
							std::make_index_sequence<sizeof...(Param)>());
						return true;
					}
				}
				return false;
			}
			template<typename ...Param>
			static bool InputTextSetT(const std::string& objName,
				const std::string& uniqueLabel,
				JEditorInputBuffHelper* helper,
				const std::string& hint,
				J_GUI_INPUT_TEXT_FLAG_ flags,
				Core::JFunctor<void, const std::string, Param...>& commitFunctor, Param... var)
			{
				std::string preValue = helper->buff;
				if (InputText("##InputText" + uniqueLabel, helper->buff, helper->result, hint, flags))
				{
					std::string dovalue = helper->result;
					std::string undovalue = JCUtil::EraseSideChar(preValue, '\0');

					using Functor = Core::JFunctor<void, const std::string, Param...>;
					using Binder = Core::JBindHandle<Functor, const std::string, Param...>;

					ExecuteWidgetSet<Binder>("Input text set value",
						"object name: " + objName + " value:" + preValue + " To" + dovalue,
						commitFunctor,
						dovalue,
						undovalue,
						std::make_tuple(var...),
						std::make_tuple(var...),
						std::make_index_sequence<sizeof...(Param)>());
					//Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>(uniqueLabel + " input text set value: " + dovalue,
					//	std::make_unique<Binder>(commitFunctor, std::move(dovalue), DecomposeTuple<>(doParamT)...),
					//	std::make_unique<Binder>(commitFunctor, std::move(undovalue), DecomposeTuple<>(undoParamT)...)));
					return true;
				}
				return false;
			}
			template<typename ...Param>
			static bool InputMultilineTextSetT(const std::string& objName,
				const std::string& uniqueLabel,
				JEditorInputBuffHelper* helper,
				const JVector2<float>& size,
				J_GUI_INPUT_TEXT_FLAG_ flags,
				Core::JFunctor<void, const std::string, Param...>& commitFunctor, Param... var)
			{
				std::string preValue = objName;
				if (InputMultilineText("##InputMultilineText" + uniqueLabel, helper->buff, helper->result, size, flags))
				{
					std::string dovalue = helper->result;
					std::string undovalue = JCUtil::EraseSideChar(preValue, '\0');

					using Functor = Core::JFunctor<void, const std::string, Param...>;
					using Binder = Core::JBindHandle<Functor, const std::string, Param...>;

					ExecuteWidgetSet<Binder>("Input multiline text set value: ",
						"object name: " + objName + " value:" + preValue + " To" + dovalue,
						commitFunctor,
						dovalue,
						undovalue,
						std::make_tuple(var...),
						std::make_tuple(var...),
						std::make_index_sequence<sizeof...(Param)>());
					//Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>(uniqueLabel + " input text set value: " + dovalue,
					//	std::make_unique<Binder>(commitFunctor, std::move(dovalue), DecomposeTuple<>(doParamT)...),
					//	std::make_unique<Binder>(commitFunctor, std::move(undovalue), DecomposeTuple<>(undoParamT)...)));
					return true;
				}
				return false;
			}
			template<typename EnumType, typename ...Param>
			static void ComoboEnumSetT(const std::string& objName,
				const std::string& uniqueLabel,
				const EnumType preValue,
				Core::JFunctor<void, const EnumType, Param...>& commitFunctor,
				Param... var)
			{
				const std::string enumName = Core::GetName<EnumType>();
				const std::string preValueName = Core::GetName(preValue);
				if (BeginCombo("##EnumCombo" + enumName + uniqueLabel, preValueName))
				{
					for (uint i = 0; i < (int)EnumType::COUNT; ++i)
					{
						std::string valueStr = Core::GetName((EnumType)i) + "##" + enumName + uniqueLabel;
						if (Selectable(valueStr))
						{
							using Functor = Core::JFunctor<void, const EnumType, Param...>;
							using Binder = Core::JBindHandle<Functor, const EnumType, Param...>;

							EnumType dovalue = (EnumType)i;
							EnumType undovalue = preValue;

							ExecuteWidgetSet<Binder>("Enum combo box setvalue",
								"object name: " + objName + " value:" + preValueName + " to" + Core::GetName(dovalue),
								commitFunctor,
								dovalue,
								undovalue,
								std::make_tuple(var...),
								std::make_tuple(var...),
								std::make_index_sequence<sizeof...(Param)>());
						}
					}
					EndCombo();
				}
			}
		private:
			template<size_t ParamIndex, typename Tuple>
			static constexpr decltype(auto) DecomposeTuple(Tuple& tuple)
			{
				using ParamType = std::tuple_element_t<ParamIndex, Tuple>;
				return std::forward<ParamType>(std::get<ParamIndex>(tuple));
			}
			template<typename Binder, typename Functor, typename Value, typename ParamTuple, size_t ...Is>
			static void ExecuteWidgetSet(const std::string& taskName,
				const std::string& taskDesc,
				Functor& functor,
				Value doValue,
				Value undoValue,
				ParamTuple t1,
				ParamTuple t2,
				std::index_sequence<Is...>)
			{
				JEditorTransition::Instance().Execute(std::make_unique<Core::JTransitionSetValueTask>(taskName,
					taskDesc,
					std::make_unique<Binder>(functor, std::move(doValue), DecomposeTuple<Is>(t1)...),
					std::make_unique<Binder>(functor, std::move(undoValue), DecomposeTuple<Is>(t2)...)));
			}
#pragma endregion
#pragma region Docking
		public: 
			static bool GetDockNodeInfoByWindowName(const std::string& windowName, _Out_ JGuiDockNodeInfo& info)noexcept;
			static bool GetDockNodeInfo(const std::string& dockNodeName, _Out_ JGuiDockNodeInfo& info)noexcept;
			static bool GetDockNodeInfo(const GuiID dockID, _Out_ JGuiDockNodeInfo& info)noexcept; 
			static bool GetDockNodeHostWindowInfo(const GuiID childDockID, _Out_ JGuiWindowInfo& info)noexcept;
			static bool GetCurrentDockNodeInfo(_Out_ JGuiDockNodeInfo& info)noexcept;
			static bool HasDockNode(const std::string& dockNodeName)noexcept;
			static bool CanUseDockHirechary()noexcept;
			static void BuildDockHirechary(const std::vector<std::unique_ptr<JGuiDockBuildNode>>& nodeVec);
		public:
			static void UpdateDockSpace(const std::string& name, const JVector2<float>& size, J_GUI_DOCK_NODE_FLAG_ dockspaceFlag);
			static void OverrideNextDockNodeFlag(J_GUI_DOCK_NODE_FLAG_ flag);
			static void CloseTabItem(const GuiID windowID);
		public:
			static std::unique_ptr<JDockUpdateHelper> CreateDockUpdateHelper(const J_EDITOR_PAGE_TYPE pageType);
#pragma endregion
		};
  
		class JGuiPrivate
		{
		private:
			friend class JEditorManager; 
		private:
			static bool StartGuiUpdate();
			static void EndGuiUpdate(); 
		private:
			static void Initialize(std::unique_ptr<Graphic::JGuiInitData>&& initData);
			static void Clear();
		private:
			static Graphic::JGuiBackendInterface* GetBackendInterface();
		private:
			static void SetAdapter(std::unique_ptr<JGuiBehaviorAdapter>&& adapter);
		private:
			static void LoadOption();
			static void StoreOption();
		};
	}
}