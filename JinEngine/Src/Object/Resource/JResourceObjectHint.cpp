#include"JResourceObjectHint.h" 
#include"JResourceObject.h"
#include"../../Core/Singleton/JSingletonHolder.h"
#include<algorithm>  

namespace JinEngine
{
	//For enum type call virtual func 
	RTypeHint::RTypeHint(const J_RESOURCE_TYPE thisType,
		const std::vector<J_RESOURCE_TYPE>& hasType,
		const bool hasGraphicResource,
		const bool isFrameResource,
		const bool isInnerResource,
		const bool isFixedAssetFile,
		const bool canKeepJFileLife,
		const bool canKeepDiskFileLife)
		:thisType(thisType),
		hasType(hasType),
		hasGraphicResource(hasGraphicResource),
		isFrameResource(isFrameResource),
		isInnerResource(isInnerResource),
		isFixedAssetFile(isFixedAssetFile),
		canKeepJFileLife(canKeepJFileLife),
		canKeepDiskFileLife(canKeepDiskFileLife)
	{}
	RTypeHint::~RTypeHint() {}

	RTypeCommonFunc::RTypeCommonFunc(GetRTypeInfoCallable& getTypeInfo,
		GetAvailableFormatCallable& getAvailableFormat,
		GetFormatIndexCallable& getFormatIndex)
		: getTypeInfo(&getTypeInfo),
		getAvailableFormat(&getAvailableFormat),
		getFormatIndex(&getFormatIndex)
	{  }
	RTypeCommonFunc::~RTypeCommonFunc()
	{
		getTypeInfo = nullptr;
		getAvailableFormat = nullptr;
		getFormatIndex = nullptr;
	}
	Core::JTypeInfo& RTypeCommonFunc::CallGetTypeInfo()
	{
		return (*getTypeInfo)(nullptr);
	}
	std::vector<std::wstring> RTypeCommonFunc::CallGetAvailableFormat()
	{
		return (*getAvailableFormat)(nullptr);
	}
	uint8 RTypeCommonFunc::CallFormatIndex(const std::wstring& format)
	{
		return (*getFormatIndex)(nullptr, format);
	}

	RTypePrivateFunc::RTypePrivateFunc(SetRFrameDirtyCallable* setFrameDirtyCallable)
		:setFrameDirtyCallable(setFrameDirtyCallable)
	{}
	RTypePrivateFunc::~RTypePrivateFunc()
	{
		setFrameDirtyCallable = nullptr;
	}
	SetRFrameDirtyCallable RTypePrivateFunc::GetSetFrameDirtyCallable()
	{
		return *setFrameDirtyCallable;
	} 
	void RTypePrivateFunc::CallSetFrameDirty(JResourceObject* jRobj)
	{
		(*setFrameDirtyCallable)(nullptr, jRobj);
	} 

	struct RTypeInfoData
	{
	public:
		std::vector<RTypeHint> hintStorage;
		std::vector<RTypeCommonFunc> cFuncStorage;
		std::vector<RTypePrivateFunc> pFuncStorage;
	public:
		RTypeInfoData()
		{
			hintStorage.resize((int)J_RESOURCE_TYPE::COUNT);
			cFuncStorage.resize((int)J_RESOURCE_TYPE::COUNT);
			pFuncStorage.resize((int)J_RESOURCE_TYPE::COUNT);
		}
		~RTypeInfoData()
		{
			hintStorage.clear();
			cFuncStorage.clear();
			pFuncStorage.clear();
		}
	};
	using RTypeInfo = Core::JSingletonHolder<RTypeInfoData>;
	//FuncStorage

	void RTypeRegister::RegisterRTypeInfo(const RTypeHint& rTypeHint, const RTypeCommonFunc& rTypeCFunc, const RTypePrivateFunc& rTypePFunc)noexcept
	{
		RTypeInfo::Instance().hintStorage[(int)rTypeHint.thisType] = rTypeHint;
		RTypeInfo::Instance().cFuncStorage[(int)rTypeHint.thisType] = rTypeCFunc;
		RTypeInfo::Instance().pFuncStorage[(int)rTypeHint.thisType] = rTypePFunc;
	}

	namespace
	{
		static std::vector<RTypeHint> SpreadHasType(const RTypeHint& info)noexcept;
		static std::vector<RTypeHint> GetDDHintVector()noexcept
		{
			std::vector<RTypeHint> overlapInfo;
			const uint infoStroageCount = (uint)RTypeInfo::Instance().hintStorage.size();
			for (uint i = 0; i < infoStroageCount; ++i)
			{
				std::vector<RTypeHint> addV = SpreadHasType(RTypeInfo::Instance().hintStorage[i]);
				overlapInfo.insert(overlapInfo.end(), addV.begin(), addV.end());
			}
			std::vector<RTypeHint> resInfo;
			resInfo.reserve((uint)J_RESOURCE_TYPE::COUNT);

			uint stuffCount = 0;
			const uint overlapInfoCount = (uint)overlapInfo.size();
			for (uint i = 0; i < overlapInfoCount; ++i)
			{
				bool hasResource = false;
				const uint resInfoCount = (uint)resInfo.size();
				for (uint j = 0; j < resInfoCount; ++j)
				{
					if (resInfo[j].thisType == overlapInfo[i].thisType)
					{
						hasResource = true;
						break;
					}
				}
				if (!hasResource)
					resInfo.push_back(overlapInfo[i]);
			}
			return resInfo;
		}
		std::vector<RTypeHint> SpreadHasType(const RTypeHint& info)noexcept
		{
			std::vector<RTypeHint> res;
			const uint hasCount = (uint)info.hasType.size();
			for (uint i = 0; i < hasCount; ++i)
			{
				const uint infoStorageCount = (uint)RTypeInfo::Instance().hintStorage.size();
				for (uint j = 0; j < infoStorageCount; ++j)
				{
					if (info.hasType[i] == RTypeInfo::Instance().hintStorage[j].thisType)
					{
						std::vector<RTypeHint> addV = SpreadHasType(RTypeInfo::Instance().hintStorage[j]);
						res.insert(res.end(), addV.begin(), addV.end());
					}
				}
			}
			res.push_back(info);
			return res;
		}
		static bool NameOrder(const RTypeHint& a, const RTypeHint& b)noexcept
		{
			return RTypeInfo::Instance().cFuncStorage[(int)a.thisType].CallGetTypeInfo().Name() < RTypeInfo::Instance().cFuncStorage[(int)b.thisType].CallGetTypeInfo().Name();
		}
	}

	std::wstring RTypeCommonCall::GetFormat(const J_RESOURCE_TYPE type, const uint8 index)
	{
		auto formatVec = CallGetAvailableFormat(type);
		if (index < formatVec.size())
			return formatVec[index];
		else
			return L"Invalid Format";
	}
	const RTypeHint RTypeCommonCall::GetRTypeHint(const J_RESOURCE_TYPE type)noexcept
	{
		return RTypeInfo::Instance().hintStorage[(int)type];
	}
	const std::vector<RTypeHint> RTypeCommonCall::GetRTypeHintVec(const J_RESOURCE_ALIGN_TYPE alignType)noexcept
	{
		switch (alignType)
		{
		case JinEngine::J_RESOURCE_ALIGN_TYPE::NONE:
		{
			return RTypeInfo::Instance().hintStorage;
		}
		case JinEngine::J_RESOURCE_ALIGN_TYPE::NAME:
		{
			std::vector<RTypeHint> infoCopy = RTypeInfo::Instance().hintStorage;
			sort(infoCopy.begin(), infoCopy.end(), NameOrder);
			return infoCopy;
		}
		case JinEngine::J_RESOURCE_ALIGN_TYPE::DEPENDENCY:
		{
			return GetDDHintVector();
		}
		case JinEngine::J_RESOURCE_ALIGN_TYPE::DEPENDENCY_REVERSE:
		{
			std::vector<RTypeHint> ddVec = GetDDHintVector();
			std::reverse(ddVec.begin(), ddVec.end());
			return ddVec;
		}
		default:
			return RTypeInfo::Instance().hintStorage;
		};
	}
	const std::vector<Core::JTypeInfo*> RTypeCommonCall::GetTypeInfoVec(const J_RESOURCE_ALIGN_TYPE alignType, const bool allowAbstractClass)noexcept
	{
		std::vector<Core::JTypeInfo*> result;
		auto rHintVec = GetRTypeHintVec(alignType);
		for (const auto& hint : rHintVec)
		{
			auto& typeInfo = RTypeCommonCall::CallGetTypeInfo(hint.thisType);
			auto derivedVec = Core::JReflectionInfo::Instance().GetDerivedTypeInfo(typeInfo, true);
			if (!allowAbstractClass)	
				result.insert(result.end(), derivedVec.begin(), derivedVec.end());
			
			else
			{
				for (const auto& type : derivedVec)
				{
					if (!type->IsAbstractType())
						result.push_back(type);
				}
			}
		}
		return result;
	}
	std::vector<std::wstring> RTypeCommonCall::CallGetAvailableFormat(const J_RESOURCE_TYPE type)
	{
		return RTypeInfo::Instance().cFuncStorage[(int)type].CallGetAvailableFormat();
	}
	Core::JTypeInfo& RTypeCommonCall::CallGetTypeInfo(const J_RESOURCE_TYPE type)
	{
		return RTypeInfo::Instance().cFuncStorage[(int)type].CallGetTypeInfo();
	}
	uint8 RTypeCommonCall::CallFormatIndex(const J_RESOURCE_TYPE type, const std::wstring& format)
	{
		return RTypeInfo::Instance().cFuncStorage[(int)type].CallFormatIndex(format);
	}
	bool RTypeCommonCall::CallIsValidFormat(const J_RESOURCE_TYPE type, const std::wstring& format)
	{
		return RTypeInfo::Instance().cFuncStorage[(int)type].CallFormatIndex(format) != JResourceObject::GetInvalidFormatIndex();
	}

	SetRFrameDirtyCallable RTypePrivateCall::GetSetFrameDirtyCallable(const J_RESOURCE_TYPE type)
	{
		return RTypeInfo::Instance().pFuncStorage[(int)type].GetSetFrameDirtyCallable();
	} 
	void RTypePrivateCall::CallSetFrameDirty(JResourceObject* jRobj)
	{
		RTypeInfo::Instance().pFuncStorage[(int)jRobj->GetResourceType()].CallSetFrameDirty(jRobj);
	} 
}