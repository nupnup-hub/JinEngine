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


#pragma once 
#include"../JObjectFlag.h"
#include"../Resource/JResourceObjectType.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Reflection/JReflectionMacro.h"
#include"../../Core/Reflection/JTypeBase.h"

namespace JinEngine
{ 
	namespace Core
	{
		class JTypeInfo;
	}
	class JResourceObject;   
	class JDirectory; 
	class JDirectoryPrivate;
	class JFilePrivate;
	struct JFileInitData;
	struct JFileData
	{
	public:
		virtual ~JFileData() = default;
	public: 
		virtual bool IsActData()const noexcept = 0;
	public:
		virtual size_t GetResourceGuid()const noexcept = 0;
		virtual J_RESOURCE_TYPE GetResourceType()const noexcept = 0;
		virtual Core::JTypeInfo& GetResourceTypeInfo()const noexcept = 0;
		virtual J_OBJECT_FLAG GetObjectFlag()const noexcept = 0;
	public:
		virtual std::wstring GetName()const noexcept = 0;
		virtual std::wstring GetFullName()const noexcept = 0;
		virtual std::wstring GetOriginalResourceFormat()const noexcept = 0;
		virtual std::wstring GetPath()const noexcept = 0;
		virtual std::wstring GetMetaFilePath()const noexcept = 0;
		virtual std::wstring GetCacheFilePath()const noexcept = 0;
	public:
		virtual JUserPtr<JDirectory> GetOwnerDirectory()const noexcept = 0;
		virtual JUserPtr<JResourceObject> GetResource()const noexcept = 0;
	};

	//Jasset file class 
	//it match one resource objecty 
	class JFile : public Core::JTypeBase
	{
		REGISTER_CLASS_USE_ALLOCATOR(JFile)
	private:
		friend class JFilePrivate;  
	private:
		std::unique_ptr<JFileData> fileData = nullptr;
	public:
		bool IsExistingResource()const noexcept;  	 
	public:
		size_t GetResourceGuid()const noexcept;
		J_RESOURCE_TYPE GetResourceType()const noexcept;
		Core::JTypeInfo& GetResourceTypeInfo()const noexcept;
		J_OBJECT_FLAG GetObjectFlag()const noexcept;
	public:
		std::wstring GetName()const noexcept;		// ret only name
		std::wstring GetFullName()const noexcept;		// name + original format
		std::wstring GetOriginalResourceFormat()const noexcept;	//ret original resource format ex) jpg, png
		std::wstring GetPath()const noexcept;
		std::wstring GetMetaFilePath()const noexcept;
		std::wstring GetCacheFilePath()const noexcept;
	public:
		JUserPtr<JDirectory> GetOwnerDirectory()const noexcept;
		JUserPtr<JResourceObject> GetResource()const noexcept;
		//if resource isn't exist load
		JUserPtr<JResourceObject> TryGetResourceUser()const noexcept;
	public: 
		static void RegisterTypeData();
	private:
		JFile(const JFileInitData& initData, const JUserPtr<JDirectory>& ownerDir);
		~JFile() = default; 
	};
}