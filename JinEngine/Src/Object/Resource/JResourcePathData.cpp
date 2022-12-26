#include"JResourcePathData.h"
#include"../../Utility/JCommonUtility.h"
#include"../../Core/File/JFileConstant.h"

namespace JinEngine
{ 
	JAssetFileLoadPathData::JAssetFileLoadPathData(const std::wstring& engineFilePath)
		:engineFilePath(engineFilePath)
	{
		std::wstring format;
		JCUtil::DecomposeFilePath(engineFilePath, folderPath, name, format);
		engineMetaFilePath = folderPath + name + Core::JFileConstant::GetMetafileFormat();
	}

	JFileImportHelpData::JFileImportHelpData(const std::wstring& oriPath)
		:oriFileWPath(oriPath), oriFilePath(JCUtil::WstrToU8Str(oriPath))
	{
		JCUtil::DecomposeFilePath(oriFileWPath, folderPath, name, format);
		engineFilePath = folderPath + L"\\" + name + Core::JFileConstant::GetFileFormat();
		engineMetaFilePath = folderPath + L"\\" + name + Core::JFileConstant::GetMetafileFormat();
		fullName = name + format;
	}
}