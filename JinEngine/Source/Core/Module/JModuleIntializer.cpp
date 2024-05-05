#include"JModuleIntializer.h"
#include"../Utility/JCommonUtility.h"

namespace JinEngine::Core
{
	JModuleIntializer::JModuleIntializer(const std::wstring& path, const JModuleDesc& desc)
		:name(JCUtil::GetFileName(path)), path(path), desc(desc)
	{}
	JModuleIntializer::JModuleIntializer(const std::wstring& name,
		const std::wstring& path,
		const JModuleDesc& desc)
		:name(name), path(path), desc(desc)
	{}
}