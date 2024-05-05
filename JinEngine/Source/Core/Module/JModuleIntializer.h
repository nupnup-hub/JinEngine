#pragma once 
#include"JModuleDesc.h" 


namespace JinEngine
{
	namespace Core
	{
		class JModuleIntializer
		{
		public:
			const std::wstring name;
			const std::wstring path;
		public:
			const JModuleDesc desc;
		public:
			JModuleIntializer(const std::wstring& path, const JModuleDesc& desc);
			JModuleIntializer(const std::wstring& name,
				const std::wstring& path,
				const JModuleDesc& desc);
		};
	}
}