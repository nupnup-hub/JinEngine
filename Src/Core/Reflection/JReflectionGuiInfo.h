#pragma once
#include<string>
#include<vector>
#include<memory> 
#include"../JDataType.h"   
#include "../Func/Callable/JCallable.h"
#include"../../Utility/JTypeUtility.h"

namespace JinEngine
{
	namespace Core
	{
		enum class J_GUI_GROUP_TYPE
		{
			NONE,
			TABLE,
			ENUM_TRIGGER
		};

		using JSupportGuiWidgetType = int;
		using JGuiFlag = int;
		using JEnum = int;
		using JGuiGroupKey = std::string;  
		namespace Constant
		{
			static constexpr JSupportGuiWidgetType NotSupportGuiWidget = 0;
			static JGuiGroupKey InvalidGroupKey = " ";
		}

		class JGuiGroupMemberInfo
		{
		private:
			const JGuiGroupKey groupKey = Constant::InvalidGroupKey;
		public:
			JGuiGroupMemberInfo(const JGuiGroupKey groupKey);
			virtual ~JGuiGroupMemberInfo() = default;
		public:
			JGuiGroupKey GetGroupKey()const noexcept;
		};
		 
		class JGuiEnumTriggerGroupMemberInfoHandle : public JGuiGroupMemberInfo
		{
		public:
			JGuiEnumTriggerGroupMemberInfoHandle(const JGuiGroupKey groupKey);
			virtual ~JGuiEnumTriggerGroupMemberInfoHandle() = default;
		public:
			virtual bool OnTrigger(const JEnum value)const noexcept = 0;
		};

		template<int buffCount>
		class JGuiEnumTriggerGroupMemberInfo : public JGuiEnumTriggerGroupMemberInfoHandle
		{
		private:
			//enum need to declare enum not enum class
			JEnum enumValue[buffCount];
		public:
			template<typename ...Param>
			JGuiEnumTriggerGroupMemberInfo(const JGuiGroupKey groupKey, Param... value)
				:JGuiEnumTriggerGroupMemberInfoHandle(groupKey)
			{  
				int i = 0;
				((enumValue[i++] = (JEnum)value), ...);
			}
		public:
			bool OnTrigger(const JEnum value)const noexcept
			{
				for (uint i = 0; i < buffCount; ++i)
				{
					if (value == enumValue[i] || (enumValue[i] & value) > 0)
						return true;
				}
				return false;
			}
		};

		class JGuiWidgetInfo
		{
		private:
			std::unique_ptr<JGuiGroupMemberInfo> groupMemberInfo = nullptr;
		public:
			JGuiWidgetInfo() = default;
			JGuiWidgetInfo(std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo);
			virtual ~JGuiWidgetInfo() = default;
		public:
			virtual JSupportGuiWidgetType GetSupportWidgetType()const noexcept = 0;
		public:
			bool IsMemberWidget()const noexcept;
			JGuiGroupKey GetGroupKey()const noexcept;
			JGuiGroupMemberInfo* GetGroupMemberInfo()const noexcept;
		};

		class JGuiInputInfo : public JGuiWidgetInfo
		{
		private:
			bool isEnterToReturn;
		public:
			JGuiInputInfo(const bool isEnterToReturn);
			JGuiInputInfo(const bool isEnterToReturn, std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo);
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
		public:
			bool IsEnterToReturn()const noexcept;
		};

		class JGuiCheckBoxInfo : public JGuiWidgetInfo
		{
		public:
			JGuiCheckBoxInfo() = default;
			JGuiCheckBoxInfo(std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo);
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
		};

		class JGuiSliderInfo : public JGuiWidgetInfo
		{
		private:
			const int minValue;
			const int maxValue;
		private:
			const bool isSupportInput;
			const bool isVertical;
		public:
			JGuiSliderInfo(float minValue, float maxValue, bool isSupportInput = true, bool isVertical = false);
			JGuiSliderInfo(float minValue, float maxValue, bool isSupportInput, bool isVertical, std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo);
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
		public:
			int GetMinValue()const noexcept;
			int GetMaxValue()const noexcept;
		public:
			bool IsSupportInput()const noexcept;
			bool IsVertical()const noexcept;
		};

		class JGuiColorPickerInfo : public JGuiWidgetInfo
		{
		private:
			const bool hasRgbInput;
		public:
			JGuiColorPickerInfo(bool hasRgbInput);
			JGuiColorPickerInfo(bool hasRgbInput, std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo);
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
		public:
			bool HasRgbInput()const noexcept;
		};

		class JGuiSelectorInfo : public JGuiWidgetInfo
		{
		private: 
			const bool isImageRtTexture;
			const bool hasSizeSlider;
		public:
			JGuiSelectorInfo(const bool isImageRtTexture, const bool hasSizeSlider);
			JGuiSelectorInfo(const bool isImageRtTexture, const bool hasSizeSlider, std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo);
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
		public:
			bool IsImageRtTexture()const noexcept;
			bool HasSizeSlider()const noexcept;
		};

		class JGuiReadOnlyTextInfo : public JGuiWidgetInfo
		{
		public:
			JGuiReadOnlyTextInfo() = default;
			JGuiReadOnlyTextInfo(std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo);
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
		};

		class JGuiEnumComboBoxInfo : public JGuiWidgetInfo
		{
		private:
			const std::string enumFullName;
		public:   
			JGuiEnumComboBoxInfo(const std::string enumName);
			JGuiEnumComboBoxInfo(const std::string enumName, std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo);
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
		public:
			std::string GetEnumFullName()const noexcept;
		private:
			std::string FindEnumFullName(const std::string enumName)const noexcept;
		};
		 
		class JGuiGroupInfo
		{
		private:
			const std::string groupName;
		public:
			JGuiGroupInfo(const std::string groupName);
			virtual ~JGuiGroupInfo() = default;
		public:
			std::string GetGroupName()const noexcept;
			virtual J_GUI_GROUP_TYPE GetGuiGroupType()const noexcept = 0;
		public:
			virtual void NotifyAddNewMember(JGuiGroupMemberInfo* mInfo) = 0;
		};

		class JGuiTableInfo : public JGuiGroupInfo
		{
		private:
			const bool isFirstColumnGuide;
		private: 
			std::vector<std::string> columnGuide;
			uint columnCount;
			uint rowCount;
		public:
			template<typename ...Param>
			JGuiTableInfo(const std::string tableName, bool isFirstColumnGuide, Param... var)
				:JGuiGroupInfo(tableName), isFirstColumnGuide(isFirstColumnGuide)
			{
				((columnGuide.push_back(var)), ...);
				columnCount = (uint)columnGuide.size();
			}
		public:
			bool IsFirstColunmGuide()const noexcept;
		public: 
			J_GUI_GROUP_TYPE GetGuiGroupType()const noexcept final;
			std::string GetColumnGuide(const uint index)const noexcept;
			uint GetColumnCount()const noexcept;
			uint GetRowCount()const noexcept; 
		public:
			void NotifyAddNewMember(JGuiGroupMemberInfo* mInfo)final;
		};

		//enum need to declare enum not enum class
		//And REGISTER_ENUM
		class JGuiEnumTriggerInfo : public JGuiGroupInfo
		{
		private:
			const std::string enumName;
			const std::string parameterName; 
		public:
			JGuiEnumTriggerInfo(const std::string& groupName, const std::string& enumName, const std::string& parameterName);
		public:
			J_GUI_GROUP_TYPE GetGuiGroupType()const noexcept final;
		public:
			std::string GetEnumName()const noexcept;
			std::string GetParamName()const noexcept;
		public:
			void NotifyAddNewMember(JGuiGroupMemberInfo* mInfo)final;
		};

		class JGuiGroupMap
		{
		public:
			static void AddGuiGroup(std::unique_ptr<JGuiGroupInfo>&& rInfo);
			static JGuiGroupInfo* GetGuiGroupInfo(const JGuiGroupKey& groupKey);
		};
	}
}