#pragma once
#include"JResourceObjectType.h"  
#include"JResourceObjectEventType.h" 
#include"JReferenceInterface.h"
#include"../JObject.h"
#include"../../Core/File/JFilePathData.h" 
#include"../../Core/Interface/JValidInterface.h"
#include"../../Core/Event/JEventManager.h"  

namespace JinEngine
{
	class JDirectory;
	class JFile;
	class JResourceObjectPrivate;
	struct RTypeHint;
	struct RTypeCommonFunc;
	struct RTypePrivateFunc;

	class JResourceObject;
	using JResourceEventManager = Core::JEventManager<size_t, J_RESOURCE_EVENT_TYPE, JResourceObject*>;
	using JResourceEventInterface = JResourceEventManager::Interface;
	class JResourceObject : public JObject, public JReferenceInterface, public Core::JValidInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JResourceObject)
	public: 
		class InitData : public JObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public: 
			uint8 formatIndex;
			J_RESOURCE_TYPE rType;
			JDirectory* directory;
		public: 
			InitData(const JTypeInfo& initTypeInfo, 
				const uint8 formatIndex, 
				const J_RESOURCE_TYPE rType, 
				JDirectory* directory);
			InitData(const JTypeInfo& initTypeInfo,
				const size_t guid,
				const uint8 formatIndex,
				const J_RESOURCE_TYPE rType,
				JDirectory* directory);
			InitData(const JTypeInfo& initTypeInfo, 
				const std::wstring& name,
				const size_t& guid,
				const J_OBJECT_FLAG flag, 
				const uint8 formatIndex,
				const J_RESOURCE_TYPE rType,
				JDirectory* directory);
		public:
			bool IsValidData()const noexcept override;
			J_RESOURCE_TYPE GetResourceType() const noexcept;
			std::wstring GetFormat()const noexcept;
		};
	protected: 
		class LoadData final : public Core::JDITypeDataBase
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadData)
		public:
			JDirectory* directory;
			const Core::JAssetFileLoadPathData pathData;
		public:
			LoadData(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData);
			~LoadData();
		public:
			bool IsValidData()const noexcept final;
		};   
	protected: 
		class StoreData final : public JObject::StoreData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(StoreData)  
		public:
			StoreData(JResourceObject* jRobj); 
		};
	private:
		friend class JResourceObjectPrivate;
		class JResourceObjectImpl;
	private:
		std::unique_ptr<JResourceObjectImpl> impl;
	public:
		static constexpr uint8 GetInvalidFormatIndex()noexcept
		{
			return 255;
		}
		static constexpr uint8 GetDefaultFormatIndex()noexcept
		{
			return 0;
		}
	public:
		//Return name + oriFormat
		std::wstring GetFullName()const noexcept; 
		std::wstring GetPath()const noexcept;  
		std::wstring GetMetaFilePath()const noexcept;
		std::wstring GetFolderPath()const noexcept;  
		static std::wstring GetDefaultFormat(const J_RESOURCE_TYPE type)noexcept;
		J_OBJECT_TYPE GetObjectType()const noexcept final;
		uint8 GetFormatIndex()const noexcept;
		static uint8 GetFormatIndex(const J_RESOURCE_TYPE type, const std::wstring& format)noexcept;
		JDirectory* GetDirectory()const noexcept;
		virtual J_RESOURCE_TYPE GetResourceType()const noexcept = 0;
		virtual std::wstring GetFormat()const noexcept = 0; 
	public:
		void SetName(const std::wstring& newName)noexcept final;
	protected:
		bool HasFile()const noexcept;
		bool HasMetafile()const noexcept; 
		bool CanMakeFile()const noexcept;	//Can make diskfile && JFile
		static bool IsResourceFormat(const J_RESOURCE_TYPE type, const std::wstring& format)noexcept;
	protected:
		//DoActivate => Load resource file in memory
		void DoActivate() noexcept override;
		//DoDeActivate => Unload resource
		void DoDeActivate()noexcept override;
	public:
		static JResourceEventInterface* EvInterface()noexcept;
	protected:
		static void RegisterRTypeInfo(const RTypeHint& rTypeHint, const RTypeCommonFunc& rTypeCFunc, const RTypePrivateFunc& rTypePFunc);
	public:
		template<typename T, std::enable_if_t<std::is_base_of_v<JResourceObject, T>, int> = 0>
		static std::wstring GetDefaultFormat()noexcept
		{
			return GetDefaultFormat(T::GetStaticResourceType());
		}
		template<typename T, std::enable_if_t<std::is_base_of_v<JResourceObject, T>, int> = 0>
		static uint8 GetFormatIndex(const std::wstring& format)noexcept
		{
			return GetFormatIndex(T::GetStaticResourceType(), format);
		}
	protected:
		JResourceObject(const InitData& initData);
		~JResourceObject();
	};
}