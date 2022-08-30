#include"JResourceObjectInterface.h" 
#include"JResourceObject.h"
#include"../../Core/Singleton/JSingletonHolder.h"
#include<algorithm> 

namespace JinEngine
{
	//For enum type call virtual func 
	JRI::RTypeHint::RTypeHint(const J_RESOURCE_TYPE thisType,
		const std::vector<J_RESOURCE_TYPE>& hasType,
		const bool hasGraphicResource,
		const bool isFrameResource,
		const bool isGraphicBuffResource)
		:thisType(thisType),
		hasType(hasType),
		hasGraphicResource(hasGraphicResource),
		isFrameResource(isFrameResource),
		isGraphicBuffResource(isGraphicBuffResource)
	{}
	JRI::RTypeHint::RTypeHint(const RTypeHint& rhs)
		:thisType(thisType),
		hasType(hasType),
		hasGraphicResource(hasGraphicResource),
		isFrameResource(isFrameResource),
		isGraphicBuffResource(isGraphicBuffResource)
	{}
	JRI::RTypeHint::~RTypeHint() {}

	JRI::RTypeCommonFunc::RTypeCommonFunc(JRI::GetTypeNameCallable& getTypeName,
		JRI::GetAvailableFormatCallable& getAvailableFormat,
		JRI::GetFormatIndexCallable& getFormatIndex)
		: getTypeName(&getTypeName),
		getAvailableFormat(&getAvailableFormat),
		getFormatIndex(&getFormatIndex)
	{  }
	JRI::RTypeCommonFunc::~RTypeCommonFunc()
	{
		getTypeName = nullptr;
		getAvailableFormat = nullptr;
		getFormatIndex = nullptr;
	}
	std::string JRI::RTypeCommonFunc::CallGetTypeName()
	{
		return (*getTypeName)(nullptr);
	}
	std::vector<std::wstring> JRI::RTypeCommonFunc::CallGetAvailableFormat()
	{
		return (*getAvailableFormat)(nullptr);
	}
	int JRI::RTypeCommonFunc::CallFormatIndex(const std::wstring& format)
	{
		return (*getFormatIndex)(nullptr, format);
	}

	JRI::RTypeInterfaceFunc::RTypeInterfaceFunc(SetFrameDirtyCallable* setFrameDirtyCallable, SetBuffIndexCallable* setBuffIndexCallable)
		:setFrameDirtyCallable(setFrameDirtyCallable), setBuffIndexCallable(setBuffIndexCallable)
	{}
	JRI::RTypeInterfaceFunc::~RTypeInterfaceFunc()
	{
		setFrameDirtyCallable = nullptr;
	}
	void JRI::RTypeInterfaceFunc::CallSetFrameDirty(JResourceObject& jRobj)
	{
		(*setFrameDirtyCallable)(nullptr, jRobj);
	}
	void JRI::RTypeInterfaceFunc::CallSetBuffIndex(JResourceObject& jRobj, const uint index)
	{
		(*setBuffIndexCallable)(nullptr, jRobj, index);
	}
	JRI::SetFrameDirtyCallable JRI::RTypeInterfaceFunc::GetSetFrameDirtyCallable()
	{
		return *setFrameDirtyCallable;
	}
	JRI::SetBuffIndexCallable JRI::RTypeInterfaceFunc::GetSetBuffIndexCallable()
	{
		return *setBuffIndexCallable;
	}

	class RTypeInfoData
	{
	public:
		 std::vector<JRI::RTypeHint> rInfoStorage;
		 std::vector<JRI::RTypeCommonFunc> rFuncStorage; 
		 std::vector<JRI::RTypeInterfaceFunc> rIntefaceStroage;
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
	int JResourceObjectInterface::CallFormatIndex(const J_RESOURCE_TYPE type, const std::wstring& format)
	{
		return RTypeInfo::Instance().rFuncStorage[(int)type].CallFormatIndex(format);
	}
	bool JResourceObjectInterface::CallIsValidFormat(const J_RESOURCE_TYPE type, const std::wstring& format)
	{
		return RTypeInfo::Instance().rFuncStorage[(int)type].CallFormatIndex(format) != -1;
	}
	void JResourceObjectInterface::CallSetFrameDirty(JResourceObject& jRobj)
	{
		RTypeInfo::Instance().rIntefaceStroage[(int)jRobj.GetResourceType()].CallSetFrameDirty(jRobj);
	}
	void JResourceObjectInterface::CallSetBuffIndex(JResourceObject& jRobj, const uint index)
	{
		RTypeInfo::Instance().rIntefaceStroage[(int)jRobj.GetResourceType()].CallSetBuffIndex(jRobj, index);
	}
	JRI::SetFrameDirtyCallable JResourceObjectInterface::GetSetFrameDirtyCallable(const J_RESOURCE_TYPE type)
	{
		return RTypeInfo::Instance().rIntefaceStroage[(int)type].GetSetFrameDirtyCallable();
	}
	JRI::SetBuffIndexCallable JResourceObjectInterface::GetSetBuffIndexCallable(const J_RESOURCE_TYPE type)
	{
		return RTypeInfo::Instance().rIntefaceStroage[(int)type].GetSetBuffIndexCallable();
	}
	int JResourceObjectInterface::GetFormatIndex(const J_RESOURCE_TYPE type, const std::wstring& format)
	{
		std::vector<std::wstring> validFormat = CallGetAvailableFormat(type);
		const uint validFormatCount = (uint)validFormat.size();
		for (uint i = 0; i < validFormatCount; ++i)
		{
			if (validFormat[i] == format)
				return i;
		}
		return -1;
	}
	const JRI::RTypeHint JResourceObjectInterface::GetRTypeHint(const J_RESOURCE_TYPE type)noexcept
	{
		return RTypeInfo::Instance().rInfoStorage[(int)type];
	}
	const std::vector<JRI::RTypeHint> JResourceObjectInterface::GetRTypeHintVec(const RESOURCE_ALIGN_TYPE alignType)noexcept
	{
		switch (alignType)
		{
		case JinEngine::RESOURCE_ALIGN_TYPE::NONE:
		{
			return RTypeInfo::Instance().rInfoStorage;
		}
		case JinEngine::RESOURCE_ALIGN_TYPE::NAME:
		{
			std::vector<JRI::RTypeHint> infoCopy = RTypeInfo::Instance().rInfoStorage;
			sort(infoCopy.begin(), infoCopy.end(), NameOrder);
			return infoCopy;
		}
		case JinEngine::RESOURCE_ALIGN_TYPE::DEPENDENCY:
		{
			std::vector<JRI::RTypeHint> overlapInfo;
			const uint infoStroageCount = (uint)RTypeInfo::Instance().rInfoStorage.size();
			for (uint i = 0; i < infoStroageCount; ++i)
			{
				std::vector<JRI::RTypeHint> addV = GetDDSortedType(RTypeInfo::Instance().rInfoStorage[i]);
				overlapInfo.insert(overlapInfo.end(), addV.begin(), addV.end());
			}
			std::vector<JRI::RTypeHint> resInfo;
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
		default:
			return RTypeInfo::Instance().rInfoStorage;
		};
	}
	
	std::vector<JRI::RTypeHint> JResourceObjectInterface::GetDDSortedType(const JRI::RTypeHint& info)noexcept
	{
		std::vector<JRI::RTypeHint> res;
		const uint hasCount = (uint)info.hasType.size();
		for (uint i = 0; i < hasCount; ++i)
		{
			const uint infoStorageCount = (uint)RTypeInfo::Instance().rInfoStorage.size();
			for (uint j = 0; j < infoStorageCount; ++j)
			{
				if (info.hasType[i] == RTypeInfo::Instance().rInfoStorage[j].thisType)
				{
					std::vector<JRI::RTypeHint> addV = GetDDSortedType(RTypeInfo::Instance().rInfoStorage[j]);
					res.insert(res.end(), addV.begin(), addV.end());
				}
			}
		}
		res.push_back(info);
		return res;
	}
	bool JResourceObjectInterface::NameOrder(const JRI::RTypeHint& a, const JRI::RTypeHint& b)noexcept
	{
		return RTypeInfo::Instance().rFuncStorage[(int)a.thisType].CallGetTypeName() < RTypeInfo::Instance().rFuncStorage[(int)b.thisType].CallGetTypeName();
	}
	void JResourceObjectInterface::RegisterTypeInfo(const JRI::RTypeHint& rTypeHint, const JRI::RTypeCommonFunc& rTypeUtil, const JRI::RTypeInterfaceFunc& rTypeIFunc)
	{
		const JRI::RTypeHint hint{ rTypeHint };
		RTypeInfo::Instance().rInfoStorage[(int)rTypeHint.thisType] = std::move(hint);
		RTypeInfo::Instance().rFuncStorage[(int)rTypeHint.thisType] = rTypeUtil;
		RTypeInfo::Instance().rIntefaceStroage[(int)rTypeHint.thisType] = rTypeIFunc;
	}
	JResourceObjectInterface::JResourceObjectInterface(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag)
		:JObject(name, guid, flag)
	{}
}