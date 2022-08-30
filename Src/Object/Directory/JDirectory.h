#pragma once
#include<string>
#include<vector> 
#include<memory>
#include"JDirectoryInterface.h"
#include"JDirectoryPathData.h"

namespace JinEngine
{
	class JFile;
	class JResourceObject;
	class JDirectory : public JDirectoryInterface
	{
		REGISTER_CLASS(JDirectory)
	private:
		JDirectory* parent;
		std::vector<JDirectory*> children;
		std::vector<JFile*> fileList;
	public:
		JDirectory* GetChildDirctory(const uint index)noexcept;
		JDirectory* GetChildDirctory(const std::wstring& path)noexcept;
		uint GetChildernDirctoryCount()const noexcept;
		uint GetFileCount()const noexcept; 
		std::wstring GetPath()const noexcept; 
		JFile* GetFile(const uint index)noexcept;
		JFile* GetRecentFile()noexcept;
		J_OBJECT_TYPE GetObjectType()const noexcept final;

		bool HasChild(const std::wstring& name)const noexcept;
		bool HasFile(const std::wstring& name)const noexcept;
		bool IsOpen()const noexcept;

		JDirectory* SearchDirectory(const std::wstring& path)noexcept;
		std::wstring MakeUniqueFileName(const std::wstring& name)noexcept;
	public:
		bool Copy(JObject* ori) final; 
	public:
		JDirectoryDestroyInterface* DestroyInterface() final;
		JDirectoryOCInterface* OCInterface() final;
	private:
		bool CreateJFile(JResourceObject& resource)noexcept;
		bool DestroyJFile(JResourceObject& resource)noexcept;
	private:
		void Destroy()final;
		void Clear();
		void DeleteResourceFile(JResourceObject& resource);
	private:
		void BeginForcedDestroy() final;
	private:
		void OpenDirectory()noexcept final;
		void CloseDirectory()noexcept final;
	private:
		static Core::J_FILE_IO_RESULT StoreObject(JDirectory* dir);
		static JDirectory* LoadObject(JDirectory* parentDir, const JDirectoryPathData& pathData);
		static void RegisterJFunc();
	private:
		JDirectory(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* parentDir);
		~JDirectory();
	};

}