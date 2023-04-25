#pragma once
#include"../JObjectPrivate.h" 

namespace JinEngine
{
	namespace Editor
	{
		class JWindowDirectory;
	}
	class JResourceManager;
	class JResourceObjectIO;
	class JResourceObject; 

	class JDirectoryPrivate final : public JObjectPrivate
	{
	public:
		class CreateInstanceInterface;
		class AssetDataIOInterface final
		{ 
		private:
			friend class JResourceObjectIO;
			friend class CreateInstanceInterface;
		private:
			static std::unique_ptr<Core::JDITypeDataBase> CreateLoadAssetDIData(JDirectory* parent, const Core::JAssetFileLoadPathData& pathData);
		private:
			static Core::JIdentifier* LoadAssetData(Core::JDITypeDataBase* data);
			static Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data);
			static Core::J_FILE_IO_RESULT LoadMetaData(std::wifstream& stream, Core::JDITypeDataBase* data);	//use initData
			static Core::J_FILE_IO_RESULT StoreMetaData(std::wofstream& stream, Core::JDITypeDataBase* data);	//use storeData
		};
		class CreateInstanceInterface final : public JObjectPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface;
		private:
			Core::JOwnerPtr<Core::JIdentifier> Create(std::unique_ptr<Core::JDITypeDataBase>&& initData) final;
		private:
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
			void RegisterCash(Core::JIdentifier* createdPtr)noexcept final; 
			void SetValidInstance(Core::JIdentifier* createdPtr)noexcept final;
			bool Copy(Core::JIdentifier* from, Core::JIdentifier* to) noexcept; 
		};
		class DestroyInstanceInterface final : public JObjectPrivate::DestroyInstanceInterface
		{
		private:
			friend class Core::JIdentifier;
		private:
			void Clear(Core::JIdentifier* ptr, const bool isForced) final;  
			void SetInvalidInstance(Core::JIdentifier* ptr)noexcept final;
			void DeRegisterCash(Core::JIdentifier* ptr)noexcept final;
		};
		class FileInterface final
		{
		private:
			friend class JResourceObject; 
			friend class JResourceObjectIO;
		private: 
			static void ConvertToActFileData(JResourceObject* rObj) noexcept;
			static void ConvertToDeActFileData(const size_t guid) noexcept;
		private:
			static JFile* CreateJFile(const JFileInitData& initData, JDirectory* owner);
			static bool DestroyJFile(const size_t rGuid);
		};
		class ActivationInterface final
		{
		private:
			friend class Editor::JWindowDirectory;
			friend class JResourceManager;
		private:
			static void OpenDirectory(JDirectory* dir)noexcept;
			static void CloseDirectory(JDirectory* dir)noexcept;
		};
		class RawDirectoryInterface final
		{
		private:
			friend class Editor::JWindowDirectory; 
		private:
			static void MoveDirectory(JDirectory* dir, JDirectory* newParent)noexcept;
			static void DeleteDirectory(JDirectory* dir)noexcept;
		};
		class DestroyInstanceInterfaceEx
		{
		private:
			friend class JResourceManager;
		private:
			static void BeginForcedDestroy(JDirectory* dir) noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
	};
}
