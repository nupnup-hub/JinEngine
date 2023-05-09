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
	using SetCFrameOffsetCallable = Core::JStaticCallable<void, JComponent*, JComponent*, bool>;

	using CreateInitDataCallable = Core::JStaticCallable<std::unique_ptr<Core::JDITypeDataBase>, JUserPtr<JGameObject>, std::unique_ptr<Core::JDITypeDataBase>&&>;
	struct CTypeHint
	{
	public:
		J_COMPONENT_TYPE thisType;
		bool hasFrameDirty;
		bool hasFrameOffset;
	public:
		CTypeHint() = default;
		CTypeHint(const J_COMPONENT_TYPE thisType, const bool hasFrameDirty = false, const bool hasFrameOffset = false);
		~CTypeHint();
	};

	struct CTypeCommonFunc
	{
	private:
		GetCTypeInfoCallable* getTypeInfo = nullptr; 
		IsAvailableOverlapCallable* isAvailableOverlapCallable = nullptr;
		CreateInitDataCallable* createIntiDataCallable = nullptr;
	public:
		CTypeCommonFunc() = default;
		CTypeCommonFunc(GetCTypeInfoCallable& getTypeInfo,  
			IsAvailableOverlapCallable& isAvailableOverlapCallable,
			CreateInitDataCallable& createIntiDataCallable);
		~CTypeCommonFunc();
	public:
		Core::JTypeInfo& CallGetTypeInfo();
		bool CallIsAvailableOverlapCallable();
		std::unique_ptr<Core::JDITypeDataBase> CallCreateInitDataCallable(JUserPtr<JGameObject> parent, std::unique_ptr<Core::JDITypeDataBase>&& parentClassInitData = nullptr);
	};

	struct CTypePrivateFunc
	{
	private:
		SetCFrameDirtyCallable* setFrameDirtyCallable = nullptr;
		SetCFrameOffsetCallable* setFrameOffsetCallable = nullptr;
	public:
		CTypePrivateFunc(SetCFrameDirtyCallable* setFrameDirtyCallable, SetCFrameOffsetCallable* setFrameOffsetCallable);
		CTypePrivateFunc() = default;
		~CTypePrivateFunc();
	public:
		void CallSetFrameDirty(JComponent* jComp);
		void CallSetFrameOffset(JComponent* jComp, JComponent* refComp, bool isCreated);
		SetCFrameDirtyCallable* GetSetFrameDirtyCallable();
		SetCFrameOffsetCallable* GetSetFrameOffsetCallable();
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
			JUserPtr<JGameObject> parent,
			std::unique_ptr<Core::JDITypeDataBase>&& parentClassInitData = nullptr);
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
		static void CallSetFrameOffset(JComponent* jComp, JComponent* refComp, bool isCreated);
		static SetCFrameDirtyCallable* GetSetFrameDirtyCallable(const J_COMPONENT_TYPE cType);
		static SetCFrameOffsetCallable* GetSetFrameOffsetCallable(const J_COMPONENT_TYPE cType);
	};
}