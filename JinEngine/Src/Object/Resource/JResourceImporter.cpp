#include"JResourceImporter.h"
#include"../JObject.h" 
#include"../../Core/File/JFilePathData.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	void JResourceImporterImpl::AddFormatInfo(const std::wstring& format, const J_RESOURCE_TYPE rType, ImportResourceF::Ptr ptr)noexcept
	{
		auto data = formatInfoMap.find(format);
		if (data == formatInfoMap.end())
		{
			formatInfoMap.emplace(format, FormatInfo{});
			data = formatInfoMap.find(format);
		}
		if (data->second.ImportCallableMap.find(rType) == data->second.ImportCallableMap.end())
			data->second.ImportCallableMap.emplace(rType, ImportResourceF::Callable{ ptr });
	}
	void JResourceImporterImpl::AddFormatInfo(const std::wstring& format, const J_RESOURCE_TYPE rType, ImportResourceF::Ptr iptr, ClassifyResourceTypeF::Ptr cptr)noexcept
	{
		auto data = formatInfoMap.find(format);
		if (data == formatInfoMap.end())
		{
			formatInfoMap.emplace(format, FormatInfo{});
			data = formatInfoMap.find(format);
		}
		if (data->second.ImportCallableMap.find(rType) == data->second.ImportCallableMap.end())
			data->second.ImportCallableMap.emplace(rType, ImportResourceF::Callable{ iptr });

		if (data->second.ClassifyCallable == nullptr)
			data->second.ClassifyCallable = std::make_unique<ClassifyResourceTypeF::Callable>(cptr);
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
				return callable(nullptr, std::move(dir), Core::JFileImportHelpData(importPathdata.oriFileWPath, importPathdata.flag));
			}
		}
		else
		{  
			//수정필요
			//이후에 JFileImporter 생성자 매개변수가 지금보다 늘어날시
			//리팩토링 필요
			//ex) callable 매개변수 참조자로 변경 or 생성함수 작성
			const std::vector<J_RESOURCE_TYPE> rType = (*data->second.ClassifyCallable)(nullptr, Core::JFileImportHelpData(importPathdata.oriFileWPath, importPathdata.flag));
			const uint rCount = (uint)rType.size();

			std::vector<JResourceObject*> retVec;
			retVec.reserve(rCount);

			for (uint i = 0; i < rCount; ++i)
			{
				auto callable = data->second.ImportCallableMap.find(rType[i]);
				if (callable != data->second.ImportCallableMap.end())
				{
					std::vector<JResourceObject*> res = (callable->second)(nullptr, std::move(dir), 
						Core::JFileImportHelpData(importPathdata.oriFileWPath, importPathdata.flag));
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