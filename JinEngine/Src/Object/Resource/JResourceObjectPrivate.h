#pragma once
#include"../JObjectPrivate.h"

namespace JinEngine
{ 
	class JDirectory;
	class JFile;
	class JResourceObject; 
	class JResourceManager; 

	namespace Editor
	{
		class JWindowDirectory;
	}
	namespace Core
	{
		struct JAssetFileLoadPathData;
	}
 
	class JResourceObjectPrivate : public JObjectPrivate
	{ 
	public:
		class CreateInstanceInterface : public JObjectPrivate::CreateInstanceInterface
		{
		protected:
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept override;
		private:
			void SetValidInstance(Core::JIdentifier* createdPtr)noexcept override;
		private:
			//오브젝트를 생성하거나 생성한 후 저장하기위해 캐싱해둔 데이터를 지운다
			virtual void TryDestroyUnUseData(Core::JIdentifier* createdPtr)noexcept;
		private:
			bool Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept final;
		};
		class DestroyInstanceInterface : public JObjectPrivate::DestroyInstanceInterface
		{
		protected:
			void Clear(Core::JIdentifier* ptr, const bool isForced) override;
		private:
			void SetInvalidInstance(Core::JIdentifier* ptr)noexcept override;
		};
		class AssetDataIOInterface
		{
		private:
			friend class JResourceObject;
			friend class JResourceManager;	//store resource
			friend class JFile;		//load resource 
			friend class CreateInstanceInterface;
		private:
			static std::unique_ptr<Core::JDITypeDataBase> CreateLoadAssetDIData(const JUserPtr<JDirectory>& owner, const Core::JAssetFileLoadPathData& pathData);
			static std::unique_ptr<Core::JDITypeDataBase> CreateStoreAssetDIDate(const JUserPtr<JResourceObject>& rObj);
		private:
			virtual JUserPtr<Core::JIdentifier> LoadAssetData(Core::JDITypeDataBase* data) = 0;
			virtual Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) = 0;
		private:
			virtual Core::J_FILE_IO_RESULT LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data) = 0;	//use initData
			virtual Core::J_FILE_IO_RESULT StoreMetaData(Core::JDITypeDataBase* data) = 0;	//use storeData	 
		protected:
			static Core::J_FILE_IO_RESULT LoadCommonMetaData(std::wifstream& stream, Core::JDITypeDataBase* data, const bool closeSream = false);	//use initData
			static Core::J_FILE_IO_RESULT StoreCommonMetaData(std::wofstream& stream, Core::JDITypeDataBase* data, const bool closeSream = false);	//use storeData	
		}; 
		class FileInterface
		{
		private:
			friend class JDirectory;
			friend class Editor::JWindowDirectory;
		private:
			static JUserPtr<JFile> CopyJFile(const JUserPtr<JResourceObject>& from, const JUserPtr<JDirectory>& toDir, bool setNewInnderGuid = false)noexcept;
		private:
			static void MoveFile(const JUserPtr<JResourceObject>& rObj, const JUserPtr<JDirectory>& toDir)noexcept;
			static void DeleteFile(JResourceObject* rObj)noexcept;	//destroy rObj + delete disk asset file
		};
		class DestroyInstanceInterfaceEx
		{ 
		private:
			friend class JResourceManager; 
		private:
			static void BeginForcedDestroy(JResourceObject* rObj) noexcept;
		};
	public:
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override;
		virtual JResourceObjectPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept = 0;
	};

}