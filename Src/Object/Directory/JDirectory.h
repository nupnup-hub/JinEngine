#pragma once
#include<string>
#include<vector> 
#include<memory>
#include"../JObject.h" 

namespace JinEngine
{
	class JFile;
	class JResourceObject;
	class JDirectory : public JObject
	{
		REGISTER_CLASS(JDirectory)
	private: 
		JDirectory* parentDir;
		std::vector<JDirectory*> childrenDir;
		std::vector<std::unique_ptr<JFile>> fileList;  
	public:
		JDirectory* GetChildDirctory(const uint index)noexcept;
		JDirectory* GetChildDirctory(const std::string& path)noexcept;
		uint GetChildernDirctoryCount()const noexcept;
		uint GetFileCount()const noexcept;
		std::string GetPath()const noexcept;
		std::wstring GetWPath()const noexcept;
		JFile* GetFile(const uint index)noexcept; 
		J_OBJECT_TYPE GetObjectType()const noexcept final;

		bool HasChild(const std::string& name)const noexcept;

		JDirectory* SearchDirectory(const std::string& path)noexcept;
		std::string MakeUniqueFileName(const std::string& name)noexcept;
	private: 
		bool AddFile(JResourceObject* resource)noexcept; 
		bool EraseFile(JResourceObject* resource)noexcept;  
		static void RegisterFunc();
	private:
		JDirectory(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* parentDir);
		~JDirectory();
	};
 
}