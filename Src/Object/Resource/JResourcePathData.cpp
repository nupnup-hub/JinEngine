#include"JResourcePathData.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	JResourcePathData::JResourcePathData(const std::wstring& oriPath)
		:wstrPath(oriPath), strPath(JCommonUtility::WstringToU8String(oriPath))
	{
		JCommonUtility::DecomposeFilePath(strPath, folderPath, name, format);
	}
}