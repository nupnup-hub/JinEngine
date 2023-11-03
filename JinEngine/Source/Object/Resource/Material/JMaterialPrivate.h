#pragma once
#include"../JResourceObjectPrivate.h" 

namespace JinEngine
{ 
	namespace Graphic
	{
		struct JMaterialConstants;
		class JGraphic;
	} 
	class JMaterial;
	class JRenderItem;
	class JPreviewResourceScene;
	class JFrameDirtyBase;
	class JDefaultMaterialSetting;
	class JResourceManager;

	class JMaterialPrivate final : public JResourceObjectPrivate
	{
	public:
		class AssetDataIOInterface final : public JResourceObjectPrivate::AssetDataIOInterface
		{
		private:
			JUserPtr<Core::JIdentifier> LoadAssetData(Core::JDITypeDataBase* data) final;
			Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) final;
		private:
			Core::J_FILE_IO_RESULT LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)final;	//use clipMetaData
			Core::J_FILE_IO_RESULT StoreMetaData(Core::JDITypeDataBase* data)final;	//use storeData	 
		};
		class CreateInstanceInterface final : public JResourceObjectPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface;
		private:
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData) final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		};
		class DestroyInstanceInterface final : public JResourceObjectPrivate::DestroyInstanceInterface
		{
		private:
			void Clear(Core::JIdentifier* ptr, const bool isForced) final;
		};
		class FrameUpdateInterface final
		{
		private:
			friend class Graphic::JGraphic;
		private: 
			static bool UpdateStart(JMaterial* mat, const bool isUpdateForced)noexcept;
			static void UpdateFrame(JMaterial* mat, Graphic::JMaterialConstants& constants)noexcept;
			static void UpdateEnd(JMaterial* mat)noexcept;
		private:
			static uint GetMaterialFrameIndex(JMaterial* mat)noexcept;
		private:
			static bool IsLastFrameUpdated(JMaterial* mat);
			static bool HasRecopyRequest(JMaterial* mat)noexcept;
		};
		class FrameIndexInterface final
		{
		private:
			friend class JRenderItem;
		private:
			static uint GetMaterialFrameIndex(JMaterial* mat)noexcept;
		};
		class UpdateShaderInterface
		{
		private:
			friend class JDefaultMaterialSetting;
			friend class JResourceManager;;
		private:
			static void OnUpdateShaderTrigger(const JUserPtr<JMaterial>& mat)noexcept;
			static void OffUpdateShaderTrigger(const JUserPtr<JMaterial>& mat)noexcept;
		private:
			static void UpdateShader(const JUserPtr<JMaterial>& mat)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
		JResourceObjectPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}