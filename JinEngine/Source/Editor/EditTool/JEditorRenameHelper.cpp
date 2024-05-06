/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JEditorRenameHelper.h"
#include"JEditorInputBuffHelper.h"
#include"../Gui/JGui.h"
#include"../Align/JEditorAlignCalculator.h"
#include"../../Core/Identity/JIdentifier.h"

namespace JinEngine
{ 
	namespace Editor
	{
		JEditorRenameHelper::JEditorRenameHelper()
		{
			renameBuff = std::make_unique<JEditorInputBuffHelper>(JGui::GetTextBuffRange());
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

			const JVector2<float> itemPos = JGui::GetCursorPos() + JGui::GetWindowPos();
			if (isMultiline)
			{
				JEditorTextAlignCalculator textAlignCal;
				textAlignCal.Update(JCUtil::EraseSideChar(renameBuff->buff, '\0'), size, false);
				const std::string aligned = textAlignCal.LeftAligned();

				JVector2<float> alphabetSize = JGui::GetAlphabetSize();
				JVector2<float> multilineSize = JGui::CalTextSize(aligned);
				multilineSize.x = size.x;
				multilineSize.y += alphabetSize.y;
				 
				renameBuff->SetBuff(aligned);
				J_GUI_INPUT_TEXT_FLAG_ flag = J_GUI_INPUT_TEXT_FLAG_NO_HORIZONTAL_SCROLL;
				if (enterisReturn)
					flag |= J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE;
				 
				const bool result = JGui::InputMultilineTextSetT(JGui::CreateGuiLabel(renameTar, "InputMultilineTextSetT"),
					uniqueLabel, 
					renameBuff.get(),
					multilineSize,
					flag,
					*renameF, Core::JUserPtr{ renameTar });

				if (result)
					Clear(); 
			}
			else
			{
				J_GUI_INPUT_TEXT_FLAG_ flag = J_GUI_INPUT_TEXT_FLAG_NO_HORIZONTAL_SCROLL;
				if (enterisReturn)
					flag |= J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE;

				if (JGui::InputTextSetT(JGui::CreateGuiLabel(renameTar, "InputTextSetT"),
					uniqueLabel, 
					renameBuff.get(),
					"New name...",
					flag,
					*renameF, Core::JUserPtr{ renameTar }))
				{
					Clear();
				}
			}

			//const JVector2<float> itemSize = JGui::GetLastItemRectSize();
			if (JGui::IsMouseClicked(Core::J_MOUSE_BUTTON::LEFT) || JGui::IsMouseClicked(Core::J_MOUSE_BUTTON::RIGHT))
			{ 
				if (!JGui::IsMouseInLastItem())
					Clear();
			} 

			if (doIdent)
				JGui::Indent();
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
		void JEditorRenameHelper::SetEnterReturnFlag(const bool value)noexcept
		{
			enterisReturn = value;
		}
	}
}