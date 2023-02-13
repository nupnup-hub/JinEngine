#pragma once 
#include<string>  
#include<bitset>
#include"../../Align/JEditorAlignType.h"
#include"../../Page/JEditorWindowFontType.h"
#include"../../Helpers/JEditorInputBuffHelper.h"
#include"../../../Utility/JVector.h"
#include"../../../Utility/JCommonUtility.h"
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
		class JGraphicResourceHandleInterface;
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
			static JVector2<int> GetAlphabetSize()noexcept;
			static void SetAlphabetSize()noexcept;
			static uint GetTextBuffRange()noexcept;
		public:
			//Font 
			static void SetFont(const J_EDITOR_FONT_TYPE fontType)noexcept;
			static void PushFont()noexcept;
			static void PopFont()noexcept;
		public:
			//Color 
			static JVector4<float> GetSelectColorFactor()noexcept;
			static JVector4<float> GetColor(ImGuiCol_ flag)noexcept;
			static ImU32 GetUColor(ImGuiCol_ flag)noexcept;
			static void SetColorToSoft(ImGuiCol_ flag, const JVector4<float>& color)noexcept;
			static void SetColor(const JVector4<float>& color, ImGuiCol_ flag)noexcept;
			//Set widget color to default
			static void SetColorToDefault(ImGuiCol_ flag)noexcept;
			static void SetAllColorToDeep(float factor)noexcept;
			//Set all widget color to default
			static void SetAllColorToDefault()noexcept;
			static void ActivateButtonColor()noexcept;
			static void DeActivateButtonColor()noexcept;
		public:
			//Window 
			static JVector2<int> GetDisplaySize()noexcept;
			static JVector2<int> GetWindowSize()noexcept;
			static JVector2<int> GetClientWindowPos()noexcept;
			static JVector2<int> GetClientWindowSize()noexcept;
			static JVector2<float> GetGuiWindowPos()noexcept;
			static JVector2<float> GetGuiWindowSize()noexcept;
			static JVector2<int> GetGuiWidnowContentsSize()noexcept;  
			static ImGuiWindow* GetGuiWindow(const ImGuiID id)noexcept;
			static ImGuiWindow* GetGuiWindow(const ImGuiID id, _Out_ int& order)noexcept;
		public:
			//Widget Start
			//Passing method for trace to ImGuI widget api calling
			static bool BeginWindow(const std::string& name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
			static void EndWindow();
			static bool BeginChildWindow(const std::string& name, const JVector2<float>& windowSize = JVector2<float>{ 0,0 }, bool border = false, ImGuiWindowFlags extra_flags = 0);
			static void EndChildWindow();
			static bool BeginPopup(const std::string& name, ImGuiPopupFlags flags = 0);
			static void EndPopup();
			static void Text(const std::string& text); 
			static bool CheckBox(const std::string& checkName, bool& v);
			static bool Button(const std::string& btnName, const JVector2<float>& jVec2 = { 0,0 });
			static bool TreeNodeEx(const std::string& nodeName, ImGuiTreeNodeFlags flags);
			static void TreePop();
			static bool Selectable(const std::string& name, bool* pSelected = nullptr, ImGuiSelectableFlags flags = 0, const JVector2<float>& sizeArg = { 0,0 });
			static bool Selectable(const std::string& name, bool selected, ImGuiSelectableFlags flags = 0, const JVector2<float>& sizeArg = { 0,0 });
			static bool InputText(const std::string& name, std::string& buff, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback txtCallback = 0, void* userData = 0);
			static bool InputText(const std::string& name, std::string& buff, std::string& result, const std::string& hint, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback txtCallback = 0, void* userData = 0);
			static bool InputMultilineText(const std::string& name, std::string& buff, std::string& result, const JVector2<float>& size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback txtCallback = 0, void* userData = 0);
			static bool InputInt(const std::string& name, int* value, ImGuiInputTextFlags flags = 0, int step = 1, int stepFast = 100);
			static bool InputFloat(const std::string& name, float* value, ImGuiInputTextFlags flags = 0, const char* format = "%.2f", float step = 0.0f, float stepFast = 0.0f);
		public:
			static bool SliderInt(const std::string& name, int* value, int vMin, int vMax, const char* format, ImGuiSliderFlags flags);
			static bool SliderFloat(const std::string& name, float* value, float vMin, float vMax, const char* format, ImGuiSliderFlags flags);
			static bool VSliderInt(const std::string& name, JVector2<float> size, int* value, int vMin, int vMax, const char* format, ImGuiSliderFlags flags);
			static bool VSliderFloat(const std::string& name, JVector2<float> size, float* value, float vMin, float vMax, const char* format, ImGuiSliderFlags flags);
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
			static bool BeginListBox(const std::string& name, const JVector2<float> size = { 0,0 });
			static void EndListBox();
		public:
			//Image  
			static void Image(Graphic::JGraphicResourceHandleInterface& handle,
				const JVector2<float>& size,
				const JVector2<float>& uv0 = JVector2<float>(0, 0),
				const JVector2<float>& uv1 = JVector2<float>(1, 1),
				const JVector4<float>& tintCol = JVector4<float>(1, 1, 1, 1),
				const JVector4<float>& borderCol = JVector4<float>(0, 0, 0, 0));
			static bool ImageButton(const std::string name,
				Graphic::JGraphicResourceHandleInterface& handle,
				const JVector2<float>& size,
				const JVector2<float>& uv0 = JVector2<float>(0, 0),
				const JVector2<float>& uv1 = JVector2<float>(1, 1),
				float framePadding = -1,
				const JVector4<float>& bgCol = JVector4<float>(0, 0, 0, 0),
				const JVector4<float>& tintCol = JVector4<float>(1, 1, 1, 1)); 
			static void AddImage(Graphic::JGraphicResourceHandleInterface& handle,
				const JVector2<float>& pMin,
				const JVector2<float>& pMax,
				bool isBack = true,
				ImU32 color = IM_COL32_WHITE,
				const JVector2<float>& uvMin = JVector2<float>(0, 0),
				const JVector2<float>& uvMax = JVector2<float>(1, 1));
		public:
			//Custom Widget 
			static bool ImageSelectable(const std::string name,
				Graphic::JGraphicResourceHandleInterface& handle,
				bool& pressed,
				bool changeValueIfPreesd,
				const JVector2<float>& size);
			static bool Switch(const std::string& name,
				bool& pSelected, 
				bool changeValueIfPreesd,
				const JVector2<float>& sizeArg = { 0,0 });
			static bool ImageSwitch(const std::string name,
				Graphic::JGraphicResourceHandleInterface& handle,
				bool& pressed,
				bool changeValueIfPreesd,
				const JVector2<float>& size,
				const ImU32 bgColor,
				const ImU32 bgDelta,
				const ImU32 frameColor = JImGuiImpl::GetUColor(ImGuiCol_FrameBg),
				const float frameThickness = 0.0f);
		public:
			//use wolrd cursor pos
			static void DrawRectFilledMultiColor(const JVector2<float>& pos, const JVector2<float>& size, const ImU32 color, const ImU32 colorDelta, const bool useRestoreCursorPos)noexcept;
			static void DrawRectFilledColor(const JVector2<float>& pos, const JVector2<float>& size, const ImU32 color, const bool useRestoreCursorPos)noexcept;
			static void DrawRectFrame(const JVector2<float>& pos, 
				const JVector2<float>& size, 
				const float thickness, 
				const ImU32 color, 
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
			//Widget End
		public:
			static bool IsLeftMouseClicked()noexcept;
			static bool IsRightMouseClicked()noexcept;
			static bool IsMiddleMouseClicked()noexcept;
			//use world pos
			static bool IsMouseInRect(const JVector2<float>& position, const JVector2<float>& size)noexcept;
			static bool IsMouseInLine(JVector2<float> st, JVector2<float> ed, const float thickness)noexcept;
			//0 = left, 1 = right, 2 = middle
			static void SetMouseClick(const ImGuiMouseButton btn, const bool value)noexcept;
		public:
			//util 
			static float GetSliderPosX(bool hasScrollbar = false)noexcept;
			static float GetSliderWidth()noexcept;
			static void SetTooltip(const std::string& message)noexcept;
		public:
			//Option
			static float GetFrameRounding()noexcept;
			static void SetFrameRounding(float value)noexcept;
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
			// 
			static void ComboSet(const std::string& uniqueLabel, int& selectedIndex, const std::vector<std::string>& strVec)
			{
				if (JImGuiImpl::BeginCombo(uniqueLabel, strVec[selectedIndex].c_str(), ImGuiComboFlags_HeightLarge))
				{
					const uint count = (uint)strVec.size();
					for (uint i = 0; i < count; i++)
					{
						bool isSelected = (selectedIndex == i);
						if (JImGuiImpl::Selectable(strVec[i], &isSelected))
							selectedIndex = i;

						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			template<typename Object, std::enable_if_t<std::is_base_of_v<Core::JIdentifier, Object>, int> = 0>
			static void ComboSet(const std::string& uniqueLabel, int& selectedIndex, const std::vector<Object*>& objVec)
			{
				if (JImGuiImpl::BeginCombo(uniqueLabel, JCUtil::WstrToU8Str(objVec[selectedIndex]->GetName()).c_str(), ImGuiComboFlags_HeightLarge))
				{
					const uint count = (uint)objVec.size();
					for (uint i = 0; i < count; i++)
					{
						bool isSelected = (selectedIndex == i);
						if (JImGuiImpl::Selectable(JCUtil::WstrToU8Str(objVec[i]->GetName()), &isSelected))
							selectedIndex = i;
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			template<typename EnumType>
			static void ComboEnumSet(const std::string& uniqueLabel, int& selectedIndex)
			{
				Core::JEnumInfo* enumInfo = Core::JReflectionInfo::Instance().GetEnumInfo(typeid(EnumType).name());
				if (JImGuiImpl::BeginCombo(uniqueLabel, enumInfo->ElementName(enumInfo->EnumValue(selectedIndex)).c_str(), ImGuiComboFlags_HeightLarge))
				{
					const uint enumCount = enumInfo->GetEnumCount();
					for (uint i = 0; i < enumCount; i++)
					{
						bool isSelected = (selectedIndex == i);
						if (JImGuiImpl::Selectable(enumInfo->ElementName(enumInfo->EnumValue(i)), &isSelected))
							selectedIndex = i;
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			//Widget Set
			//Support Redo undo transition
			template<typename ...Param>
			static bool CheckBoxSetT(const std::string& uniqueLabel, const bool preValue, Core::JFunctor<void, const bool, Param...>& commitFunctor, Param... var)
			{
				bool nowValue = preValue;
				if (JImGuiImpl::CheckBox("##CheckBox" + uniqueLabel, nowValue))
				{
					using Functor = Core::JFunctor<void, const bool, Param...>;
					using Binder = Core::JBindHandle<Functor, const bool, Param...>;

					bool dovalue = nowValue;
					bool undovalue = preValue;

					ExecuteWidgetSet<Binder>(uniqueLabel + " checkbox set value: " + std::to_string(nowValue),
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
			static bool InputIntSetT(const std::string& uniqueLabel, const int preValue, Core::JFunctor<void, const int, Param...>& commitFunctor, Param... var)
			{
				int nowValue = preValue;
				if (JImGuiImpl::InputInt("##InputInt" + uniqueLabel, &nowValue))
				{
					if (nowValue != preValue)
					{
						using Functor = Core::JFunctor<void, const int, Param...>;
						using Binder = Core::JBindHandle<Functor, const int, Param...>;

						int dovalue = nowValue;
						int undovalue = preValue;

						ExecuteWidgetSet<Binder>(uniqueLabel + " input int set value: " + std::to_string(nowValue),
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
			static bool InputFloatSetT(const std::string& uniqueLabel, const float preValue, Core::JFunctor<void, const float, Param...>& commitFunctor, Param... var)
			{
				float nowValue = preValue;
				if (JImGuiImpl::InputFloat("##IntputFloat" + uniqueLabel, &nowValue))
				{
					if (nowValue != preValue)
					{
						using Functor = Core::JFunctor<void, const float, Param...>;
						using Binder = Core::JBindHandle<Functor, const float, Param...>;

						float dovalue = nowValue;
						float undovalue = preValue;

						ExecuteWidgetSet<Binder>(uniqueLabel + " input float set value: " + std::to_string(nowValue),
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
			static bool InputTextSetT(const std::string& uniqueLabel,
				JEditorInputBuffHelper* helper,
				const std::string& hint,
				ImGuiInputTextFlags flags,
				Core::JFunctor<void, const std::string, Param...>& commitFunctor, Param... var)
			{
				std::string preValue = helper->buff;
				if (JImGuiImpl::InputText("##InputText" + uniqueLabel, helper->buff, helper->result, hint, flags))
				{
					std::string dovalue = helper->result;
					std::string undovalue = JCUtil::EraseSideChar(preValue, '\0');

					using Functor = Core::JFunctor<void, const std::string, Param...>;
					using Binder = Core::JBindHandle<Functor, const std::string, Param...>;

					ExecuteWidgetSet<Binder>(uniqueLabel + " input text set value: " + dovalue,
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
			static bool InputMultilineTextSetT(const std::string& uniqueLabel,
				JEditorInputBuffHelper* helper,
				const JVector2<float>& size,
				ImGuiInputTextFlags flags,
				Core::JFunctor<void, const std::string, Param...>& commitFunctor, Param... var)
			{
				ImGuiInputTextState* state = ImGui::GetInputTextState(ImGui::GetCurrentWindow()->GetID(("##InputMultilineText" + uniqueLabel).c_str()));
				if (state != nullptr)
				{
					int nowBuffEnd = helper->buff.find_first_of('\0');
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
					state->CurLenW = ImTextStrFromUtf8(state->TextW.Data, helper->buff.size(), &helper->buff[0], NULL, &buf_end);
					state->TextAIsValid = true;
					state->TextA.resize(state->TextW.Size * 4 + 1);
					state->CurLenA = ImTextStrToUtf8(state->TextA.Data, state->TextA.Size, state->TextW.Data, NULL);
				}

				std::string preValue = helper->buff;
				if (JImGuiImpl::InputMultilineText("##InputMultilineText" + uniqueLabel, helper->buff, helper->result, size, flags))
				{
					std::string dovalue = helper->result;
					std::string undovalue = JCUtil::EraseSideChar(preValue, '\0');

					using Functor = Core::JFunctor<void, const std::string, Param...>;
					using Binder = Core::JBindHandle<Functor, const std::string, Param...>;

					ExecuteWidgetSet<Binder>(uniqueLabel + " input multiline text set value: " + dovalue,
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
			static void ComoboEnumSetT(const std::string& uniqueLabel, const EnumType preValue, Core::JFunctor<void, const EnumType, Param...>& commitFunctor, Param... var)
			{
				const std::string enumName = Core::GetName<EnumType>();
				const std::string preValueName = Core::GetName(preValue);
				if (JImGuiImpl::BeginCombo("##EnumCombo" + enumName + uniqueLabel, preValueName.c_str()))
				{
					for (uint i = 0; i < (int)EnumType::COUNT; ++i)
					{
						std::string valueStr = Core::GetName((EnumType)i) + "##" + enumName + uniqueLabel;
						if (JImGuiImpl::Selectable(valueStr))
						{
							using Functor = Core::JFunctor<void, const EnumType, Param...>;
							using Binder = Core::JBindHandle<Functor, const EnumType, Param...>;

							EnumType dovalue = (EnumType)i;
							EnumType undovalue = preValue;

							ExecuteWidgetSet<Binder>(enumName + "::" + valueStr,
								commitFunctor,
								dovalue,
								undovalue,
								std::make_tuple(var...),
								std::make_tuple(var...),
								std::make_index_sequence<sizeof...(Param)>());
						}
					}
					JImGuiImpl::EndCombo();
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
				Functor& functor,
				Value doValue,
				Value undoValue,
				ParamTuple t1,
				ParamTuple t2,
				std::index_sequence<Is...>)
			{
				Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>(taskName,
					std::make_unique<Binder>(functor, std::move(doValue), DecomposeTuple<Is>(t1)...),
					std::make_unique<Binder>(functor, std::move(undoValue), DecomposeTuple<Is>(t2)...)));
			}
		};
	}
}
