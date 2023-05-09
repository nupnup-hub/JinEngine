#pragma once 
#include"../JObject.h" 
#include"JComponentType.h"  

namespace JinEngine
{ 
	class JGameObject;
	class JComponentPrivate;
	struct CTypeHint;
	struct CTypeCommonFunc;
	struct CTypePrivateFunc;

	class JComponent : public JObject
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JComponent)
	public: 
		class InitData : public JObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			JUserPtr<JGameObject> owner = nullptr;
		public:
			InitData(const Core::JTypeInfo& typeInfo, const JUserPtr<JGameObject>& owner);
			InitData(const Core::JTypeInfo& typeInfo, const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner);
		public:
			bool IsValidData()const noexcept override;
		};
	protected: 
		class LoadData final : public Core::JDITypeDataBase
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadData)
		public:
			JUserPtr<JGameObject> owner = nullptr;
			std::wifstream& stream;
		public:
			LoadData(JUserPtr<JGameObject> owner, std::wifstream& stream);
			~LoadData();
		public:
			bool IsValidData()const noexcept final;
		};
		 
		class StoreData final : public JObject::StoreData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(StoreData)
		public:
			std::wofstream& stream;
		public:
			StoreData(JUserPtr<JComponent> comp, std::wofstream& stream);
		public:
			bool IsValidData()const noexcept final;
		};
	private:
		using JObject::SetName;
	private:
		friend class JComponentPrivate;
		class JComponentImpl;
	private:
		std::unique_ptr<JComponentImpl> impl;
	public:
		J_OBJECT_TYPE GetObjectType()const noexcept final;
		JUserPtr<JGameObject> GetOwner()const noexcept;
		virtual J_COMPONENT_TYPE GetComponentType()const noexcept = 0;
	public:
		virtual bool IsAvailableOverlap()const noexcept = 0;
		/*Defect Inspection When Use JComponent */
		virtual bool PassDefectInspection()const noexcept;
	protected:
		//step) DoActivate => RegistComponent
		void DoActivate()noexcept override;
		//step) DoDeActivate => DeRegistComponent
		void DoDeActivate()noexcept override;
	protected:
		static bool RegisterComponent(const JUserPtr<JComponent>& comp)noexcept;	//Register component in scene
		static bool DeRegisterComponent(const JUserPtr<JComponent>& comp)noexcept;		//DeRegister component in scene
		static bool ReRegisterComponent(const JUserPtr<JComponent>& comp)noexcept;		//ReRegister component in scene
	protected:
		static void RegisterCTypeInfo(const Core::JTypeInfo& typeInfo, const CTypeHint& cTypeHint, const CTypeCommonFunc& cTypeCFunc, const CTypePrivateFunc& cTypePFunc);
	public:
		//Compoent는 상위클래스에 초기화변수 이외에 오직 Parent변수만을 입력받아 초기화된다
		//상위클래스 변수는 모두 기본값을 가지고 있다 
		static std::unique_ptr<Core::JDITypeDataBase> CreateInitDIData(const J_COMPONENT_TYPE cType, JUserPtr<JGameObject> parent, std::unique_ptr<Core::JDITypeDataBase>&& parentClassInitData = nullptr);
		template<typename T, std::enable_if_t <std::is_base_of_v<JComponent, T> && !std::is_same_v<JComponent, T>, int> = 0>
		static std::unique_ptr<Core::JDITypeDataBase> CreateInitDIData(JUserPtr<JGameObject> parent, std::unique_ptr<Core::JDITypeDataBase>&& parentClassInitData = nullptr)
		{
			return CreateInitDIData(T::GetStaticComponentType(), parent, std::move(parentClassInitData));
		}
	protected:
		JComponent(const InitData& initData) noexcept;
		~JComponent();
	};
}