#pragma once

#ifdef _WIN32
#ifdef BUILDING_CORE
#define CORE_API __declspec(dllexport)
#define MODULE_API __declspec(dllimport)
#else
#define CORE_API __declspec(dllimport)
#define MODULE_API __declspec(dllexport)
#endif
#else
#define CORE_API
#define MODULE_API
#endif

#define CORE_FUNC extern "C" CORE_API
#define MODULE_FUNC extern "C" MODULE_API
 
#define INIT_MODULE "InitModule"
#define FREE_MODULE "FreeModule" 
 
#define REGISTER_MODULE(path, ...)	\
inline static JinEngine::Core::JModuleRegister<name> name##ModuleRegister(#path, __VA_ARGS__);   \
																								 \
