#pragma once
#include<string>    
#include"../Resource/JResourceObjectType.h"
#include"../../Core/Pointer/JOwnerPtr.h"
 
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
	public:
		virtual std::wstring GetName()const noexcept = 0;
		virtual std::wstring GetFullName()const noexcept = 0;
		virtual std::wstring GetOriginalResourceFormat()const noexcept = 0;
		virtual std::wstring GetPath()const noexcept = 0;
		virtual std::wstring GetMetaFilePath()const noexcept = 0;
		virtual std::wstring GetCacheFilePath()const noexcept = 0;
	public:
		virtual JDirectory* GetOwnerDirectory()const noexcept = 0;
		virtual JResourceObject* GetResource()const noexcept = 0; 
	};
	//Jasset file class 
	//it match one resource objecty 
	class JFile
	{
	private:
		friend class JFilePrivate; 
		friend std::unique_ptr<JFile>::deleter_type;
		friend std::unique_ptr<JFile>;
	private:
		std::unique_ptr<JFileData> fileData = nullptr;
	public:
		bool IsExistingResource()const noexcept;  	 
	public:
		size_t GetResourceGuid()const noexcept;
		J_RESOURCE_TYPE GetResourceType()const noexcept;
		Core::JTypeInfo& GetResourceTypeInfo()const noexcept;
	public:
		std::wstring GetName()const noexcept;		// ret only name
		std::wstring GetFullName()const noexcept;		// name + original format
		std::wstring GetOriginalResourceFormat()const noexcept;	//ret original resource format ex) jpg, png
		std::wstring GetPath()const noexcept;
		std::wstring GetMetaFilePath()const noexcept;
		std::wstring GetCacheFilePath()const noexcept;
	public:
		JDirectory* GetOwnerDirectory()const noexcept;
		JResourceObject* GetResource()const noexcept;
		//if resource isn't exist load
		Core::JUserPtr<JResourceObject> TryGetResourceUser()const noexcept;
	private:
		JFile(const JFileInitData& initData, JDirectory* ownerDir);
		~JFile() = default; 
	};
}