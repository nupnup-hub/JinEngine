#include"JResourceObjectImporter.h"
#include"../JObject.h" 
#include"../../Core/File/JFilePathData.h"
#include"../../Core/Utility/JCommonUtility.h"

namespace JinEngine
{ 
	JResourceObjectImportDesc::JResourceObjectImportDesc(const Core::JFileImportHelpData& importPathData, const JUserPtr<JDirectory>& dir)
		: importPathData(importPathData), dir(dir)
	{}

	void JResourceObjectImporterImpl::AddFormatInfo(const std::wstring& format, const J_RESOURCE_TYPE rType, ImportResourceF::Ptr ptr)noexcept
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
	void JResourceObjectImporterImpl::AddFormatInfo(const std::wstring& format, const J_RESOURCE_TYPE rType, ImportResourceF::Ptr iptr, ClassifyResourceTypeF::Ptr cptr)noexcept
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
	std::vector<JUserPtr<JResourceObject>> JResourceObjectImporterImpl::ImportResource(const JResourceObjectImportDesc* desc)const noexcept
	{
		if (desc == nullptr || desc->dir == nullptr)
			return {};

		auto data = GetFormatInfo(desc->importPathData.format);
		if (data == nullptr)
			return {};
		 
		if (data->ClassifyCallable == nullptr)
		{
			if (data->ImportCallableMap.size() > 1)
				return {};
			else
			{
				auto callable = data->ImportCallableMap.begin()->second;
				return callable(nullptr, desc);
			}
		}
		else
		{
			//수정필요
			//이후에 JFileImporter 생성자 매개변수가 지금보다 늘어날시
			//리팩토링 필요
			//ex) callable 매개변수 참조자로 변경 or 생성함수 작성
			const std::vector<J_RESOURCE_TYPE> rType = (*data->ClassifyCallable)(nullptr, desc->importPathData);
			const uint rCount = (uint)rType.size();
			 
			std::vector<JUserPtr<JResourceObject>> retVec;
			retVec.reserve(rCount);

			for (uint i = 0; i < rCount; ++i)
			{
				auto callableData = data->ImportCallableMap.find(rType[i]);
				if (callableData != data->ImportCallableMap.end())
				{ 
					auto callable = callableData->second;
					const std::vector<JUserPtr<JResourceObject>> res = callable(nullptr, desc);
					const uint resCount = (uint)res.size();
					for (uint j = 0; j < resCount; ++j)
					{
						if (res[j] != nullptr)
							retVec.push_back(res[j]);
					}
				}
			}
			return retVec;
		}
	}
	std::vector<J_RESOURCE_TYPE> JResourceObjectImporterImpl::DeterminFileResourceType(const Core::JFileImportHelpData importPathData)const noexcept
	{
		auto data = GetFormatInfo(importPathData.format); 
		if (data == nullptr)
			return std::vector<J_RESOURCE_TYPE>();

		if (data->ClassifyCallable != nullptr)
			return (*data->ClassifyCallable)(nullptr, importPathData);

		std::vector<J_RESOURCE_TYPE> registeredType;
		for (const auto& data : data->ImportCallableMap)
			registeredType.push_back(data.first);
		return registeredType;
	}
	const JResourceObjectImporterImpl::FormatInfo* JResourceObjectImporterImpl::GetFormatInfo(const std::wstring& format)const noexcept
	{
		auto data = formatInfoMap.find(format); 
		return data != formatInfoMap.end() ? &data->second : nullptr;
	} 
	bool JResourceObjectImporterImpl::IsValidFormat(const std::wstring& format)const noexcept
	{
		return GetFormatInfo(format) != nullptr;
	}
}