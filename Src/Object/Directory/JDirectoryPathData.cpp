#include"JDirectoryPathData.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	JDirectoryPathData::JDirectoryPathData(const std::wstring& oripath)
		:wstrPath(oripath), strPath(JCommonUtility::WstrToU8Str(oripath))
	{
		JCommonUtility::DecomposeFolderPath(wstrPath, folderPath, name);
	}
}