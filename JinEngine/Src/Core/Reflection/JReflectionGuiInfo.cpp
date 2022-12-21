#include"JReflectionGuiInfo.h"
#include"JReflectionInfo.h"
#include"JEnumInfo.h"
#include"../../Editor/GuiLibEx/JGuiWidgetType.h"
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{
		JGuiGroupMemberInfo::JGuiGroupMemberInfo(const JGuiGroupKey groupKey)
			:groupKey(groupKey)
		{}
		JGuiGroupKey JGuiGroupMemberInfo::GetGroupKey()const noexcept
		{
			return groupKey;
		}
		JGuiEnumTriggerGroupMemberInfoHandle::JGuiEnumTriggerGroupMemberInfoHandle(const JGuiGroupKey groupKey)
			:JGuiGroupMemberInfo(groupKey)
		{}

		JGuiWidgetInfo::JGuiWidgetInfo(std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo)
		{
			JGuiGroupInfo* groupInfo = JGuiGroupMap::GetGuiGroupInfo(newGroupMemberInfo->GetGroupKey());
			if (groupInfo != nullptr)
			{
				groupMemberInfo = std::move(newGroupMemberInfo);
				groupInfo->NotifyAddNewMember(groupMemberInfo.get());
			}
		}
		bool JGuiWidgetInfo::IsMemberWidget()const noexcept
		{
			return groupMemberInfo != nullptr;
		}
		JGuiGroupKey JGuiWidgetInfo::GetGroupKey()const noexcept
		{
			return IsMemberWidget() ? groupMemberInfo->GetGroupKey() : Constants::InvalidGroupKey;
		}
		JGuiGroupMemberInfo* JGuiWidgetInfo::GetGroupMemberInfo()const noexcept
		{
			return groupMemberInfo.get();
		}

		JGuiInputInfo::JGuiInputInfo(const bool isEnterToReturn)
			:JGuiWidgetInfo(), isEnterToReturn(isEnterToReturn)
		{}
		JGuiInputInfo::JGuiInputInfo(const bool isEnterToReturn, std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo)
			: JGuiWidgetInfo(std::move(newGroupMemberInfo)), isEnterToReturn(isEnterToReturn)
		{}
		JSupportGuiWidgetType JGuiInputInfo::GetSupportWidgetType()const noexcept
		{
			return Editor::J_GUI_WIDGET_INPUT;
		}
		bool JGuiInputInfo::IsEnterToReturn()const noexcept
		{
			return isEnterToReturn;
		}

		JGuiCheckBoxInfo::JGuiCheckBoxInfo(std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo)
			:JGuiWidgetInfo(std::move(newGroupMemberInfo))
		{}
		JSupportGuiWidgetType JGuiCheckBoxInfo::GetSupportWidgetType()const noexcept
		{
			return Editor::J_GUI_WIDGET_CHECKBOX;
		}

		JGuiSliderInfo::JGuiSliderInfo(float minValue, float maxValue, bool isSupportInput, bool isVertical)
			:minValue(minValue),
			maxValue(maxValue),
			isSupportInput(isSupportInput),
			isVertical(isVertical)
		{}
		JGuiSliderInfo::JGuiSliderInfo(float minValue, float maxValue, bool isSupportInput, bool isVertical, std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo)
			:JGuiWidgetInfo(std::move(newGroupMemberInfo)),
			minValue(minValue),
			maxValue(maxValue),
			isSupportInput(isSupportInput),
			isVertical(isVertical)
		{}
		JSupportGuiWidgetType JGuiSliderInfo::GetSupportWidgetType()const noexcept
		{
			return Editor::J_GUI_WIDGET_SLIDER;
		}
		int JGuiSliderInfo::GetMinValue()const noexcept
		{
			return minValue;
		}
		int JGuiSliderInfo::GetMaxValue()const noexcept
		{
			return maxValue;
		}
		bool JGuiSliderInfo::IsSupportInput()const noexcept
		{
			return isSupportInput;
		}
		bool JGuiSliderInfo::IsVertical()const noexcept
		{
			return isVertical;
		}

		JGuiColorPickerInfo::JGuiColorPickerInfo(bool hasRgbInput)
			:hasRgbInput(hasRgbInput)
		{}
		JGuiColorPickerInfo::JGuiColorPickerInfo(bool hasRgbInput, std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo)
			: JGuiWidgetInfo(std::move(newGroupMemberInfo)), hasRgbInput(hasRgbInput)
		{}
		JSupportGuiWidgetType JGuiColorPickerInfo::GetSupportWidgetType()const noexcept
		{
			return Editor::J_GUI_WIDGET_COLOR_PICKER;
		}
		bool JGuiColorPickerInfo::HasRgbInput()const noexcept
		{
			return hasRgbInput;
		}

		JGuiSelectorInfo::JGuiSelectorInfo(const bool isImageRtTexture, const bool hasSizeSlider)
			:isImageRtTexture(isImageRtTexture), hasSizeSlider(hasSizeSlider)
		{}
		JGuiSelectorInfo::JGuiSelectorInfo(const bool isImageRtTexture, const bool hasSizeSlider, std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo)
			: JGuiWidgetInfo(std::move(newGroupMemberInfo)), isImageRtTexture(isImageRtTexture), hasSizeSlider(hasSizeSlider)
		{}
		JSupportGuiWidgetType JGuiSelectorInfo::GetSupportWidgetType()const noexcept
		{
			return Editor::J_GUI_WIDGET_SELECTOR;
		}
		bool JGuiSelectorInfo::IsImageRtTexture()const noexcept
		{
			return isImageRtTexture;
		}
		bool JGuiSelectorInfo::HasSizeSlider()const noexcept
		{
			return hasSizeSlider;
		}

		JGuiReadOnlyTextInfo::JGuiReadOnlyTextInfo(std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo)
			:JGuiWidgetInfo(std::move(newGroupMemberInfo))
		{}
		JSupportGuiWidgetType JGuiReadOnlyTextInfo::GetSupportWidgetType()const noexcept
		{
			return Editor::J_GUI_WIDGET_READONLY_TEXT;
		}

		JGuiEnumComboBoxInfo::JGuiEnumComboBoxInfo(const std::string enumName)
			:enumFullName(FindEnumFullName(enumName))
		{

		}
		JGuiEnumComboBoxInfo::JGuiEnumComboBoxInfo(const std::string enumName, std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo)
			:JGuiWidgetInfo(std::move(newGroupMemberInfo)), enumFullName(FindEnumFullName(enumName))
		{

		}
		JSupportGuiWidgetType JGuiEnumComboBoxInfo::GetSupportWidgetType()const noexcept
		{
			return Editor::J_GUI_WIDGET_ENUM_COMBO;
		}
		std::string JGuiEnumComboBoxInfo::GetEnumFullName()const noexcept
		{ 
			return enumFullName;
		}
		std::string JGuiEnumComboBoxInfo::FindEnumFullName(const std::string enumName)const noexcept
		{
			return JReflectionInfo::Instance().FindEnumInfo(enumName)->FullName();
		}

		JGuiGroupInfo::JGuiGroupInfo(const std::string groupName)
			:groupName(groupName)
		{}
		std::string JGuiGroupInfo::GetGroupName()const noexcept
		{
			return groupName;
		}	
		bool JGuiTableInfo::IsFirstColunmGuide()const noexcept
		{
			return isFirstColumnGuide;
		}
		J_GUI_GROUP_TYPE JGuiTableInfo::GetGuiGroupType()const noexcept
		{
			return J_GUI_GROUP_TYPE::TABLE;
		}
		std::string JGuiTableInfo::GetColumnGuide(const uint index)const noexcept
		{
			return columnGuide[index];
		}
		uint JGuiTableInfo::GetColumnCount()const noexcept
		{
			return columnCount;
		}
		uint JGuiTableInfo::GetRowCount()const noexcept
		{
			return rowCount;
		}
		void JGuiTableInfo::NotifyAddNewMember(JGuiGroupMemberInfo* mInfo)
		{
			if (mInfo != nullptr && mInfo->GetGroupKey() == GetGroupName())
				++rowCount;
		} 

		JGuiEnumTriggerInfo::JGuiEnumTriggerInfo(const std::string& groupName, const std::string& enumName, const std::string& parameterName)
			:JGuiGroupInfo(groupName), enumName(enumName), parameterName(parameterName)
		{}
		J_GUI_GROUP_TYPE JGuiEnumTriggerInfo::GetGuiGroupType()const noexcept
		{
			return J_GUI_GROUP_TYPE::ENUM_TRIGGER;
		}
		std::string JGuiEnumTriggerInfo::GetEnumName()const noexcept
		{
			return enumName;
		}
		std::string JGuiEnumTriggerInfo::GetParamName()const noexcept
		{
			return parameterName;
		}
		void JGuiEnumTriggerInfo::NotifyAddNewMember(JGuiGroupMemberInfo* mInfo){}

		namespace
		{
			static std::unordered_map<std::string, std::unique_ptr<JGuiGroupInfo>> groupInfoMap;
		}

		void JGuiGroupMap::AddGuiGroup(std::unique_ptr<JGuiGroupInfo>&& rInfo)
		{
			groupInfoMap.emplace(rInfo->GetGroupName(), std::move(rInfo));
		}
		JGuiGroupInfo* JGuiGroupMap::GetGuiGroupInfo(const JGuiGroupKey& groupKey)
		{
			auto data = groupInfoMap.find(groupKey);
			return data != groupInfoMap.end() ? data->second.get() : nullptr;
		}
	}
}