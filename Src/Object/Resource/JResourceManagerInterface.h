#pragma once
#include<vector>
#include"JResourceData.h"  
#include"JResourceType.h" 

namespace JinEngine
{
	namespace Application
	{
		class JApplication;
	}

	class JResourceMangerOwnerInterface
	{
	private:
		friend class Application::JApplication;
	protected:
		virtual ~JResourceMangerOwnerInterface() = default;
	public:
		virtual JResourceMangerOwnerInterface* OwnerInterface() = 0;
	private:
		virtual void LoadSelectorResource() = 0;
		virtual void LoadProjectResource() = 0;
		virtual void Terminate() = 0;
	};

	class JResourceObject; 
	class JResourceRemoveInterface : public JResourceMangerOwnerInterface
	{
	private:
		friend class JResourceObject; 
	public:
		virtual JResourceRemoveInterface* ResourceRemoveInterface() = 0;
	private:
		virtual bool RemoveResource(JResourceObject& resource)noexcept = 0;
	};

	class JDirectory;
	class JDirectoryRemoveInterface : public JResourceRemoveInterface
	{
	private:
		friend class JDirectory;
	public:
		virtual JDirectoryRemoveInterface* DirectoryRemoveInterface() = 0;
	private:
		virtual bool RemoveJDirectory(JDirectory& dir)noexcept = 0;
	};
}