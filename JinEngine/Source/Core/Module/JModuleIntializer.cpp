#include"JModuleIntializer.h"

namespace JinEngine::Core
{
	JModuleIntializer::JModuleIntializer(const std::wstring& name,
		const std::wstring& path,
		const JModuleDesc& desc)
		:name(name), path(path), desc(desc)
	{}
}