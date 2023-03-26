#include"JGuiWidgetInfo.h"
#include"JReflectionInfo.h"
#include"JEnumInfo.h"
#include"JGuiWidgetInfoHandleBase.h"
#include"../../Editor/GuiLibEx/JGuiWidgetType.h"  

namespace JinEngine
{
	namespace Core
	{
#pragma region Group
		JGuiExtraFunctionUserInfo::JGuiExtraFunctionUserInfo(const JGuiExtraFunctionInfoMapName infoMapName)
			:infoMapName(infoMapName)
		{}
		JGuiExtraFunctionInfoMapName JGuiExtraFunctionUserInfo::GetRefInfoMapName()const noexcept
		{
			return infoMapName;
		}
		J_GUI_EXTRA_FUNCTION_TYPE JGuiExtraFunctionUserInfo::GetRefInfoMapType()const noexcept
		{
			return JGuiExtraFunctionInfoMap::GetExtraFunctionInfo(infoMapName)->GetExtraFunctionType();
		}

		JGuiTableUserInfo::JGuiTableUserInfo(const JGuiExtraFunctionInfoMapName infoMapName,
			const uint useColumnCount,
			const bool canAddRowCount)
			:JGuiExtraFunctionUserInfo(infoMapName), useColumnCount(useColumnCount)
		{
			if (canAddRowCount)
			{
				JGuiExtraFunctionInfo* info = JGuiExtraFunctionInfoMap::GetExtraFunctionInfo(infoMapName);
				static_cast<JGuiGroupInfo*>(info)->NotifyAddNewMember(this);
			}
		}
		uint JGuiTableUserInfo::GetUseColumnCount()const noexcept
		{
			return useColumnCount;
		}

		JGuiEnumConditionUserInfoBase::JGuiEnumConditionUserInfoBase(const JGuiExtraFunctionInfoMapName infoMapName)
			:JGuiExtraFunctionUserInfo(infoMapName)
		{}
		JGuiEnumConditionUserInfoBase::JGuiEnumConditionUserInfoBase(const JGuiExtraFunctionInfoMapName infoMapName, const std::string ownerTypeParameterName)
			: JGuiExtraFunctionUserInfo(infoMapName), ownerTypeParameterName(ownerTypeParameterName), isRefUser(true)
		{}
		std::string JGuiEnumConditionUserInfoBase::GetOwnerTypeParameterNameInRefClass()const noexcept
		{
			return ownerTypeParameterName;
		}
		bool JGuiEnumConditionUserInfoBase::IsRefUser()const noexcept
		{
			return isRefUser;
		}

		JGuiExtraFunctionInfo::JGuiExtraFunctionInfo(const std::string extraFunctionName)
			: extraFunctionName(extraFunctionName)
		{}
		std::string JGuiExtraFunctionInfo::GetName()const noexcept
		{
			return extraFunctionName;
		}

		JGuiGroupInfo::JGuiGroupInfo(const std::string extraFunctionName)
			:JGuiExtraFunctionInfo(extraFunctionName)
		{}
		J_GUI_EXTRA_FUNCTION_TYPE JGuiGroupInfo::GetExtraFunctionType()const noexcept
		{
			return J_GUI_EXTRA_FUNCTION_TYPE::GROUP;
		}

		bool JGuiTableInfo::IsFirstColunmGuide()const noexcept
		{
			return isFirstColumnGuide;
		}
		J_GUI_EXTRA_GROUP_TYPE JGuiTableInfo::GetGroupType()const noexcept
		{
			return J_GUI_EXTRA_GROUP_TYPE::TABLE;
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
		void JGuiTableInfo::NotifyAddNewMember(JGuiExtraFunctionUserInfo* mInfo)
		{
			if (mInfo != nullptr && mInfo->GetRefInfoMapName() == GetName())
				++rowCount;
		}

		JGuiConditionInfo::JGuiConditionInfo(const std::string extraFunctionName)
			:JGuiExtraFunctionInfo(extraFunctionName)
		{}
		J_GUI_EXTRA_FUNCTION_TYPE JGuiConditionInfo::GetExtraFunctionType()const noexcept
		{
			return J_GUI_EXTRA_FUNCTION_TYPE::CONDITION;
		}

		JGuiEnumConditionInfo::JGuiEnumConditionInfo(const std::string& extraFunctionName, const std::string& enumName, const std::string& parameterName)
			:JGuiConditionInfo(extraFunctionName), enumName(enumName), parameterName(parameterName)
		{}
		J_GUI_EXTRA_CONDITION_TYPE JGuiEnumConditionInfo::GetConditionType()const noexcept
		{
			return J_GUI_EXTRA_CONDITION_TYPE::ENUM;
		}
		std::string JGuiEnumConditionInfo::GetEnumName()const noexcept
		{
			return enumName;
		}
		std::string JGuiEnumConditionInfo::GetParamName()const noexcept
		{
			return parameterName;
		}

		static struct ExtraInfoMap
		{
		public:
			static std::unordered_map<std::string, std::unique_ptr<JGuiExtraFunctionInfo>>& Data()
			{
				static std::unordered_map<std::string, std::unique_ptr<JGuiExtraFunctionInfo>> infoMap;
				return infoMap;
			}
		}ExtraInfoMap;

		void JGuiExtraFunctionInfoMap::AddExtraFunctionInfo(std::unique_ptr<JGuiExtraFunctionInfo>&& info)
		{
			ExtraInfoMap::Data().emplace(info->GetName(), std::move(info));
		}
		JGuiExtraFunctionInfo* JGuiExtraFunctionInfoMap::GetExtraFunctionInfo(const JGuiExtraFunctionInfoMapName& name)
		{
			auto data = ExtraInfoMap::Data().find(name);
			return data != ExtraInfoMap::Data().end() ? data->second.get() : nullptr;
		}
#pragma endregion

#pragma region Widget
		bool JGuiWidgetInfo::IsExtraFunctionUser()const noexcept
		{
			return isExtraFunctionUser;
		}
		JGuiExtraFunctionInfoMapName JGuiWidgetInfo::GetExtraFunctionMapName(const J_GUI_EXTRA_FUNCTION_TYPE type)const noexcept
		{
			return extraUserInfo[(int)type] ? extraUserInfo[(int)type]->GetRefInfoMapName() : Constants::invalidName;
		}
		JGuiExtraFunctionUserInfo* JGuiWidgetInfo::GetExtraFunctionUserInfo(const J_GUI_EXTRA_FUNCTION_TYPE type)const noexcept
		{
			return extraUserInfo[(int)type].get();
		}

		JGuiInputInfo::JGuiInputInfo(const bool isEnterToReturn)
			:JGuiWidgetInfo(),
			isEnterToReturn(isEnterToReturn)
		{}
		JGuiInputInfo::JGuiInputInfo(const bool isEnterToReturn, const J_PARAMETER_TYPE fixedParam)
			: JGuiWidgetInfo(),
			isEnterToReturn(isEnterToReturn),
			fixedParam(fixedParam)
		{}
		JSupportGuiWidgetType JGuiInputInfo::GetSupportWidgetType()const noexcept
		{
			return Editor::J_GUI_WIDGET_INPUT;
		}
		J_PARAMETER_TYPE JGuiInputInfo::GetFixedParameter()const noexcept
		{
			return fixedParam;
		}
		bool JGuiInputInfo::IsEnterToReturn()const noexcept
		{
			return isEnterToReturn;
		}
		bool JGuiInputInfo::HasValidFixedParameter()const noexcept
		{
			return fixedParam == J_PARAMETER_TYPE::Bool ||
				fixedParam == J_PARAMETER_TYPE::Int ||
				fixedParam == J_PARAMETER_TYPE::Float;
		}


		JSupportGuiWidgetType JGuiCheckBoxInfo::GetSupportWidgetType()const noexcept
		{
			return Editor::J_GUI_WIDGET_CHECKBOX;
		}

		JGuiSliderInfo::JGuiSliderInfo(const float minValue, const float maxValue)
			:JGuiWidgetInfo(),
			minValue(minValue),
			maxValue(maxValue)
		{}
		JGuiSliderInfo::JGuiSliderInfo(const float minValue, const float maxValue, const bool isSupportInput)
			: JGuiWidgetInfo(),
			minValue(minValue),
			maxValue(maxValue),
			isSupportInput(isSupportInput)
		{}
		JGuiSliderInfo::JGuiSliderInfo(const float minValue, const float maxValue, const bool isSupportInput, const bool isVertical)
			: JGuiWidgetInfo(),
			minValue(minValue),
			maxValue(maxValue),
			isSupportInput(isSupportInput),
			isVertical(isVertical)
		{

		}
		JSupportGuiWidgetType JGuiSliderInfo::GetSupportWidgetType()const noexcept
		{
			return Editor::J_GUI_WIDGET_SLIDER;
		}
		float JGuiSliderInfo::GetMinValue()const noexcept
		{
			return minValue;
		}
		float JGuiSliderInfo::GetMaxValue()const noexcept
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

		JSupportGuiWidgetType JGuiColorPickerInfo::GetSupportWidgetType()const noexcept
		{
			return Editor::J_GUI_WIDGET_COLOR_PICKER;
		}
		bool JGuiColorPickerInfo::HasRgbInput()const noexcept
		{
			return hasRgbInput;
		}

		JGuiSelectorInfo::JGuiSelectorInfo(const J_GUI_SELECTOR_IMAGE imageType, const bool hasSizeSlider, GetElemntVecPtr getElementVecPtr)
			:JGuiWidgetInfo(),
			imageType(imageType),
			hasSizeSlider(hasSizeSlider),
			getElementVecPtr(getElementVecPtr)
		{}
		JSupportGuiWidgetType JGuiSelectorInfo::GetSupportWidgetType()const noexcept
		{
			return Editor::J_GUI_WIDGET_SELECTOR;
		}
		J_GUI_SELECTOR_IMAGE JGuiSelectorInfo::GetPreviewImageType()const noexcept
		{
			return imageType;
		}
		JGuiSelectorInfo::GetElemntVecPtr JGuiSelectorInfo::GetElementVecPtr()const noexcept
		{
			return getElementVecPtr;
		}
		bool JGuiSelectorInfo::HasSizeSlider()const noexcept
		{
			return hasSizeSlider;
		}

		JSupportGuiWidgetType JGuiReadOnlyTextInfo::GetSupportWidgetType()const noexcept
		{
			return Editor::J_GUI_WIDGET_READONLY_TEXT;
		}

		JSupportGuiWidgetType JGuiEnumComboBoxInfo::GetSupportWidgetType()const noexcept
		{
			return Editor::J_GUI_WIDGET_ENUM_COMBO;
		}
		std::string JGuiEnumComboBoxInfo::GetEnumFullName()const noexcept
		{
			return enumFullName;
		}

		JGuiListInfo::JGuiListInfo(const J_GUI_LIST_TYPE listType, const bool canDisplayElementGui, CreateElementPtr createElementPtr)
			:JGuiWidgetInfo(),
			listType(listType),
			canDisplayElementGui(canDisplayElementGui),
			createElementPtr(createElementPtr)
		{ }
		JSupportGuiWidgetType JGuiListInfo::GetSupportWidgetType()const noexcept
		{
			return Editor::J_GUI_WIDGET_LIST;
		}
		J_GUI_LIST_TYPE JGuiListInfo::GetListType()const noexcept
		{
			return listType;
		}
		bool JGuiListInfo::CanDisplayElementGui()const noexcept
		{
			return canDisplayElementGui;
		}
		JGuiListInfo::CreateElementPtr JGuiListInfo::GetCreateElementPtr()const noexcept
		{
			return createElementPtr;
		}

#pragma endregion

	}
}