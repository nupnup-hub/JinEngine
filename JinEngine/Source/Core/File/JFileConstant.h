#pragma once
#include<string> 
#include"../Reflection/JReflectionInfo.h"
#include"../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{ 
		class JFileConstant final
		{
		public:
			static std::wstring GetFileFormat()noexcept;
			static std::wstring GetMetaFileFormat()noexcept;
			static std::wstring GetCacheFileFormat()noexcept;
			static std::wstring MakeFilePath(const std::wstring& folderPath, const std::wstring& name);
			//format = L".format"
			static std::wstring MakeFilePath(const std::wstring& folderPath, const std::wstring& name, const std::wstring& format);
		public:
			//can't combine
			static std::wstring StreamObjGuidSymbol()noexcept;
			static std::wstring StreamObjFlagSymbol()noexcept;
			static std::wstring StreamTypeGuidSymbol()noexcept;
			static std::wstring StreamHasObjGuidSymbol()noexcept;
			static std::wstring StreamUncopiableGuidSymbol()noexcept;
			static std::wstring StreamEndFixedDataSymbol()noexcept;
			static std::wstring StreamFormatSymbol()noexcept;
			static std::wstring StreamFormatIndexSymbol()noexcept;
			static std::wstring StreamCombineFileEnd()noexcept;
		public:
			//can combine typename
			static std::wstring StreamLastOpenSymbol(const JTypeInfo& typeInfo)noexcept;
			//can combine notification
			static std::wstring StreamLine(const std::wstring& notification)noexcept;
		public:
			template<typename EnumType>
			static std::wstring StreamTypeSymbol()noexcept
			{
				JEnumInfo* eInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(EnumType).name());
				if (eInfo == nullptr)
					return L"Null Enum: ";
				else
					return JCUtil::StrToWstr(eInfo->Name() + ": ");
			} 
		};
	}
}