#pragma once 
#include<string>  
#include<bitset>
#include"../../Page/JEditorPageType.h"
#include"../../Event/JEditorEventType.h"   
#include"../../Event/JEditorEventStruct.h"  
#include"../../../Utility/JVector.h"
#include"../../../../Lib/imgui/imgui.h"
#include"../../../../Lib/imgui/imgui_internal.h"
#include"../../../Core/Empty/EmptyBase.h"
#include"../../../Core/Event/JEventListener.h"   
#include"../../../Core/Event/JEventManager.h"

namespace JinEngine
{
    class JEditorManager;
	namespace Graphic
	{
		class JGraphicTexture;
	}
    namespace Editor
    {
		class JImGuiImpl
		{
		private:
			friend class JEditorManager;
		private:
			using JEvInterface = Core::JEventInterface<size_t, J_EDITOR_EVENT, JEditorEventStruct*>;
		public:
			//Text
			static JVector2<int> GetTextSize()noexcept;
			static JVector2<int> GetTextSizeOffset()noexcept;
			static void SetTextSize()noexcept; 
			static void SetTextSizeOffset(JVector2<int> offset)noexcept;
		public:
			//Font
			//0 = default, 1 = bigRegular, 2 = smallRegular
			static void SetFont(int index)noexcept;
			static void PushFont()noexcept;
			static void PopFont()noexcept;
		public:
			//Color
			static ImVec4 GetColor(ImGuiCol_ flag)noexcept;
			static void SetColorToDeep(ImGuiCol_ flag, float factor)noexcept;
			static void SetColor(const ImVec4& color, ImGuiCol_ flag)noexcept;
			//Set widget color to default
			static void SetColorToDefault(ImGuiCol_ flag)noexcept;
			//Set all widget color to default
			static void SetAllColorToDefault()noexcept;
		public:
			//Window 
			static JVector2<int> GetDisplaySize()noexcept; 
			static JVector2<int> GetWindowSize()noexcept;
			static JVector2<int> GetClientWindowPos()noexcept;
			static JVector2<int> GetClientWindowSize()noexcept;
			static JVector2<float> GetGuiWindowPos()noexcept;
			static JVector2<float> GetGuiWindowSize()noexcept;
			static JVector2<float> GetCursorPos()noexcept;
		public:
			//Passing method for trace to ImGuI widget api calling
			static bool BeginWindow(const std::string& name, bool* p_open = 0, ImGuiWindowFlags flags = 0);
			static void EndWindow();
			static bool BeginChildWindow(const std::string& name, const JVector2<float>& windowSize, bool border, ImGuiWindowFlags extra_flags);
			static void EndChildWindow();
			static void Text(const std::string& text);
			static bool CheckBox(const std::string& checkName, bool& v);
			static bool Button(const std::string& btnName, const JVector2<float>& jVec2);
			//ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1)); 
			static bool ImageButton(Graphic::JGraphicTexture& graphicTexture,
				const JVector2<float>& size,
				const JVector2<float>& uv0 = JVector2<float>(0,0),
				const JVector2<float>& uv1 = JVector2<float>(1,1),
				int framePadding = -1, 
				const JVector4<float>& bgCol = JVector4<float>(0,0,0,0),
				const JVector4<float>& tintCol = JVector4<float>(1,1,1,1));
			static bool TreeNodeEx(const std::string& nodeName, ImGuiTreeNodeFlags flags);
			static void TreePop();
			static bool Selectable(const std::string& name, bool* pSelected = nullptr, ImGuiSelectableFlags flags = 0, const JVector2<float>& sizeArg = { 0,0 });
			static bool InputText(const std::string& name, char* buf, size_t bufSize, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback txtCallback = 0, void* userData = 0);
			static bool InputInt(const std::string& name, int* value, int step = 1, int stepFast = 100, ImGuiInputTextFlags flags = 0);
			static bool InputFloat(const std::string& name, float* value, float step = 0.0f, float stepFast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
		public:
			static bool SliderFloat(const std::string& name, float* value, float vMin, float vMax, const char* format, ImGuiSliderFlags flags);
		public:
			static bool BeginTabBar(const std::string& name, const ImGuiTabBarFlags flags = 0);
			static void EndTabBar();
			static bool TabItemButton(const std::string& name, const ImGuiTabItemFlags flags = 0);
		public:
			static bool BeginTable(const std::string& name, int columnCount, ImGuiTableFlags flags = 0, const JVector2<float> outerSize = { 0,0 }, float innerWidth = 0);
			static bool EndTable();
			static void TableSetupColumn(const std::string& name, ImGuiTableColumnFlags flags = 0, float init_Width_or_Weight = 0, ImGuiID userId = 0);
			static void TableHeadersRow();
			static void TableNextRow();
			static void TableSetColumnIndex(int index);
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
			static bool BeginCombo(const std::string& name, const char* previewValue, ImGuiComboFlags flags = 0);
			static void EndCombo();
			//Widget End
		public:
			static bool IsLeftMouseClick()noexcept;
			static bool IsRightMouseClick()noexcept;
			static bool IsMiddleMouseClick()noexcept;
			static bool IsMouseInWindow(const JVector2<float>& position, const JVector2<float>& size)noexcept;
			//0 = left, 1 = right, 2 = middle
			static void SetMouseClick(const ImGuiMouseButton btn, const bool value)noexcept;
			static void SetMouseDrag(bool value)noexcept;	
		public:
			//util 
			static uint GetSliderWidth()noexcept;
		public:
			//Option
			static bool IsFullScreen()noexcept;
			static bool IsWindowPadding()noexcept;
		public:
			static JEvInterface* EvInterface();
			static JEditorEventStruct* RegisterEvStruct(std::unique_ptr<JEditorEventStruct> evStruct);
			static void ClearEvStructQueue();
		public:
			static void RegisterPage(const J_EDITOR_PAGE_TYPE pageType, bool hasOpenInitObjType)noexcept;
			static void UnRegisterPage(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static void ClearPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept;
		public:
			static bool HasValidOpenPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept;
		public:
			static JEditorOpenPageEvStruct GetOpendPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static JObject* GetSelectedObj(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static void SetPageOpenData(JEditorOpenPageEvStruct* evStruct)noexcept;
			static void SetSelectObj(const J_EDITOR_PAGE_TYPE pageType, const Core::JUserPtr<JObject>& selectObj)noexcept;
		private:
			static void StartEditorUpdate();
			static void EndEditorUpdate();
		private:
			static void Initialize();
			static void Clear();
		}; 
    }
}
