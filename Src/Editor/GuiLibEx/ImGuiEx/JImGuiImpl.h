#pragma once 
#include<string>  
#include<bitset>
#include"../../Page/JEditorWindowFontType.h"
#include"../../../Utility/JVector.h"
#include"../../../Core/JDataType.h"
#include"../../../Core/Empty/EmptyBase.h"
#include"../../../Core/Reflection/JReflection.h"
#include"../../../Core/Func/Functor/JFunctor.h"
#include"../../../Core/Undo/JTransition.h"
#include"../../../../Lib/imgui/imgui.h"
#include"../../../../Lib/imgui/imgui_internal.h"

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
	w = jVec4.w;
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

namespace JinEngine
{
	template<typename T>
	JVector2<T>::JVector2(const ImVec2& v)
		:x(v.x), y(v.y)
	{}
	template<typename T>
	JVector4<T>::JVector4(const ImVec4& v)
		: x(v.x), y(v.y), z(v.z), w(v.w)
	{}
	template<typename T>
	JVector2<T>& JVector2<T>::operator=(const ImVec2& rhs)
	{
		x = rhs.x;
		y = rhs.y;
		return *this;
	}
	template<typename T>
	JVector4<T>& JVector4<T>::operator=(const ImVec4& rhs)
	{
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		w = rhs.w;
		return *this;
	}

	namespace Graphic
	{
		class JGraphicTexture;
	}
    namespace Editor
	{
		class JEditorManager;
		class JImGuiImpl
		{
		private:
			friend class JEditorManager;
		public:
			//Text
			static JVector2<int> GetTextSize()noexcept;
			static JVector2<int> GetTextSizeOffset()noexcept;
			static void SetTextSize()noexcept; 
			static void SetTextSizeOffset(JVector2<int> offset)noexcept;
		public:
			//Font 
			static void SetFont(const J_EDITOR_FONT_TYPE fontType)noexcept;
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
			//Widget Start
			//Passing method for trace to ImGuI widget api calling
			static bool BeginWindow(const std::string& name, bool* p_open = 0, ImGuiWindowFlags flags = 0);
			static void EndWindow();
			static bool BeginChildWindow(const std::string& name, const JVector2<float>& windowSize, bool border, ImGuiWindowFlags extra_flags);
			static void EndChildWindow();
			static void Text(const std::string& text);
			static bool CheckBox(const std::string& checkName, bool& v);
			static bool Button(const std::string& btnName, const JVector2<float>& jVec2 = { 0,0 });
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
			static void EndTable();
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
		public:
			//Image
			static void Image(Graphic::JGraphicTexture& graphicTexture,
				const JVector2<float>& size,
				const JVector2<float>& uv0 = JVector2<float>(0, 0),
				const JVector2<float>& uv1 = JVector2<float>(1, 1),
				const JVector4<float>& tintCol = JVector4<float>(1, 1, 1, 1),
				const JVector4<float>& borderCol = JVector4<float>(0, 0, 0, 0));
			static bool ImageButton(Graphic::JGraphicTexture& graphicTexture,
				const JVector2<float>& size,
				const JVector2<float>& uv0 = JVector2<float>(0, 0),
				const JVector2<float>& uv1 = JVector2<float>(1, 1),
				int framePadding = -1,
				const JVector4<float>& bgCol = JVector4<float>(0, 0, 0, 0),
				const JVector4<float>& tintCol = JVector4<float>(1, 1, 1, 1));
			static void AddImage(Graphic::JGraphicTexture& graphicTexture,
				const JVector2<float>& pMin,
				const JVector2<float>& pMax,
				bool isBack = true,
				ImU32 color = IM_COL32_WHITE,
				const JVector2<float>& uvMin = JVector2<float>(0, 0),
				const JVector2<float>& uvMax = JVector2<float>(1, 1));
			//Widget End
		public:
			static bool IsLeftMouseClicked()noexcept;
			static bool IsRightMouseClicked()noexcept;
			static bool IsMiddleMouseClicked()noexcept;
			static bool IsDraggingMouse()noexcept;
			static bool IsMouseInWindow(const JVector2<float>& position = GetGuiWindowPos(), const JVector2<float>& size = GetGuiWindowSize())noexcept;
			//0 = left, 1 = right, 2 = middle
			static void SetMouseClick(const ImGuiMouseButton btn, const bool value)noexcept;
			static void SetMouseDrag(bool value)noexcept;	
		public:
			//util 
			static float GetSliderWidth()noexcept;
		public:
			//Option
			static bool IsFullScreen()noexcept;
			static bool IsWindowPadding()noexcept;
			static bool IsEnablePopup()noexcept;
			static bool IsEnableSelector()noexcept;
		private:
			static void StartEditorUpdate();
			static void MouseUpdate();
			static void EndEditorUpdate();
		private:
			static void Initialize();
			static void Clear();
		public:
			//Widget Set
			//Support Redo undo
			template<typename ...Param>
			static bool CheckBoxSet(const std::string& uniqueLabel, const bool preValue, Core::JFunctor<void, const bool, Param...>& commitFunctor, Param... var)
			{
				bool nowValue = preValue;
				if (JImGuiImpl::CheckBox(("##CheckBox" + uniqueLabel).c_str(), nowValue))
				{
					using Functor = Core::JFunctor<void, const bool, Param...>;
					using Binder = Core::JBindHandle<Functor, const bool, Param...>;

					bool dovalue = nowValue;
					bool undovalue = preValue;

					Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>(uniqueLabel + " checkbox set value: " + std::to_string(nowValue),
						std::make_unique<Binder>(commitFunctor, std::move(dovalue), std::forward<Param>(var)...),
						std::make_unique<Binder>(commitFunctor, std::move(undovalue), std::forward<Param>(var)...)));
					return true;
				}
				return false;
			}
			template<typename ...Param>
			static bool InputIntSet(const std::string& uniqueLabel, const int preValue, Core::JFunctor<void, const int, Param...>& commitFunctor, Param... var)
			{
				int nowValue = preValue; 
				if (JImGuiImpl::InputInt(("##InputInt" + uniqueLabel).c_str(), &nowValue))
				{
					if (nowValue != preValue)
					{
						using Functor = Core::JFunctor<void, const int, Param...>;
						using Binder = Core::JBindHandle<Functor, const int, Param...>;

						int dovalue = nowValue;
						int undovalue = preValue;

						Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>(uniqueLabel + " input int set value: " + std::to_string(nowValue),
							std::make_unique<Binder>(commitFunctor, std::move(dovalue), std::forward<Param>(var)...),
							std::make_unique<Binder>(commitFunctor, std::move(undovalue), std::forward<Param>(var)...)));
						return true;
					}
				}
				return false;
			}
			template<typename ...Param>
			static bool InputFloatSet(const std::string& uniqueLabel, const float preValue, Core::JFunctor<void, const float, Param...>& commitFunctor, Param... var)
			{
				float nowValue = preValue;
				if (JImGuiImpl::InputFloat(("##IntputFloat" + uniqueLabel).c_str(), &nowValue))
				{
					if (nowValue != preValue)
					{
						using Functor = Core::JFunctor<void, const float, Param...>;
						using Binder = Core::JBindHandle<Functor, const float, Param...>;

						float dovalue = nowValue;
						float undovalue = preValue;

						Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>(uniqueLabel + " input float set value: " + std::to_string(nowValue),
							std::make_unique<Binder>(commitFunctor, std::move(dovalue), std::forward<Param>(var)...),
							std::make_unique<Binder>(commitFunctor, std::move(undovalue), std::forward<Param>(var)...)));
						return true;
					}
				}
				return false;
			}
			template<typename ...Param>
			static bool InputTextSet(const std::string& uniqueLabel, 
				const std::string& preValue, 
				const size_t bufMaxLength,
				ImGuiInputTextFlags flags,
				Core::JFunctor<void, const std::string, Param...>& commitFunctor, Param... var)
			{
				std::string nowValue = preValue;
				if (JImGuiImpl::InputText(("##InputText" + uniqueLabel).c_str(), &nowValue[0], bufMaxLength, flags))
				{
					if (nowValue != preValue)
					{
						std::string dovalue = nowValue;
						std::string undovalue = preValue;

						using Functor = Core::JFunctor<void, const std::string, Param...>;
						using Binder = Core::JBindHandle<Functor, const std::string, Param...>;
						Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>(uniqueLabel + " input text set value: " + nowValue,
							std::make_unique<Binder>(commitFunctor, std::move(dovalue), std::forward<Param>(var)...),
							std::make_unique<Binder>(commitFunctor, std::move(undovalue), std::forward<Param>(var)...)));
						return true;
					}
				}
				return false;
			}
			template<typename EnumType, typename ...Param>
			static void ComoboSet(const std::string& uniqueLabel, const EnumType preValue, Core::JFunctor<void, const EnumType, Param...>& clickFunctor, Param... var)
			{
				const std::string enumName = Core::GetName<EnumType>();
				const std::string preValueName = Core::GetName(preValue);
				if (JImGuiImpl::BeginCombo(("##EnumCombo" + enumName + uniqueLabel).c_str(), preValueName.c_str()))
				{
					for (uint i = 0; i < (int)EnumType::COUNT; ++i)
					{ 
						std::string valueStr = Core::GetName((EnumType)i) + "##" + enumName + uniqueLabel;
						if (JImGuiImpl::Selectable(valueStr.c_str()))
						{
							using Functor = Core::JFunctor<void, const EnumType, Param...>;
							using Binder = Core::JBindHandle<Functor, const EnumType, Param...>;

							EnumType dovalue = (EnumType)i;
							EnumType undovalue = preValue;

							Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>(enumName + "::" + valueStr,
								std::make_unique<Binder>(clickFunctor, std::move(dovalue), std::forward<Param>(var)...),
								std::make_unique<Binder>(clickFunctor, std::move(undovalue), std::forward<Param>(var)...)));
						}
					}
					JImGuiImpl::EndCombo();
				}
			}
		}; 
    }
}
