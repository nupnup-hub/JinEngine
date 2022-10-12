#include"JCommonUtility.h"
#include <atlbase.h>
#include <atlconv.h>
#include<Windows.h>
#include <algorithm> 
#include <optional> 

namespace JinEngine
{
	int JCUtil::GetPathLastBackSlash(const std::wstring& path)noexcept
	{
		return (int)path.find_last_of(L"\\") + 1;
	}
	int JCUtil::GetPathLastBackSlash(const std::string& path)noexcept
	{
		return (int)path.find_last_of("\\") + 1;
	}
	int JCUtil::GetPathLastPeriod(const std::wstring& path)noexcept
	{
		return (int)path.find_last_of(L".");
	}
	int JCUtil::GetPathLastPeriod(const std::string& path)noexcept
	{
		return (int)path.find_last_of(".");
	}
	std::wstring JCUtil::GetFileFormat(const std::wstring& path)noexcept
	{
		size_t index = path.find_last_of(L".");
		return path.substr(index, path.size() - index);
	}
	std::string JCUtil::GetFileFormat(const std::string& path)noexcept
	{
		size_t index = path.find_last_of(".");
		return path.substr(index, path.size() - index);
	}
	size_t JCUtil::StringToInt(const std::string& str)noexcept
	{
		std::string number = EraseSideChar(str, ' '); 

		size_t res = 0; 
		for (uint i = 0; i < number.size(); ++i)
			res = res * 10 + (number[i] - '0'); 
		return res;
	}
	size_t JCUtil::WstringToInt(const std::wstring& wstr)noexcept
	{
		return StringToInt(WstrToU8Str(wstr));
	}
	float JCUtil::StringToFloat(const std::string& str)noexcept
	{
		std::string number = EraseSideChar(str, ' ');
		int dotIndex = number.find_first_of(".");
		if (dotIndex == -1)
			return StringToInt(str);
		else
		{
			std::string integarStr = number.substr(0, dotIndex);
			std::string floatStr = number.substr(dotIndex + 1);
			float m = pow(10, floatStr.size());
			return StringToInt(integarStr) + ((float)StringToInt(floatStr) / m);
		}
	}
	float JCUtil::WstringToFloat(const std::wstring& wstr)noexcept
	{
		return StringToFloat(WstrToU8Str(wstr));
	}
 
	const wchar_t* JCUtil::CharToWChar(const char* src)
	{
		size_t newsize = strlen(src) + 1;
		wchar_t* wcstring = new wchar_t[newsize];
		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, wcstring, newsize, src, _TRUNCATE);
		return wcstring;
	}
	std::wstring JCUtil::StrToWstr(const std::string& str)noexcept
	{
		return std::wstring(CA2W(str.c_str()));
	}
	std::string JCUtil::WstrToU8Str(const std::wstring& wstr)noexcept
	{
		return std::string(CW2A(wstr.c_str(), CP_UTF8));
	}
	std::wstring JCUtil::U8StrToWstr(const std::string& str)noexcept
	{
		return std::wstring(CA2W(str.c_str(), CP_UTF8));
	}
	std::string JCUtil::EraseChar(const std::string& str, const char ch)noexcept
	{
		std::string newStr;
		for (uint i = 0; i < str.size(); ++i)
		{
			if (str[i] != ch && str[i] != 0x00d)
				newStr.push_back(str[i]);
		}
		return newStr;
	}
	std::wstring JCUtil::EraseWChar(const std::wstring& wstr, const wchar_t ch)noexcept
	{
		std::wstring newStr;
		for (uint i = 0; i < wstr.size(); ++i)
		{
			if (wstr[i] != ch && wstr[i] != 0x00d)
				newStr.push_back(wstr[i]);
		}
		return newStr;
	}
	std::string JCUtil::EraseSideChar(const std::string& str, const char ch)noexcept
	{ 
		std::string newStr = str;
		for (int i = (int)newStr.size() - 1; i >= 0; --i)
		{
			if (newStr[i] != ch && newStr[i] != 0x00d)
				break;
			else
				newStr.pop_back();
		}

		for (int i = 0; i < newStr.size(); ++i)
		{
			if (newStr[i] != ch)
				break;
			else
				newStr = newStr.substr(i + 1);
		}
		return newStr;
	}
	std::wstring JCUtil::EraseSideWChar(const std::wstring& wstr, const wchar_t ch)noexcept
	{
		std::wstring newWStr = wstr;
		for (int i = (int)newWStr.size() - 1; i >= 0; --i)
		{
			if (newWStr[i] != ch && newWStr[i] != 0x00d)
				break;
			else
				newWStr.pop_back();
		}

		for (int i = 0; i < newWStr.size(); ++i)
		{
			if (newWStr[i] != ch)
				break;
			else
				newWStr = newWStr.substr(i + 1);
		}
		return newWStr;
	}
	std::wstring JCUtil::ComporessWstring(const std::wstring& wstr, const uint lange)noexcept
	{
		if (wstr.size() >= lange)
			return wstr.substr(0, lange) + L"...";
		else
			return wstr;
	}
	std::string JCUtil::ComporessString(const std::string& str, const uint lange)noexcept
	{
		if (str.size() >= lange)
			return str.substr(0, lange) + "...";
		else
			return str;
	}
	bool JCUtil::IsNumber(const char ch)noexcept
	{
		return (ch > 47 && ch < 58);
	}
	bool JCUtil::IsNumber(const wchar_t ch)noexcept
	{
		return (ch > 47 && ch < 58);
	}
	bool JCUtil::Contain(const std::wstring& source, const std::wstring& target)noexcept
	{
		bool findSt = false;
		int i = 0;
		for (; i < source.size(); ++i)
		{
			if (source[i] == target[0])
			{
				findSt = true;
				break;
			}
		}

		if (!findSt)
			return false;

		if (target.size() + i - 1 < source.size())
		{
			for (int j = i, k = 0; k < target.size(); ++j, ++k)
			{
				if (source[j] != target[k])
					return false;
			}
			return true;
		}
		else
			return false;
	}
	bool JCUtil::Contain(const std::string& source, const std::string& target)noexcept
	{
		bool findSt = false;
		int i = 0;
		for (; i < source.size(); ++i)
		{
			if (source[i] == target[0])
			{
				findSt = true;
				break;
			}
		}

		if (!findSt)
			return false;

		if (target.size() + i - 1 < source.size())
		{
			for (int j = i, k = 0; k < target.size(); ++j, ++k)
			{
				if (source[j] != target[k])
					return false;
			}
			return true;
		}
		else
			return false;
	}
	void JCUtil::DecomposeFolderPath(const std::wstring& path, std::wstring& folderPath, std::wstring& name)noexcept
	{
		const int nameStIndex = GetPathLastBackSlash(path);
		if (nameStIndex != -1)
		{
			name = path.substr(nameStIndex, (path.size() - nameStIndex));
			folderPath = path.substr(0, nameStIndex);
		}
		else
		{
			folderPath = L"";
			name = path;
		}
	}
	void JCUtil::DecomposeFilePath(const std::wstring& path, std::wstring& folderPath, std::wstring& name, std::wstring& format, bool eraseFolderPathBackSlash)noexcept
	{
		const int nameStIndex = GetPathLastBackSlash(path);
		const int formatStIndex = GetPathLastPeriod(path);

		if (nameStIndex != -1)
		{
			name = path.substr(nameStIndex, (formatStIndex - nameStIndex));
			folderPath = path.substr(0, nameStIndex);
		}
		else if (formatStIndex != -1)
		{
			name = path.substr(0, formatStIndex);
			folderPath = L"";
		}
		else
		{
			name = path;
			folderPath = L"";
		}

		if (formatStIndex != -1)
			format = path.substr(formatStIndex, path.size() - formatStIndex);
		else
			format = L"";

		if (eraseFolderPathBackSlash)
		{
			const int lastBackSlash = GetPathLastBackSlash(folderPath);
			if (lastBackSlash != -1)
				folderPath = folderPath.substr(0, lastBackSlash - 2);
		}
	} 
	void JCUtil::DecomposeFileName(const std::wstring& oriname, std::wstring& name, std::wstring& format)
	{
		const int formatStIndex = GetPathLastPeriod(oriname);
		if (formatStIndex != -1)
		{
			name = oriname.substr(0, formatStIndex);
			format = oriname.substr(formatStIndex, oriname.size() - formatStIndex);
		}
	}
	std::wstring JCUtil::DecomposeFileFormat(const std::wstring& path)noexcept
	{
		const int formatStIndex = GetPathLastPeriod(path);
		return formatStIndex != -1 ? path.substr(formatStIndex, path.size() - formatStIndex) : L"Error";
	}
	void JCUtil::MakeFileName(std::wstring& name, const std::wstring& format, const std::wstring& folderPath) noexcept
	{
		bool result = true;
		int count = 0;
		while (result)
		{
			if (!IsOverlappedFilepath(name + format, folderPath))
				result = false;
			else
			{
				ModifyOverlappedName(name, name.length(), count);
				++count;
			}
		}
	}
	void JCUtil::MakeFileName(std::string& name, const std::string& format, const std::string& folderPath) noexcept
	{
		bool result = true;
		int count = 0;
		while (result)
		{
			if (!IsOverlappedFilepath(U8StrToWstr(name + format), U8StrToWstr(folderPath)))
				result = false;
			else
			{
				ModifyOverlappedName(name, name.length(), count);
				++count;
			}
		}
	}
	bool JCUtil::IsOverlappedFilepath(const std::wstring& name, const std::wstring& folderPath) noexcept
	{
		WIN32_FIND_DATA  findFileData;
		HANDLE hFindFile = FindFirstFile((folderPath + L"\\*.*").c_str(), &findFileData);

		BOOL bResult = TRUE;

		while (bResult)
		{
			if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if (wcscmp(findFileData.cFileName, L".") && wcscmp(findFileData.cFileName, L".."))
				{
					if (name == findFileData.cFileName)
						return true;
				}
			}
			bResult = FindNextFile(hFindFile, &findFileData);
		}
		FindClose(hFindFile);
		return false;
	}
	bool JCUtil::IsOverlappedDirectoryPath(const std::wstring& name, const std::wstring& folderPath) noexcept
	{
		WIN32_FIND_DATA  findFileData;
		HANDLE hFindFile = FindFirstFile((folderPath + L"\\*.*").c_str(), &findFileData);

		BOOL bResult = TRUE;

		while (bResult)
		{
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (wcscmp(findFileData.cFileName, L".") && wcscmp(findFileData.cFileName, L".."))
				{
					if (name == findFileData.cFileName)
						return true;
				}
			}
			bResult = FindNextFile(hFindFile, &findFileData);
		}
		FindClose(hFindFile);
		return false;
	}
	void JCUtil::ModifyOverlappedName(std::wstring& name, size_t length, int newNumber)noexcept
	{
		bool isChangedname = true;
		int index = (int)name.find_last_of('(');
		if (name[length - 1] == ')' && index > -1)
		{
			for (int i = index + 1; i < length - 1; ++i)
			{
				if (!IsNumber(name[i]))
				{
					isChangedname = false;
					break;
				}
			}
		}
		else
			isChangedname = false;

		if (isChangedname)
		{
			std::wstring newNumberWstr = std::to_wstring(newNumber);

			name.erase(name.begin() + index, name.end());
			name.push_back('(');
			name += newNumberWstr;
			name.push_back(')');
			length = name.length();
		}
		else
		{
			name.push_back('(');
			name.push_back('0');
			name.push_back(')');
			length = name.length();
		}
	}
	void JCUtil::ModifyOverlappedName(std::string& name, size_t length, int newNumber)noexcept
	{
		bool isChangedname = true;
		int index = (int)name.find_last_of('(');
		if (name[length - 1] == ')' && index > -1)
		{
			for (int i = index + 1; i < length - 1; ++i)
			{
				if (!IsNumber(name[i]))
				{
					isChangedname = false;
					break;
				}
			}
		}
		else
			isChangedname = false;

		if (isChangedname)
		{
			std::string newNumberWstr = std::to_string(newNumber);

			name.erase(name.begin() + index, name.end());
			name.push_back('(');
			name += newNumberWstr;
			name.push_back(')');
			length = name.length();
		}
		else
		{
			name.push_back('(');
			name.push_back('0');
			name.push_back(')');
			length = name.length();
		}
	}
	void JCUtil::DeleteDirectoryFile(const std::string& path)
	{
		remove(path.c_str());
	}
	size_t JCUtil::CalculateGuid(const std::string& path)noexcept
	{ 
		return std::hash<std::string>{}(path);
	}
	size_t JCUtil::CalculateGuid(const std::wstring& path)noexcept
	{ 
		return  std::hash<std::wstring>{}(path);
	}
}