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
}