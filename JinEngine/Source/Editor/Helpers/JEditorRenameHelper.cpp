#include"JEditorRenameHelper.h"
#include"JEditorInputBuffHelper.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../Align/JEditorAlignCalculator.h"
#include"../../Core/Identity/JIdentifier.h"

namespace JinEngine
{ 
	namespace Editor
	{
		JEditorRenameHelper::JEditorRenameHelper()
		{
			renameBuff = std::make_unique<JEditorInputBuffHelper>(JImGuiImpl::GetTextBuffRange());
			auto renameLam = [](const std::string newName, JUserPtr<Core::JIdentifier> obj)
			{
				obj->SetName(JCUtil::U8StrToWstr(JCUtil::EraseChar(newName, '\n')));
			};

			renameF = std::make_unique<RenameF>(renameLam); 
			uniqueLabel = std::to_string(Core::MakeGuid());
		}
		void JEditorRenameHelper::Clear()noexcept
		{
			renameTar.Clear();
			renameBuff->Clear();
		}
		void JEditorRenameHelper::Update(const bool doIdent)
		{	 
			DoUpdate(doIdent);
		}
		void JEditorRenameHelper::UpdateMultiline(const JVector2<float>& size, const bool doIdent)
		{
			DoUpdate(doIdent, size, true);
		}
		void JEditorRenameHelper::DoUpdate(const bool doIdent, const JVector2<float>& size, const bool isMultiline)
		{
			if (!IsActivated())
				return;

			const JVector2<float> itemPos = ImGui::GetCursorPos() + ImGui::GetWindowPos();
			if (isMultiline)
			{
				JEditorTextAlignCalculator textAlignCal;
				textAlignCal.Update(JCUtil::EraseSideChar(renameBuff->buff, '\0'), size, false);
				const std::string aligned = textAlignCal.LeftAligned();

				JVector2<float> alphabetSize = JImGuiImpl::GetAlphabetSize();
				JVector2<float> multilineSize = ImGui::CalcTextSize(aligned.c_str());			 
				multilineSize.x = size.x;
				multilineSize.y += alphabetSize.y * 2;
				 
				renameBuff->SetBuff(aligned);
				ImGuiInputTextFlags flag = ImGuiInputTextFlags_NoHorizontalScroll;

				JImGuiImpl::InputMultilineTextSetT(JCUtil::WstrToU8Str(renameTar->GetName()),
					uniqueLabel, 
					renameBuff.get(),
					multilineSize,
					flag,
					*renameF, Core::JUserPtr{ renameTar });

				if (ImGui::IsKeyDown(ImGuiKey_Enter))
					Clear(); 
			}
			else
			{ 
				if (JImGuiImpl::InputTextSetT(JCUtil::WstrToU8Str(renameTar->GetName()), 
					uniqueLabel, 
					renameBuff.get(),
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
		void JEditorRenameHelper::Activate(JUserPtr<Core::JIdentifier> newRenameTar)noexcept
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