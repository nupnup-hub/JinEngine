#pragma once
#include<string> 
#include"../Reflection/JReflectionInfo.h"
#include"../Utility/JCommonUtility.h"

#define CREATE_STRING_GET_FUNC(name, contents)							\
static std::string Get##name()noexcept{return contents;}						\
static std::wstring Get##name##W()noexcept{return L##contents;}					\

namespace JinEngine
{
	namespace Core
	{
		class JFileConstant final
		{
		public:
			CREATE_STRING_GET_FUNC(FileFormat, ".jAsset")
			CREATE_STRING_GET_FUNC(MetaFileFormat, ".jAssetMeta")
			CREATE_STRING_GET_FUNC(CacheFileFormat, ".jCache")
		public:
			//can't combine
			CREATE_STRING_GET_FUNC(ObjGuidSymbol, "ObjGuid: ")
			CREATE_STRING_GET_FUNC(ObjFlagSymbol, "ObjFlag: ")
			CREATE_STRING_GET_FUNC(TypeGuidSymbol, "TypeGuid: ")
			CREATE_STRING_GET_FUNC(SelectedSymbol, "IsSelected: ")
			CREATE_STRING_GET_FUNC(ActivatedSymbol, "IsActivated: ")
			CREATE_STRING_GET_FUNC(HasObjGuidSymbol, "HasObjGuid: ")
			CREATE_STRING_GET_FUNC(UncopiableGuidSymbol, "UncopiableObjGuid: ")
			CREATE_STRING_GET_FUNC(EndFixedDataSymbol, "---End-FinxedData---: ")
			CREATE_STRING_GET_FUNC(FormatSymbol, "Format: ")
			CREATE_STRING_GET_FUNC(FormatIndexSymbol, "FormatIdnex: ")
			CREATE_STRING_GET_FUNC(CombineFileEnd, "---End-Combine--: ") 
		public:
			//can combine typename
			static std::string GetLastOpenSymbol(const JTypeInfo& typeInfo)noexcept;
			static std::wstring GetLastOpenSymbolW(const JTypeInfo& typeInfo)noexcept;
			//can combine notification
			static std::string GetLineSymbol(const std::string& notification)noexcept;
			static std::wstring GetLineSymbolW(const std::wstring& notification)noexcept;
		public:
			template<typename EnumType>
			static std::string GetTypeSymbol()noexcept
			{
				JEnumInfo* eInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(EnumType).name());
				if (eInfo == nullptr)
					return "Null Enum: ";
				else
					return eInfo->Name() + ": ";
			}
			template<typename EnumType>
			static std::wstring GetTypeSymbolW()noexcept
			{
				JEnumInfo* eInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(EnumType).name());
				if (eInfo == nullptr)
					return L"Null Enum: ";
				else
					return JCUtil::StrToWstr(eInfo->Name() + ": ");
			}
		public:
			static std::wstring MakeFilePath(const std::wstring& folderPath, const std::wstring& name);
			//format = L".format"
			static std::wstring MakeFilePath(const std::wstring& folderPath, const std::wstring& name, const std::wstring& format);
		};
	}
}