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


#pragma once
#include"JParameterType.h"
#include"JGuiWidgetType.h"     
#include"../Utility/JTypeTraitUtility.h" 
#include"../Utility/JSimpleCompiler.h"
#include"../Pointer/JOwnerPtr.h"
#include "../../../ThirdParty/Loki/Typelist/Typelist.h"
 
namespace JinEngine
{
	namespace Core
	{
		class JIdentifier; 
		enum class J_GUI_EXTRA_FUNCTION_TYPE
		{ 
			TABLE,		//Unuse
			CONDITION,	//widget display condition
			GROUP,		//manage widget list by group(open and close ...)
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
				const std::string refOwnerName;
			public:
				RefParamInfo(const std::string& refOwnerName);
			};
		private:
			std::unique_ptr<RefParamInfo> refParamInfo; 
		public:
			JGuiParamConditionUserInfoBase(const JGuiExtraFunctionInfoMapName infoMapName);
			JGuiParamConditionUserInfoBase(const JGuiExtraFunctionInfoMapName infoMapName, const std::string& refOwnerName);
		public: 
			std::string GetRefOwnerName()const noexcept;
			bool IsOwnRefParam()const noexcept;
		};
		class JGuiBoolParmConditionUserInfo : public JGuiParamConditionUserInfoBase
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiBoolParmConditionUserInfo, JGuiParamConditionUserInfoBase)
		private:
			const bool condValue;
		public:
			JGuiBoolParmConditionUserInfo(const JGuiExtraFunctionInfoMapName infoMapName, const bool value);
			JGuiBoolParmConditionUserInfo(const JGuiExtraFunctionInfoMapName infoMapName, const std::string& refParamOwnerName, const bool value);
		public:
			bool OnTrigger(const bool value)const noexcept;
		};
		class JGuiEnumParamConditionUserInfoInterface : public JGuiParamConditionUserInfoBase
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiEnumParamConditionUserInfoInterface, JGuiParamConditionUserInfoBase)
		public:
			JGuiEnumParamConditionUserInfoInterface(const JGuiExtraFunctionInfoMapName infoMapName);
			JGuiEnumParamConditionUserInfoInterface(const JGuiExtraFunctionInfoMapName infoMapName, const std::string& refParamOwnerName);
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
			JGuiEnumParamConditionUserInfo(const JGuiExtraFunctionInfoMapName infoMapName, const std::string& refOwnerName, Param... value)
				: JGuiEnumParamConditionUserInfoInterface(infoMapName, refOwnerName)
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

		class JGuiGroupUserInfo : public JGuiExtraFunctionUserInfo
		{
			DECLARE_CHILD_CLASS_TYPE_LIST(JGuiGroupUserInfo, JGuiExtraFunctionUserInfo)
		public:
			JGuiGroupUserInfo(const JGuiExtraFunctionInfoMapName infoMapName);
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
			int userCount = 0;
		public:
			JGuiExtraFunctionInfo(const std::string& extraFunctionName);
		protected:
			virtual ~JGuiExtraFunctionInfo() = default;
		public:
			std::string GetName()const noexcept;
			int GetUserCount()const noexcept;
			virtual J_GUI_EXTRA_FUNCTION_TYPE GetExtraFunctionType()const noexcept = 0; 
		public:
			virtual void NotifyAddMember(JGuiExtraFunctionUserInfo* user);
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
			JGuiTableInfo(const std::string& extraFunctionName, const bool isFirstColumnGuide, Param... var)
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
			void NotifyAddMember(JGuiExtraFunctionUserInfo* user)final;
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
			JGuiConditionInfo(const std::string& extraFunctionName);
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
			const std::string refName;		//condition trigger parameter
			const bool isRefMethod;
		public:
			JGuiEnumParamConditionInfo(const std::string& extraFunctionName, 
				const std::string& enumName,
				const std::string& refName,
				const bool isRefMethod);
		public:
			J_GUI_EXTRA_CONDITION_TYPE GetConditionType()const noexcept final;
			std::string GetEnumName()const noexcept;
			std::string GetRefName()const noexcept;
		public:
			bool IsRefMethod()const noexcept;
		};

		class JGuiGroupInfo : public JGuiExtraFunctionInfo
		{ 
		private: 
			bool isOpen = false;
		public:
			JGuiGroupInfo(const std::string& extraFunctionName);
		public: 
			J_GUI_EXTRA_FUNCTION_TYPE GetExtraFunctionType()const noexcept;
		public:
			void SetOpenTrigger(const bool value)noexcept;
		public:
			bool IsOpen()const noexcept; 
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
			std::vector<JOwnerPtr<JGuiExtraFunctionUserInfo>> invalidExtraUserInfo;
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
							widgetInfo->SetExtraFunctionUserInfo(info, std::move(ownerInfo));
						else
							widgetInfo->invalidExtraUserInfo.push_back(std::move(ownerInfo));
					}
				}; 

				((pushUserInfoLam(this, std::forward<Param>(var))), ...);
			}
		protected:
			virtual ~JGuiWidgetInfo() = default;
		public:
			virtual J_GUI_WIDGET_TYPE GetSupportWidgetType()const noexcept = 0;
			JGuiExtraFunctionInfoMapName GetExtraFunctionMapName(const J_GUI_EXTRA_FUNCTION_TYPE type)const noexcept;
			JUserPtr<JGuiExtraFunctionUserInfo> GetExtraFunctionUserInfo(const J_GUI_EXTRA_FUNCTION_TYPE type)const noexcept;
		private:
			void SetExtraFunctionUserInfo(const JUserPtr<JGuiExtraFunctionInfo>& info, JOwnerPtr<JGuiExtraFunctionUserInfo>&& userInfo);
		public:
			bool IsExtraFunctionUser()const noexcept;
			bool IsExtraFunctionUser(const J_GUI_EXTRA_FUNCTION_TYPE type)const noexcept;
		public:
			/**
			* JGuiWidgetInfo중에 일부 JGuiExtraFunctionUserInfo는 다른 class에 JGuiExtraFunctionInfo를 참조하며
			* 이는 컴파일 순서에 따라서 JGuiWidgetInfo생성 단계에서 해당하는 JGuiExtraFunctionUserInfo가 invalid처리될 경우가
			* 있음을 의미한다.
			* 그런 invalid처리된 .JGuiExtraFunctionUserInfo들이 다시한번 할당될수있는 기회를 부여하는 함수이다.
			* (이미 해당하는 type에 JGuiExtraFunctionUserInfo가 있을경우 할당되지않는다.)
			*/
			void TryReSettingExtraUserInfo();
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
			const uint floatDigit = 3;
		private:
			const bool isSupportInput = true;
			const bool isVertical = false;
		public: 
			JGuiSliderInfo(const float minValue, const float maxValue);
			JGuiSliderInfo(const float minValue, const float maxValue, const bool isSupportInput);
			JGuiSliderInfo(const float minValue, const float maxValue, const bool isSupportInput, const bool isVertical);
			JGuiSliderInfo(const float minValue, const float maxValue, const bool isSupportInput, const bool isVertical, const uint floatDigit);
			template<typename ...Param>
			JGuiSliderInfo(const float minValue, const float maxValue, const bool isSupportInput, const bool isVertical, const uint floatDigit, Param... var)
				: JGuiWidgetInfo(std::forward<Param>(var)...),
				minValue(minValue),
				maxValue(maxValue),
				isSupportInput(isSupportInput),
				isVertical(isVertical),
				floatDigit(floatDigit)
			{}
		public:
			J_GUI_WIDGET_TYPE GetSupportWidgetType()const noexcept final;
		public:
			float GetMinValue()const noexcept;
			float GetMaxValue()const noexcept;
			uint GetFloatDigit()const noexcept;
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
			const bool isReadOnly;
			const bool hasSizeSlider; 
		private:
			GetElemntVecPtr getElementVecPtr = nullptr;
		public: 
			JGuiSelectorInfo(const J_GUI_SELECTOR_IMAGE imageType, const bool isReadOnly, const bool hasSizeSlider, GetElemntVecPtr getElementVecPtr = nullptr);
			template<typename ...Param>
			JGuiSelectorInfo(const J_GUI_SELECTOR_IMAGE imageType, const bool isReadOnly, const bool hasSizeSlider, GetElemntVecPtr getElementVecPtr, Param... var)
				: JGuiWidgetInfo(std::forward<Param>(var)...),
				imageType(imageType),
				isReadOnly(isReadOnly),
				hasSizeSlider(hasSizeSlider),
				getElementVecPtr(getElementVecPtr)
			{ }
		public:
			J_GUI_WIDGET_TYPE GetSupportWidgetType()const noexcept final;
		public:
			J_GUI_SELECTOR_IMAGE GetPreviewImageType()const noexcept;
			GetElemntVecPtr GetElementVecPtr()const noexcept;
		public:
			bool IsReadOnly()const noexcept;
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
			const std::vector<JCommandToken> token;
		public:  
			JGuiEnumComboBoxInfo(const std::string& enumFullName);
			template<typename ...Param>
			JGuiEnumComboBoxInfo(const std::string& enumFullName, const std::string& displayCommand, Param... var)
				: JGuiWidgetInfo(std::forward<Param>(var)...),
				enumFullName(enumFullName), token(CreateToken(displayCommand))
			{} 
		public:
			J_GUI_WIDGET_TYPE GetSupportWidgetType()const noexcept final;
			std::string GetEnumFullName()const noexcept;  
			const std::vector<JCommandToken>& GetToken()const noexcept;
		private:
			static std::vector<JCommandToken> CreateToken(std::string displayCommand);
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