#pragma once
#include"JComponentType.h"
#include"../../Core/Func/Callable/JCallable.h"
#include"../../Core/DI/JDIDataBase.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include<memory>

namespace JinEngine
{
	class JComponent;
	class JGameObject;
	using GetCTypeInfoCallable = Core::JStaticCallable<Core::JTypeInfo&>; 
	using IsAvailableOverlapCallable = Core::JStaticCallable<bool>;
	using SetCFrameDirtyCallable = Core::JStaticCallable<void, JComponent*>; 
 
	//parent comp child is chid comp
	//child comp에 파생 class가 존재할시( ex) JBehavior) const Core::JTypeInfo& parameter로 식별한다
	//그외에 child comp에게는 유효하지 않다.
	using CreateInitDataCallable = Core::JStaticCallable<std::unique_ptr<Core::JDITypeDataBase>, const Core::JTypeInfo&, JUserPtr<JGameObject>, std::unique_ptr<Core::JDITypeDataBase>&&>;
	
	struct CTypeHint
	{
	public:
		J_COMPONENT_TYPE thisType;
		bool hasFrameDirty; 
	public:
		CTypeHint() = default;
		CTypeHint(const J_COMPONENT_TYPE thisType, const bool hasFrameDirty = false);
		~CTypeHint();
	};

	struct CTypeCommonFunc
	{
	private:
		GetCTypeInfoCallable* getTypeInfo = nullptr; 
		IsAvailableOverlapCallable* isAvailableOverlapCallable = nullptr;
		CreateInitDataCallable* createInitDataCallable = nullptr;
	public:
		CTypeCommonFunc() = default;
		CTypeCommonFunc(GetCTypeInfoCallable& getTypeInfo,  
			IsAvailableOverlapCallable& isAvailableOverlapCallable,
			CreateInitDataCallable& createInitDataCallable);
		~CTypeCommonFunc();
	public:
		Core::JTypeInfo& CallGetTypeInfo();
		bool CallIsAvailableOverlapCallable();
		std::unique_ptr<Core::JDITypeDataBase> CallCreateInitDataCallable(const Core::JTypeInfo& typeInfo,
			JUserPtr<JGameObject> owner,
			std::unique_ptr<Core::JDITypeDataBase>&& parentInitData = nullptr);
	};

	struct CTypePrivateFunc
	{ 
	private:
		SetCFrameDirtyCallable* setFrameDirtyCallable = nullptr; 
	public:
		CTypePrivateFunc(SetCFrameDirtyCallable* setFrameDirtyCallable);
		CTypePrivateFunc() = default;
		~CTypePrivateFunc();
	public:
		void CallSetFrameDirty(JComponent* jComp); 
		SetCFrameDirtyCallable* GetSetFrameDirtyCallable(); 
	};

	class CTypeRegister
	{
	private:
		friend class JComponent;
	private:
		static void RegisterCTypeInfo(const Core::JTypeInfo& info, 
			const CTypeHint& cTypeHint, 
			const CTypeCommonFunc& cTypeCFunc,
			const CTypePrivateFunc& cTypePFunc)noexcept;
	};

	class CTypeCommonCall
	{
	public:
		static CTypeHint GetCTypeHint(const J_COMPONENT_TYPE cType);
		static std::vector<CTypeHint> GetCTypeHint(const COMPONET_ALIGN_TYPE alignType = COMPONET_ALIGN_TYPE::NONE)noexcept;
	public:
		static Core::JTypeInfo& CallGetTypeInfo(const J_COMPONENT_TYPE cType);
		static bool CallIsAvailableOverlap(const J_COMPONENT_TYPE cType);
		static std::unique_ptr<Core::JDITypeDataBase> CallCreateInitDataCallable(const J_COMPONENT_TYPE cType,
			const Core::JTypeInfo& typeInfo, 
			JUserPtr<JGameObject> owner,
			std::unique_ptr<Core::JDITypeDataBase>&& parentInitData = nullptr);
	public:
		static J_COMPONENT_TYPE ConvertCompType(const Core::JTypeInfo& info);
	public:
		static bool NameOrder(const CTypeHint& a, const CTypeHint& b)noexcept; 
	};

	class CTypePrivateCall
	{
	private:
		friend class JGameObject;	//conp owner type
		friend class JScene;		//game obj owner type
	private:
		static void CallSetFrameDirty(JComponent* jComp); 
		static SetCFrameDirtyCallable* GetSetFrameDirtyCallable(const J_COMPONENT_TYPE cType); 
	};
}