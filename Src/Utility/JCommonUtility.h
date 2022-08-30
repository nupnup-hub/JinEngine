#pragma once 
#include <string> 
#include <functional>  
#include <unordered_map> 
#include"../Core/JDataType.h"
#include"../Core/Exception/JExceptionMacro.h"

namespace JinEngine
{
	class JCommonUtility
	{
	public:
		static int GetPathLastBackSlash(const std::wstring& path)noexcept;
		static int GetPathLastBackSlash(const std::string& path)noexcept;

		static int GetPathLastPeriod(const std::wstring& path)noexcept;
		static int GetPathLastPeriod(const std::string& path)noexcept;

		static std::wstring GetFileFormat(const std::wstring& path)noexcept;
		static std::string GetFileFormat(const std::string& path)noexcept;

		static int StrToInt(const std::string& str)noexcept;
		static int WstrToInt(const std::wstring& str)noexcept;
		static const wchar_t* CharToWChar(const char* src);
		 
		static std::wstring StrToWstr(const std::string& str)noexcept; 
		static std::string WstrToU8Str(const std::wstring& wstr)noexcept;
		static std::wstring U8StrToWstr(const std::string& str)noexcept;

		static std::string EraseEmptySpace(const std::string& str)noexcept;
		static std::wstring EraseEmptySpace(const std::wstring& wstr)noexcept;

		static std::wstring ComporessWstring(const std::wstring& wstr, const uint lange)noexcept;
		static std::string ComporessString(const std::string& str, const uint lange)noexcept;

		static bool IsNumber(const char ch)noexcept;
		static bool IsNumber(const wchar_t ch)noexcept;

		static bool Contain(const std::wstring& source, const std::wstring& target)noexcept;
		static bool Contain(const std::string& source, const std::string& target)noexcept;

		static void DecomposeFolderPath(const std::wstring& path, std::wstring& folderPath, std::wstring& name)noexcept;
	 	static void DecomposeFolderPath(const std::string& path, std::string& folderPath, std::string& name)noexcept;
	
		static void DecomposeFilePath(const std::wstring& path, std::wstring& folderPath, std::wstring& name, std::wstring& format, bool eraseFolderPathBackSlash = false)noexcept;
		static void DecomposeFilePath(const std::string& path, std::string& folderPath, std::string& name, std::string& format, bool eraseFolderPathBackSlash = false)noexcept;
 
		static std::string DecomposeFileFormat(const std::string& path)noexcept;

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

		template<typename Type>
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
		template<typename Type>
		static std::string MakeUniqueName(const std::vector<Type*>& vec, const std::string& name)
		{
			bool result = true;
			const uint eleCount = (uint)vec.size();
			uint index = 0;
			uint overlapCount = 0;

			std::string uniqueName = name;
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
		template<typename Type>
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
		template<typename Type>
		static std::string MakeUniqueName(const std::vector<std::unique_ptr<Type>>& vec, const std::string& name)
		{
			bool result = true;
			const uint eleCount = (uint)vec.size();
			uint index = 0;
			uint overlapCount = 0;

			std::string uniqueName = name;
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
	};

	using JCUtil = JCommonUtility;
}

