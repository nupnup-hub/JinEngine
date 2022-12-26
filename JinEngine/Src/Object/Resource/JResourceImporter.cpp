#include"JResourceImporter.h"
#include"../JObject.h" 
#include"../../Core/File/JFilePathData.h"

namespace JinEngine
{
	void JResourceImporterImpl::AddFormatInfo(const std::wstring& format, const J_RESOURCE_TYPE rType, ImportF::Ptr ptr)noexcept
	{
		auto data = formatInfoMap.find(format);
		if (data == formatInfoMap.end())
		{
			formatInfoMap.emplace(format, FormatInfo{});
			data = formatInfoMap.find(format);
		}
		if (data->second.ImportCallableMap.find(rType) == data->second.ImportCallableMap.end())
			data->second.ImportCallableMap.emplace(rType, ImportF::Callable{ ptr });
	}
	void JResourceImporterImpl::AddFormatInfo(const std::wstring& format, const J_RESOURCE_TYPE rType, ImportF::Ptr iptr, ClassifyTypeF::Ptr cptr)noexcept
	{
		auto data = formatInfoMap.find(format);
		if (data == formatInfoMap.end())
		{
			formatInfoMap.emplace(format, FormatInfo{});
			data = formatInfoMap.find(format);
		}
		if (data->second.ImportCallableMap.find(rType) == data->second.ImportCallableMap.end())
			data->second.ImportCallableMap.emplace(rType, ImportF::Callable{ iptr });

		if (data->second.ClassifyCallable == nullptr)
			data->second.ClassifyCallable = std::make_unique<ClassifyTypeF::Callable>(cptr);
	}
	std::vector<JResourceObject*> JResourceImporterImpl::ImportResource(JDirectory* dir, const Core::JFileImportHelpData& importPathdata)noexcept
	{
		auto data = formatInfoMap.find(importPathdata.format);
		if (data == formatInfoMap.end())
			return {nullptr};

		if (data->second.ClassifyCallable == nullptr)
		{
			if (data->second.ImportCallableMap.size() > 1)
				return {};
			else
			{
				auto callable = data->second.ImportCallableMap.begin()->second;
				return callable(nullptr, std::move(dir), Core::JFileImportHelpData(importPathdata.oriFileWPath));
			}
		}
		else
		{  
			const std::vector<J_RESOURCE_TYPE> rType = (*data->second.ClassifyCallable)(nullptr, importPathdata);
			const uint rCount = (uint)rType.size();

			std::vector<JResourceObject*> retVec;
			retVec.reserve(rCount);

			for (uint i = 0; i < rCount; ++i)
			{
				auto callable = data->second.ImportCallableMap.find(rType[i]);
				if (callable != data->second.ImportCallableMap.end())
				{
					std::vector<JResourceObject*> res = (callable->second)(nullptr, std::move(dir), Core::JFileImportHelpData(importPathdata.oriFileWPath));
					const uint resCount = (uint)res.size();
					for (uint j = 0; j < resCount; ++j)
					{
						if(res[j] != nullptr)
							retVec.push_back(res[j]);
					}
				}
			}
			return retVec;
		}
	}
	bool JResourceImporterImpl::IsValidFormat(const std::wstring& format)const noexcept
	{ 
		return formatInfoMap.find(format) != formatInfoMap.end();
	}
}