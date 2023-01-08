#pragma once 
#include <string> 
#include <functional>  
#include <unordered_map> 
#include"../Core/JDataType.h"
#include"../Core/Exception/JExceptionMacro.h" 

namespace JinEngine
{
	namespace Core
	{
		class JIdentifier;
	}
	class JCommonUtility
	{
	public:
		static constexpr int searchFail = -1;
	public:
		static int GetPathLastBackSlash(const std::wstring& path)noexcept;
		static int GetPathLastBackSlash(const std::string& path)noexcept;

		static int GetPathLastPeriod(const std::wstring& path)noexcept;
		static int GetPathLastPeriod(const std::string& path)noexcept;

		static std::wstring GetFileFormat(const std::wstring& path)noexcept;
		static std::string GetFileFormat(const std::string& path)noexcept;

		static size_t StringToInt(const std::string& str)noexcept;
		static size_t WstringToInt(const std::wstring& wstr)noexcept;
		static float StringToFloat(const std::string& str)noexcept;
		static float WstringToFloat(const std::wstring& wstr)noexcept; 
		static const wchar_t* CharToWChar(const char* src);
		 
		static std::wstring StrToWstr(const std::string& str)noexcept; 
		static std::string WstrToU8Str(const std::wstring& wstr)noexcept;
		static std::wstring U8StrToWstr(const std::string& str)noexcept;

		static std::string EraseChar(const std::string& str, const char ch)noexcept;
		static std::wstring EraseWChar(const std::wstring& wstr, const wchar_t ch)noexcept;

		static std::string EraseSideChar(const std::string& str, const char ch)noexcept;
		static std::wstring EraseSideWChar(const std::wstring& wstr, const wchar_t ch)noexcept;

		static std::wstring ComporessWstring(const std::wstring& wstr, const uint lange)noexcept;
		static std::string ComporessString(const std::string& str, const uint lange)noexcept;

		static bool IsNumber(const char ch)noexcept;
		static bool IsNumber(const wchar_t ch)noexcept;
		static bool IsAlphabet(const char ch)noexcept;
		static bool IsAlphabet(const wchar_t ch)noexcept;
		//Classify case-sensitive
		static bool IsSameAlphabet(const char src, const char tar)noexcept;
		static bool IsSameAlphabet(const wchar_t src, const wchar_t tar)noexcept;

		static bool Contain(const std::wstring& source, const std::wstring& target, const bool caseSensitive = true)noexcept;
		static bool Contain(const std::string& source, const std::string& target, const bool caseSensitive = true)noexcept;
 
		static void DecomposeFolderPath(const std::wstring& path, std::wstring& folderPath, std::wstring& name)noexcept;
		static void DecomposeFilePath(const std::wstring& path, std::wstring& folderPath, std::wstring& name, std::wstring& format, bool eraseFolderPathBackSlash = false)noexcept;
		static void DecomposeFileName(const std::wstring& oriname, std::wstring& name, std::wstring& format);
		static std::wstring DecomposeFileFormat(const std::wstring& path)noexcept;

		static void MakeFileName(std::wstring& name, const std::wstring& format, const std::wstring& folderPath) noexcept;
		static void MakeFileName(std::string& name, const std::string& format, const std::string& folderPath) noexcept;

		static bool IsOverlappedFilepath(const std::wstring& name, const std::wstring& folderPath) noexcept; 
		static bool IsOverlappedDirectoryPath(const std::wstring& name, const std::wstring& folderPath) noexcept; 

		static void ModifyOverlappedName(std::wstring& name, size_t length, int newNumber)noexcept;
		static void ModifyOverlappedName(std::string& name, size_t length, int newNumber)noexcept;
		
		static void DeleteDirectoryFile(const std::string& path);
		 
		static size_t CalculateGuid(const std::string& path)noexcept;
		static size_t CalculateGuid(const std::wstring& path)noexcept;
	public:
		template <class T>
		static inline void hash_combine(std::size_t& s, const T& v)
		{
			std::hash<T> h;
			s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
		}
		template<typename T>
		static void UnordedMapGuidKeyChange(size_t oldGuid, size_t newGuid, std::unordered_map<size_t, T>& unordedMap)
		{
			if (unordedMap.find(oldGuid) == unordedMap.end())
				return;

			T data = std::move(unordedMap[oldGuid]);
			unordedMap.erase(oldGuid);
			unordedMap.emplace(newGuid, std::move(data));
		} 

		static std::wstring MakeUniqueName(const std::wstring& ori, const std::wstring& tar)
		{
			if (ori == tar)
			{
				std::wstring newStr = tar;
				const uint lastIndex = (uint)ori.size() - 1;
				if (ori[lastIndex] >= '0' && ori[lastIndex] <= '9')
				{
					if (ori[lastIndex] == '9')
					{
						std::wstring newStr = ori + L'0';
						newStr[lastIndex] = '1';
					}
					else
					{
						std::wstring newStr = ori;
						newStr[lastIndex] = ori[lastIndex] + 1;
					}
				}
				else
					newStr += L'0';
				return newStr;
			}
			else
				return ori;	 
		}
		template<typename Type>
		static std::wstring MakeUniqueName(const std::vector<Type*>& vec, const std::wstring& name, std::wstring(Type::*ptr)()const)
		{
			bool result = true;
			const uint eleCount = (uint)vec.size();
			uint index = 0;
			uint overlapCount = 0;

			std::wstring uniqueName = name;
			while (result)
			{
				if (index >= eleCount)
					break;
				else
				{
					if ((vec[index]->*ptr)() == uniqueName)
					{
						ModifyOverlappedName(uniqueName, uniqueName.length(), overlapCount);
						++overlapCount;
						index = 0;
					}
					else
						++index;
				}
			}
			return uniqueName;
		}
		template<typename Type, std::enable_if_t<std::is_base_of_v<Core::JIdentifier, Type>, int> = 0>
		static std::wstring MakeUniqueName(const std::vector<Type*>& vec, const std::wstring& name)
		{
			bool result = true;
			const uint eleCount = (uint)vec.size();
			uint index = 0;
			uint overlapCount = 0;

			std::wstring uniqueName = name;
			while (result)
			{
				if (index >= eleCount)
					break;
				else
				{
					if (vec[index]->GetName() == uniqueName)
					{
						ModifyOverlappedName(uniqueName, uniqueName.length(), overlapCount);
						++overlapCount;
						index = 0;
					}
					else
						++index;
				}
			}
			return uniqueName;
		}
		template<typename Type, std::enable_if_t<std::is_base_of_v<Core::JIdentifier, Type>, int> = 0>
		static std::wstring MakeUniqueName(const std::vector<std::unique_ptr<Type>>& vec, const std::wstring& name)
		{
			bool result = true;
			const uint eleCount = (uint)vec.size();
			uint index = 0;
			uint overlapCount = 0;

			std::wstring uniqueName = name;
			while (result)
			{
				if (index >= eleCount)
					break;
				else
				{
					if (vec[index]->GetName() == uniqueName)
					{
						ModifyOverlappedName(uniqueName, uniqueName.length(), overlapCount);
						++overlapCount;
						index = 0;
					}
					else
						++index;
				}
			}
			return uniqueName;
		}

		template <typename T>
		static std::string ToStringWithPrecision(const T value, const int n = 6)
		{
			std::ostringstream out;
			out.precision(n);
			out << std::fixed << value;
			return out.str();
		}	 
		template<typename Type>
		static int GetJIdenIndex(std::vector<Type*>& vec, const size_t guid, size_t(Type::*ptr)()const)
		{
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{
				if ((vec[i]->*ptr)() == guid)
					return i;
			}
			return searchFail;
		}	 
		template<typename Type>
		static int GetJIdenIndex(std::vector<std::unique_ptr<Type>>& vec, const size_t guid, size_t(Type::* ptr)()const)
		{
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{ 
				if (((*vec[i]).*ptr)() == guid)
					return i;
			}
			return searchFail;
		}	 
		template<typename Type, std::enable_if_t<std::is_base_of_v<Core::JIdentifier, Type>, int> = 0>
		static int GetJIdenIndex(const std::vector<Type*>& vec, const size_t guid)
		{
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{
				if(vec[i]->GetGuid() == guid)
					return i;
			}
			return searchFail;
		} 
		template<typename Type, std::enable_if_t<std::is_base_of_v<Core::JIdentifier, Type>, int> = 0>
		static int GetJIdenIndex(const std::vector<std::unique_ptr<Type>>& vec, const size_t guid)
		{
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{
				if (vec[i]->GetGuid() == guid)
					return i;
			}
			return searchFail;
		}
		template<typename Type, typename ...Param>
		static int GetJIndex(const std::vector<Type*>& vec, bool(*ptr)(Type*, Param...), Param... var)
		{
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{
				if (ptr(vec[i], std::forward<Param>(var)...))
					return i;
			}
			return searchFail;
		}
 
	};

	using JCUtil = JCommonUtility;
}

