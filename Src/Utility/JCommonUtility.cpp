#include"JCommonUtility.h"
#include <atlbase.h>
#include <atlconv.h>
#include<Windows.h>
#include <algorithm> 
#include <optional> 

namespace JinEngine
{
	int JCommonUtility::GetPathLastBackSlash(const std::wstring& path)noexcept
	{
		return (int)path.find_last_of(L"\\") + 1;
	}
	int JCommonUtility::GetPathLastBackSlash(const std::string& path)noexcept
	{
		return (int)path.find_last_of("\\") + 1;
	}
	int JCommonUtility::GetPathLastPeriod(const std::wstring& path)noexcept
	{
		return (int)path.find_last_of(L".");
	}
	int JCommonUtility::GetPathLastPeriod(const std::string& path)noexcept
	{
		return (int)path.find_last_of(".");
	}
	std::wstring JCommonUtility::GetFileFormat(const std::wstring& path)noexcept
	{
		size_t index = path.find_last_of(L".");
		return path.substr(index, path.size() - index);
	}
	std::string JCommonUtility::GetFileFormat(const std::string& path)noexcept
	{
		size_t index = path.find_last_of(".");
		return path.substr(index, path.size() - index);
	}
	int JCommonUtility::StringToInt(const std::string& str)noexcept
	{
		int res = 0;
		int digit = (int)str.size();
		float mul = (float)pow(10, digit - 1);

		for (int i = 0; i < digit; ++i)
		{
			res += (int)((str[i] - '0') * (int)mul);
			mul *= 0.1f;
		}
		return res;
	}
	int JCommonUtility::WStringToInt(const std::wstring& str)noexcept
	{
		return StringToInt(WstringToU8String(str));
	}
	const wchar_t* JCommonUtility::CharToWChar(const char* src)
	{
		size_t newsize = strlen(src) + 1;
		wchar_t* wcstring = new wchar_t[newsize];
		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, wcstring, newsize, src, _TRUNCATE);
		return wcstring;
	}
	std::wstring JCommonUtility::StringToWstring(const std::string& str)noexcept
	{
		return std::wstring(CA2W(str.c_str()));
	}
	std::string JCommonUtility::WstringToU8String(const std::wstring& wstr)noexcept
	{
		return std::string(CW2A(wstr.c_str(), CP_UTF8));
	}
	std::wstring JCommonUtility::U8StringToWstring(const std::string& str)noexcept
	{
		return std::wstring(CA2W(str.c_str(), CP_UTF8));
	}
	std::string JCommonUtility::EraseEmptySpace(const std::string& str)noexcept
	{
		std::string newStr = str;
		for (int i = newStr.size() - 1; i >= 0; --i)
		{
			if (newStr[i] != ' ')
				break;
			else
				newStr.pop_back();
		}

		for (int i = 0; i < newStr.size(); ++i)
		{
			if (newStr[i] != ' ')
				break;
			else
				newStr = newStr.substr(i + 1);
		}
		return newStr;
	}
	std::wstring JCommonUtility::EraseEmptySpace(const std::wstring& wstr)noexcept
	{
		std::wstring newStr = wstr;
		for (int i = newStr.size() - 1; i >= 0; --i)
		{
			if (newStr[i] != L' ')
				break;
			else
				newStr.pop_back();
		}

		for (int i = 0; i < newStr.size(); ++i)
		{
			if (newStr[i] != L' ')
				break;
			else
				newStr = newStr.substr(i + 1);
		}
		return newStr;
	}
	std::wstring JCommonUtility::ComporessWstring(const std::wstring& wstr, const uint lange)noexcept
	{
		if (wstr.size() >= lange)
			return wstr.substr(0, lange) + L"...";
		else
			return wstr;
	}
	std::string JCommonUtility::ComporessString(const std::string& str, const uint lange)noexcept
	{
		if (str.size() >= lange)
			return str.substr(0, lange) + "...";
		else
			return str;
	}
	bool JCommonUtility::IsNumber(const char ch)noexcept
	{
		return (ch > 47 && ch < 58);
	}
	bool JCommonUtility::IsNumber(const wchar_t ch)noexcept
	{
		return (ch > 47 && ch < 58);
	}
	bool JCommonUtility::Contain(const std::wstring& source, const std::wstring& target)noexcept
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
	bool JCommonUtility::Contain(const std::string& source, const std::string& target)noexcept
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
	void JCommonUtility::DecomposeFolderPath(const std::wstring& path, std::wstring& folderParentPath, std::wstring& folderName)noexcept
	{
		const int nameStIndex = GetPathLastBackSlash(path);
		if (nameStIndex != -1)
		{
			folderName = path.substr(nameStIndex, (path.size() - nameStIndex));
			folderParentPath = path.substr(0, nameStIndex);
		}
		else
		{
			folderParentPath = L"";
			folderName = path;
		}
	}
	void JCommonUtility::DecomposeFolderPath(const std::string& path, std::string& folderParentPath, std::string& folderName)noexcept
	{
		const int nameStIndex = GetPathLastBackSlash(path);
		if (nameStIndex != -1)
		{
			folderName = path.substr(nameStIndex, (path.size() - nameStIndex));
			folderParentPath = path.substr(0, nameStIndex);
		}
		else
		{
			folderParentPath = "";
			folderName = path;
		}
	}
	void JCommonUtility::DecomposeFilePath(const std::wstring& path, std::wstring& folderPath, std::wstring& name, std::wstring& format, bool eraseFolderPathBackSlash)noexcept
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
	void JCommonUtility::DecomposeFilePath(const std::string& path, std::string& folderPath, std::string& name, std::string& format, bool eraseFolderPathBackSlash)noexcept
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
			folderPath = "";
		}
		else
		{
			name = path;
			folderPath = "";
		}

		if (formatStIndex != -1)
			format = path.substr(formatStIndex, path.size() - formatStIndex);
		else
			format = "";

		if (eraseFolderPathBackSlash)
		{
			const int lastBackSlash = GetPathLastBackSlash(folderPath);
			if (lastBackSlash != -1)
				folderPath = folderPath.substr(0, lastBackSlash - 2);
		}
	}
	std::string JCommonUtility::DecomposeFileFormat(const std::string& path)noexcept
	{
		const int formatStIndex = GetPathLastPeriod(path);
		return formatStIndex != -1 ? path.substr(formatStIndex + 1) : "";
	}
	void JCommonUtility::MakeFileName(std::wstring& name, const std::wstring& format, const std::wstring& folderPath) noexcept
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
	void JCommonUtility::MakeFileName(std::string& name, const std::string& format, const std::string& folderPath) noexcept
	{
		bool result = true;
		int count = 0;
		while (result)
		{
			if (!IsOverlappedFilepath(U8StringToWstring(name + format), U8StringToWstring(folderPath)))
				result = false;
			else
			{
				ModifyOverlappedName(name, name.length(), count);
				++count;
			}
		}
	}
	bool JCommonUtility::IsOverlappedFilepath(const std::wstring& name, const std::wstring& folderPath) noexcept
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
	bool JCommonUtility::IsOverlappedDirectoryPath(const std::wstring& name, const std::wstring& folderPath) noexcept
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
	void JCommonUtility::ModifyOverlappedName(std::wstring& name, size_t length, int newNumber)noexcept
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
	void JCommonUtility::ModifyOverlappedName(std::string& name, size_t length, int newNumber)noexcept
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
	void JCommonUtility::DeleteDirectoryFile(const std::string& path)
	{
		remove(path.c_str());
	}
	size_t JCommonUtility::CalculateGuid(const std::string& path)noexcept
	{
		static std::hash<std::string> stringHash;
		return stringHash(path);
	}

}