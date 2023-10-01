#include"JFilePathData.h"
#include"../Utility/JCommonUtility.h"
#include"../File/JFileConstant.h"

namespace JinEngine
{ 
	namespace Core
	{
		JAssetFileLoadPathData::JAssetFileLoadPathData(const std::wstring& engineFileWPath)
			:engineFileWPath(engineFileWPath)
		{
			JCUtil::DecomposeFilePath(engineFileWPath, folderPath, name, format);
			engineMetaFileWPath = folderPath + name + Core::JFileConstant::GetMetaFileFormat();
		}
		JFileImportHelpData::JFileImportHelpData(const std::wstring& oriPath, const int flag)
			: oriFileWPath(oriPath), oriFilePath(JCUtil::WstrToU8Str(oriPath)), flag(flag)
		{
			JCUtil::DecomposeFilePath(oriFileWPath, folderPath, name, format);
			engineFileWPath = folderPath + name + Core::JFileConstant::GetFileFormat();
			engineMetaFileWPath = folderPath + name + Core::JFileConstant::GetMetaFileFormat();
			fullName = name + format;
		}
	}
}