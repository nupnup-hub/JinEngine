#pragma once
#include<vector> 
#include"JResourceType.h" 

namespace JinEngine
{
	namespace Application
	{
		class JApplication;
	}
	 
	class JResourceMangerAppInterface
	{
	private:
		friend class Application::JApplication;
	protected:
		virtual ~JResourceMangerAppInterface() = default;
	public:
		virtual JResourceMangerAppInterface* AppInterface() = 0;
	private:
		virtual void Initialize() = 0;
		virtual void Terminate() = 0;
		virtual void StoreProjectResource() = 0;
		virtual void LoadSelectorResource() = 0;
		virtual void LoadProjectResource() = 0;
	};

	class JResourceObject; 
	class JResourceStorageInterface : public JResourceMangerAppInterface
	{
	private:
		friend class JResourceObject; 
	public:
		virtual JResourceStorageInterface* ResourceStorageInterface() = 0;
	private:
		virtual bool AddResource(JResourceObject& resource)noexcept = 0;
		virtual bool RemoveResource(JResourceObject& resource)noexcept = 0;
	};

	class JDirectory;
	class JDirectoryStorageInterface : public JResourceStorageInterface
	{
	private:
		friend class JDirectory;
	public:
		virtual JDirectoryStorageInterface* DirectoryStorageInterface() = 0;
	private:
		virtual bool AddJDirectory(JDirectory& dir)noexcept = 0;
		virtual bool RemoveJDirectory(JDirectory& dir)noexcept = 0;
	};
}