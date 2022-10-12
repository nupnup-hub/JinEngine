#pragma once
#include<string> 
#include"../Reflection/JReflectionInfo.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{
		class JFileConstant final
		{
		public:
			static std::wstring GetFileFormat()noexcept;
			static std::wstring GetMetafileFormat()noexcept;
			static std::wstring MakeFilePath(const std::wstring& folderPath, const std::wstring& name);
			//format = L".format"
			static std::wstring MakeFilePath(const std::wstring& folderPath, const std::wstring& name, const std::wstring& format);
			static std::wstring StreamHasObjGuidSymbol()noexcept;
			static std::wstring StreamEndFixedDataSymbol()noexcept;
		public:
			template<typename EnumType>
			static std::wstring StreamTypeSymbol()noexcept
			{
				JEnumInfo* eInfo = JReflectionInfo::Instance().GetEnumInfo(typeid(EnumType).name());
				if (eInfo == nullptr)
					return L"Null Enum";
				else
					return JCUtil::StrToWstr("---" + eInfo->Name() + "---");
			}
		};
	}
}