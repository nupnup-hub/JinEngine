#include"JResourceObjectInterface.h" 
#include"JResourceObject.h"
#include"../../Core/Singleton/JSingletonHolder.h"
#include<algorithm> 

namespace JinEngine
{
	//For enum type call virtual func 
	JResourceObjectInterface::RTypeHint::RTypeHint(const J_RESOURCE_TYPE thisType,
		const std::vector<J_RESOURCE_TYPE>& hasType,
		const bool hasGraphicResource,
		const bool isFrameResource)
		:thisType(thisType),
		hasType(hasType),
		hasGraphicResource(hasGraphicResource),
		isFrameResource(isFrameResource)
	{}
	JResourceObjectInterface::RTypeHint::~RTypeHint() {}

	JResourceObjectInterface::RTypeCommonFunc::RTypeCommonFunc(JResourceObjectInterface::GetTypeNameCallable& getTypeName,
		JResourceObjectInterface::GetAvailableFormatCallable& getAvailableFormat,
		JResourceObjectInterface::GetFormatIndexCallable& getFormatIndex)
		: getTypeName(&getTypeName),
		getAvailableFormat(&getAvailableFormat),
		getFormatIndex(&getFormatIndex)
	{  }
	JResourceObjectInterface::RTypeCommonFunc::~RTypeCommonFunc()
	{
		getTypeName = nullptr;
		getAvailableFormat = nullptr;
		getFormatIndex = nullptr;
	}
	std::string JResourceObjectInterface::RTypeCommonFunc::CallGetTypeName()
	{
		return (*getTypeName)(nullptr);
	}
	std::vector<std::wstring> JResourceObjectInterface::RTypeCommonFunc::CallGetAvailableFormat()
	{
		return (*getAvailableFormat)(nullptr);
	}
	uint8 JResourceObjectInterface::RTypeCommonFunc::CallFormatIndex(const std::wstring& format)
	{
		return (*getFormatIndex)(nullptr, format);
	}

	JResourceObjectInterface::RTypeInterfaceFunc::RTypeInterfaceFunc(SetFrameDirtyCallable* setFrameDirtyCallable, SetFrameBuffIndexCallable* setFrameBuffIndexCallable)
		:setFrameDirtyCallable(setFrameDirtyCallable), setFrameBuffIndexCallable(setFrameBuffIndexCallable)
	{}
	JResourceObjectInterface::RTypeInterfaceFunc::~RTypeInterfaceFunc()
	{
		setFrameDirtyCallable = nullptr;
	}
	void JResourceObjectInterface::RTypeInterfaceFunc::CallSetFrameDirty(JResourceObject& jRobj)
	{
		(*setFrameDirtyCallable)(nullptr, jRobj);
	}
	void JResourceObjectInterface::RTypeInterfaceFunc::CallSetFrameBuffIndex(JResourceObject& jRobj, const uint value)
	{
		(*setFrameBuffIndexCallable)(nullptr, jRobj, value);
	}
	JResourceObjectInterface::SetFrameDirtyCallable JResourceObjectInterface::RTypeInterfaceFunc::GetSetFrameDirtyCallable()
	{
		return *setFrameDirtyCallable;
	}
	JResourceObjectInterface::SetFrameBuffIndexCallable JResourceObjectInterface::RTypeInterfaceFunc::GetSetFrameBuffIndexCallable()
	{
		return *setFrameBuffIndexCallable;
	}

	class RTypeInfoData
	{
	public:
		std::vector<JResourceObjectInterface::RTypeHint> rInfoStorage;
		std::vector<JResourceObjectInterface::RTypeCommonFunc> rFuncStorage;
		std::vector<JResourceObjectInterface::RTypeInterfaceFunc> rIntefaceStroage;
	public:
		RTypeInfoData()
		{
			rInfoStorage.resize((int)J_RESOURCE_TYPE::COUNT);
			rFuncStorage.resize((int)J_RESOURCE_TYPE::COUNT);
			rIntefaceStroage.resize((int)J_RESOURCE_TYPE::COUNT);
		}
		~RTypeInfoData()
		{
			rInfoStorage.clear();
			rFuncStorage.clear();
			rIntefaceStroage.clear();
		}
	};
	using RTypeInfo = Core::JSingletonHolder<RTypeInfoData>;
	//FuncStorage

	std::vector<std::wstring> JResourceObjectInterface::CallGetAvailableFormat(const J_RESOURCE_TYPE type)
	{
		return RTypeInfo::Instance().rFuncStorage[(int)type].CallGetAvailableFormat();
	}
	std::string JResourceObjectInterface::CallGetTypeName(const J_RESOURCE_TYPE type)
	{
		return RTypeInfo::Instance().rFuncStorage[(int)type].CallGetTypeName();
	}
	uint8 JResourceObjectInterface::CallFormatIndex(const J_RESOURCE_TYPE type, const std::wstring& format)
	{
		return RTypeInfo::Instance().rFuncStorage[(int)type].CallFormatIndex(format);
	}
	bool JResourceObjectInterface::CallIsValidFormat(const J_RESOURCE_TYPE type, const std::wstring& format)
	{
		return RTypeInfo::Instance().rFuncStorage[(int)type].CallFormatIndex(format) != JResourceObject::GetInvalidFormatIndex();
	}
	void JResourceObjectInterface::CallSetFrameDirty(JResourceObject& jRobj)
	{
		RTypeInfo::Instance().rIntefaceStroage[(int)jRobj.GetResourceType()].CallSetFrameDirty(jRobj);
	}
	void JResourceObjectInterface::CallSetFrameBuffIndex(JResourceObject& jRobj, const uint value)
	{
		RTypeInfo::Instance().rIntefaceStroage[(int)jRobj.GetResourceType()].CallSetFrameBuffIndex(jRobj, value);
	} 

	JResourceObjectInterface::SetFrameDirtyCallable JResourceObjectInterface::GetSetFrameDirtyCallable(const J_RESOURCE_TYPE type)
	{
		return RTypeInfo::Instance().rIntefaceStroage[(int)type].GetSetFrameDirtyCallable();
	}
	JResourceObjectInterface::SetFrameBuffIndexCallable JResourceObjectInterface::GetSetFrameBuffIndexCallable(const J_RESOURCE_TYPE type)
	{
		return RTypeInfo::Instance().rIntefaceStroage[(int)type].GetSetFrameBuffIndexCallable();
	}
	const JResourceObjectInterface::RTypeHint JResourceObjectInterface::GetRTypeHint(const J_RESOURCE_TYPE type)noexcept
	{
		return RTypeInfo::Instance().rInfoStorage[(int)type];
	}
	const std::vector<JResourceObjectInterface::RTypeHint> JResourceObjectInterface::GetRTypeHintVec(const J_RESOURCE_ALIGN_TYPE alignType)noexcept
	{
		switch (alignType)
		{
		case JinEngine::J_RESOURCE_ALIGN_TYPE::NONE:
		{
			return RTypeInfo::Instance().rInfoStorage;
		}
		case JinEngine::J_RESOURCE_ALIGN_TYPE::NAME:
		{
			std::vector<JResourceObjectInterface::RTypeHint> infoCopy = RTypeInfo::Instance().rInfoStorage;
			sort(infoCopy.begin(), infoCopy.end(), NameOrder);
			return infoCopy;
		}
		case JinEngine::J_RESOURCE_ALIGN_TYPE::DEPENDENCY:
		{			 
			return GetDDHintVector();
		}
		case JinEngine::J_RESOURCE_ALIGN_TYPE::DEPENDENCY_REVERSE:
		{
			std::vector<JResourceObjectInterface::RTypeHint> ddVec = GetDDHintVector();
			std::reverse(ddVec.begin(), ddVec.end());
			return ddVec;
		}
		default:
			return RTypeInfo::Instance().rInfoStorage;
		};
	}
	std::vector<JResourceObjectInterface::RTypeHint> JResourceObjectInterface::GetDDHintVector()noexcept
	{
		std::vector<JResourceObjectInterface::RTypeHint> overlapInfo;
		const uint infoStroageCount = (uint)RTypeInfo::Instance().rInfoStorage.size();
		for (uint i = 0; i < infoStroageCount; ++i)
		{
			std::vector<JResourceObjectInterface::RTypeHint> addV = SpreadHasType(RTypeInfo::Instance().rInfoStorage[i]);
			overlapInfo.insert(overlapInfo.end(), addV.begin(), addV.end());
		}
		std::vector<JResourceObjectInterface::RTypeHint> resInfo;
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
	std::vector<JResourceObjectInterface::RTypeHint> JResourceObjectInterface::SpreadHasType(const JResourceObjectInterface::RTypeHint& info)noexcept
	{
		std::vector<JResourceObjectInterface::RTypeHint> res;
		const uint hasCount = (uint)info.hasType.size();
		for (uint i = 0; i < hasCount; ++i)
		{
			const uint infoStorageCount = (uint)RTypeInfo::Instance().rInfoStorage.size();
			for (uint j = 0; j < infoStorageCount; ++j)
			{
				if (info.hasType[i] == RTypeInfo::Instance().rInfoStorage[j].thisType)
				{
					std::vector<JResourceObjectInterface::RTypeHint> addV = SpreadHasType(RTypeInfo::Instance().rInfoStorage[j]);
					res.insert(res.end(), addV.begin(), addV.end());
				}
			}
		}
		res.push_back(info);
		return res;
	}
	bool JResourceObjectInterface::NameOrder(const JResourceObjectInterface::RTypeHint& a, const JResourceObjectInterface::RTypeHint& b)noexcept
	{
		return RTypeInfo::Instance().rFuncStorage[(int)a.thisType].CallGetTypeName() < RTypeInfo::Instance().rFuncStorage[(int)b.thisType].CallGetTypeName();
	}
	void JResourceObjectInterface::RegisterTypeInfo(const JResourceObjectInterface::RTypeHint& rTypeHint, const JResourceObjectInterface::RTypeCommonFunc& rTypeUtil, const JResourceObjectInterface::RTypeInterfaceFunc& rTypeIFunc)
	{
		RTypeInfo::Instance().rInfoStorage[(int)rTypeHint.thisType] = rTypeHint;
		RTypeInfo::Instance().rFuncStorage[(int)rTypeHint.thisType] = rTypeUtil;
		RTypeInfo::Instance().rIntefaceStroage[(int)rTypeHint.thisType] = rTypeIFunc;
	}
	JResourceObjectInterface::JResourceObjectInterface(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag)
		:JObject(name, guid, flag)
	{}
}