#pragma once
#include"../JGuiType.h" 
#include"../JGuiUser.h"
#include"../JGuiImageInfo.h" 
#include"../JGuiWindow.h"
#include"../Data/JGuiPrivateData.h" 
#include"../../Align/JEditorAlignType.h"
#include"../../Page/JEditorPageEnum.h"
#include"../../../Core/Math/JVector.h"
#include"../../../Core/Reflection/JGuiWidgetType.h"
#include"../../../Core/Input/JMouse.h"
#include"../../../Core/Input/JKeyboard.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceType.h"
#include"../../../Graphic/Device/JGraphicDeviceUser.h"
#include"../../../Graphic/Gui/JGuiData.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceUserAccess;
	}
	namespace Editor
	{
		class JDockUpdateHelper;
		class JGuiBehaviorAdaptee : public JGuiUser, public Graphic::JGraphicDeviceUser
		{
		public:
			virtual void Initialize(std::unique_ptr<Graphic::JGuiInitData>&& initData) = 0;
			virtual void Clear() = 0;
		public:
			virtual void UpdateGuiBackend() = 0;
		public:
			virtual void SettingGuiDrawing() = 0;
			virtual void Draw(std::unique_ptr<Graphic::JGuiDrawData>&& drawData) = 0;
#pragma region Color
		public: 
			uint32 ConvertUColor(const JVector4<float>& color)const noexcept;
			virtual JVector4<float> GetColor(const J_GUI_COLOR flag)noexcept = 0;
			virtual uint GetUColor(const J_GUI_COLOR flag)const noexcept = 0; 
			virtual void SetColor(const J_GUI_COLOR flag, const JVector4<float>& color)noexcept = 0;
			//Set widget color to default
			virtual void SetColorToDefault(const J_GUI_COLOR flag)noexcept = 0;
			virtual void SetAllColorToSoft(const JVector4<float>& factor)noexcept = 0;
			//Set all widget color to default
			virtual void SetAllColorToDefault()noexcept = 0;  
			virtual void PushColor(const J_GUI_COLOR colType, const JVector4<float>& color) = 0;
			virtual void PopColor(const uint count) = 0;
#pragma endregion
#pragma region Style
		public:
			virtual JVector2<float> GetWindowPadding()const noexcept = 0;
			virtual float GetWindowBorderSize()const noexcept = 0;
			virtual float GetWindowRounding()const noexcept = 0;
			virtual JVector2<float> GetFramePadding()const noexcept = 0;
			virtual float GetFrameBorderSize()const noexcept = 0;
			virtual JVector2<float> GetItemSpacing()const noexcept = 0;
			virtual JVector2<float> GetItemInnerSpacing()const noexcept = 0;
			virtual float GetScrollBarSize()const noexcept = 0;
			virtual float GetStyleValueF(const J_GUI_STYLE style)const noexcept = 0;
			virtual JVector2<float> GetStyleValueV2(const J_GUI_STYLE style)const noexcept = 0;
			virtual void SetWindowPadding(const JVector2<float>& padding)noexcept = 0;
			virtual void SetWindowBorderSize(const float size)noexcept = 0;
			virtual void SetWindowRounding(const float factor)noexcept = 0;
			virtual void SetFramePadding(const JVector2<float>& padding)noexcept = 0;
			virtual void SetFrameBorderSize(const float size)noexcept = 0;
			virtual void SetItemSpacing(const JVector2<float>& spacing)noexcept = 0;
			virtual void SetItemInnerSpacing(const JVector2<float>& spacing)noexcept = 0;
			virtual void SetScrollBarSize(const float size)noexcept = 0;
			virtual void PushStyle(const J_GUI_STYLE style, const float value) = 0;
			virtual void PushStyle(const J_GUI_STYLE style, const JVector2<float>& value) = 0;
			virtual void PopStyle(const uint count) = 0;
#pragma endregion
#pragma region IO
		public:
			//return '0' size
			virtual JVector2<float> GetAlphabetSize()const noexcept = 0;
			virtual float GetFontSize()const noexcept = 0;
			virtual JVector2<float> CalTextSize(const std::string& str)const noexcept = 0;
		public:
			virtual float GetGlobalFontScale()const noexcept = 0;
			virtual float GetCurrentWindowFontScale()const noexcept = 0;
			virtual void SetGlobalFontScale(const float scale)noexcept = 0;
			virtual void SetCurrentWindowFontScale(const float scale)noexcept = 0;
			virtual void SetFont(const J_GUI_FONT_TYPE fontType) = 0;
			virtual void PushFont()const noexcept = 0;
			virtual void PopFont()const noexcept = 0;
		public:
			//임시함수 keyboard, mouse class 생성후 폐기
			virtual uint GetMouseClickedCount(const Core::J_MOUSE_BUTTON btn)const noexcept = 0;
			virtual float GetMouseWheel()const noexcept = 0;
			virtual JVector2<float> GetMousePos() const noexcept = 0;
			virtual JVector2<float> GetMouseDragDelta() const noexcept = 0;
			virtual JVector2<float> GetCursorPos()const noexcept = 0;				//local
			virtual JVector2<float> GetCursorScreenPos()const noexcept = 0;			//world
			virtual void SetCursorPos(const JVector2<float>& pos)noexcept = 0;		//local
			virtual void SetCursorScreenPos(const JVector2<float>& pos)noexcept = 0; //world
			virtual bool IsMouseClicked(const Core::J_MOUSE_BUTTON btn)const noexcept = 0;
			virtual bool IsMouseReleased(const Core::J_MOUSE_BUTTON btn)const noexcept = 0;
			virtual bool IsMouseDown(const Core::J_MOUSE_BUTTON btn)const noexcept = 0;
			virtual bool IsMouseDragging(const Core::J_MOUSE_BUTTON btn)const noexcept = 0;
			virtual bool IsMouseInRect(const JVector2<float>& min, const JVector2<float>& max)const noexcept = 0;
			virtual bool IsMouseHoveringRect(const JVector2<float>& min, const JVector2<float>& max, const bool clip)const noexcept = 0;
			virtual bool IsMouseInLine(JVector2<float> st, JVector2<float> ed, const float thickness)const noexcept = 0;
			bool CanFocusByMouseRightClick()noexcept;
		public:
			virtual bool IsKeyPressed(const Core::J_KEYCODE key)const noexcept = 0;
			virtual bool IsKeyReleased(const Core::J_KEYCODE key)const noexcept = 0;
			virtual bool IsKeyDown(const Core::J_KEYCODE key)const noexcept = 0;
		public:
			virtual bool BeginDragDropSource(J_GUI_DRAG_DROP_FLAG_ flag) = 0;
			virtual bool SetDragDropPayload(const std::string& typeName, Core::JTypeInstanceSearchHint* draggingHint, J_GUI_CONDIITON cond) = 0;
			virtual void EndDragDropSource() = 0;
			virtual bool BeginDragDropTarget() = 0;
			virtual Core::JTypeInstanceSearchHint* TryGetTypeHintDragDropPayload(const std::string& typeName, J_GUI_DRAG_DROP_FLAG_ flag) = 0;
			virtual void EndDragDropTarget() = 0;
#pragma endregion
#pragma region Widget
		public:
			virtual bool BeginWindow(const std::string& name, bool* p_open, J_GUI_WINDOW_FLAG flags) = 0;
			virtual void EndWindow() = 0;
			virtual bool BeginChildWindow(const std::string& name, const JVector2<float>& windowSize, const bool border, J_GUI_WINDOW_FLAG flags) = 0;
			virtual void EndChildWindow() = 0;
			virtual void OpenPopup(const std::string& name, J_GUI_POPUP_FLAG flags) = 0;
			virtual void CloseCurrentPopup() = 0;
			virtual bool BeginPopup(const std::string& name, J_GUI_WINDOW_FLAG flags) = 0;
			virtual void EndPopup() = 0;
			virtual void BeginGroup() = 0;
			virtual void EndGroup() = 0;
			virtual void Text(const std::string& text)const noexcept = 0;
			virtual bool CheckBox(const std::string& checkName, bool& v) = 0;
			virtual bool Button(const std::string& btnName, const JVector2<float>& jVec2) = 0;
			virtual bool IsTreeNodeOpend(const std::string& name, J_GUI_TREE_NODE_FLAG_ flags) = 0;
			virtual bool TreeNodeEx(const std::string& nodeName, J_GUI_TREE_NODE_FLAG flags) = 0;
			virtual void TreePop() = 0;
			virtual bool Selectable(const std::string& name, bool* pSelected, J_GUI_SELECTABLE_FLAG flags, const JVector2<float>& sizeArg) = 0;
			virtual bool Selectable(const std::string& name, bool selected, J_GUI_SELECTABLE_FLAG flags, const JVector2<float>& sizeArg) = 0;
			virtual bool InputText(const std::string& name, std::string& buff, J_GUI_INPUT_TEXT_FLAG flags) = 0;
			virtual bool InputText(const std::string& name, std::string& buff, const size_t size, J_GUI_INPUT_TEXT_FLAG flags) = 0;
			virtual bool InputText(const std::string& name, std::string& buff, std::string& result, const std::string& hint, J_GUI_INPUT_TEXT_FLAG flags) = 0;
			virtual bool InputMultilineText(const std::string& name, std::string& buff, std::string& result, const JVector2<float>& size, J_GUI_INPUT_TEXT_FLAG flags) = 0;
			virtual bool InputInt(const std::string& name, int* value, J_GUI_INPUT_TEXT_FLAG flags, int step) = 0;
			virtual bool InputFloat(const std::string& name, float* value, J_GUI_INPUT_TEXT_FLAG flags, const char* format, float step) = 0;
		public:
			virtual bool SliderInt(const std::string& name, int* value, int vMin, int vMax, const char* format, J_GUI_SLIDER_FLAG flags) = 0;
			virtual bool SliderFloat(const std::string& name, float* value, float vMin, float vMax, const char* format, J_GUI_SLIDER_FLAG flags) = 0;
			virtual bool VSliderInt(const std::string& name, JVector2<float> size, int* value, int vMin, int vMax, const char* format, J_GUI_SLIDER_FLAG flags) = 0;
			virtual bool VSliderFloat(const std::string& name, JVector2<float> size, float* value, float vMin, float vMax, const char* format, J_GUI_SLIDER_FLAG flags) = 0;
		public:
			virtual bool BeginTabBar(const std::string& name, J_GUI_TAB_BAR_FLAG flags) = 0;
			virtual void EndTabBar() = 0;
			virtual bool BeginTabItem(const std::string& name, bool* pOpen, J_GUI_TAB_ITEM_FLAG flags) = 0;
			virtual void EndTabItem() = 0;
			virtual bool TabItemButton(const std::string& name, J_GUI_TAB_ITEM_FLAG flags) = 0;
		public:
			virtual bool BeginTable(const std::string& name, const int columnCount, J_GUI_TABLE_FLAG flags, const JVector2<float> outerSize, const float innerWidth) = 0;
			virtual void EndTable() = 0;
			virtual void TableSetupColumn(const std::string& name, J_GUI_TABLE_COLUMN_FLAG flags, float initWeight) = 0;
			virtual void TableHeadersRow() = 0;
			virtual void TableNextRow() = 0;
			virtual void TableSetColumnIndex(const int index) = 0;
		public:
			virtual bool BeginMainMenuBar() = 0;
			virtual void EndMainMenuBar() = 0;
			virtual bool BeginMenuBar() = 0;
			virtual void EndMenuBar() = 0;
			virtual bool BeginMenu(const std::string& name, bool enable = true) = 0;
			virtual void EndMenu() = 0;
			virtual bool MenuItem(const std::string& name, bool selected, bool enabled) = 0;
			virtual bool MenuItem(const std::string& name, const std::string& shortcut, bool selected, bool enabled) = 0;
		public:
			virtual bool BeginCombo(const std::string& name, const std::string& preview, J_GUI_COMBO_FLAG flags) = 0;
			virtual void EndCombo() = 0;
		public:
			virtual bool BeginListBox(const std::string& name, const JVector2<float> size) = 0;
			virtual void EndListBox() = 0;
		public:
			virtual bool ColorPicker(const std::string& name, JVector3<float>& color, J_GUI_COLOR_EDIT_FALG_ flags) = 0;
			virtual bool ColorPicker(const std::string& name, JVector4<float>& color, J_GUI_COLOR_EDIT_FALG_ flags) = 0;
			virtual void Tooltip(const std::string& message)noexcept = 0;
		public:
			//Image  
			virtual void InvalidImage(const JVector2<float>& size,
				const JVector2<float>& uv0,
				const JVector2<float>& uv1,
				const JVector4<float>& tintCol,
				const JVector4<float>& borderCol) = 0;
			virtual void Image(const JGuiImageInfo& info,
				const JVector2<float>& size,
				const JVector2<float>& uv0,
				const JVector2<float>& uv1,
				const JVector4<float>& tintCol,
				const JVector4<float>& borderCol) = 0;
			//Use ImGui::ImageButtonEx
			//display one image
			virtual bool ImageButton(const std::string name,
				const JGuiImageInfo& info,
				const JVector2<float>& size,
				const JVector2<float>& uv0,
				const JVector2<float>& uv1,
				float framePadding,
				const JVector4<float>& bgCol,
				const JVector4<float>& tintCol) = 0;
			virtual void AddImage(const JGuiImageInfo& info,
				const JVector2<float>& pMin,
				const JVector2<float>& pMax,
				const bool isFront,
				const JVector4<float>& color,
				const JVector2<float>& uvMin,
				const JVector2<float>& uvMax) = 0;
			virtual void AddRoundedImage(const JGuiImageInfo& info,
				const JVector2<float>& pMin,
				const JVector2<float>& pMax,
				const bool isFront,
				const float rounding,
				J_GUI_DRAW_FLAG_ flag,
				const JVector4<float>& color,
				const JVector2<float>& uvMin,
				const JVector2<float>& uvMax) = 0;
		public:
			//Custom Widget 
			//display one image
			virtual bool ImageSelectable(const std::string name,
				JGuiImageInfo info,
				bool& pressed,
				bool changeValueIfPreesd,
				const JVector2<float>& size,
				const bool useRestoreCursorPos) = 0;
			virtual bool Switch(const std::string& name,
				bool& pSelected,
				bool changeValueIfPreesd,
				const JVector2<float>& size) = 0;
			//display one image
			virtual bool ImageSwitch(const std::string name,
				JGuiImageInfo info,
				bool& pressed,
				bool changeValueIfPreesd,
				const JVector2<float>& size,
				const JVector4<float>& bgColor, 
				const JVector4<float>& frameColor,
				const float frameThickness) = 0;
			//Use ImGui::Selectable and ImGui::Image
			//display one image
			virtual bool ImageButton(const std::string name,
				JGuiImageInfo info,
				const JVector2<float>& size,
				const JVector4<float>& bgColor, 
				const JVector4<float>& frameColor,
				const float frameThickness) = 0;
			virtual bool MaximizeButton(const bool isLocatedCloseBtnLeftSide) = 0;
			virtual bool MinimizeButton(const bool isLocatedCloseBtnLeftSide, const bool isLocatedMaximizeBtnLeftSize) = 0;
			virtual bool PreviousSizeButton(const bool isLocatedCloseBtnLeftSide, const bool isLocatedMaximizeBtnLeftSize) = 0;
		public:
			//use wolrd cursor pos
			virtual void DrawRectFilledMultiColor(const JVector2<float>& pos,
				const JVector2<float>& size,
				const JVector4<float>& upLeftCol,
				const JVector4<float>& upRightCol,
				const JVector4<float>& downLeftCol,
				const JVector4<float>& downRightCol,
				const float rounding,
				const J_GUI_DRAW_FLAG_ drawFlag,
				const bool useRestoreCursorPos)noexcept = 0;
			virtual void DrawRectFilledColor(const JVector2<float>& pos,
				const JVector2<float>& size,
				const JVector4<float>& color,
				const float rounding,
				const J_GUI_DRAW_FLAG_ drawFlag,
				const bool useRestoreCursorPos)noexcept = 0;
			virtual void DrawRectFrame(const JVector2<float>& pos,
				const JVector2<float>& size,
				const JVector4<float>& color,
				const float thickness,
				const float rounding,
				const J_GUI_DRAW_FLAG_ drawFlag,
				const bool useRestoreCursorPos)noexcept = 0;
			virtual void DrawToolTipBox(const std::string& uniqueLabel,
				const std::string& tooltip,
				const JVector2<float>& pos,
				const JVector2<float>& padding,
				const bool useRestoreCursorPos) = 0;
			virtual void DrawToolTipBox(const std::string& uniqueLabel,
				const std::string& tooltip,
				const JVector2<float>& pos,
				const float maxWidth,
				const JVector2<float>& padding,
				const J_EDITOR_ALIGN_TYPE alignType,
				const bool useRestoreCursorPos) = 0;
			virtual void DrawCircleFilledColor(const JVector2<float>& centerPos,
				const float radius,
				const JVector4<float>& color,
				const bool useRestoreCursorPos) = 0;
			virtual void DrawCircle(const JVector2<float>& centerPos,
				const float radius,
				const JVector4<float>& color,
				const bool useRestoreCursorPos,
				const float thickness) = 0;
		public:
			virtual void PushClipRect(const JVector2<float>& rectMinP, const JVector2<float>& rectMaxP, const bool intersectWithCurrentClipRect) = 0;
			virtual void AddLine(const JVector2<float>& p1, const JVector2<float>& p2, const JVector4<float>& color, const float thickness) = 0;
			virtual void AddTriangle(const JVector2<float>& p1, const JVector2<float>& p2, const JVector2<float>& p3, const JVector4<float>& color, const float thickness) = 0;
			virtual void AddTriangleFilled(const JVector2<float>& p1, const JVector2<float>& p2, const JVector2<float>& p3, const JVector4<float>& color) = 0;
			//Widget End 
		public:
			virtual void NewLine()noexcept = 0;
			virtual void SameLine()noexcept = 0;
			virtual void Separator()noexcept = 0;
			virtual void Indent()noexcept = 0;
			virtual void UnIndent()noexcept = 0;
		public:
			// Work Area
			virtual JVector2<float> GetMainWorkPos()const noexcept = 0;
			// Work Area
			virtual JVector2<float> GetMainWorkSize()const noexcept = 0;
			virtual JVector2<float> GetWindowPos()const noexcept = 0;
			virtual JVector2<float> GetWindowSize()const noexcept = 0;
			//size - cursor pos
			virtual JVector2<float> GetRestWindowSpace()const noexcept = 0;
			//applied padding border
			virtual JVector2<float> GetWindowContentsSize()const noexcept = 0;
			//applied padding border cursor pos
			virtual JVector2<float> GetRestWindowContentsSize()const noexcept = 0; 
			virtual JVector2<float> GetWindowMenuBarSize()const noexcept = 0;
			virtual JVector2<float> GetWindowTitleBarSize()const noexcept = 0; 
			virtual int GetWindowOrder(const GuiID windowID)const noexcept = 0;
			virtual bool GetWindowInfo(const std::string& wndName, _Out_ JGuiWindowInfo& info)const noexcept = 0;
			virtual bool GetWindowInfo(const GuiID windowID, _Out_ JGuiWindowInfo& info)const noexcept = 0; 
			virtual bool GetCurrentWindowInfo(_Out_ JGuiWindowInfo& info)const noexcept = 0; 
			virtual std::vector<JGuiWindowInfo> GetDisplayedWindowInfo(const bool isSortedBackToFront)const noexcept = 0;
			virtual void SetNextWindowPos(const JVector2<float>& pos, J_GUI_CONDIITON flag)noexcept = 0;
			virtual void SetNextWindowSize(const JVector2<float>& size, J_GUI_CONDIITON flag)noexcept = 0;	
			virtual void SetNextWindowFocus()noexcept = 0;
			virtual bool IsCurrentWindowFocused(J_GUI_FOCUS_FLAG flag)const noexcept = 0; 
			virtual void FocusWindow(const GuiID windowID) = 0;
			virtual void FocusCurrentWindow() = 0;
			virtual void RestoreFromMaximize(const GuiID windowID, const std::vector<GuiID>& preTabItemID) = 0;
		public:
			virtual JVector2<float> GetLastItemRectMin()const noexcept = 0;
			virtual JVector2<float> GetLastItemRectMax()const noexcept = 0;
			virtual JVector2<float> GetLastItemRectSize()const noexcept = 0;
			virtual void SetNextItemWidth(const float width)noexcept = 0;
			virtual void SetNextItemOpen(const bool value, J_GUI_CONDIITON flag)noexcept = 0;
			virtual void SetLastItemDefaultFocus() = 0;
			virtual bool IsLastItemActivated()const noexcept = 0;
			virtual bool IsLastItemHovered(J_GUI_HOVERED_FLAG flag)const noexcept = 0;
			virtual bool IsLastItemClicked(const Core::J_MOUSE_BUTTON btn)const noexcept = 0;
			virtual bool IsTreeNodeArrowHovered(const std::string& nodeName)const noexcept = 0;
			virtual bool CanUseWidget(const Core::J_GUI_WIDGET_TYPE type)const noexcept = 0;
			virtual JVector2<float> CalDefaultButtonSize(const std::string& label)const noexcept = 0;
			virtual JVector2<float> CalButtionSize(const std::string& label, const JVector2<float>& size)const noexcept = 0;
			virtual float CalCloseButtionSize()const noexcept = 0;
			virtual void PushItemFlag(J_GUI_ITEM_FLAG_ flag, const bool value)noexcept = 0;
			virtual void PopItemFlag()noexcept = 0;
			virtual void PushItemWidth(const float width)noexcept = 0;
			virtual void PopItemWidth()noexcept = 0;
#pragma endregion
#pragma region Docking 
			virtual bool GetDockNodeInfoByWindowName(const std::string& windowName, _Out_ JGuiDockNodeInfo& info)const noexcept = 0;
			virtual bool GetDockNodeInfo(const std::string& dockNodeName, _Out_ JGuiDockNodeInfo& info)const noexcept = 0;
			virtual bool GetDockNodeInfo(const GuiID dockID, _Out_ JGuiDockNodeInfo& info)const noexcept = 0; 
			virtual bool GetDockNodeHostWindowInfo(const GuiID childDockID, _Out_ JGuiWindowInfo& info) const noexcept = 0;
			virtual bool GetCurrentDockNodeInfo(_Out_ JGuiDockNodeInfo& info)const noexcept = 0;
			virtual bool HasDockNode(const std::string& dockNodeName)const noexcept = 0;
			virtual bool CanUseDockHirechary()noexcept = 0;
			virtual void BuildDockHirechary(const std::vector<std::unique_ptr<JGuiDockBuildNode>>& nodeVec) = 0;
		public:				
			virtual void UpdateDockSpace(const std::string& name, const JVector2<float>& size, J_GUI_DOCK_NODE_FLAG_ dockspaceFlag) = 0;
			virtual void OverrideNextDockNodeFlag(J_GUI_DOCK_NODE_FLAG_ flag) = 0;
			virtual void CloseTabItem(const GuiID windowID) = 0;
		public:
			virtual std::unique_ptr<JDockUpdateHelper> CreateDockUpdateHelper(const J_EDITOR_PAGE_TYPE pageType) = 0;
#pragma endregion

		};
	}
}