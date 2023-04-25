#pragma once
#include"../../Core/Func/Callable/JCallable.h"
#include"../../Core/Singleton/JSingletonHolder.h"
#include"JResourceObjectType.h"
#include<string>
#include<vector>
#include<unordered_map>
#include<memory>

namespace JinEngine
{ 
	class JDirectory;
	class JResourceObject;
	namespace Core
	{
		struct JFileImportHelpData;
		template<typename T> class JCreateUsingNew;
	}

	using ClassifyResourceTypeF = Core::JStaticCallableType<std::vector<J_RESOURCE_TYPE>, const Core::JFileImportHelpData>;
	using ImportResourceF = Core::JStaticCallableType<std::vector<JResourceObject*>, JDirectory*, const Core::JFileImportHelpData>;

	class JResourceObjectImporterImpl
	{
	private:
		template<typename T> friend class Core::JCreateUsingNew;
	private:
		struct FormatInfo
		{
		public:
			std::unique_ptr<ClassifyResourceTypeF::Callable> ClassifyCallable;
			std::unordered_map<J_RESOURCE_TYPE, ImportResourceF::Callable> ImportCallableMap;
		};
	private:
		std::unordered_map<std::wstring, FormatInfo> formatInfoMap;
	public:
		void AddFormatInfo(const std::wstring& format, const J_RESOURCE_TYPE rType, ImportResourceF::Ptr ptr)noexcept;
		void AddFormatInfo(const std::wstring& format, const J_RESOURCE_TYPE rType, ImportResourceF::Ptr iptr, ClassifyResourceTypeF::Ptr cptr)noexcept;
	public:
		std::vector<JResourceObject*> ImportResource(JDirectory* dir, const Core::JFileImportHelpData& importPathdata)noexcept;
	public:
		bool IsValidFormat(const std::wstring& format)const noexcept;
	private:
		JResourceObjectImporterImpl() = default;
		~JResourceObjectImporterImpl() = default;
	};
	using JResourceObjectImporter = Core::JSingletonHolder<JResourceObjectImporterImpl>;
}