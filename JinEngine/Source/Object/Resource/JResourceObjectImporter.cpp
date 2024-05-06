/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JResourceObjectImporter.h"
#include"../JObject.h" 
#include"../../Core/File/JFilePathData.h"
#include"../../Core/Utility/JCommonUtility.h"

namespace JinEngine
{ 
	JResourceObjectImportDesc::JResourceObjectImportDesc(const Core::JFileImportPathData& importPathData, const JUserPtr<JDirectory>& dir)
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

		auto data = GetFormatInfo(desc->importPathData.oriFileFormat);
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
			//�����ʿ�
			//���Ŀ� JFileImporter ������ �Ű������� ���ݺ��� �þ��
			//�����丵 �ʿ�
			//ex) callable �Ű����� �����ڷ� ���� or �����Լ� �ۼ�
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
	std::vector<J_RESOURCE_TYPE> JResourceObjectImporterImpl::DeterminFileResourceType(const Core::JFileImportPathData importPathData)const noexcept
	{
		auto data = GetFormatInfo(importPathData.oriFileFormat);
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