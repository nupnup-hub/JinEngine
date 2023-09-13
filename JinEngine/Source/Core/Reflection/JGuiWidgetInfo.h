#pragma once
#include"JParameterType.h"
#include"JGuiWidgetType.h"
#include"../JCoreEssential.h"      
#include"../Utility/JTypeTraitUtility.h" 
#include"../Pointer/JOwnerPtr.h"
#include "../../../ThirdParty/Loki/Typelist/Typelist.h"
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
			TABLE,
			CONDITION, 
			COUNT,
		}; 

		enum class J_GUI_EXTRA_CONDITION_TYPE
		{
			BOOLEAN_PARAM,
			ENUM_PARAM
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
		 
		using JGuiFlag = int;
		using JEnum = int;
		using JGuiExtraFunctionInfoMapName = std::string;  
		namespace Constants
		{ 
			static JGuiExtraFunctionInfoMapName invalidName = "";

			//static constexpr int guiSelectorNoneImage = 0;
		//	static constexpr int guiSelectorIconImage = 1;
		//	static constexpr int guiSelectorImage = 2;

			static constexpr int guiStaticList = 0;
			static constexpr int guiDynamicList = 1;
		} 

		//WidgetInfo class들은 TypeInfo를 이용해 ChildType으로 Convert할수없으므로
		//TypeList를 사용한여 Convert한다

#pragma region Extra
		//It is used for extra action of widget 
		//1. groupping
		//2. condition 
		
		//attibute has order dependency
		//groupping is always called at first
		 
		class JGuiExtraFunctionInfo;
		class JGuiExtraFunctionUserInfo
		{  
			DECLARE_BASE_CLASS_TYPE_LIST(JGuiExtraFunctionUserInfo)
		private:
			template<typename T> friend class JOwnerPtr;
		private:
			const JGuiExtraFunctionInfoMapName infoMapName = Constants::invalidName;
		public:
			JGuiExtraFunctionUserInfo(const JGuiExtraFunctionInfoMapName infoMapName);
		protected:
			virtual ~JGuiExtraFunctionUserInfo() = default;
		public:
			JGuiExtraFunctionInfoMapName GetExtraFunctionName()const noexcept; 
			JUserPtr<JGuiExtraFunctionInfo> GetExtraFunctionInfo()const noexcept;
		};

		class JGuiTableUserInfo : public JGuiExtraFunctionUserInfo
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiTableUserInfo, JGuiExtraFunctionUserInfo) 
		private:
			const uint useColumnCount; 
		public:
			JGuiTableUserInfo(const JGuiExtraFunctionInfoMapName infoMapName,
				const uint useColumnCount, 
				const bool canAddRowCount);
		public:
			uint GetUseColumnCount()const noexcept; 
		};

		class JGuiParamConditionUserInfoBase : public JGuiExtraFunctionUserInfo
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiParamConditionUserInfoBase, JGuiExtraFunctionUserInfo)
		private:
			//user는 ref param을 소유하는 class 가아닌경우 ref param을 참조하기위한 추가 정보가 필요하다
			struct RefParamInfo
			{
			public:
				//ref param을 소유하고 있는 class에 typeInfo로부터 param을 불러오기위한 name
				const std::string refParamOwnerName;
			public:
				RefParamInfo(const std::string& refParamOwnerName);
			};
		private:
			std::unique_ptr<RefParamInfo> refParamInfo; 
		public:
			JGuiParamConditionUserInfoBase(const JGuiExtraFunctionInfoMapName infoMapName);
			JGuiParamConditionUserInfoBase(const JGuiExtraFunctionInfoMapName infoMapName, const std::string refParamOwnerName);
		public: 
			std::string GetRefParamOwnerName()const noexcept;
			bool IsOwnRefParam()const noexcept;
		};

		class JGuiBoolParmConditionUserInfo : public JGuiParamConditionUserInfoBase
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiBoolParmConditionUserInfo, JGuiParamConditionUserInfoBase)
		private:
			const bool condValue;
		public:
			JGuiBoolParmConditionUserInfo(const JGuiExtraFunctionInfoMapName infoMapName, const bool value);
			JGuiBoolParmConditionUserInfo(const JGuiExtraFunctionInfoMapName infoMapName, const std::string refParamOwnerName, const bool value);
		public:
			bool OnTrigger(const bool value)const noexcept;
		};

		class JGuiEnumParamConditionUserInfoInterface : public JGuiParamConditionUserInfoBase
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiEnumParamConditionUserInfoInterface, JGuiParamConditionUserInfoBase)
		public:
			JGuiEnumParamConditionUserInfoInterface(const JGuiExtraFunctionInfoMapName infoMapName);
			JGuiEnumParamConditionUserInfoInterface(const JGuiExtraFunctionInfoMapName infoMapName, const std::string refParamOwnerName);
		public:
			virtual bool OnTrigger(const JEnum value)const noexcept = 0; 
		};

		template<int buffCount>
		class JGuiEnumParamConditionUserInfo : public JGuiEnumParamConditionUserInfoInterface
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiEnumParamConditionUserInfo, JGuiEnumParamConditionUserInfoInterface)
		private:
			//enum need to declare enum not enum class
			JEnum condValue[buffCount];
		public:
			template<typename ...Param>
			JGuiEnumParamConditionUserInfo(const JGuiExtraFunctionInfoMapName infoMapName, const std::string refParamOwnerName, Param... value)
				: JGuiEnumParamConditionUserInfoInterface(infoMapName, refParamOwnerName)
			{
				int i = 0;
				((condValue[i++] = (JEnum)value), ...);
			}
		public:
			bool OnTrigger(const JEnum value)const noexcept final
			{
				for (uint i = 0; i < buffCount; ++i)
				{
					if (value == condValue[i] || (condValue[i] & value) > 0)
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

		class JGuiExtraFunctionInfo
		{
			DECLARE_BASE_CLASS_TYPE_LIST(JGuiExtraFunctionInfo)
		private:
			template<typename T> friend class JOwnerPtr;
		private:
			const std::string extraFunctionName;
		public:
			JGuiExtraFunctionInfo(const std::string extraFunctionName);
		protected:
			virtual ~JGuiExtraFunctionInfo() = default;
		public:
			std::string GetName()const noexcept;
			virtual J_GUI_EXTRA_FUNCTION_TYPE GetExtraFunctionType()const noexcept = 0; 
		};

		class JGuiTableInfo : public JGuiExtraFunctionInfo
		{ 
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiTableInfo, JGuiExtraFunctionInfo)
		private:
			const bool isFirstColumnGuide;
		private:
			std::vector<std::string> columnGuide;
			uint columnCount = 0;
			uint rowCount = 0;
		public:  
			JGuiTableInfo(const bool isFirstColumnGuide, const std::vector<std::string>& columnGuide)
				:JGuiExtraFunctionInfo(Constants::invalidName), isFirstColumnGuide(isFirstColumnGuide), columnGuide(columnGuide)
			{
				columnCount = (uint)columnGuide.size();
			}
			template<typename ...Param>
			JGuiTableInfo(const std::string extraFunctionName, const bool isFirstColumnGuide, Param... var)
				: JGuiExtraFunctionInfo(extraFunctionName), isFirstColumnGuide(isFirstColumnGuide)
			{
				((columnGuide.push_back(var)), ...);
				columnCount = (uint)columnGuide.size();
			}
		public:
			bool IsFirstColunmGuide()const noexcept;
		public:  
			J_GUI_EXTRA_FUNCTION_TYPE GetExtraFunctionType()const noexcept final;
			std::string GetColumnGuide(const uint index)const noexcept;
			uint GetColumnCount()const noexcept;
			uint GetRowCount()const noexcept;
		public:
			void NotifyAddMember(JGuiExtraFunctionUserInfo* user);
		};

		//Set gui enum condition 
		//scenario
		//1) register enum info 
		//2) user search enum info and get reference parameter
		//3) compare reference parameter value and user registerd condition value
		 
		//Caution! enum need to declare enum not enum class
		//or REGISTER_ENUM, REGISTER_ENUM_CLASS

		class JGuiConditionInfo : public JGuiExtraFunctionInfo
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiConditionInfo, JGuiExtraFunctionInfo)
		public:
			JGuiConditionInfo(const std::string extraFunctionName);
		public:
			virtual J_GUI_EXTRA_CONDITION_TYPE GetConditionType()const noexcept = 0;
			J_GUI_EXTRA_FUNCTION_TYPE GetExtraFunctionType()const noexcept;
		};

		class JGuiBoolParamConditionInfo : public JGuiConditionInfo
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiBoolParamConditionInfo, JGuiConditionInfo)
		private: 
			const std::string refName;		//condition trigger parameter
			const bool isRefMethod;
		public:
			JGuiBoolParamConditionInfo(const std::string& extraFunctionName, 
				const std::string& refParamName,
				const bool isRefMethod);
		public:
			J_GUI_EXTRA_CONDITION_TYPE GetConditionType()const noexcept final; 
			std::string GetRefName()const noexcept;
		public:
			bool IsRefMethod()const noexcept;
		};

		class JGuiEnumParamConditionInfo : public JGuiConditionInfo
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiEnumParamConditionInfo, JGuiConditionInfo)
		private:
			const std::string enumName;
			const std::string refParamName;		//condition trigger parameter
		public:
			JGuiEnumParamConditionInfo(const std::string& extraFunctionName, const std::string& enumName, const std::string& refParamName);
		public:
			J_GUI_EXTRA_CONDITION_TYPE GetConditionType()const noexcept final;
			std::string GetEnumName()const noexcept;
			std::string GetRefParamName()const noexcept;
		};

		class JGuiExtraFunctionInfoMap
		{
		public:
			static void Register(JOwnerPtr<JGuiExtraFunctionInfo>&& info);
		};
#pragma endregion

#pragma region Widget

		class JGuiWidgetInfo
		{
			DECLARE_BASE_CLASS_TYPE_LIST(JGuiWidgetInfo)
		private:
			template<typename T> friend class JOwnerPtr;
		private: 
			JOwnerPtr<JGuiExtraFunctionUserInfo> extraUserInfo[(int)J_GUI_EXTRA_FUNCTION_TYPE::COUNT];
		private:
			bool isExtraFunctionUser = false;
		public: 
			JGuiWidgetInfo() = default;
			template<typename ...Param>
			JGuiWidgetInfo(Param... var)
			{
				auto pushUserInfoLam = [](JGuiWidgetInfo* widgetInfo, JOwnerPtr<JGuiExtraFunctionUserInfo>&& ownerInfo)
				{
					if (ownerInfo != nullptr)
					{
						//Register if has extra info
						JUserPtr<JGuiExtraFunctionInfo> info = ownerInfo->GetExtraFunctionInfo();
						if (info != nullptr)
						{ 			 
							const J_GUI_EXTRA_FUNCTION_TYPE type = info->GetExtraFunctionType();
							widgetInfo->isExtraFunctionUser = true;
							switch (type)
							{ 
							case JinEngine::Core::J_GUI_EXTRA_FUNCTION_TYPE::TABLE:
							{
								if (widgetInfo->extraUserInfo[(int)type] == nullptr)
									widgetInfo->extraUserInfo[(int)type] = std::move(ownerInfo);
								break;
							}
							case JinEngine::Core::J_GUI_EXTRA_FUNCTION_TYPE::CONDITION:
							{
								if (widgetInfo->extraUserInfo[(int)type] == nullptr)
									widgetInfo->extraUserInfo[(int)type] = std::move(ownerInfo);
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
		protected:
			virtual ~JGuiWidgetInfo() = default;
		public:
			virtual J_GUI_WIDGET_TYPE GetSupportWidgetType()const noexcept = 0;
		public:
			bool IsExtraFunctionUser()const noexcept;
			JGuiExtraFunctionInfoMapName GetExtraFunctionMapName(const J_GUI_EXTRA_FUNCTION_TYPE type)const noexcept;
			JUserPtr<JGuiExtraFunctionUserInfo> GetExtraFunctionUserInfo(const J_GUI_EXTRA_FUNCTION_TYPE type)const noexcept;
		};
		class JGuiInputInfo : public JGuiWidgetInfo
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiInputInfo, JGuiWidgetInfo)
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
			J_GUI_WIDGET_TYPE GetSupportWidgetType()const noexcept final;
			J_PARAMETER_TYPE GetFixedParameter()const noexcept;
		public:
			bool IsEnterToReturn()const noexcept;
			bool HasValidFixedParameter()const noexcept;
		};
		class JGuiCheckBoxInfo : public JGuiWidgetInfo
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiCheckBoxInfo, JGuiWidgetInfo)
		public: 
			template<typename ...Param>
			JGuiCheckBoxInfo(Param... var)
				:JGuiWidgetInfo(std::forward<Param>(var)...)
			{}
		public:
			J_GUI_WIDGET_TYPE GetSupportWidgetType()const noexcept final;
		};
		class JGuiSliderInfo : public JGuiWidgetInfo
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiSliderInfo, JGuiWidgetInfo)
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
			J_GUI_WIDGET_TYPE GetSupportWidgetType()const noexcept final;
		public:
			float GetMinValue()const noexcept;
			float GetMaxValue()const noexcept;
		public:
			bool IsSupportInput()const noexcept;
			bool IsVertical()const noexcept;
		};
		class JGuiColorPickerInfo : public JGuiWidgetInfo
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiColorPickerInfo, JGuiWidgetInfo)
		private:
			const bool hasRgbInput;
		public:
			template<typename ...Param>
			JGuiColorPickerInfo(bool hasRgbInput, Param... var)
				: JGuiWidgetInfo(std::forward<Param>(var)...),
				hasRgbInput(hasRgbInput)
			{}
		public:
			J_GUI_WIDGET_TYPE GetSupportWidgetType()const noexcept final;
		public:
			bool HasRgbInput()const noexcept;
		};
		class JGuiSelectorInfo : public JGuiWidgetInfo
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiSelectorInfo, JGuiWidgetInfo)
		private:
			using GetElemntVecPtr = std::vector<JUserPtr<JIdentifier>>(*)(JUserPtr<JIdentifier>);
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
			J_GUI_WIDGET_TYPE GetSupportWidgetType()const noexcept final;
		public:
			J_GUI_SELECTOR_IMAGE GetPreviewImageType()const noexcept;
			GetElemntVecPtr GetElementVecPtr()const noexcept;
		public:
			bool HasSizeSlider()const noexcept;
		};
		class JGuiReadOnlyTextInfo : public JGuiWidgetInfo
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiReadOnlyTextInfo, JGuiWidgetInfo)
		public:
			template<typename ...Param>
			JGuiReadOnlyTextInfo(Param... var)
				: JGuiWidgetInfo(std::forward<Param>(var)...)
			{}
		public:
			J_GUI_WIDGET_TYPE GetSupportWidgetType()const noexcept final;
		};
		class JGuiEnumComboBoxInfo : public JGuiWidgetInfo
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiEnumComboBoxInfo, JGuiWidgetInfo)
		private:
			const std::string enumFullName;
			const std::string displayCommand;
		public:  
			JGuiEnumComboBoxInfo(const std::string enumFullName);
			template<typename ...Param>
			JGuiEnumComboBoxInfo(const std::string enumFullName, const std::string displayCommand, Param... var)
				: JGuiWidgetInfo(std::forward<Param>(var)...),
				enumFullName(enumFullName), displayCommand(displayCommand)
			{} 
		public:
			J_GUI_WIDGET_TYPE GetSupportWidgetType()const noexcept final;
		public:
			std::string GetEnumFullName()const noexcept; 
			std::string GetDisplayCommand()const noexcept;
		};	 
		class JGuiListInfo : public JGuiWidgetInfo
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiListInfo, JGuiWidgetInfo)
		public:  
			//새롭게 모든 JIdenfier를 상속받는 object는 IdenCreator에서 생성된다
			//이는 IdenCreator만이 BeginCrete에 접근할 수 있기때문이다
			//Instance는 ownerPtr로 typeInfo class에 저장되고 pointer를 반환한다
			using CreateElementPtr = JUserPtr<JIdentifier>(*)(JUserPtr<JIdentifier>);
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
			J_GUI_WIDGET_TYPE GetSupportWidgetType()const noexcept final;
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
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiMultiWidgetBase, JGuiWidgetInfo)
		public:
			JGuiMultiWidgetBase() = default;
		public:
			J_GUI_WIDGET_TYPE GetSupportWidgetType()const noexcept final;
			virtual uint GetIInnerWidgetCount()const noexcept = 0;
			virtual JGuiWidgetInfo* GetInnerWidget(const uint index)const noexcept = 0;
		};

#pragma endregion
	}
}