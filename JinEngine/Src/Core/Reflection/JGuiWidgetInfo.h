#pragma once
#include"JParameterType.h"
#include"../JDataType.h"     
#include"../../Utility/JTypeUtility.h"
#include<string>
#include<vector>
#include<memory> 
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{
		class JIdentifier;

		enum class J_GUI_EXTRA_FUNCTION_TYPE
		{ 
			GROUP,
			CONDITION, 
			COUNT,
		};
		enum class J_GUI_EXTRA_GROUP_TYPE
		{
			TABLE,
		};
		enum class J_GUI_EXTRA_CONDITION_TYPE
		{
			ENUM,
		};

		enum class J_GUI_EXTRA_USER_TYPE
		{
			ENUM,
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
		using JGuiExtraFunctionInfoMapName = std::string;  
		namespace Constants
		{
			static constexpr JSupportGuiWidgetType NotSupportGuiWidget = 0;
			static JGuiExtraFunctionInfoMapName invalidName = "";

			//static constexpr int guiSelectorNoneImage = 0;
		//	static constexpr int guiSelectorIconImage = 1;
		//	static constexpr int guiSelectorImage = 2;

			static constexpr int guiStaticList = 0;
			static constexpr int guiDynamicList = 1;
		} 

#pragma region Extra
		//It is used for extra action of widget 
		//1. groupping
		//2. condition 
		
		//attibute has order dependency
		//groupping is always called at first
		 
		class JGuiExtraFunctionUserInfo
		{
		private:
			const JGuiExtraFunctionInfoMapName infoMapName = Constants::invalidName;
		public:
			JGuiExtraFunctionUserInfo(const JGuiExtraFunctionInfoMapName infoMapName);
			virtual ~JGuiExtraFunctionUserInfo() = default;
		public:
			JGuiExtraFunctionInfoMapName GetRefInfoMapName()const noexcept; 
			J_GUI_EXTRA_FUNCTION_TYPE GetRefInfoMapType()const noexcept;
		};

		class JGuiTableUserInfo : public JGuiExtraFunctionUserInfo
		{
		private:
			const uint useColumnCount; 
		public:
			JGuiTableUserInfo(const JGuiExtraFunctionInfoMapName infoMapName,
				const uint useColumnCount, 
				const bool canAddRowCount);
		public:
			uint GetUseColumnCount()const noexcept; 
		};


		class JGuiEnumConditionUserInfoBase : public JGuiExtraFunctionUserInfo
		{
		private:
			//Ref user
			const std::string ownerTypeParameterName;
			const bool isRefUser = false;
		public:
			JGuiEnumConditionUserInfoBase(const JGuiExtraFunctionInfoMapName infoMapName);
			JGuiEnumConditionUserInfoBase(const JGuiExtraFunctionInfoMapName infoMapName, const std::string ownerTypeParameterName);
			virtual ~JGuiEnumConditionUserInfoBase() = default;
		public:
			virtual bool OnTrigger(const JEnum value)const noexcept = 0; 
		public:
			std::string GetOwnerTypeParameterNameInRefClass()const noexcept;
			bool IsRefUser()const noexcept;
		};

		template<int buffCount>
		class JGuiEnumConditionUserInfo : public JGuiEnumConditionUserInfoBase
		{ 
		private:
			//enum need to declare enum not enum class
			JEnum enumValue[buffCount];
		public:
			template<typename ...Param>
			JGuiEnumConditionUserInfo(const JGuiExtraFunctionInfoMapName infoMapName, Param... value)
				:JGuiEnumConditionUserInfoBase(infoMapName)
			{  
				int i = 0;
				((enumValue[i++] = (JEnum)value), ...);
			}
			template<typename ...Param>
			JGuiEnumConditionUserInfo(const JGuiExtraFunctionInfoMapName infoMapName,
				const std::string ownerTypeParameterName,
				Param... value)
				: JGuiEnumConditionUserInfoBase(ownerTypeParameterName, infoMapName)
			{
				int i = 0;
				((enumValue[i++] = (JEnum)value), ...);
			}
		public:
			bool OnTrigger(const JEnum value)const noexcept final
			{
				for (uint i = 0; i < buffCount; ++i)
				{
					if (value == enumValue[i] || (enumValue[i] & value) > 0)
						return true;
				}
				return false;
			}
		};

		template<int buffCount>
		class JGuiEnumConditionRefUserInfo : public JGuiEnumConditionUserInfo<buffCount>
		{
		public:
			template<typename ...Param>
			JGuiEnumConditionRefUserInfo(const JGuiExtraFunctionInfoMapName infoMapName,
				const std::string ownerTypeParameterName,
				Param... value)
				:JGuiEnumConditionUserInfo<buffCount>(ownerTypeParameterName, infoMapName, value...)
			{}
		};

		//controll gui extra action
		//ex) table groupping, display condition, type conditon
		//scenario
		//1.. declare group info by GUI_GROUP... macro
		//2.. memeber gui send JGuiGroupMemberInfo

		class JGuiExtraFunctionInfo
		{
		private:
			const std::string extraFunctionName;
		public:
			JGuiExtraFunctionInfo(const std::string extraFunctionName);
			virtual ~JGuiExtraFunctionInfo() = default;
		public:
			std::string GetName()const noexcept;
			virtual J_GUI_EXTRA_FUNCTION_TYPE GetExtraFunctionType()const noexcept = 0; 
		};

		class JGuiGroupInfo : public JGuiExtraFunctionInfo
		{
		public:
			JGuiGroupInfo(const std::string extraFunctionName);
		public:
			virtual void NotifyAddNewMember(JGuiExtraFunctionUserInfo* mInfo) = 0;
		public: 
			virtual J_GUI_EXTRA_GROUP_TYPE GetGroupType()const noexcept = 0;
			J_GUI_EXTRA_FUNCTION_TYPE GetExtraFunctionType()const noexcept;
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
			JGuiTableInfo(const bool isFirstColumnGuide, const std::vector<std::string>& columnGuide)
				:JGuiGroupInfo(Constants::invalidName), isFirstColumnGuide(isFirstColumnGuide), columnGuide(columnGuide)
			{
				columnCount = (uint)columnGuide.size();
			}
			template<typename ...Param>
			JGuiTableInfo(const std::string extraFunctionName, const bool isFirstColumnGuide, Param... var)
				:JGuiGroupInfo(extraFunctionName), isFirstColumnGuide(isFirstColumnGuide)
			{
				((columnGuide.push_back(var)), ...);
				columnCount = (uint)columnGuide.size();
			}
		public:
			bool IsFirstColunmGuide()const noexcept;
		public:  
			J_GUI_EXTRA_GROUP_TYPE GetGroupType()const noexcept final;
			std::string GetColumnGuide(const uint index)const noexcept;
			uint GetColumnCount()const noexcept;
			uint GetRowCount()const noexcept;
		public:
			void NotifyAddNewMember(JGuiExtraFunctionUserInfo* mInfo)final;
		};

		//Set gui condition 
		//display gui when parameter is registered enum
		//enum need to declare enum not enum class
		//or REGISTER_ENUM, REGISTER_ENUM_CLASS

		class JGuiConditionInfo : public JGuiExtraFunctionInfo
		{
		public:
			JGuiConditionInfo(const std::string extraFunctionName);
		public:
			virtual J_GUI_EXTRA_CONDITION_TYPE GetConditionType()const noexcept = 0;
			J_GUI_EXTRA_FUNCTION_TYPE GetExtraFunctionType()const noexcept;
		};

		class JGuiEnumConditionInfo : public JGuiConditionInfo
		{
		private:
			const std::string enumName;
			//condition trigger parameter
			const std::string parameterName;
		public:
			JGuiEnumConditionInfo(const std::string& extraFunctionName, const std::string& enumName, const std::string& parameterName);
		public:
			J_GUI_EXTRA_CONDITION_TYPE GetConditionType()const noexcept final;
			std::string GetEnumName()const noexcept;
			std::string GetParamName()const noexcept; 
		};

		class JGuiExtraFunctionInfoMap
		{
		public:
			static void AddExtraFunctionInfo(std::unique_ptr<JGuiExtraFunctionInfo>&& aInfo);
			static JGuiExtraFunctionInfo* GetExtraFunctionInfo(const JGuiExtraFunctionInfoMapName& groupKey);
		};
#pragma endregion

#pragma region Widget

		class JGuiWidgetInfo
		{
		private:
			std::unique_ptr<JGuiExtraFunctionUserInfo> extraUserInfo[(int)J_GUI_EXTRA_FUNCTION_TYPE::COUNT];
		private:
			bool isExtraFunctionUser = false;
		public: 
			JGuiWidgetInfo() = default;
			template<typename ...Param>
			JGuiWidgetInfo(Param... var)
			{
				auto pushUserInfoLam = [](JGuiWidgetInfo* widgetInfo, std::unique_ptr<JGuiExtraFunctionUserInfo>&& extraInfo)
				{
					if (extraInfo != nullptr)
					{
						//Register if has extra info
						JGuiExtraFunctionInfo* info = JGuiExtraFunctionInfoMap::GetExtraFunctionInfo(extraInfo->GetRefInfoMapName());
						if (info != nullptr)
						{
							const J_GUI_EXTRA_FUNCTION_TYPE type = info->GetExtraFunctionType();
							widgetInfo->isExtraFunctionUser = true;

							switch (type)
							{ 
							case JinEngine::Core::J_GUI_EXTRA_FUNCTION_TYPE::GROUP:
							{
								if (widgetInfo->extraUserInfo[(int)type] == nullptr)
									widgetInfo->extraUserInfo[(int)type] = std::move(extraInfo); 		 
								break;
							}
							case JinEngine::Core::J_GUI_EXTRA_FUNCTION_TYPE::CONDITION:
							{
								if (widgetInfo->extraUserInfo[(int)type] == nullptr)
									widgetInfo->extraUserInfo[(int)type] = std::move(extraInfo);
								break;
							}
							default:
								break;
							}
						}
					}
				}; 

				((pushUserInfoLam(this, std::forward<Param>(var))), ...);
			}
			virtual ~JGuiWidgetInfo() = default;
		public:
			virtual JSupportGuiWidgetType GetSupportWidgetType()const noexcept = 0;
		public:
			bool IsExtraFunctionUser()const noexcept;
			JGuiExtraFunctionInfoMapName GetExtraFunctionMapName(const J_GUI_EXTRA_FUNCTION_TYPE type)const noexcept;
			JGuiExtraFunctionUserInfo* GetExtraFunctionUserInfo(const J_GUI_EXTRA_FUNCTION_TYPE type)const noexcept;
		};

		class JGuiInputInfo : public JGuiWidgetInfo
		{
		private:
			bool isEnterToReturn;
		private:
			J_PARAMETER_TYPE fixedParam = J_PARAMETER_TYPE::UnKnown;
			bool useFixedParam = false;
		public:
			JGuiInputInfo(const bool isEnterToReturn); 
			JGuiInputInfo(const bool isEnterToReturn, const J_PARAMETER_TYPE fixedParam);
			template<typename ...Param>
			JGuiInputInfo(const bool isEnterToReturn, const J_PARAMETER_TYPE fixedParam, Param... var)
				:JGuiWidgetInfo(std::forward<Param>(var)...),
				isEnterToReturn(isEnterToReturn),
				fixedParam(fixedParam)
			{}
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
			J_PARAMETER_TYPE GetFixedParameter()const noexcept;
		public:
			bool IsEnterToReturn()const noexcept;
			bool HasValidFixedParameter()const noexcept;
		};

		class JGuiCheckBoxInfo : public JGuiWidgetInfo
		{
		public: 
			template<typename ...Param>
			JGuiCheckBoxInfo(Param... var)
				:JGuiWidgetInfo(std::forward<Param>(var)...)
			{}
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
		};

		class JGuiSliderInfo : public JGuiWidgetInfo
		{
		private:
			//const int minValue;
			//const int maxValue;
			const float minValue;
			const float maxValue;
		private:
			const bool isSupportInput = true;
			const bool isVertical = false;
		public: 
			JGuiSliderInfo(const float minValue, const float maxValue);
			JGuiSliderInfo(const float minValue, const float maxValue, const bool isSupportInput);
			JGuiSliderInfo(const float minValue, const float maxValue, const bool isSupportInput, const bool isVertical);
			template<typename ...Param>
			JGuiSliderInfo(const float minValue, const float maxValue, const bool isSupportInput, const bool isVertical, Param... var)
				: JGuiWidgetInfo(std::forward<Param>(var)...),
				minValue(minValue),
				maxValue(maxValue),
				isSupportInput(isSupportInput),
				isVertical(isVertical)
			{}
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
		public:
			float GetMinValue()const noexcept;
			float GetMaxValue()const noexcept;
		public:
			bool IsSupportInput()const noexcept;
			bool IsVertical()const noexcept;
		};

		class JGuiColorPickerInfo : public JGuiWidgetInfo
		{
		private:
			const bool hasRgbInput;
		public:
			template<typename ...Param>
			JGuiColorPickerInfo(bool hasRgbInput, Param... var)
				: JGuiWidgetInfo(std::forward<Param>(var)...),
				hasRgbInput(hasRgbInput)
			{}
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
		public:
			bool HasRgbInput()const noexcept;
		};

		class JGuiSelectorInfo : public JGuiWidgetInfo
		{ 
		private:
			using GetElemntVecPtr = std::vector<JIdentifier*>(*)(JIdentifier*);
		private:  
			const J_GUI_SELECTOR_IMAGE imageType;
			const bool hasSizeSlider;
		private:
			GetElemntVecPtr getElementVecPtr = nullptr;
		public: 
			JGuiSelectorInfo(const J_GUI_SELECTOR_IMAGE imageType, const bool hasSizeSlider, GetElemntVecPtr getElementVecPtr = nullptr);
			template<typename ...Param>
			JGuiSelectorInfo(const J_GUI_SELECTOR_IMAGE imageType, const bool hasSizeSlider, GetElemntVecPtr getElementVecPtr, Param... var)
				: JGuiWidgetInfo(std::forward<Param>(var)...),
				imageType(imageType),
				hasSizeSlider(hasSizeSlider),
				getElementVecPtr(getElementVecPtr)
			{ }
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
		public:
			J_GUI_SELECTOR_IMAGE GetPreviewImageType()const noexcept;
			GetElemntVecPtr GetElementVecPtr()const noexcept;
		public:
			bool HasSizeSlider()const noexcept;
		};

		class JGuiReadOnlyTextInfo : public JGuiWidgetInfo
		{
		public:
			template<typename ...Param>
			JGuiReadOnlyTextInfo(Param... var)
				: JGuiWidgetInfo(std::forward<Param>(var)...)
			{}
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
		};

		class JGuiEnumComboBoxInfo : public JGuiWidgetInfo
		{
		private:
			const std::string enumFullName;
		public:   
			template<typename ...Param>
			JGuiEnumComboBoxInfo(const std::string enumName, Param... var)
				: JGuiWidgetInfo(std::forward<Param>(var)...),
				enumFullName(FindEnumFullName(enumName))
			{} 
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
			using CreateElementPtr = JIdentifier* (*)(JIdentifier*);
		private:
			//0.. static
			//1.. dynamic can add element
			const J_GUI_LIST_TYPE listType;
			const bool canDisplayElementGui; 
			CreateElementPtr createElementPtr = nullptr;
		public: 
			JGuiListInfo(const J_GUI_LIST_TYPE listType, const bool canDisplayElementGui, CreateElementPtr createElementPtr);
			template<typename ...Param>
			JGuiListInfo(const J_GUI_LIST_TYPE listType, const bool canDisplayElementGui, CreateElementPtr createElementPtr, Param... var)
				: JGuiWidgetInfo(std::forward<Param>(var)...),
				listType(listType),
				canDisplayElementGui(canDisplayElementGui),
				createElementPtr(createElementPtr)
			{}
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
		public:
			J_GUI_LIST_TYPE GetListType()const noexcept;
			bool CanDisplayElementGui()const noexcept;
		public: 
			CreateElementPtr GetCreateElementPtr()const noexcept;
		};

		//multi widget don't have extra
		//because inner widget have extra

		//mostly inner widget is used by condition
		class JGuiMultiWidgetBase : public JGuiWidgetInfo
		{
		public:
			JGuiMultiWidgetBase() = default;
		public:
			JSupportGuiWidgetType GetSupportWidgetType()const noexcept final;
			virtual uint GetIInnerWidgetCount()const noexcept = 0;
			virtual JGuiWidgetInfo* GetInnerWidget(const uint index)const noexcept = 0;
		};

#pragma endregion
	}
}