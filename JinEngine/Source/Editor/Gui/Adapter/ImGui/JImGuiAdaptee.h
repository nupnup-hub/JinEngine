#pragma once
#include"../JGuiBehaviorAdaptee.h" 

namespace JinEngine
{
	namespace Editor
	{ 
		struct JImGuiInitData;
		struct JImGuiPrivateData;
		class JImGuiAdaptee : public JGuiBehaviorAdaptee
		{
		private:
			std::unique_ptr<JImGuiPrivateData> data;
		public:
			void Initialize(std::unique_ptr<Graphic::JGuiInitData>&& initData) final;
			void Clear() final;
		protected:
			virtual void IntiailizeBackend(JImGuiInitData* initData) = 0;
			virtual void ClearBackend() = 0;
		public:
			void LoadGuiData()final;
			void StoreGuiData()final;
		public:
			J_GUI_TYPE GetGuiType()const noexcept final;
#pragma region Color
		public:
			JVector4<float> GetColor(const J_GUI_COLOR flag)noexcept final;
			uint GetUColor(const J_GUI_COLOR flag)const noexcept final; 
			void SetColor(const J_GUI_COLOR flag, const JVector4<float>& color)noexcept final;
			//Set widget color to default
			void SetColorToDefault(const J_GUI_COLOR flag)noexcept final;
			void SetAllColorToSoft(const JVector4<float>& factor)noexcept final;
			//Set all widget color to default
			void SetAllColorToDefault()noexcept final; 
			void PushColor(const J_GUI_COLOR colType, const JVector4<float>& color) final;
			void PopColor(const uint count)final;
#pragma endregion
#pragma region Style
		public:
			JVector2<float> GetWindowPadding()const noexcept final;
			float GetWindowBorderSize()const noexcept final;
			float GetWindowRounding()const noexcept final;
			JVector2<float> GetFramePadding()const noexcept final;
			float GetFrameBorderSize()const noexcept final;
			JVector2<float> GetItemSpacing()const noexcept final;
			JVector2<float> GetItemInnerSpacing()const noexcept final;
			float GetScrollBarSize()const noexcept final;
			float GetStyleValueF(const J_GUI_STYLE style)const noexcept final;
			JVector2<float> GetStyleValueV2(const J_GUI_STYLE style)const noexcept final;
			void SetWindowPadding(const JVector2<float>& padding)noexcept final;
			void SetWindowBorderSize(const float size)noexcept final;
			void SetWindowRounding(const float factor)noexcept final;
			void SetFramePadding(const JVector2<float>& padding)noexcept final;
			void SetFrameBorderSize(const float size)noexcept final;
			void SetItemSpacing(const JVector2<float>& spacing)noexcept final;
			void SetItemInnerSpacing(const JVector2<float>& spacing)noexcept final;
			void SetScrollBarSize(const float size)noexcept final;
			void PushStyle(const J_GUI_STYLE style, const float value) final;
			void PushStyle(const J_GUI_STYLE style, const JVector2<float>& value) final;
			void PopStyle(const uint count) final;
#pragma endregion
#pragma region IO
		public:
			JVector2<float> GetAlphabetSize()const noexcept final;
			float GetFontSize()const noexcept final;
			JVector2<float> CalTextSize(const std::string& str)const noexcept final;
		public:
			float GetGlobalFontScale()const noexcept final;
			float GetCurrentWindowFontScale()const noexcept final;
			void SetGlobalFontScale(const float scale)noexcept final;
			void SetCurrentWindowFontScale(const float scale)noexcept final;
			void SetFont(const J_GUI_FONT_TYPE fontType) final;
			void PushFont()const noexcept final;
			void PopFont()const noexcept final;
		public:
			//임시함수 gui lib에 input을 사용중이지만 추후에 keyboard, mouse class 설계 구현후 폐기
			uint GetMouseClickedCount(const Core::J_MOUSE_BUTTON btn)const noexcept final;
			float GetMouseWheel()const noexcept final;
			JVector2<float> GetMousePos() const noexcept final;
			JVector2<float> GetMouseDragDelta() const noexcept final;
			JVector2<float> GetCursorPos()const noexcept final;
			JVector2<float> GetCursorScreenPos()const noexcept final;
			void SetCursorPos(const JVector2<float>& pos)noexcept final;
			void SetCursorScreenPos(const JVector2<float>& pos)noexcept final;
			bool IsMouseClicked(const Core::J_MOUSE_BUTTON btn)const noexcept final;
			bool IsMouseReleased(const Core::J_MOUSE_BUTTON btn)const noexcept final;
			bool IsMouseDown(const Core::J_MOUSE_BUTTON btn)const noexcept final;
			bool IsMouseDragging(const Core::J_MOUSE_BUTTON btn)const noexcept final;
			bool IsMouseInRect(const JVector2<float>& min, const JVector2<float>& max)const noexcept final;
			bool IsMouseHoveringRect(const JVector2<float>& min, const JVector2<float>& max, const bool clip)const noexcept final;
			bool IsMouseInLine(JVector2<float> st, JVector2<float> ed, const float thickness)const noexcept final;
		public:
			bool IsKeyPressed(const Core::J_KEYCODE key)const noexcept final;
			bool IsKeyReleased(const Core::J_KEYCODE key)const noexcept final;
			bool IsKeyDown(const Core::J_KEYCODE key)const noexcept final;
		public:
			bool BeginDragDropSource(J_GUI_DRAG_DROP_FLAG_ flag)final;
			bool SetDragDropPayload(const std::string& typeName, Core::JTypeInstanceSearchHint* draggingHint, J_GUI_CONDIITON cond)final;
			void EndDragDropSource() final;
			bool BeginDragDropTarget() final;
			Core::JTypeInstanceSearchHint* TryGetTypeHintDragDropPayload(const std::string& typeName, J_GUI_DRAG_DROP_FLAG_ flag) final;
			void EndDragDropTarget() final;
#pragma endregion
#pragma region Widget
		public:
			bool BeginWindow(const std::string& name, bool* pOpen, J_GUI_WINDOW_FLAG flags) final;
			void EndWindow() final;
			bool BeginChildWindow(const std::string& name, const JVector2<float>& windowSize, bool border, J_GUI_WINDOW_FLAG flags) final;
			void EndChildWindow() final;
			void OpenPopup(const std::string& name, J_GUI_POPUP_FLAG flags) final;
			void CloseCurrentPopup() final;
			bool BeginPopup(const std::string& name, J_GUI_WINDOW_FLAG flags) final;
			void EndPopup() final;
			void BeginGroup() final;
			void EndGroup() final;
			void Text(const std::string& text)const noexcept final;
			bool CheckBox(const std::string& checkName, bool& v) final;
			bool Button(const std::string& btnName, const JVector2<float>& jVec2, J_GUI_BUTTON_FLAG flag) final;
			bool ArrowButton(const std::string& name, const JVector2<float>& jVec2, const float arrowScale, J_GUI_BUTTON_FLAG flag, J_GUI_CARDINAL_DIR dir)final;
			bool IsTreeNodeOpend(const std::string& name, J_GUI_TREE_NODE_FLAG_ flags) final;
			bool TreeNodeEx(const std::string& nodeName, J_GUI_TREE_NODE_FLAG flags) final;
			void TreePop() final;
			bool Selectable(const std::string& name, bool* pSelected, J_GUI_SELECTABLE_FLAG flags, const JVector2<float>& sizeArg) final;
			bool Selectable(const std::string& name, bool selected, J_GUI_SELECTABLE_FLAG flags, const JVector2<float>& sizeArg) final;
			bool InputText(const std::string& name, std::string& buff, J_GUI_INPUT_TEXT_FLAG flags) final;
			bool InputText(const std::string& name, std::string& buff, const size_t size, J_GUI_INPUT_TEXT_FLAG flags)final;
			bool InputText(const std::string& name, std::string& buff, std::string& result, const std::string& hint, J_GUI_INPUT_TEXT_FLAG flags) final;
			bool InputMultilineText(const std::string& name, std::string& buff, std::string& result, const JVector2<float>& size, J_GUI_INPUT_TEXT_FLAG flags) final;
			bool InputInt(const std::string& name, int* value, J_GUI_INPUT_TEXT_FLAG flags, int step) final;
			bool InputFloat(const std::string& name, float* value, J_GUI_INPUT_TEXT_FLAG flags, const uint formatDigit, float step) final;
		public:
			bool SliderInt(const std::string& name, int* value, int vMin, int vMax,  J_GUI_SLIDER_FLAG flags) final;
			bool SliderFloat(const std::string& name, float* value, float vMin, float vMax, const uint formatDigit, J_GUI_SLIDER_FLAG flags) final;
			bool VSliderInt(const std::string& name, JVector2<float> size, int* value, int vMin, int vMax, J_GUI_SLIDER_FLAG flags) final;
			bool VSliderFloat(const std::string& name, JVector2<float> size, float* value, float vMin, float vMax, const uint formatDigit, J_GUI_SLIDER_FLAG flags) final;
		public:
			bool BeginTabBar(const std::string& name, J_GUI_TAB_BAR_FLAG flags) final;
			void EndTabBar() final;
			bool BeginTabItem(const std::string& name, bool* pOpen, J_GUI_TAB_ITEM_FLAG flags) final;
			void EndTabItem() final;
			bool TabItemButton(const std::string& name, J_GUI_TAB_ITEM_FLAG flags) final;
		public:
			bool BeginTable(const std::string& name, const int columnCount, J_GUI_TABLE_FLAG flags, const JVector2<float> outerSize, const float innerWidth) final;
			void EndTable() final;
			void TableSetupColumn(const std::string& name, J_GUI_TABLE_COLUMN_FLAG flags, float initWeight) final;
			void TableHeadersRow() final;
			void TableNextRow() final;
			void TableSetColumnIndex(const int index) final;
		public:
			bool BeginMainMenuBar() final;
			void EndMainMenuBar() final;
			bool BeginMenuBar() final;
			void EndMenuBar() final;
			bool BeginMenu(const std::string& name, bool enable = true) final;
			void EndMenu() final;
			bool MenuItem(const std::string& name, bool selected, bool enabled) final;
			bool MenuItem(const std::string& name, const std::string& shortcut, bool selected, bool enabled) final;
		public:
			bool BeginCombo(const std::string& name, const std::string& preview, J_GUI_COMBO_FLAG flags) final;
			void EndCombo() final;
		public:
			bool BeginListBox(const std::string& name, const JVector2<float> size) final;
			void EndListBox() final;
		public:
			bool ColorPicker(const std::string& name, JVector3<float>& color, J_GUI_COLOR_EDIT_FALG_ flags)final;
			bool ColorPicker(const std::string& name, JVector4<float>& color, J_GUI_COLOR_EDIT_FALG_ flags)final;
			void Tooltip(const std::string& message)noexcept final;
		public:
			//Image  
			void InvalidImage(const JVector2<float>& size,
				const JVector2<float>& uv0 = JVector2<float>(0, 0),
				const JVector2<float>& uv1 = JVector2<float>(1, 1),
				const JVector4<float>& tintCol = JVector4<float>(1, 1, 1, 1),
				const JVector4<float>& borderCol = JVector4<float>(0, 0, 0, 0)) final;
			void Image(const JGuiImageInfo& info,
				const JVector2<float>& size,
				const JVector2<float>& uv0 = JVector2<float>(0, 0),
				const JVector2<float>& uv1 = JVector2<float>(1, 1),
				const JVector4<float>& tintCol = JVector4<float>(1, 1, 1, 1),
				const JVector4<float>& borderCol = JVector4<float>(0, 0, 0, 0)) final;
			//Use ImGui::ImageButtonEx
			//display one image
			bool ImageButton(const std::string name,
				const JGuiImageInfo& info,
				const JVector2<float>& size,
				const JVector2<float>& uv0,
				const JVector2<float>& uv1,
				float framePadding,
				const JVector4<float>& bgCol,
				const JVector4<float>& tintCol) final;
			void AddImage(const JGuiImageInfo& info,
				const JVector2<float>& pMin,
				const JVector2<float>& pMax,
				const bool isFront,
				const JVector4<float>& color,
				const JVector2<float>& uvMin,
				const JVector2<float>& uvMax) final;
			void AddRoundedImage(const JGuiImageInfo& info,
				const JVector2<float>& pMin,
				const JVector2<float>& pMax,
				const bool isFront,
				const float rounding,
				J_GUI_DRAW_FLAG_ flag,
				const JVector4<float>& color,
				const JVector2<float>& uvMin,
				const JVector2<float>& uvMax) final;
		public:
			//Custom Widget 
			//display one image
			bool ImageSelectable(const std::string name,
				JGuiImageInfo info,
				bool& pressed,
				bool changeValueIfPreesd,
				const JVector2<float>& size,
				const bool useRestoreCursorPos) final;
			bool Switch(const std::string& name,
				bool& pSelected,
				bool changeValueIfPreesd,
				const JVector2<float>& size) final;
			//display one image
			bool ImageSwitch(const std::string name,
				JGuiImageInfo info,
				bool& pressed,
				bool changeValueIfPreesd,
				const JVector2<float>& size,
				const JVector4<float>& bgColor, 
				const JVector4<float>& frameColor,
				const float frameThickness) final;
			//Use ImGui::Selectable and ImGui::Image
			//display one image
			bool ImageButton(const std::string name,
				JGuiImageInfo info,
				const JVector2<float>& size,
				const JVector4<float>& bgColor, 
				const JVector4<float>& frameColor,
				const float frameThickness) final;
			bool MaximizeButton(const bool isLocatedCloseBtnLeftSide) final;
			bool MinimizeButton(const bool isLocatedCloseBtnLeftSide, const bool isLocatedMaximizeBtnLeftSize)  final;
			bool PreviousSizeButton(const bool isLocatedCloseBtnLeftSide, const bool isLocatedMaximizeBtnLeftSize) final;
		public:
			//use wolrd cursor pos 
			void DrawRectFilledMultiColor(const JVector2<float>& pos,
				const JVector2<float>& size,
				const JVector4<float>& upLeftCol,
				const JVector4<float>& upRightCol,
				const JVector4<float>& downLeftCol,
				const JVector4<float>& downRightCol,
				const float rounding,
				const J_GUI_DRAW_FLAG_ drawFlag,
				const bool useRestoreCursorPos)noexcept final;
			void DrawRectFilledColor(const JVector2<float>& pos,
				const JVector2<float>& size,
				const JVector4<float>& color,
				const float rounding,
				const J_GUI_DRAW_FLAG_ drawFlag,
				const bool useRestoreCursorPos)noexcept final;
			void DrawRectFrame(const JVector2<float>& pos,
				const JVector2<float>& size,
				const JVector4<float>& color,
				const float thickness,
				const float rounding,
				const J_GUI_DRAW_FLAG_ drawFlag,
				const bool useRestoreCursorPos)noexcept final;
			void DrawToolTipBox(const std::string& uniqueLabel,
				const std::string& tooltip,
				const JVector2<float>& pos,
				const JVector2<float>& padding,
				const bool useRestoreCursorPos) final;
			void DrawToolTipBox(const std::string& uniqueLabel,
				const std::string& tooltip,
				const JVector2<float>& pos,
				const float maxWidth,
				const JVector2<float>& padding,
				const J_EDITOR_ALIGN_TYPE alignType,
				const bool useRestoreCursorPos) final;
			void DrawCircleFilledColor(const JVector2<float>& centerPos,
				const float radius,
				const JVector4<float>& color,
				const bool useRestoreCursorPos) final;
			void DrawCircle(const JVector2<float>& centerPos,
				const float radius,
				const JVector4<float>& color,
				const bool useRestoreCursorPos,
				const float thickness) final;
		public:
			void PushClipRect(const JVector2<float>& rectMinP, const JVector2<float>& rectMaxP, const bool intersectWithCurrentClipRect)final;
			void AddLine(const JVector2<float>& p1, const JVector2<float>& p2, const JVector4<float>& color, const float thickness)final;
			void AddTriangle(const JVector2<float>& p1, const JVector2<float>& p2, const JVector2<float>& p3, const JVector4<float>& color, const float thickness)final;
			void AddTriangleFilled(const JVector2<float>& p1, const JVector2<float>& p2, const JVector2<float>& p3, const JVector4<float>& color) final;
			//Widget End 
		public:
			void NewLine()noexcept final;
			void SameLine()noexcept final;
			void Separator()noexcept final;
			void Indent(const float width)noexcept final;
			void UnIndent(const float width)noexcept final;
			float IndentMovementPixel(const float width)const noexcept final;
		public:
			// Work Area: Position of the viewport minus task bars, menus bars, status bars (>= Pos)
			JVector2<float> GetMainWorkPos()const noexcept final;
			// Work Area: Size of the viewport minus task bars, menu bars, status bars (<= Size)
			JVector2<float> GetMainWorkSize()const noexcept final;
			JVector2<float> GetWindowPos()const noexcept final;
			JVector2<float> GetWindowSize()const noexcept final;
			//size - cursor pos
			JVector2<float> GetRestWindowSpace()const noexcept final;
			//applied padding border
			JVector2<float> GetWindowContentsSize()const noexcept final;
			//applied padding border cursor pos
			JVector2<float> GetRestWindowContentsSize()const noexcept final;
			JVector2<float> GetWindowMenuBarSize()const noexcept final;
			JVector2<float> GetWindowTitleBarSize()const noexcept final;
			int GetWindowOrder(const GuiID windowID)const noexcept final;
			bool GetWindowInfo(const std::string& wndName, _Inout_ JGuiWindowInfo& info)const noexcept final;
			bool GetWindowInfo(const GuiID windowID, _Inout_ JGuiWindowInfo& info)const noexcept final;
			bool GetCurrentWindowInfo(_Inout_ JGuiWindowInfo& info)const noexcept final;
			std::vector<JGuiWindowInfo> GetDisplayedWindowInfo(const bool isSortedBackToFront)const noexcept final;
			std::set<GuiID> GetWindowOpendTreeNodeID(const GuiID windowID)const noexcept final;
			void SetNextWindowPos(const JVector2<float>& pos, J_GUI_CONDIITON flag)noexcept final;
			void SetNextWindowSize(const JVector2<float>& size, J_GUI_CONDIITON flag)noexcept final;
			void SetNextWindowFocus()noexcept final; 
			bool IsCurrentWindowFocused(J_GUI_FOCUS_FLAG flag)const noexcept final; 
			void FocusWindow(const GuiID windowID)final;
			void FocusCurrentWindow() final;
			void RestoreFromMaximize(const GuiID windowID, const std::vector<GuiID>& preTabItemID) final;
			GuiID CalCurrentWindowItemID(const std::string& label)const noexcept final;
		public:
			JVector2<float> GetLastItemRectMin()const noexcept final;
			JVector2<float> GetLastItemRectMax()const noexcept final;
			JVector2<float> GetLastItemRectSize()const noexcept final;
			void SetNextItemWidth(const float width)noexcept final;
			void SetNextItemOpen(const bool value, J_GUI_CONDIITON flag)noexcept final;
			void SetLastItemDefaultFocus() final;
			bool IsLastItemActivated()const noexcept final;
			bool IsLastItemHovered(J_GUI_HOVERED_FLAG flag)const noexcept final;
			bool IsLastItemClicked(const Core::J_MOUSE_BUTTON btn)const noexcept final;
			bool IsTreeNodeArrowHovered(const std::string& nodeName)const noexcept final;
			bool CanUseWidget(const Core::J_GUI_WIDGET_TYPE type)const noexcept final;
			JVector2<float> CalDefaultButtonSize(const std::string& label)const noexcept final;
			JVector2<float> CalButtionSize(const std::string& label, const JVector2<float>& size)const noexcept final; 
			float CalCloseButtionSize()const noexcept final;
			void PushItemFlag(J_GUI_ITEM_FLAG_ flag, const bool value)noexcept final;
			void PopItemFlag()noexcept final;
			void PushItemWidth(const float width)noexcept final;
			void PopItemWidth()noexcept final;
#pragma endregion
#pragma region Docking 
			bool GetDockNodeInfoByWindowName(const std::string& windowName, _Inout_ JGuiDockNodeInfo& info)const noexcept final;
			bool GetDockNodeInfo(const std::string& dockNodeName, _Inout_ JGuiDockNodeInfo& info)const noexcept final;
			bool GetDockNodeInfo(const GuiID dockID, _Inout_ JGuiDockNodeInfo& info)const noexcept final;
			bool GetDockNodeHostWindowInfo(const GuiID childDockID, _Inout_ JGuiWindowInfo& info) const noexcept final;
			bool GetCurrentDockNodeInfo(_Inout_ JGuiDockNodeInfo& info)const noexcept final;
			bool HasDockNode(const std::string& dockNodeName)const noexcept final;
			bool CanUseDockHirechary()noexcept final;
			void BuildDockHirechary(const std::vector<std::unique_ptr<JGuiDockBuildNode>>& nodeVec) final;
		public:
			void UpdateDockSpace(const std::string& name, const JVector2<float>& size, J_GUI_DOCK_NODE_FLAG_ dockspaceFlag) final;
			void OverrideNextDockNodeFlag(J_GUI_DOCK_NODE_FLAG_ flag)final;
			void CloseTabItem(const GuiID windowID) final;
		public:
			std::unique_ptr<JDockUpdateHelper> CreateDockUpdateHelper(const J_EDITOR_PAGE_TYPE pageType) final;
#pragma endregion
		};
	}
}