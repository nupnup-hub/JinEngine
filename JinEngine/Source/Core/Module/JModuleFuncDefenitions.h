#pragma once
namespace JinEngine
{
	class JObject;
	namespace Core
	{
		typedef bool (*CreateModuleMetaFileFunc)(const wchar_t*); 
		typedef void (*JInitModuleFunc)();
		typedef void (*JFreeModuleFunc)();
	}
}