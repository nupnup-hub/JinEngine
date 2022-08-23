#pragma once
#include<string>
#include<vector> 
#include<memory>
#include"JDirectoryInterface.h"

namespace JinEngine
{
	class JFile;
	class JResourceObject;
	class JDirectory : public JDirectoryOCInterface
	{
		REGISTER_CLASS(JDirectory)
	private:
		JDirectory* parent;
		std::vector<JDirectory*> children;
		std::vector<JFile*> fileList;
	public:
		JDirectory* GetChildDirctory(const uint index)noexcept;
		JDirectory* GetChildDirctory(const std::string& path)noexcept;
		uint GetChildernDirctoryCount()const noexcept;
		uint GetFileCount()const noexcept;
		std::string GetPath()const noexcept;
		std::wstring GetWPath()const noexcept;
		JFile* GetFile(const uint index)noexcept;
		JFile* GetRecentFile()noexcept;
		J_OBJECT_TYPE GetObjectType()const noexcept final;

		bool HasChild(const std::string& name)const noexcept;
		bool HasFile(const std::string& name)const noexcept;
		bool IsOpen()const noexcept;

		JDirectory* SearchDirectory(const std::string& path)noexcept;
		std::string MakeUniqueFileName(const std::string& name)noexcept;
	public:
		JDirectoryDestroyInterface* DestroyInterface() final;
		JDirectoryOCInterface* OCInterface() final;
	private:
		bool CreateJFile(JResourceObject& resource)noexcept;
		bool DestroyJFile(JResourceObject& resource)noexcept;
	protected:
		void Destroy()final;
	private:
		void BeginForcedDestroy() final;
	private:
		void OpenDirectory()noexcept final;
		void CloseDirectory()noexcept final;
		static void RegisterJFunc();
	private:
		JDirectory(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* parentDir);
		~JDirectory();
	};

}