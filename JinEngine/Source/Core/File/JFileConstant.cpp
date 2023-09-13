#include"JFileConstant.h"
#include"../Reflection/JTypeInfo.h"
#include"../Utility/JCommonUtility.h"
#include<fstream> 
namespace JinEngine
{
	namespace Core
	{
		std::wstring JFileConstant::GetFileFormat()noexcept
		{
			return L".jAsset";
		}
		std::wstring JFileConstant::GetMetaFileFormat()noexcept
		{
			return L".jAssetMeta";
		}
		std::wstring JFileConstant::GetCacheFileFormat()noexcept
		{
			return L".jCache";
		}
		std::wstring JFileConstant::MakeFilePath(const std::wstring& folderPath, const std::wstring& name)
		{
			return folderPath + L"\\" + name;
		}
		std::wstring JFileConstant::MakeFilePath(const std::wstring& folderPath, const std::wstring& name, const std::wstring& format)
		{
			return folderPath + L"\\" + name + format;
		}
		std::wstring JFileConstant::StreamObjGuidSymbol()noexcept
		{
			return L"ObjGuid: ";
		}
		std::wstring JFileConstant::StreamObjFlagSymbol()noexcept
		{
			return L"ObjFlag: ";
		}
		std::wstring JFileConstant::StreamTypeGuidSymbol()noexcept
		{
			return L"TypeGuid: ";
		}
		std::wstring JFileConstant::StreamHasObjGuidSymbol()noexcept
		{
			return L"HasObjGuid: ";
		}
		std::wstring JFileConstant::StreamUncopiableGuidSymbol()noexcept
		{
			return L"UncopiableObjGuid: ";
		}
		std::wstring JFileConstant::StreamEndFixedDataSymbol()noexcept
		{
			return L"---End-FinxedData---";
		}
		std::wstring JFileConstant::StreamFormatSymbol()noexcept
		{
			return L"FormatIdnex: ";
		}
		std::wstring JFileConstant::StreamFormatIndexSymbol()noexcept
		{
			return L"Format: ";
		}
		std::wstring JFileConstant::StreamCombineFileEnd()noexcept
		{
			return L"---End-Combine---";
		}
		std::wstring JFileConstant::StreamLastOpenSymbol(const JTypeInfo& typeInfo)noexcept
		{
			return JCUtil::StrToWstr(typeInfo.Name()) + L"IsOpen: ";
		}
		std::wstring JFileConstant::StreamLine(const std::wstring& notification)noexcept
		{
			return L"_________" + notification + L"_________";
		}
	}
}