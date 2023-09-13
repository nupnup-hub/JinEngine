#include"JComponentHint.h"
#include"JComponent.h" 
#include<algorithm>

namespace JinEngine
{
	CTypeHint::CTypeHint(const J_COMPONENT_TYPE thisType, const bool hasFrameDirty)
		:thisType(thisType), 
		hasFrameDirty(hasFrameDirty)
	{}
	CTypeHint::~CTypeHint() {}

	CTypeCommonFunc::CTypeCommonFunc(GetCTypeInfoCallable& getTypeInfo,
		IsAvailableOverlapCallable& isAvailableOverlapCallable,
		CreateInitDataCallable& createInitDataCallable)
		: getTypeInfo(&getTypeInfo), 
		isAvailableOverlapCallable(&isAvailableOverlapCallable),
		createInitDataCallable(&createInitDataCallable)
	{}
	CTypeCommonFunc::~CTypeCommonFunc()
	{
	}
	JinEngine::Core::JTypeInfo& CTypeCommonFunc::CallGetTypeInfo()
	{
		return (*getTypeInfo)(nullptr);
	}
	bool CTypeCommonFunc::CallIsAvailableOverlapCallable()
	{
		return (*isAvailableOverlapCallable)(nullptr);
	}
	std::unique_ptr<Core::JDITypeDataBase> CTypeCommonFunc::CallCreateInitDataCallable(const Core::JTypeInfo& typeInfo,
		JUserPtr<JGameObject> owner,
		std::unique_ptr<Core::JDITypeDataBase>&& parentInitData)
	{
		return (*createInitDataCallable)(nullptr, typeInfo, owner, std::move(parentInitData));
	}

	CTypePrivateFunc::CTypePrivateFunc(SetCFrameDirtyCallable* setFrameDirtyCallable)
		:setFrameDirtyCallable(setFrameDirtyCallable)
	{}
	CTypePrivateFunc::~CTypePrivateFunc()
	{
		setFrameDirtyCallable = nullptr;
	}
	void CTypePrivateFunc::CallSetFrameDirty(JComponent* jComp)
	{
		(*setFrameDirtyCallable)(nullptr, jComp);
	}
	SetCFrameDirtyCallable* CTypePrivateFunc::GetSetFrameDirtyCallable()
	{
		return setFrameDirtyCallable;
	}

	struct CTypeInfoData
	{
	public:
		std::vector<CTypeHint> hintStorage;
		std::vector<CTypeCommonFunc> cFuncStorage;
		std::vector<CTypePrivateFunc> pFuncStorage;
	public:
		std::unordered_map<size_t, J_COMPONENT_TYPE> typeMap;	//key is typeInfo typeGuid
	public:
		CTypeInfoData()
		{
			hintStorage.resize((uint)J_COMPONENT_TYPE::COUNT);
			cFuncStorage.resize((uint)J_COMPONENT_TYPE::COUNT);
			pFuncStorage.resize((uint)J_COMPONENT_TYPE::COUNT);
		}
	};
	using CTypeInfo = Core::JSingletonHolder<CTypeInfoData>;

	void CTypeRegister::RegisterCTypeInfo(const Core::JTypeInfo& info, 
		const CTypeHint& cTypeHint,
		const CTypeCommonFunc& cTypeCFunc,
		const CTypePrivateFunc& cTypePFunc)noexcept
	{
		CTypeInfo::Instance().hintStorage[(uint)cTypeHint.thisType] = cTypeHint;
		CTypeInfo::Instance().cFuncStorage[(uint)cTypeHint.thisType] = cTypeCFunc;
		CTypeInfo::Instance().pFuncStorage[(uint)cTypeHint.thisType] = cTypePFunc;
		CTypeInfo::Instance().typeMap.emplace(info.TypeGuid(), cTypeHint.thisType);
	}


	CTypeHint CTypeCommonCall::GetCTypeHint(const J_COMPONENT_TYPE cType)
	{
		return CTypeInfo::Instance().hintStorage[(uint)cType];
	}
	std::vector<CTypeHint> CTypeCommonCall::GetCTypeHint(const COMPONET_ALIGN_TYPE alignType)noexcept
	{
		switch (alignType)
		{
		case JinEngine::COMPONET_ALIGN_TYPE::NONE:
		{
			return CTypeInfo::Instance().hintStorage;
		}
		case JinEngine::COMPONET_ALIGN_TYPE::NAME:
		{
			std::vector<CTypeHint> infoCopy = CTypeInfo::Instance().hintStorage;
			sort(infoCopy.begin(), infoCopy.end(), NameOrder);
			return infoCopy;
		}
		default:
		{
			return CTypeInfo::Instance().hintStorage;
		}
		}
	}
	Core::JTypeInfo& CTypeCommonCall::CallGetTypeInfo(const J_COMPONENT_TYPE cType)
	{
		return CTypeInfo::Instance().cFuncStorage[(uint)cType].CallGetTypeInfo();
	}
	bool CTypeCommonCall::CallIsAvailableOverlap(const J_COMPONENT_TYPE cType)
	{
		return CTypeInfo::Instance().cFuncStorage[(uint)cType].CallIsAvailableOverlapCallable();
	}
	std::unique_ptr<Core::JDITypeDataBase> CTypeCommonCall::CallCreateInitDataCallable(const J_COMPONENT_TYPE cType,
		const Core::JTypeInfo& typeInfo,
		JUserPtr<JGameObject> owner,
		std::unique_ptr<Core::JDITypeDataBase>&& parentInitData)
	{
		return CTypeInfo::Instance().cFuncStorage[(uint)cType].CallCreateInitDataCallable(typeInfo, owner, std::move(parentInitData));
	}
	J_COMPONENT_TYPE CTypeCommonCall::ConvertCompType(const Core::JTypeInfo& info)
	{ 
		auto& typeMap = CTypeInfo::Instance().typeMap;
		auto data = typeMap.find(info.TypeGuid());
		if (data == typeMap.end())
		{
			auto nextInfo = info.GetParent();
			while (nextInfo != nullptr && data == typeMap.end())
			{
				data = typeMap.find(nextInfo->TypeGuid());
				nextInfo = nextInfo->GetParent();
			}
			return data != typeMap.end() ? data->second : (J_COMPONENT_TYPE)-1;
		}
		else
			return data->second;
	}
	bool CTypeCommonCall::NameOrder(const CTypeHint& a, const CTypeHint& b)noexcept
	{
		return CTypeInfo::Instance().cFuncStorage[(uint)a.thisType].CallGetTypeInfo().Name() < CTypeInfo::Instance().cFuncStorage[(uint)b.thisType].CallGetTypeInfo().Name();
	}
	void CTypePrivateCall::CallSetFrameDirty(JComponent* jComp)
	{
		return CTypeInfo::Instance().pFuncStorage[(uint)jComp->GetComponentType()].CallSetFrameDirty(jComp);
	}
	SetCFrameDirtyCallable* CTypePrivateCall::GetSetFrameDirtyCallable(const J_COMPONENT_TYPE cType)
	{
		return CTypeInfo::Instance().pFuncStorage[(uint)cType].GetSetFrameDirtyCallable();
	}
}