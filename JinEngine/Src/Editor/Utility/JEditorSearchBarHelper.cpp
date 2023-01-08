#include"JEditorSearchBarHelper.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../Object/JObject.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorSearchBarHelper::JEditorSearchBarHelper(const bool caseSensitive)
			:caseSensitive(caseSensitive)
		{
			inputHelper = std::make_unique<JEditorInputBuffHelper>(JImGuiImpl::GetTextBuffRange());
		}
		void JEditorSearchBarHelper::ClearInputBuffer()noexcept
		{
			inputHelper->Clear(); 
		}
		void JEditorSearchBarHelper::UpdateSearchBar(const std::string& uniqueLabel, const bool isReadonly)
		{ 
			ImGuiInputTextFlags_ flag=  ImGuiInputTextFlags_None;
			if (isReadonly)
				flag = ImGuiInputTextFlags_ReadOnly;
			 
			uint preCount = (uint)inputHelper->result.size();

			ImVec2 preFramePadding = ImGui::GetStyle().FramePadding;
			ImGui::GetStyle().FramePadding.y = 0.8f;

			JImGuiImpl::InputText("##SearchBarHelper" + uniqueLabel, inputHelper->buff, inputHelper->result, flag);
			if (inputHelper->result.size() != preCount)
				isUpdateInputData = true;
			else
				isUpdateInputData = false;
			ImGui::GetStyle().FramePadding = preFramePadding;
		}
		std::string JEditorSearchBarHelper::GetInputData()const noexcept
		{
			return inputHelper->result;
		}
		bool JEditorSearchBarHelper::CanSrcNameOnScreen(const std::string& srcName)const noexcept
		{
			if (HasInputData())
				return JCUtil::Contain(srcName, inputHelper->result, caseSensitive);
			else
				return true;
		}
		bool JEditorSearchBarHelper::CanSrcNameOnScreen(const std::wstring& srcName)const noexcept
		{
			if (HasInputData())
				return JCUtil::Contain(JCUtil::WstrToU8Str(srcName), inputHelper->result, caseSensitive);
			else
				return true;
		}
		bool JEditorSearchBarHelper::HasInputData()const noexcept
		{
			return inputHelper->result.size() > 0;
		}
		bool JEditorSearchBarHelper::IsUpdateInputData()const noexcept
		{
			return isUpdateInputData;
		}
	}
}