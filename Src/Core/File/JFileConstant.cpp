#include"JFileConstant.h"
#include"../../Utility/JCommonUtility.h"
#include<fstream> 
namespace JinEngine
{
	namespace Core
	{
		std::wstring JFileConstant::GetFileFormat()noexcept
		{
			return L".jAsset";
		}
		std::wstring JFileConstant::GetMetafileFormat()noexcept
		{
			return L".jAssetMeta";
		}
		std::wstring JFileConstant::MakeFilePath(const std::wstring& folderPath, const std::wstring& name)
		{
			return folderPath + L"\\" + name;
		}
		std::wstring JFileConstant::MakeFilePath(const std::wstring& folderPath, const std::wstring& name, const std::wstring& format)
		{
			return folderPath + L"\\" + name + format;
		}
		std::wstring JFileConstant::StreamHasObjGuidSymbol()noexcept
		{
			return L"HasObjGuid: ";
		}
		std::wstring JFileConstant::StreamEndFixedDataSymbol()noexcept
		{
			return L"---End-FinxedData---";
		}
	}
}