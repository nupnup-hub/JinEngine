#include"JEditorSearchBarHelper.h"
#include"../Gui/JGui.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Object/JObject.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorSearchBarHelper::JEditorSearchBarHelper(const bool caseSensitive)
			:guid(Core::MakeGuid()), caseSensitive(caseSensitive)
		{
			inputHelper = std::make_unique<JEditorInputBuffHelper>(JGui::GetTextBuffRange());
		}
		void JEditorSearchBarHelper::ClearInputBuffer()noexcept
		{
			inputHelper->Clear(); 
		}
		void JEditorSearchBarHelper::UpdateSearchBar(const bool isReadonly)
		{ 
			J_GUI_INPUT_TEXT_FLAG_ flag= J_GUI_INPUT_TEXT_FLAG_NONE;
			if (isReadonly)
				flag = J_GUI_INPUT_TEXT_FLAG_READ_ONLY;
			 
			const uint preCount = (uint)inputHelper->result.size();
			const JVector2<float> preFramePadding = JGui::GetFramePadding();
			JGui::SetFramePadding({ JGui::GetFramePadding().x , JGui::GetFramePadding().y * 0.8f });
			 
			const JVector2<float> prePos = JGui::GetCursorScreenPos();
			JGui::InputText("##" +std::to_string(guid), inputHelper->buff, inputHelper->result, "Search...", flag);
			if (inputHelper->result.size() != preCount)
				isUpdateInputData = true;
			else
				isUpdateInputData = false;
			 
			const JVector2<float> inputSize = JGui::GetLastItemRectSize();
			JGui::SetFramePadding(preFramePadding); 
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