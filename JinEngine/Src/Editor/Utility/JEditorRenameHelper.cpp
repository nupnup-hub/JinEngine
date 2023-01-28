#include"JEditorRenameHelper.h"
#include"JEditorInputBuffHelper.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../Object/JObject.h"

namespace JinEngine
{ 
	namespace Editor
	{
		JEditorRenameHelper::JEditorRenameHelper()
		{
			renameBuff = std::make_unique<JEditorInputBuffHelper>(JImGuiImpl::GetTextBuffRange());
			auto renameLam = [](const std::string newName, Core::JUserPtr<JObject> obj) {obj->SetName(JCUtil::U8StrToWstr(newName)); };
			renameF = std::make_unique<RenameF>(renameLam); 
		}
		void JEditorRenameHelper::Clear()noexcept
		{
			renameTar.Clear();
			renameBuff->Clear();
		}
		void JEditorRenameHelper::Update(const std::string& uniqueLabel, const bool doIdent)
		{	 
			DoUpdate(uniqueLabel, doIdent);
		}
		void JEditorRenameHelper::Update(const std::string& uniqueLabel, const JVector2<float>& size, const bool doIdent)
		{
			DoUpdate(uniqueLabel, doIdent, size, true);
		}
		void JEditorRenameHelper::DoUpdate(const std::string& uniqueLabel, const bool doIdent, const JVector2<float>& size, const bool isMultiline)
		{
			if (!IsActivated())
				return;

			const JVector2<float> itemPos = ImGui::GetCursorPos() + ImGui::GetWindowPos();
			if (isMultiline)
			{ 
				if (JImGuiImpl::InputMultiLineTextSetT(uniqueLabel, renameBuff.get(),
					size,
					ImGuiInputTextFlags_EnterReturnsTrue,
					*renameF, Core::JUserPtr{ renameTar }))
				{
					Clear();
				}
			}
			else
			{ 
				if (JImGuiImpl::InputTextSetT(uniqueLabel, renameBuff.get(),
					"New name...",
					ImGuiInputTextFlags_EnterReturnsTrue,
					*renameF, Core::JUserPtr{ renameTar }))
				{
					Clear();
				}
			}

			if (doIdent)
				ImGui::Indent();

			const ImVec2 itemSize = ImGui::GetItemRectSize();
			if (JImGuiImpl::IsRightMouseClicked() || JImGuiImpl::IsLeftMouseClicked())
			{
				if (!JImGuiImpl::IsMouseInRect(itemPos, itemSize))
					Clear();
			}
		}
		void JEditorRenameHelper::Activate(Core::JUserPtr<JObject> newRenameTar)noexcept
		{
			renameTar = newRenameTar;
			if(IsActivated())
				renameBuff->SetBuff(JCUtil::WstrToU8Str(renameTar->GetName()));
		}
		bool JEditorRenameHelper::IsActivated()const noexcept
		{
			return renameTar.IsValid();
		}
		bool JEditorRenameHelper::IsRenameTar(const size_t guid)const noexcept
		{
			return renameTar.IsValid() ? renameTar->GetGuid() == guid : false;
		}
		void JEditorRenameHelper::SetBuffSize(const uint newBuffSize)noexcept
		{ 
			renameBuff->SetCapacity(newBuffSize);
			if (IsActivated())
				renameBuff->SetBuff(JCUtil::WstrToU8Str(renameTar->GetName()));
		}
	}
}