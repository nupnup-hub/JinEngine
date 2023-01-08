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
			activateF = std::make_unique<ActivateF>(&JEditorRenameHelper::Activate, this);
		}
		void JEditorRenameHelper::Clear()noexcept
		{
			renameTar.Clear();
			renameBuff->Clear();
		}
		void JEditorRenameHelper::Update(const std::string& uniqueLabel, const bool doIdent)
		{	 
			if (!IsActivated())
				return;

			const ImVec2 itemPos = ImGui::GetCursorPos() + ImGui::GetWindowPos();
			if (JImGuiImpl::InputTextSet(uniqueLabel, renameBuff.get(),
				ImGuiInputTextFlags_EnterReturnsTrue,
				*renameF, Core::JUserPtr{ renameTar }))
			{
				Clear();
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
		JEditorRenameHelper::ActivateF* JEditorRenameHelper::GetActivateFunctor()const noexcept
		{
			return activateF.get();
		}
		bool JEditorRenameHelper::IsActivated()const noexcept
		{
			return renameTar.IsValid();
		}
		bool JEditorRenameHelper::IsRenameTar(const size_t guid)const noexcept
		{
			return renameTar.IsValid() ? renameTar->GetGuid() == guid : false;
		}
	}
}