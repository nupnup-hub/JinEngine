#pragma once
#include"../../Core/Func/Callable/JCallable.h"
#include"../../Core/Singleton/JSingletonHolder.h"
#include"../../Core/DI/JDIDataBase.h"
#include"../../Core/File/JFilePathData.h"
#include"JResourceObjectType.h"   

namespace JinEngine
{ 
	class JDirectory;
	class JResourceObject;
	namespace Core
	{
		struct JFileImportPathData;
		template<typename T> class JCreateUsingNew;
	}
 
	class JResourceObjectImportDesc : public Core::JDITypeDataBase
	{
		REGISTER_CLASS_ONLY_USE_TYPEINFO(JResourceObjectImportDesc)
	public: 
		Core::JFileImportPathData importPathData;
		JUserPtr<JDirectory> dir;
	public: 
		JResourceObjectImportDesc(const Core::JFileImportPathData& importPathData, const JUserPtr<JDirectory>& dir = nullptr);
	};

	//.fbx처럼 하나의 format에 여려 resourceType이 포함되어있을때 유효한 resourceType을 분류하는 함수
	//.jpg, .png처럼 resource가 명확한 경우 등록할 필요가 없다.
	using ClassifyResourceTypeF = Core::JStaticCallableType<std::vector<J_RESOURCE_TYPE>, const Core::JFileImportPathData>;
	using ImportResourceF = Core::JStaticCallableType<std::vector<JUserPtr<JResourceObject>>, const JResourceObjectImportDesc*>;

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
		std::vector<JUserPtr<JResourceObject>> ImportResource(const JResourceObjectImportDesc* desc)const noexcept;
		std::vector<J_RESOURCE_TYPE> DeterminFileResourceType(const Core::JFileImportPathData importPathData)const noexcept;
	private:
		const FormatInfo* GetFormatInfo(const std::wstring& format)const noexcept;
	public:
		bool IsValidFormat(const std::wstring& format)const noexcept;
	private:
		JResourceObjectImporterImpl() = default;
		~JResourceObjectImporterImpl() = default;
	};
	using JResourceObjectImporter = Core::JSingletonHolder<JResourceObjectImporterImpl>;
}