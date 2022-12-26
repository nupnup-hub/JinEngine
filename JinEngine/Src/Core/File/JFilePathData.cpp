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
		JFileImportHelpData::JFileImportHelpData(const std::wstring& oriPath, const J_OBJECT_FLAG flag)
			: oriFileWPath(oriPath), oriFilePath(JCUtil::WstrToU8Str(oriPath)), flag(flag)
		{
			JCUtil::DecomposeFilePath(oriFileWPath, folderPath, name, format);
			engineFileWPath = folderPath + L"\\" + name + Core::JFileConstant::GetFileFormat();
			engineMetaFileWPath = folderPath + L"\\" + name + Core::JFileConstant::GetMetafileFormat();
			fullName = name + format;
		}
	}
}