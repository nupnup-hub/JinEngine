#pragma once 
#include <string> 
#include <functional>  
#include <unordered_map> 
#include"../JCoreEssential.h"
#include"../Pointer/JOwnerPtr.h"
#include"../Exception/JExceptionMacro.h" 
#include"../Func/Callable/JCallable.h"

namespace JinEngine
{
	namespace Core
	{
		class JTypeBase;
	}
	class JCommonUtility
	{ 
	public:
		enum class J_TEXT_ORDER_TYPE
		{
			RIGHT,
			LEFT,
			MIDDLE
		};
	public:
		static constexpr int searchFail = -1;
	public:
		static int GetPathLastBackSlash(const std::wstring& path)noexcept;
		static int GetPathLastBackSlash(const std::string& path)noexcept;

		static int GetPathLastPeriod(const std::wstring& path)noexcept;
		static int GetPathLastPeriod(const std::string& path)noexcept;

		static std::wstring GetFileFormat(const std::wstring& path)noexcept;
		static std::string GetFileFormat(const std::string& path)noexcept;
		static std::wstring GetFileName(const std::wstring& path)noexcept;
		static std::wstring GetFileNameWithOutFormat(const std::wstring& path)noexcept; 
		static std::wstring GetFolderPath(const std::wstring& path)noexcept;
		static std::wstring GetPathWithOutFormat(const std::wstring& path)noexcept;

		static size_t StringToInt(const std::string& str)noexcept;
		static size_t WstringToInt(const std::wstring& wstr)noexcept;
		static float StringToFloat(const std::string& str)noexcept;
		static float WstringToFloat(const std::wstring& wstr)noexcept; 
		static const wchar_t* CharToWChar(const char* src);
		 
		static std::wstring StrToWstr(const std::string& str)noexcept; 
		static std::string WstrToU8Str(const std::wstring& wstr)noexcept;
		static std::wstring U8StrToWstr(const std::string& str)noexcept;
 
		//문장에서 모든 ch를 제거
		static std::wstring EraseWChar(const std::wstring& wstr, const wchar_t ch)noexcept;
		static std::string EraseChar(const std::string& str, const char ch)noexcept;

		//ch가아닌 문자가 나올때까지 ch제거
		static std::wstring EraseSideWChar(const std::wstring& wstr, const wchar_t ch)noexcept;
		static std::string EraseSideChar(const std::string& str, const char ch)noexcept;

		static std::wstring ChangeWord(const std::wstring& oriSentence, const std::wstring& fromWord, const std::wstring& toWord);
		static std::string ChangeWord(const std::string& oriSentence, const std::string& fromWord, const std::string& toWord);

		static std::wstring CompressWstring(const std::wstring& wstr, const uint length)noexcept;
		static std::string CompressString(const std::string& str, const uint length)noexcept;

		static std::wstring CompressWstirngPath(const std::wstring& path, const uint length)noexcept;
		static std::string CompressStringPath(const std::string& path, const uint length)noexcept;

		static std::wstring InsertSpace(const std::wstring& src, const uint spacePerLength, _Out_ int& lastSpaceIndex)noexcept;
		static std::wstring OrderTexture(const std::wstring& src, const uint spacePerLength, const J_TEXT_ORDER_TYPE type)noexcept;

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

		static std::wstring ChangeFileFormat(const std::wstring& path, const std::wstring& newFormat)noexcept;

		//Unuse
		static void MakeFileName(std::wstring& name, const std::wstring& format, const std::wstring& folderPath) noexcept;
		//Unuse
		static void MakeFileName(std::string& name, const std::string& format, const std::string& folderPath) noexcept;

		static bool IsOverlappedFilepath(const std::wstring& name, const std::wstring& folderPath) noexcept; 
		static bool IsOverlappedDirectoryPath(const std::wstring& name, const std::wstring& folderPath) noexcept; 

		static void ModifyOverlappedName(std::wstring& name, size_t length, int newNumber)noexcept;
		static void ModifyOverlappedName(std::string& name, size_t length, int newNumber)noexcept;
		
		static void DeleteDirectoryFile(const std::string& path);
		 
		static size_t CalculateGuid(const std::string& path)noexcept;
		static size_t CalculateGuid(const std::wstring& path)noexcept; 

		static bool FindFirstFilePathByName(const std::wstring& folderPath, const std::wstring& name, _Out_ std::wstring& filePath, const bool allowSearchChildDir = true);
		static bool FindFirstFilePathByFormat(const std::wstring& folderPath, const std::wstring& format, _Out_ std::wstring& filePath, const bool allowSearchChildDir = true);
		static void FindFilePathByFormat(const std::wstring& folderPath, const std::wstring& format, _Out_ std::vector<std::wstring>& filePath, const bool allowSearchChildDir = true);
		static void FindFilePathByFormat(const std::string& folderPath, const std::string& format, _Out_ std::vector<std::string>& filePath, const bool allowSearchChildDir = true);
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
		//have to defined GetName function
		template<typename Type>
		static std::wstring MakeUniqueName(const std::vector<Type>& vec, const std::wstring& name)
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
		template<typename Type, std::enable_if_t<std::is_base_of_v<Core::JTypeBase, Type>, int> = 0>
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
		
		template <typename T>
		static std::string ToStringWithPrecision(const T value, const int n = 6)
		{
			std::ostringstream out;
			out.precision(n);
			out << std::fixed << value;
			return out.str();
		}	 
		template<typename Type, typename FuncDeclaredType>
		static int GetTypeIndex(std::vector<Type*>& vec, const size_t guid, size_t(FuncDeclaredType::* ptr)()const)
		{
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{
				if ((vec[i]->*ptr)() == guid)
					return i;
			}
			return searchFail;
		}
		template<typename Type, typename FuncDeclaredType>
		static int GetTypeIndex(std::vector<std::unique_ptr<Type>>& vec, const size_t guid, size_t(FuncDeclaredType::* ptr)()const)
		{
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{ 
				if (((*vec[i]).*ptr)() == guid)
					return i;
			}
			return searchFail;
		}	 
		template<typename Type, std::enable_if_t<std::is_base_of_v<Core::JTypeBase, Type>, int> = 0>
		static int GetTypeIndex(const std::vector<Type*>& vec, const size_t guid)
		{
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{
				if(vec[i]->GetGuid() == guid)
					return i;
			}
			return searchFail;
		} 
		template<typename Type, std::enable_if_t<std::is_base_of_v<Core::JTypeBase, Type>, int> = 0>
		static int GetTypeIndex(const std::vector<JUserPtr<Type>>& vec, const size_t guid)
		{
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{
				if (vec[i]->GetGuid() == guid)
					return i;
			}
			return searchFail;
		}
		template<typename Type, std::enable_if_t<std::is_base_of_v<Core::JTypeBase, Type>, int> = 0>
		static int GetTypeIndex(const std::vector<std::unique_ptr<Type>>& vec, const size_t guid)
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
		static int GetIndex(const std::vector<Type*>& vec, bool(*condFunc)(Type*, Param...), Param... var)
		{
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{
				if (condFunc(vec[i], std::forward<Param>(var)...))
					return i;
			}
			return searchFail;
		}
		template<typename Type, typename ...Param>
		static int GetIndex(const std::vector<Type>& vec, bool(*condFunc)(const Type&, Param...), Param... var)
		{
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{
				if (condFunc(vec[i], std::forward<Param>(var)...))
					return i;
			}
			return searchFail;
		}
		template<typename Type, typename ...Param>
		static int GetIndex(const std::vector<std::unique_ptr<Type>>& vec, bool(*condFunc)(Type*, Param...), Param... var)
		{
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{
				if (condFunc(vec[i].get(), std::forward<Param>(var)...))
					return i;
			}
			return searchFail;
		}
		template<typename Type>
		static int GetIndex(std::vector<Type*>& vec, Type* ptr)
		{
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{
				if (&vec[i] == ptr)
					return i;
			}
			return searchFail;
		}

		template<typename Type, typename VecType, typename ...Param>
		static std::vector<VecType> GetPassConditionElement(const std::vector<Type>& vec, bool(*condPtr)(const Type&, Param...), Param... var)
		{
			std::vector<VecType> result;
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{
				if (condPtr(vec[i], std::forward<Param>(var)...))
					result.push_back(vec[i]);
			}
			return result;
		}
		template<typename Type, typename ...Param>
		static std::vector<Type*> GetPassConditionElement(const std::vector<Type*>& vec, bool(*condPtr)(Type*, Param...), Param... var)
		{
			std::vector<Type*> result;			
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{
				if (condPtr(vec[i], std::forward<Param>(var)...))
					result.push_back(vec[i]);
			}
			return result;
		}
		template<typename Type, typename ...Param>
		static std::vector<Type*> GetPassConditionElement(const std::vector<std::unique_ptr<Type>>& vec, bool(*condPtr)(Type*, Param...), Param... var)
		{
			std::vector<Type*> result;
			const uint vecCount = (uint)vec.size();
			for (uint i = 0; i < vecCount; ++i)
			{
				if (condPtr(vec[i].get(), std::forward<Param>(var)...))
					result.push_back(vec[i].get());
			}
			return result;
		}
		template<typename Ret, typename T, typename ...Param>
		static void ApplyFunc(const uint st, Core::JStaticCallable<Ret, T*, Param...> callable, std::vector<T*>& objectVec, Param&&... var)
		{
			const uint objectCount = (uint)objectVec.size();
			for (uint i = st; i < objectCount; ++i)
				callable(nullptr, objectVec[i], std::forward<Param>(var)...);
		}
		template<typename Ret, typename T, typename ...Param>
		static void ApplyFunc(const uint st, Core::JStaticCallable<Ret, T*, Param...> callable, std::vector<Core::JTypeBase*>& objectVec, Param&&... var)
		{
			const uint objectCount = (uint)objectVec.size();
			for (uint i = st; i < objectCount; ++i)
				callable(nullptr, static_cast<T*>(objectVec[i]), std::forward<Param>(var)...);
		}
		template<typename Ret, typename T, typename ...Param>
		static void ApplyFuncUseIndex(const uint st, Core::JStaticCallable<Ret, T*, uint, Param...> callable, std::vector<T*>& objectVec, Param&&... var)
		{
			const uint objectCount = (uint)objectVec.size();
			for (uint i = st; i < objectCount; ++i)
				callable(nullptr, objectVec[i], i, std::forward<Param>(var)...);
		}
		template<typename Ret, typename T, typename ...Param>
		static void ApplyFuncUseIndex(const uint st, Core::JStaticCallable<Ret, T*, uint,  Param...> callable, std::vector<Core::JTypeBase*>& objectVec, Param&&... var)
		{
			const uint objectCount = (uint)objectVec.size();
			for (uint i = st; i < objectCount; ++i)
				callable(nullptr, static_cast<T*>(objectVec[i]), i, std::forward<Param>(var)...);
		}
	};

	using JCUtil = JCommonUtility;
}

