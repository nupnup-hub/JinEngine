#include"JFilePathData.h"
#include"../Utility/JCommonUtility.h"
#include"../File/JFileConstant.h"

namespace JinEngine
{ 
	namespace Core
	{
		JFilePathData::JFilePathData(const std::wstring& oriPath)
			:path(oriPath)
		{ 
			JCUtil::DecomposeFilePath(oriPath, folderPath, name, format);
			fullName = name + format;
		}
		JAssetFilePathData::JAssetFilePathData(const std::wstring& path)
			: JFilePathData(path)
		{ 
			metaFilePath = folderPath + name + Core::JFileConstant::GetMetaFileFormatW();
		}
		JFileImportPathData::JFileImportPathData(const std::wstring& oriPath, const int flag)
			:JAssetFilePathData(JCUtil::ChangeFileFormat(oriPath, Core::JFileConstant::GetFileFormatW())),
			oriFileWPath(oriPath), 
			oriFilePath(JCUtil::WstrToU8Str(oriPath)),
			flag(flag)
		{ 
			oriFileFormat = JCUtil::GetFileFormat(oriFileWPath);
		}
	}
}