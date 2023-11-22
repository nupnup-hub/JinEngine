#include"JGuiWidgetInfo.h"
#include"JReflectionInfo.h"
#include"JEnumInfo.h"
#include"JGuiWidgetInfoHandleBase.h"
#include"JGuiWidgetType.h"  

namespace JinEngine
{
	namespace Core
	{
		namespace
		{
			static struct ExtraInfoMap
			{
			public:
				static std::unordered_map<std::string, JOwnerPtr<JGuiExtraFunctionInfo>>& Data()
				{
					static std::unordered_map<std::string, JOwnerPtr<JGuiExtraFunctionInfo>> infoMap;
					return infoMap;
				}
			}ExtraInfoMap;
		}

#pragma region Group
		JGuiExtraFunctionUserInfo::JGuiExtraFunctionUserInfo(const JGuiExtraFunctionInfoMapName infoMapName)
			:infoMapName(infoMapName)
		{}
		JGuiExtraFunctionInfoMapName JGuiExtraFunctionUserInfo::GetExtraFunctionName()const noexcept
		{
			return infoMapName;
		}
		JUserPtr<JGuiExtraFunctionInfo> JGuiExtraFunctionUserInfo::GetExtraFunctionInfo()const noexcept
		{
			auto data = ExtraInfoMap::Data().find(infoMapName);
			if (data == ExtraInfoMap::Data().end())
				return nullptr;

			return data->second;
		}

		JGuiTableUserInfo::JGuiTableUserInfo(const JGuiExtraFunctionInfoMapName infoMapName,
			const uint useColumnCount,
			const bool canAddRowCount)
			:JGuiExtraFunctionUserInfo(infoMapName), useColumnCount(useColumnCount)
		{
			if (canAddRowCount)
			{
				JGuiExtraFunctionInfo* info = GetExtraFunctionInfo().Get();
				static_cast<JGuiTableInfo*>(info)->NotifyAddMember(this);
			}
		}
		uint JGuiTableUserInfo::GetUseColumnCount()const noexcept
		{
			return useColumnCount;
		}

		JGuiParamConditionUserInfoBase::RefParamInfo::RefParamInfo(const std::string& refOwnerName)
			:refOwnerName(refOwnerName)
		{}
		JGuiParamConditionUserInfoBase::JGuiParamConditionUserInfoBase(const JGuiExtraFunctionInfoMapName infoMapName)
			: JGuiExtraFunctionUserInfo(infoMapName), refParamInfo(nullptr)
		{}
		JGuiParamConditionUserInfoBase::JGuiParamConditionUserInfoBase(const JGuiExtraFunctionInfoMapName infoMapName, const std::string& refParamOwnerName)
			: JGuiExtraFunctionUserInfo(infoMapName)
		{
			if (!refParamOwnerName.empty())
				refParamInfo = std::make_unique<RefParamInfo>(refParamOwnerName);
		}
		std::string JGuiParamConditionUserInfoBase::GetRefOwnerName()const noexcept
		{
			return refParamInfo != nullptr ? refParamInfo->refOwnerName : std::string();
		}
		bool JGuiParamConditionUserInfoBase::IsOwnRefParam()const noexcept
		{
			return refParamInfo == nullptr;
		}

		JGuiBoolParmConditionUserInfo::JGuiBoolParmConditionUserInfo(const JGuiExtraFunctionInfoMapName infoMapName, const bool value)
			:JGuiParamConditionUserInfoBase(infoMapName), condValue(value)
		{}
		JGuiBoolParmConditionUserInfo::JGuiBoolParmConditionUserInfo(const JGuiExtraFunctionInfoMapName infoMapName, const std::string& refParamOwnerName, const bool value)
			: JGuiParamConditionUserInfoBase(infoMapName, refParamOwnerName), condValue(value)
		{}
		bool JGuiBoolParmConditionUserInfo::OnTrigger(const bool value)const noexcept
		{
			return condValue == value;
		}

		JGuiEnumParamConditionUserInfoInterface::JGuiEnumParamConditionUserInfoInterface(const JGuiExtraFunctionInfoMapName infoMapName)
			: JGuiParamConditionUserInfoBase(infoMapName)
		{}
		JGuiEnumParamConditionUserInfoInterface::JGuiEnumParamConditionUserInfoInterface(const JGuiExtraFunctionInfoMapName infoMapName, const std::string& refOwnerName)
			: JGuiParamConditionUserInfoBase(infoMapName, refOwnerName)
		{}

		JGuiGroupUserInfo::JGuiGroupUserInfo(const JGuiExtraFunctionInfoMapName infoMapName)
			: JGuiExtraFunctionUserInfo(infoMapName)
		{}

		JGuiExtraFunctionInfo::JGuiExtraFunctionInfo(const std::string& extraFunctionName)
			: extraFunctionName(extraFunctionName)
		{}
		std::string JGuiExtraFunctionInfo::GetName()const noexcept
		{
			return extraFunctionName;
		}
		int JGuiExtraFunctionInfo::GetUserCount()const noexcept
		{
			return userCount;
		}
		void JGuiExtraFunctionInfo::NotifyAddMember(JGuiExtraFunctionUserInfo* user)
		{
			if (user == nullptr)
				return;
			++userCount;
		}

		bool JGuiTableInfo::IsFirstColunmGuide()const noexcept
		{
			return isFirstColumnGuide;
		}
		J_GUI_EXTRA_FUNCTION_TYPE JGuiTableInfo::GetExtraFunctionType()const noexcept
		{
			return J_GUI_EXTRA_FUNCTION_TYPE::TABLE;
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
		void JGuiTableInfo::NotifyAddMember(JGuiExtraFunctionUserInfo* user)
		{
			JGuiExtraFunctionInfo::NotifyAddMember(user);
			if (user == nullptr)
				return;

			if (user->GetExtraFunctionName() == GetName())
				++rowCount;
		}

		JGuiConditionInfo::JGuiConditionInfo(const std::string& extraFunctionName)
			:JGuiExtraFunctionInfo(extraFunctionName)
		{}
		J_GUI_EXTRA_FUNCTION_TYPE JGuiConditionInfo::GetExtraFunctionType()const noexcept
		{
			return J_GUI_EXTRA_FUNCTION_TYPE::CONDITION;
		}

		JGuiBoolParamConditionInfo::JGuiBoolParamConditionInfo(const std::string& extraFunctionName,
			const std::string& refName,
			const bool isRefMethod)
			:JGuiConditionInfo(extraFunctionName), refName(refName), isRefMethod(isRefMethod)
		{}
		J_GUI_EXTRA_CONDITION_TYPE JGuiBoolParamConditionInfo::GetConditionType()const noexcept
		{
			return J_GUI_EXTRA_CONDITION_TYPE::BOOLEAN_PARAM;
		}
		std::string JGuiBoolParamConditionInfo::GetRefName()const noexcept
		{
			return refName;
		}
		bool JGuiBoolParamConditionInfo::IsRefMethod()const noexcept
		{
			return isRefMethod;
		}

		JGuiEnumParamConditionInfo::JGuiEnumParamConditionInfo(const std::string& extraFunctionName,
			const std::string& enumName,
			const std::string& refName,
			const bool isRefMethod)
			:JGuiConditionInfo(extraFunctionName),
			enumName(enumName),
			refName(refName),
			isRefMethod(isRefMethod)
		{}
		J_GUI_EXTRA_CONDITION_TYPE JGuiEnumParamConditionInfo::GetConditionType()const noexcept
		{
			return J_GUI_EXTRA_CONDITION_TYPE::ENUM_PARAM;
		}
		std::string JGuiEnumParamConditionInfo::GetEnumName()const noexcept
		{
			return enumName;
		}
		std::string JGuiEnumParamConditionInfo::GetRefName()const noexcept
		{
			return refName;
		}
		bool JGuiEnumParamConditionInfo::IsRefMethod()const noexcept
		{
			return isRefMethod;
		}

		JGuiGroupInfo::JGuiGroupInfo(const std::string& extraFunctionName)
			:JGuiExtraFunctionInfo(extraFunctionName)
		{}
		J_GUI_EXTRA_FUNCTION_TYPE JGuiGroupInfo::GetExtraFunctionType()const noexcept
		{
			return J_GUI_EXTRA_FUNCTION_TYPE::GROUP;
		}
		void JGuiGroupInfo::SetOpenTrigger(const bool value)noexcept
		{
			isOpen = value;
		}
		bool JGuiGroupInfo::IsOpen()const noexcept
		{
			return isOpen;
		}

		void JGuiExtraFunctionInfoMap::Register(JOwnerPtr<JGuiExtraFunctionInfo>&& info)
		{
			auto& data = ExtraInfoMap::Data();
			if (data.find(info->GetName()) != data.end())
				return;

			data.emplace(info->GetName(), std::move(info));
		}
#pragma endregion

#pragma region Widget
		JGuiExtraFunctionInfoMapName JGuiWidgetInfo::GetExtraFunctionMapName(const J_GUI_EXTRA_FUNCTION_TYPE type)const noexcept
		{
			return extraUserInfo[(int)type] != nullptr ? extraUserInfo[(int)type]->GetExtraFunctionName() : Constants::invalidName;
		}
		JUserPtr<JGuiExtraFunctionUserInfo> JGuiWidgetInfo::GetExtraFunctionUserInfo(const J_GUI_EXTRA_FUNCTION_TYPE type)const noexcept
		{
			return extraUserInfo[(int)type];
		}
		void JGuiWidgetInfo::SetExtraFunctionUserInfo(const JUserPtr<JGuiExtraFunctionInfo>& info, JOwnerPtr<JGuiExtraFunctionUserInfo>&& userInfo)
		{
			if (info == nullptr || userInfo == nullptr)
				return;

			const J_GUI_EXTRA_FUNCTION_TYPE type = info->GetExtraFunctionType();
			if (extraUserInfo[(int)type] == nullptr)
			{
				extraUserInfo[(int)type] = std::move(userInfo);
				extraUserInfo[(int)type]->GetExtraFunctionInfo()->NotifyAddMember(extraUserInfo[(int)type].Get());
			}
		}
		bool JGuiWidgetInfo::IsExtraFunctionUser()const noexcept
		{
			return isExtraFunctionUser;
		}
		bool JGuiWidgetInfo::IsExtraFunctionUser(const J_GUI_EXTRA_FUNCTION_TYPE type)const noexcept
		{
			return extraUserInfo[(int)type] != nullptr;
		}
		void JGuiWidgetInfo::TryReSettingExtraUserInfo()
		{
			const uint count = (uint)invalidExtraUserInfo.size();
			for (uint i = 0; i < count; ++i)
			{
				if (invalidExtraUserInfo[i] == nullptr)
					continue;

				JUserPtr<JGuiExtraFunctionInfo> info = invalidExtraUserInfo[i]->GetExtraFunctionInfo();
				if (info != nullptr)
					SetExtraFunctionUserInfo(info, std::move(invalidExtraUserInfo[i]));
			}
			invalidExtraUserInfo.clear();
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
		J_GUI_WIDGET_TYPE JGuiInputInfo::GetSupportWidgetType()const noexcept
		{
			return J_GUI_WIDGET_TYPE::INPUT;
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


		J_GUI_WIDGET_TYPE JGuiCheckBoxInfo::GetSupportWidgetType()const noexcept
		{
			return J_GUI_WIDGET_TYPE::CHECKBOX;
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
		{}
		JGuiSliderInfo::JGuiSliderInfo(const float minValue, const float maxValue, const bool isSupportInput, const bool isVertical, const uint floatDigit)
			: JGuiWidgetInfo(),
			minValue(minValue),
			maxValue(maxValue),
			isSupportInput(isSupportInput),
			isVertical(isVertical),
			floatDigit(floatDigit)
		{

		}
		J_GUI_WIDGET_TYPE JGuiSliderInfo::GetSupportWidgetType()const noexcept
		{
			return J_GUI_WIDGET_TYPE::SLIDER;
		}
		float JGuiSliderInfo::GetMinValue()const noexcept
		{
			return minValue;
		}
		float JGuiSliderInfo::GetMaxValue()const noexcept
		{
			return maxValue;
		}
		uint JGuiSliderInfo::GetFloatDigit()const noexcept
		{
			return floatDigit;
		}
		bool JGuiSliderInfo::IsSupportInput()const noexcept
		{
			return isSupportInput;
		}
		bool JGuiSliderInfo::IsVertical()const noexcept
		{
			return isVertical;
		}

		J_GUI_WIDGET_TYPE JGuiColorPickerInfo::GetSupportWidgetType()const noexcept
		{
			return J_GUI_WIDGET_TYPE::COLOR_PICKER;
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
		J_GUI_WIDGET_TYPE JGuiSelectorInfo::GetSupportWidgetType()const noexcept
		{
			return J_GUI_WIDGET_TYPE::SELECTOR;
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

		J_GUI_WIDGET_TYPE JGuiReadOnlyTextInfo::GetSupportWidgetType()const noexcept
		{
			return J_GUI_WIDGET_TYPE::READONLY_TEXT;
		}

		JGuiEnumComboBoxInfo::JGuiEnumComboBoxInfo(const std::string& enumFullName)
			:JGuiWidgetInfo(), enumFullName(enumFullName)
		{}
		J_GUI_WIDGET_TYPE JGuiEnumComboBoxInfo::GetSupportWidgetType()const noexcept
		{
			return J_GUI_WIDGET_TYPE::COMBO;
		}
		std::string JGuiEnumComboBoxInfo::GetEnumFullName()const noexcept
		{
			return enumFullName;
		}
		const std::vector<JCommandToken>& JGuiEnumComboBoxInfo::GetToken()const noexcept
		{
			return token;
		}
		std::vector<JCommandToken> JGuiEnumComboBoxInfo::CreateToken(std::string displayCommand)
		{
			return displayCommand.empty() ? std::vector<JCommandToken> {} : JSimpleCompiler::Scan(displayCommand);
		}

		JGuiListInfo::JGuiListInfo(const J_GUI_LIST_TYPE listType, const bool canDisplayElementGui, CreateElementPtr createElementPtr)
			:JGuiWidgetInfo(),
			listType(listType),
			canDisplayElementGui(canDisplayElementGui),
			createElementPtr(createElementPtr)
		{ }
		J_GUI_WIDGET_TYPE JGuiListInfo::GetSupportWidgetType()const noexcept
		{
			return J_GUI_WIDGET_TYPE::LIST;
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