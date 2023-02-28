#include"JComponentInterface.h"
#include"JComponent.h"
#include"../../Core/Singleton/JSingletonHolder.h"
#include<algorithm>

namespace JinEngine
{
	JCI::CTypeHint::CTypeHint(const J_COMPONENT_TYPE thisType, const bool hasFrameDirty, const bool hasFrameOffset)
		:thisType(thisType), hasFrameDirty(hasFrameDirty), hasFrameOffset(hasFrameOffset)
	{}
	JCI::CTypeHint::~CTypeHint() {}

	JCI::CTypeCommonFunc::CTypeCommonFunc(GetTypeNameCallable& getTypeName,
		GetTypeInfoCallable& getTypeInfo,
		IsAvailableOverlapCallable& isAvailableOverlapCallable)
		: getTypeName(&getTypeName), getTypeInfo(&getTypeInfo), isAvailableOverlapCallable(&isAvailableOverlapCallable)
	{}
	JCI::CTypeCommonFunc::~CTypeCommonFunc()
	{
		getTypeName = nullptr;
	}
	std::string JCI::CTypeCommonFunc::CallGetTypeName()
	{
		return (*getTypeName)(nullptr);
	}
	JinEngine::Core::JTypeInfo& JCI::CTypeCommonFunc::CallGetTypeInfo()
	{
		return (*getTypeInfo)(nullptr);
	}
	bool JCI::CTypeCommonFunc::CallIsAvailableOverlapCallable()
	{
		return (*isAvailableOverlapCallable)(nullptr);
	}

	JCI::CTypeInterfaceFunc::CTypeInterfaceFunc(SetFrameDirtyCallable* setFrameDirtyCallable, SetFrameOffsetCallable* setFrameOffsetCallable)
		:setFrameDirtyCallable(setFrameDirtyCallable), setFrameOffsetCallable(setFrameOffsetCallable)
	{}
	JCI::CTypeInterfaceFunc::~CTypeInterfaceFunc()
	{
		setFrameDirtyCallable = nullptr;
	}
	void JCI::CTypeInterfaceFunc::CallSetFrameDirty(JComponent& jComp)
	{
		(*setFrameDirtyCallable)(nullptr, jComp);
	}
	void JCI::CTypeInterfaceFunc::CallSetFrameOffset(JComponent& jComp, JComponent* refComp, bool isCreated)
	{
		(*setFrameOffsetCallable)(nullptr, jComp, refComp, isCreated);
	}
	JCI::SetFrameDirtyCallable* JCI::CTypeInterfaceFunc::GetSetFrameDirtyCallable()
	{
		return setFrameDirtyCallable;
	}
	JCI::SetFrameOffsetCallable* JCI::CTypeInterfaceFunc::GetSetFrameOffsetCallable()
	{
		return setFrameOffsetCallable;
	}

	class CTypeInfoData
	{
	public:
		std::vector<JCI::CTypeHint> cInfoStorage;
		std::vector<JCI::CTypeCommonFunc> cFuncStorage;
		std::vector<JCI::CTypeInterfaceFunc> cInterfaceStorage;
	public:
		CTypeInfoData()
		{
			cInfoStorage.resize((uint)J_COMPONENT_TYPE::COUNT);
			cFuncStorage.resize((uint)J_COMPONENT_TYPE::COUNT);
			cInterfaceStorage.resize((uint)J_COMPONENT_TYPE::COUNT);
		}
	};
	using CTypeInfo = Core::JSingletonHolder<CTypeInfoData>;

	JCI::CTypeHint JComponentInterface::GetCTypeHint(const J_COMPONENT_TYPE cType)
	{
		return CTypeInfo::Instance().cInfoStorage[(uint)cType];
	}
	std::vector<JCI::CTypeHint> JComponentInterface::GetCTypeHint(const COMPONET_ALIGN_TYPE alignType)noexcept
	{
		switch (alignType)
		{
		case JinEngine::COMPONET_ALIGN_TYPE::NONE:
		{
			return CTypeInfo::Instance().cInfoStorage;
		}
		case JinEngine::COMPONET_ALIGN_TYPE::NAME:
		{
			std::vector<JCI::CTypeHint> infoCopy = CTypeInfo::Instance().cInfoStorage;
			sort(infoCopy.begin(), infoCopy.end(), NameOrder);
			return infoCopy;
		}
		default:
		{
			return CTypeInfo::Instance().cInfoStorage;
		}
		}
	}
	JinEngine::Core::JTypeInfo& JComponentInterface::CallGetTypeInfo(const J_COMPONENT_TYPE cType)
	{
		return CTypeInfo::Instance().cFuncStorage[(uint)cType].CallGetTypeInfo();
	}
	std::string JComponentInterface::CallGetTypeName(const J_COMPONENT_TYPE cType)
	{
		return CTypeInfo::Instance().cFuncStorage[(uint)cType].CallGetTypeName();
	}
	bool JComponentInterface::CallIsAvailableOverlap(const J_COMPONENT_TYPE cType)
	{
		return CTypeInfo::Instance().cFuncStorage[(uint)cType].CallIsAvailableOverlapCallable();
	}
	void JComponentInterface::CallSetFrameDirty(JComponent& jComp)
	{
		return CTypeInfo::Instance().cInterfaceStorage[(uint)jComp.GetComponentType()].CallSetFrameDirty(jComp);
	}
	void JComponentInterface::CallSetFrameOffset(JComponent& jComp, JComponent* refComp, bool isCreated)
	{
		return CTypeInfo::Instance().cInterfaceStorage[(uint)jComp.GetComponentType()].CallSetFrameOffset(jComp, refComp, isCreated);
	}
	JCI::SetFrameDirtyCallable* JComponentInterface::GetSetFrameDirtyCallable(const J_COMPONENT_TYPE cType)
	{
		return CTypeInfo::Instance().cInterfaceStorage[(uint)cType].GetSetFrameDirtyCallable();
	}
	JCI::SetFrameOffsetCallable* JComponentInterface::GetSetFrameOffsetCallable(const J_COMPONENT_TYPE cType)
	{
		return CTypeInfo::Instance().cInterfaceStorage[(uint)cType].GetSetFrameOffsetCallable();
	}
	bool JComponentInterface::NameOrder(const CTypeHint& a, const CTypeHint& b)noexcept
	{
		return CTypeInfo::Instance().cFuncStorage[(uint)a.thisType].CallGetTypeName() < CTypeInfo::Instance().cFuncStorage[(uint)b.thisType].CallGetTypeName();
	}
	void JComponentInterface::RegisterTypeInfo(const JCI::CTypeHint& cTypeHint, const JCI::CTypeCommonFunc& cTypeCFunc, const JCI::CTypeInterfaceFunc& cTypeIFunc)noexcept
	{
		CTypeInfo::Instance().cInfoStorage[(uint)cTypeHint.thisType] = cTypeHint;
		CTypeInfo::Instance().cFuncStorage[(uint)cTypeHint.thisType] = cTypeCFunc;
		CTypeInfo::Instance().cInterfaceStorage[(uint)cTypeHint.thisType] = cTypeIFunc;
	}
	JComponentInterface::JComponentInterface(const std::wstring& cTypeName, const size_t guid, J_OBJECT_FLAG flag)
		:JObject(cTypeName, guid, flag)
	{}
}