#include"JCplusScriptCreator.h"
#include"../Utility/JMacroUtility.h"
 
namespace JinEngine::Core
{
	std::string JCplusScriptCreator::CreateProjectDefaultHeader(const std::string& name)
	{ 
		std::string header;
		header += "// JinEngine User Project: " + name + "\n\n";
		header += "#pragma once \n";	
		header += "#include \"Core/Module/JModuleMacro.h\"\n";
		header += "class " + name + "{};\n";
		header += "MODULE_FUNC void Test();\n";
		return header;
	}
	std::string JCplusScriptCreator::CreateProjectDefaultCpp(const std::string& name)
	{
		/*
namespace JinEngine::Core
{
	class _JModuleRegister
	{
	public:
		_JModuleRegister(const std::string& name, const JModuleDesc& desc = JModuleDesc())
		{
			std::wstring wName = JCUtil::U8StrToWstr(name);
			std::wstring modulePath = JApplicationProject::OutDirPath() + L"\\" + wName + L".dll";
			_JModuleManager::Instance().RegisterModule(JModuleIntializer{ wName , modulePath, desc });
		}
	};
}
		*/
		std::string cpp; 
		cpp += "#include \"" + name + ".h\"\n";
		cpp += "#include \"Core/Module/JModuleUser.h\"\n";
		cpp += "namespace JinEngine::Core\n";
		cpp += "{\n";
		cpp += "	class _JModuleRegister\n";
		cpp += "	{\n";
		cpp += "	public:\n";
		cpp += "		_JModuleRegister(const std::string& name, const JModuleDesc& desc = JModuleDesc())\n";
		cpp += "	{\n";
		cpp += "	std::wstring wName = JCUtil::U8StrToWstr(name);\n";
		cpp += "	std::wstring modulePath = JApplicationProject::OutDirPath() + L\"\\ \" + wName + L\".dll\";\n";
		cpp += "	_JModuleManager::Instance().RegisterModule(JModuleIntializer{ wName , modulePath, desc });\n";
		cpp += "	}\n";
		cpp += "	public:\n";
		cpp += "	};\n";
		cpp += "}\n";
		cpp += "inline static struct MRegister{MRegister(){JinEngine::Core::_JModuleRegister(\""+ name +"\");}}MRegister;\n";
		cpp += "inline static struct Caller{Caller(){MessageBox(0, JinEngine::Application::JApplicationProject::RootPath().c_str(),L\"Caller\", 0);}}Caller;\n";
		cpp += "MODULE_FUNC void Test(){}\n";
		return cpp;
	}
	std::string JCplusScriptCreator::CreateProjectDllMainCpp(const std::string& name)
	{ 
		std::string cpp;
		cpp += "#include \"" + name + ".h\"\n";
		cpp += "#include \"Pch.h\"\n";
		cpp += "\n";
		cpp += "BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)\n";
		cpp += "{\n";		
		cpp += "	switch (ul_reason_for_call)\n";
		cpp += "	{\n";
		cpp += "	case DLL_PROCESS_ATTACH:\n";
		cpp += "	case DLL_THREAD_ATTACH:\n";
		cpp += "	case DLL_THREAD_DETACH:\n";
		cpp += "	case DLL_PROCESS_DETACH:\n";
		cpp += "		break;\n";
		cpp += "	}\n";
		cpp += "	return TRUE;";
		cpp += "}\n";
		return cpp;
	}
	std::string JCplusScriptCreator::CreateProjectPchHeader()
	{
		std::string header; 
		header += "#pragma once \n";
		header += "#include <windows.h>\n";
		return header;
	}
	std::string JCplusScriptCreator::CreateProjectPchCpp()
	{
		std::string cpp; 
		cpp += "#include \"pch.h\"\n"; 
		return cpp;
	}
	std::string JCplusScriptCreator::CreateBehaviorHeader(const std::string& name)
	{
		std::string header;
		header += "#pragma once\n";
		header += "#include \"JBehavior.h\" \n";
		header += "#include \"JBehaviorMacro.h\" \n";
		header += "\n";
		header += "namespace JinEngine\n";
		header += "{\n";
		header += "	class " + name + ": public JBehavior\n";
		header += "	{\n";
		header += "		REGISTER_CLASS_BEHAVIOR_DERIVED_LINE(" + name + ")\n";
		header += "	public:\n";
		header += "		class InitData final : public JBehavior::InitData\n";
		header += "		{\n";
		header += "			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)\n";
		header += "		public:\n";
		header += "			InitData(const JUserPtr<JGameObject>& owner);\n";
		header += "		};\n";
		header += "		private:\n";
		header += "			void NotifyActivate() final;\n";
		header += "			void NotifyDeActivate() final;\n";
		header += "		private:\n";
		header += "			void Initialize() final;							/*call after create object*/\n";
		header += "			void Clear() final;									/*call before destroy object*/\n";
		header += "			bool Copy(JUserPtr<Core::JIdentifier> to)final;		/*call after JBehavior copied*/\n";
		header += "		private:\n";
		header += "			void Update()final;\n";
		header += "		private:\n";
		header += "			void CallOneceWhenRegisterTypeData();\n";
		header += "		private:\n";
		header += "			" + name + "(const InitData& initData);\n";
		header += "			~" + name + "\n";
		header += "    };\n";
		header += "}\n";
		return header;
	}
	std::string JCplusScriptCreator::CreateBehaviorCpp(const std::string& name)
	{ 
		std::string cpp;
		cpp += "#include \"" + name + ".h\"\n";
		cpp += "\n";
		cpp += "namespace JinEngine\n";
		cpp += "{\n";
		cpp += "    " + name + "::InitData::InitData(const JUserPtr<JGameObject>&owner)\n";
		cpp += "		:JBehavior::InitData(name::StaticTypeInfo(), owner)\n";
		cpp += "    {}\n";
		cpp += "    void name::NotifyActivate()\n";
		cpp += "	{\n";
		cpp += "\n";
		cpp += "	}\n";
		cpp += "    void name::NotifyDeActivate()\n";
		cpp += "	{\n";
		cpp += "\n";
		cpp += "	}\n";
		cpp += "    void name::Initialize()\n";
		cpp += "	{\n";
		cpp += "\n";
		cpp += "	}\n";
		cpp += "    void name::Clear()\n";
		cpp += "	{\n";
		cpp += "\n";
		cpp += "	}\n";
		cpp += "    bool name::Copy(JUserPtr<Core::JIdentifier> to)\n";
		cpp += "	{\n";
		cpp += "		return JBehavior::Copy(to)\n;";
		cpp += "	}\n";
		cpp += "    void name::Update()\n";
		cpp += "	{\n";
		cpp += "\n";
		cpp += "	}\n";
		cpp += "    void name::CallOneceWhenRegisterTypeData()\n";
		cpp += "	{\n";
		cpp += "\n";
		cpp += "	}\n";
		cpp += "	" + name + "::" + name + "(const InitData& initData)\n";
		cpp += "		:JBehavior(initData)\n";
		cpp += "	{}\n";
		cpp += "	" + name + "::~" + name + "(const InitData& initData)\n";
		cpp += "	{}\n";
		cpp += "}\n";
		return cpp;
	}
}