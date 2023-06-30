#pragma once
#include<string>
namespace JinEngine
{
	namespace Core
	{
		class JScriptCreator
		{
		public:
			static std::wstring CreateBehaviorHeaderFile(const std::wstring& name)
			{
				std::wstring header;
				header += L"#pragma once\n";
				header += L"#include \"JBehavior.h\" \n";
				header += L"\n";
				header += L"namespace JinEngine\n";
				header += L"{\n";
				header += L"	class " + name + L": public JBehavior\n";
				header += L"	{\n";
				header += L"		REGISTER_CLASS_BEHAVIOR_DERIVED_LINE(" + name + L")\n";
				header += L"	public:\n";
				header += L"		class InitData final : public JBehavior::InitData\n";
				header += L"		{\n";
				header += L"			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)\n";
				header += L"		public:\n";
				header += L"			InitData(const JUserPtr<JGameObject>& owner);\n";
				header += L"		};\n";
				header += L"		private:\n";
				header += L"			void NotifyActivate() final;\n";
				header += L"			void NotifyDeActivate() final;\n";
				header += L"		private:\n";
				header += L"			void Initialize() final;							/*call after create object*/\n";
				header += L"			void Clear() final;									/*call before destroy object*/\n";
				header += L"			bool Copy(JUserPtr<Core::JIdentifier> to)final;		/*call after JBehavior copied*/\n";	
				header += L"		private:\n";
				header += L"			void Update()final;\n";
				header += L"		private:\n";
				header += L"			void CallOneceWhenRegisterTypeData();\n";
				header += L"		private:\n";
				header += L"			" + name + L"(const InitData& initData);\n";
				header += L"			~" + name + L"\n";
				header += L"    };\n";
				header += L"}\n";
				return header;
			}
			static std::wstring CreateBehaviorCppFile(const std::wstring& name)
			{
				std::wstring cpp;
				cpp += L"#include \"" + name + L".h\"";
				cpp += L"\n";
				cpp += L"namespace JinEngine\n";
				cpp += L"{\n";
				cpp += L"    " + name + L"::InitData::InitData(const JUserPtr<JGameObject>&owner)\n";
				cpp += L"		:JBehavior::InitData(name::StaticTypeInfo(), owner)\n";
				cpp += L"    {}\n";
				cpp += L"    void name::NotifyActivate()\n";
				cpp += L"	{\n";
				cpp += L"\n";
				cpp += L"	}\n";
				cpp += L"    void name::NotifyDeActivate()\n";
				cpp += L"	{\n";
				cpp += L"\n";
				cpp += L"	}\n";
				cpp += L"    void name::Initialize()\n";
				cpp += L"	{\n";
				cpp += L"\n";
				cpp += L"	}\n";
				cpp += L"    void name::Clear()\n";
				cpp += L"	{\n";
				cpp += L"\n";
				cpp += L"	}\n";
				cpp += L"    bool name::Copy(JUserPtr<Core::JIdentifier> to)\n";
				cpp += L"	{\n";
				cpp += L"		return JBehavior::Copy(to)\n;";
				cpp += L"	}\n";
				cpp += L"    void name::Update()\n";
				cpp += L"	{\n";
				cpp += L"\n";
				cpp += L"	}\n";
				cpp += L"    void name::CallOneceWhenRegisterTypeData()\n";
				cpp += L"	{\n";
				cpp += L"\n";
				cpp += L"	}\n";
				cpp += L"	" + name + L"::" + name + L"(const InitData& initData)\n";
				cpp += L"		:JBehavior(initData)\n";
				cpp += L"	{}\n";
				cpp += L"	" + name + L"::~" + name + L"(const InitData& initData)\n";
				cpp += L"	{}\n";
				cpp += L"}\n";
				return cpp;
			}
		};
	}
}
