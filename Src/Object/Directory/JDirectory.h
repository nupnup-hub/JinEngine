#pragma once
#include<string>
#include<vector> 
#include<memory>
#include"JDirectoryInterface.h"
#include"../../Core/File/JFilePathData.h"

namespace JinEngine
{
	class JFile;
	class JResourceObject;
	class JDirectory final : public JDirectoryInterface
	{
		REGISTER_CLASS(JDirectory)
	private:
		JDirectory* parent;
		std::vector<JDirectory*> children;
		std::vector<JFile*> fileList;
	public:
		JDirectory* GetParent()noexcept;
		JDirectory* GetChildDirctory(const uint index)noexcept;
		JDirectory* GetChildDirctory(const std::wstring& path)noexcept;
		uint GetChildernDirctoryCount()const noexcept;
		uint GetFileCount()const noexcept; 
		std::wstring GetPath()const noexcept;  
		std::wstring GetMetafilePath()const noexcept;
		JFile* GetFile(const uint index)noexcept;
		JFile* GetFile(const std::wstring name)noexcept;
		JFile* GetRecentFile()noexcept;
		J_OBJECT_TYPE GetObjectType()const noexcept final;
		void SetName(const std::wstring& name)noexcept final;

		bool HasChild(const std::wstring& name)const noexcept;
		bool HasFile(const std::wstring& name)const noexcept;
		bool IsOpen()const noexcept;

		JDirectory* SearchDirectory(const std::wstring& path)noexcept;
		std::wstring MakeUniqueFileName(const std::wstring& name)noexcept;
	public:
		JDirectoryOCInterface* OCInterface() final;
	private:
		void DoCopy(JObject* ori) final;  
	private:
		bool CreateJFile(JResourceObject& resource)noexcept;
		bool DestroyJFile(JResourceObject& resource)noexcept;
	private:
		bool Destroy()final;
		void Clear(); 
	private:
		void OpenDirectory()noexcept final;
		void CloseDirectory()noexcept final;
	private:
		//Create or exist dir is true
		bool CreateDirectoryFile();
		void DeleteDirectoryFile(); 
	private:
		bool RegisterCashData()noexcept final;
		bool DeRegisterCashData()noexcept final;
	private:
		static Core::J_FILE_IO_RESULT StoreObject(JDirectory* dir);
		static JDirectory* LoadObject(JDirectory* parentDir, const Core::JAssetFileLoadPathData& pathData);
		static void RegisterJFunc();
	private:
		JDirectory(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* parentDir);
		~JDirectory();
	};

}