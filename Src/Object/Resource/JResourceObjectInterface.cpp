#include"JResourceObjectInterface.h" 
#include"../../Core/Singleton/JSingletonHolder.h"
#include<algorithm> 

namespace JinEngine
{
	//For enum type call virtual func 
	JRI::RTypeHint::RTypeHint(const J_RESOURCE_TYPE thisType, const std::vector<J_RESOURCE_TYPE>& hasType, bool isGraphicResource)
		:thisType(thisType),
		hasType(hasType),
		isGraphicResource(isGraphicResource)
	{}
	JRI::RTypeHint::~RTypeHint() {}

	JRI::RTypeUtil::RTypeUtil(JRI::GetTypeNameCallable& getTypeName,
		JRI::GetAvailableFormatCallable& getAvailableFormat,
		JRI::GetFormatIndexCallable& getFormatIndex)
		: getTypeName(&getTypeName),
		getAvailableFormat(&getAvailableFormat),
		getFormatIndex(&getFormatIndex)
	{  }
	JRI::RTypeUtil::~RTypeUtil()
	{
		getTypeName = nullptr;
		getAvailableFormat = nullptr;
		getFormatIndex = nullptr;
	}
	std::string JRI::RTypeUtil::CallGetTypeName()
	{
		return (*getTypeName)(nullptr);
	}
	std::vector<std::string> JRI::RTypeUtil::CallGetAvailableFormat()
	{
		return (*getAvailableFormat)(nullptr);
	}
	int JRI::RTypeUtil::CallFormatIndex(const std::string& format)
	{
		return (*getFormatIndex)(nullptr, format);
	}

	class RTypeInfoImpl
	{
	public:
		 std::vector<JRI::RTypeHint> infoStorage;;
		 std::unordered_map<J_RESOURCE_TYPE, JRI::RTypeUtil> funcStorage;
	};
	using RTypeInfo = Core::JSingletonHolder<RTypeInfoImpl>;
	//FuncStorage

	std::vector<std::string> JResourceObjectInterface::CallGetAvailableFormat(const J_RESOURCE_TYPE type)
	{
		return RTypeInfo::Instance().funcStorage.find(type)->second.CallGetAvailableFormat();
	}
	std::string JResourceObjectInterface::CallGetTypeName(const J_RESOURCE_TYPE type)
	{
		return RTypeInfo::Instance().funcStorage.find(type)->second.CallGetTypeName();
	}
	int JResourceObjectInterface::CallFormatIndex(const J_RESOURCE_TYPE type, const std::string& format)
	{
		return RTypeInfo::Instance().funcStorage.find(type)->second.CallFormatIndex(format);
	}
	bool JResourceObjectInterface::CallIsValidFormat(const J_RESOURCE_TYPE type, const std::string& format)
	{
		return RTypeInfo::Instance().funcStorage.find(type)->second.CallFormatIndex(format) != -1;
	}
	int JResourceObjectInterface::GetFormatIndex(const J_RESOURCE_TYPE type, const std::string& format)
	{
		std::vector<std::string> validFormat = CallGetAvailableFormat(type);
		const uint validFormatCount = (uint)validFormat.size();
		for (uint i = 0; i < validFormatCount; ++i)
		{
			if (validFormat[i] == format)
				return i;
		}
		return -1;
	}
	const std::vector<JRI::RTypeHint> JResourceObjectInterface::GetRInfo(const RESOURCE_ALIGN_TYPE alignType)
	{
		switch (alignType)
		{
		case JinEngine::RESOURCE_ALIGN_TYPE::NONE:
		{
			return RTypeInfo::Instance().infoStorage;
		}
		case JinEngine::RESOURCE_ALIGN_TYPE::NAME:
		{
			std::vector<JRI::RTypeHint> infoCopy = RTypeInfo::Instance().infoStorage;
			sort(infoCopy.begin(), infoCopy.end(), NameOrder);
			return infoCopy;
		}
		case JinEngine::RESOURCE_ALIGN_TYPE::DEPENDENCY:
		{
			std::vector<JRI::RTypeHint> overlapInfo;
			const uint infoStroageCount = (uint)RTypeInfo::Instance().infoStorage.size();
			for (uint i = 0; i < infoStroageCount; ++i)
			{
				std::vector<JRI::RTypeHint> addV = GetDDSortedType(RTypeInfo::Instance().infoStorage[i]);
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
			return RTypeInfo::Instance().infoStorage;
		};
	}
	 
	JResourceObjectInterface::JResourceObjectInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag)
		:JObject(name, guid, flag)
	{}

	void JResourceObjectInterface::RegisterTypeInfo(const JRI::RTypeHint& rTypeHint, const JRI::RTypeUtil& rTypeUtil)
	{
		bool hasOverlap = false;
		const uint infoStorageCount = (uint)RTypeInfo::Instance().infoStorage.size();
		for (uint i = 0; i < infoStorageCount; ++i)
		{
			if (RTypeInfo::Instance().infoStorage[i].thisType == rTypeHint.thisType)
			{
				hasOverlap = true;
				break;
			}
		}

		if (!hasOverlap)
			RTypeInfo::Instance().infoStorage.push_back(rTypeHint);

		if (RTypeInfo::Instance().funcStorage.empty() ||
			RTypeInfo::Instance().funcStorage.find(rTypeHint.thisType) == RTypeInfo::Instance().funcStorage.end())
			RTypeInfo::Instance().funcStorage.emplace(rTypeHint.thisType, rTypeUtil);
	}
	std::vector<JRI::RTypeHint> JResourceObjectInterface::GetDDSortedType(const JRI::RTypeHint& info)noexcept
	{
		std::vector<JRI::RTypeHint> res;
		const uint hasCount = (uint)info.hasType.size();
		for (uint i = 0; i < hasCount; ++i)
		{
			const uint infoStorageCount = (uint)RTypeInfo::Instance().infoStorage.size();
			for (uint j = 0; j < infoStorageCount; ++j)
			{
				if (info.hasType[i] == RTypeInfo::Instance().infoStorage[j].thisType)
				{
					std::vector<JRI::RTypeHint> addV = GetDDSortedType(RTypeInfo::Instance().infoStorage[j]);
					res.insert(res.end(), addV.begin(), addV.end());
				}
			}
		}
		res.push_back(info);
		return res;
	}
	bool JResourceObjectInterface::NameOrder(const JRI::RTypeHint& a, const JRI::RTypeHint& b)noexcept
	{
		return RTypeInfo::Instance().funcStorage.find(a.thisType)->second.CallGetTypeName() < RTypeInfo::Instance().funcStorage.find(b.thisType)->second.CallGetTypeName();
	}
}