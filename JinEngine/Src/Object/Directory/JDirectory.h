#pragma once 
#include"../JObject.h"
#include"../../Core/File/JFilePathData.h" 

namespace JinEngine
{
	struct JFileInitData;
	class JFile;
	class JResourceObject;
	class JDirectoryPrivate;

	class JDirectory final : public JObject
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JDirectory)
	public: 
		class InitData : public JObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			JDirectory* parent = nullptr;
		public:
			InitData(JDirectory* parent);
			InitData(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* parent);
		};
	protected: 
		class LoadData final : public Core::JDITypeDataBase
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadData)
		public:
			JDirectory* parent;
			Core::JAssetFileLoadPathData pathData;
		public:
			LoadData(JDirectory* parent, const Core::JAssetFileLoadPathData& pathData);
			~LoadData();
		};
	private:
		friend class JDirectoryPrivate;
		class JDirectoryImpl;
	private:
		std::unique_ptr< JDirectoryImpl>impl;
	public:
		std::wstring GetPath()const noexcept;
		std::wstring GetMetaFilePath()const noexcept;
		uint GetChildernDirctoryCount()const noexcept;
		uint GetFileCount()const noexcept;
		JDirectory* GetParent()const noexcept;
		JDirectory* GetChildDirctory(const uint index)const noexcept;
		JDirectory* GetChildDirctoryByName(const std::wstring& name)const noexcept;
		JDirectory* GetChildDirctoryByPath(const std::wstring& path)const noexcept;
		JFile* GetDirectoryFile(const uint index)const noexcept;
		JFile* GetDirectoryFile(const std::wstring oriFormatName)const noexcept;
		JFile* GetRecentFile()const noexcept;
		J_OBJECT_TYPE GetObjectType()const noexcept final;
		Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final; 
	public:
		void SetName(const std::wstring& newName)noexcept final;
	public:
		bool HasChild(const std::wstring& name)const noexcept;
		bool HasFile(const size_t guid)const noexcept;
		bool IsParent(JDirectory* dir)const noexcept;
		bool IsOpen()const noexcept;
	public:
		JDirectory* SearchDirectory(const std::wstring& path)const noexcept;
		JFile* SearchFile(const std::wstring& oriFormatName)const noexcept;	//search this dir -> subDir...
		static JFile* SearchFile(const size_t resourceGuid)noexcept;	//use static file map
	public:
		std::wstring MakeUniqueFileName(const std::wstring& name, const std::wstring& format, const size_t guid)const noexcept;
	private:
		JDirectory(const InitData& initData);
		~JDirectory();
	};
}