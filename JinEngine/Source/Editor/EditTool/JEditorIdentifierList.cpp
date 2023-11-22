#include"JEditorIdentifierList.h" 
#include"../Gui/JGui.h"
#include"../../Core/Identity/JIdentifier.h"

namespace JinEngine::Editor
{
	JEditorIdentifierList::DisplayDataSet::DisplayDataSet(const std::string& unqLabel)
		:unqLabel(unqLabel)
	{} 

	void JEditorIdentifierList::Display(const DisplayDataSet& dataSet)noexcept
	{
		auto typeInfo = _JReflectionInfo::Instance().GetTypeInfo(typeGuid);
		if (typeInfo == nullptr || !typeInfo->IsChildOf<Core::JIdentifier>())
			return;

		if (dataSet.callWindowAutomatically)
		{
			J_GUI_WINDOW_FLAG_ flag = J_GUI_WINDOW_FLAG_NO_TITLE_BAR | J_GUI_WINDOW_FLAG_NO_SAVE | J_GUI_WINDOW_FLAG_NO_DOCKING;
			flag = Core::AddSQValue(flag, dataSet.customWindowFlag);
			if (dataSet.winodwSize != JVector2F::Zero())
				JGui::SetNextWindowSize(dataSet.winodwSize);
			JGui::SetCursorPosX(0);
			JGui::BeginWindow(("##JEditorIdentifierList" + dataSet.unqLabel), 0, flag);
		}
		auto rawVec = typeInfo->GetInstanceRawPtrVec();
		auto preSelected = typeInfo->GetInstanceRawPtr<Core::JIdentifier>(preSelectedGuid);
		if (preSelected == nullptr)
		{
			preSelectedGuid = rawVec.size() > 0 ? static_cast<Core::JIdentifier*>(rawVec[0])->GetGuid() : 0;
			preSelected = typeInfo->GetInstanceRawPtr<Core::JIdentifier>(preSelectedGuid);
		}
		if (dataSet.canDisplayContents)
		{
			if (dataSet.listWidth != 0)
				JGui::SetNextItemWidth(dataSet.listWidth);

			J_GUI_COMBO_FLAG_ flag = dataSet.customComboFlag;
			if (flag == J_GUI_COMBO_FLAG_NONE)
				flag = J_GUI_COMBO_FLAG_HEIGHT_LARGE;

			const std::string comboLabel = "##IdenList" + dataSet.unqLabel;
			if (JGui::BeginCombo(comboLabel, JGui::CreatePreviewName(preSelected), flag))
			{
				for (const auto& data : rawVec)
				{
					auto iden = static_cast<Core::JIdentifier*>(data);
					const bool passCond = dataSet.canDisplayB != nullptr ? (*dataSet.canDisplayB)(std::move(iden)) : true;
					if (!passCond)
						continue;

					bool isSelected = iden->GetGuid() == preSelectedGuid;
					if (JGui::Selectable(JGui::CreateGuiLabel(iden, dataSet.unqLabel), isSelected))
					{
						if (dataSet.selectF != nullptr)
							(*dataSet.selectF)(iden);
						if (dataSet.selectB != nullptr)
							dataSet.selectB->InvokeCompletelyBind();
						preSelectedGuid = iden->GetGuid();
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (isSelected)
						JGui::SetLastItemDefaultFocus();
				}
				JGui::EndCombo();
			}
		}
		if(dataSet.callWindowAutomatically)
			JGui::EndWindow();
	}
	size_t JEditorIdentifierList::GetSelectedGuid()const noexcept
	{
		return preSelectedGuid;
	}
	void JEditorIdentifierList::SetTypeGuid(const size_t guid)
	{
		typeGuid = guid;
	}
	void JEditorIdentifierList::SetSelecetdGuid(const size_t guid)noexcept
	{
		preSelectedGuid = guid;
	} 
}