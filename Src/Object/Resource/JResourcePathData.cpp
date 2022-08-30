#include"JResourcePathData.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	JResourcePathData::JResourcePathData(const std::wstring& oriPath)
		:wstrPath(oriPath), strPath(JCommonUtility::WstrToU8Str(oriPath))
	{
		JCommonUtility::DecomposeFilePath(wstrPath, folderPath, name, format);
		fullName = name + format;
	}
}