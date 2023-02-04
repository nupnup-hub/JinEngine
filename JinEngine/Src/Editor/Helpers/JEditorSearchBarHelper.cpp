#include"JEditorSearchBarHelper.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../Object/JObject.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorSearchBarHelper::JEditorSearchBarHelper(const bool caseSensitive)
			:guid(Core::MakeGuid()), caseSensitive(caseSensitive)
		{
			inputHelper = std::make_unique<JEditorInputBuffHelper>(JImGuiImpl::GetTextBuffRange());
		}
		void JEditorSearchBarHelper::ClearInputBuffer()noexcept
		{
			inputHelper->Clear(); 
		}
		void JEditorSearchBarHelper::UpdateSearchBar(const bool isReadonly)
		{ 
			ImGuiInputTextFlags_ flag=  ImGuiInputTextFlags_None;
			if (isReadonly)
				flag = ImGuiInputTextFlags_ReadOnly;
			 
			const uint preCount = (uint)inputHelper->result.size();
			const JVector2<float> preFramePadding = ImGui::GetStyle().FramePadding;
			ImGui::GetStyle().FramePadding.y = 0.8f;

			const JVector2<float> prePos = ImGui::GetCursorScreenPos();
			JImGuiImpl::InputText("##" +std::to_string(guid), inputHelper->buff, inputHelper->result, "Search...", flag);
			if (inputHelper->result.size() != preCount)
				isUpdateInputData = true;
			else
				isUpdateInputData = false;
			 
			const JVector2<float> inputSize = ImGui::GetItemRectSize();
			JImGuiImpl::DrawRectFrame(prePos, inputSize, 2.0f, JImGuiImpl::GetUColor(ImGuiCol_FrameBg), false);
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