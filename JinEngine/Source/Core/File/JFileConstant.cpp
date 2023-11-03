#include"JFileConstant.h"
#include"../Reflection/JTypeInfo.h"
#include"../Utility/JCommonUtility.h"
#include<fstream> 
namespace JinEngine
{
	namespace Core
	{ 
		std::string JFileConstant::GetLastOpenSymbol(const JTypeInfo& typeInfo)noexcept
		{
			return typeInfo.Name() + "IsOpen: ";
		}
		std::string JFileConstant::GetLineSymbol(const std::string& notification)noexcept
		{
			return "_________" + notification + "_________";
		}
		std::wstring JFileConstant::GetLastOpenSymbolW(const JTypeInfo& typeInfo)noexcept
		{
			return JCUtil::StrToWstr(typeInfo.Name()) + L"IsOpen: ";
		}
		std::wstring JFileConstant::GetLineSymbolW(const std::wstring& notification)noexcept
		{
			return L"_________" + notification + L"_________";
		}
		std::wstring JFileConstant::MakeFilePath(const std::wstring& folderPath, const std::wstring& name)
		{
			return folderPath + L"\\" + name;
		}
		std::wstring JFileConstant::MakeFilePath(const std::wstring& folderPath, const std::wstring& name, const std::wstring& format)
		{
			return folderPath + L"\\" + name + format;
		}
	}
}