#include"JFilePathData.h"
#include"../../Utility/JCommonUtility.h"
#include"../../Core/File/JFileConstant.h"

namespace JinEngine
{ 
	namespace Core
	{
		JAssetFileLoadPathData::JAssetFileLoadPathData(const std::wstring& engineFileWPath)
			:engineFileWPath(engineFileWPath)
		{ 
			JCUtil::DecomposeFilePath(engineFileWPath, folderPath, name, format);
			engineMetaFileWPath = folderPath + L"\\" + name + Core::JFileConstant::GetMetafileFormat();
		}
		JFileImportPathData::JFileImportPathData(const std::wstring& oriPath)
			: oriFileWPath(oriPath), oriFilePath(JCUtil::WstrToU8Str(oriPath))
		{
			JCUtil::DecomposeFilePath(oriFileWPath, folderPath, name, format);
			engineFileWPath = folderPath + L"\\" + name + Core::JFileConstant::GetFileFormat();
			engineMetaFileWPath = folderPath + L"\\" + name + Core::JFileConstant::GetMetafileFormat();
			fullName = name + format;
		}
	}
}