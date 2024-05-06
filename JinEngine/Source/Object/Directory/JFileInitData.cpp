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


#include"JFileInitData.h" 
#include"../Resource/JResourceObject.h"

namespace JinEngine
{
	JFileInitData::JFileInitData(JResourceObject* rObj)
		:name(rObj->GetName()),
		rGuid(rObj->GetGuid()),
		rTypeInfo(rObj->GetTypeInfo()),
		resourceType(rObj->GetResourceType()),
		flag(rObj->GetFlag()),
		formatIndex(rObj->GetFormatIndex())
	{}
	JFileInitData::JFileInitData(const JUserPtr<JResourceObject>& rObj)
		:name(rObj->GetName()),
		rGuid(rObj->GetGuid()),
		rTypeInfo(rObj->GetTypeInfo()),
		resourceType(rObj->GetResourceType()),
		flag(rObj->GetFlag()),
		formatIndex(rObj->GetFormatIndex())
	{}
	JFileInitData::JFileInitData(const std::wstring& name,
		const size_t rGuid,
		Core::JTypeInfo& rTypeInfo,
		const J_RESOURCE_TYPE resourceType,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex)
		:name(name), 
		rGuid(rGuid), 
		rTypeInfo(rTypeInfo), 
		resourceType(resourceType), 
		flag(flag),
		formatIndex(formatIndex)
	{}
}