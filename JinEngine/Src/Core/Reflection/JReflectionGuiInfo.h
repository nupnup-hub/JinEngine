#pragma once
#include"../JDataType.h"   
#include "../Func/Callable/JCallable.h"
#include "../Func/Functor/JFunctor.h"
#include"../../Utility/JTypeUtility.h"
#include<string>
#include<vector>
#include<memory> 

namespace JinEngine
{
	namespace Core
	{
		class JIdentifier;

		enum class J_GUI_GROUP_TYPE
		{
			NONE,
			TABLE,
			ENUM_TRIGGER
		};

		enum class J_GUI_SELECTOR_IMAGE
		{
			NONE,
			ICON,
			IMAGE
		};
		enum class J_GUI_LIST_TYPE
		{
			 STATIC,	
			 DYNAMIC	// Can Add element
		};

		using JSupportGuiWidgetType = int;
		using JGuiFlag = int;
		using JEnum = int;
		using JGuiGroupKey = std::string;  
		namespace Constants
		{
			static constexpr JSupportGuiWidgetType NotSupportGuiWidget = 0;
			static JGuiGroupKey InvalidGroupKey = " ";

			//static constexpr int guiSelectorNoneImage = 0;
		//	static constexpr int guiSelectorIconImage = 1;
		//	static constexpr int guiSelectorImage = 2;

			static constexpr int guiStaticList = 0;
			static constexpr int guiDynamicList = 1;
		} 

#pragma region Group
		class JGuiGroupMemberInfo
		{
		private:
			const JGuiGroupKey groupKey = Constants::InvalidGroupKey;
		public:
			JGuiGroupMemberInfo(const JGuiGroupKey groupKey);
			virtual ~JGuiGroupMemberInfo() = default;
		public:
			JGuiGroupKey GetGroupKey()const noexcept;
		};
		 
		class JGuiEnumConditionGroupMemberInfoHandle : public JGuiGroupMemberInfo
		{
		public:
			JGuiEnumConditionGroupMemberInfoHandle(const JGuiGroupKey groupKey);
			virtual ~JGuiEnumConditionGroupMemberInfoHandle() = default;
		public:
			virtual bool OnTrigger(const JEnum value)const noexcept = 0;
		};

		template<int buffCount>
		class JGuiEnumConditionGroupMemberInfo : public JGuiEnumConditionGroupMemberInfoHandle
		{
		private:
			//enum need to declare enum not enum class
			JEnum enumValue[buffCount];
		public:
			template<typename ...Param>
			JGuiEnumConditionGroupMemberInfo(const JGuiGroupKey groupKey, Param... value)
				:JGuiEnumConditionGroupMemberInfoHandle(groupKey)
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
		 
		//controll gui extra action
		//ex) table groupping, display condition, type conditon
		//scenario
		//1.. declare group info by GUI_GROUP... macro
		//2.. memeber gui send JGuiGroupMemberInfo
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

		//Set gui condition 
		//display gui when parameter is registered enum
		//enum need to declare enum not enum class
		//or REGISTER_ENUM, REGISTER_ENUM_CLASS
		class JGuiEnumConditionInfo : public JGuiGroupInfo
		{
		private:
			const std::string enumName;
			const std::string parameterName;
		public:
			JGuiEnumConditionInfo(const std::string& groupName, const std::string& enumName, const std::string& parameterName);
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
#pragma endregion

#pragma region Widget

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
			using GetElemntVecF = Core::JSFunctorType<std::vector<JIdentifier*>, JIdentifier*>;
		private:  
			const J_GUI_SELECTOR_IMAGE imageType;
			const bool hasSizeSlider;
		private:
			std::unique_ptr<GetElemntVecF::Functor> getElementVecFunctor = nullptr;
		public:
			JGuiSelectorInfo(const J_GUI_SELECTOR_IMAGE imageType,
				const bool hasSizeSlider,
				GetElemntVecF::Ptr getElementVecPtr = nullptr,
				std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo = nullptr);
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
		public:
			J_GUI_SELECTOR_IMAGE GetPreviewImageType()const noexcept;
			GetElemntVecF::Functor* GetElementVecFunctor()const noexcept;
		public:
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
	 
		class JGuiListInfo : public JGuiWidgetInfo
		{
		public: 
			//모든 JIdenfier를 상속받는 object는 Factory에서 생성되며
			//Instance는 ownerPtr로 typeInfo class에 저장되고 pointer를 반환한다
			using AddElementF = Core::JSFunctorType<JIdentifier*, JIdentifier*>;
		private:
			//0.. static
			//1.. dynamic can add element
			const J_GUI_LIST_TYPE listType;
			const bool canDisplayElementGui; 
			std::unique_ptr<AddElementF::Functor> addElementF = nullptr;
		public:
			JGuiListInfo(const J_GUI_LIST_TYPE listType,
				const bool canDisplayElementGui, 
				AddElementF::Ptr addElementPtr = nullptr,
				std::unique_ptr<JGuiGroupMemberInfo>&& newGroupMemberInfo = nullptr);
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
		public:
			J_GUI_LIST_TYPE GetListType()const noexcept;
			bool CanDisplayElementGui()const noexcept;
		public: 
			AddElementF::Functor* GetAddElementFunctor()const noexcept;
		};
#pragma endregion
	}
}