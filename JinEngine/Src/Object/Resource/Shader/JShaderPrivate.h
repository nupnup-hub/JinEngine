#pragma once
#include"../JResourceObjectPrivate.h" 

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphic;
		class JHZBOccCulling;
	}
	class JShader;
	class JShaderPrivate final : public JResourceObjectPrivate
	{
	public:
		class AssetDataIOInterface final : public JResourceObjectPrivate::AssetDataIOInterface
		{
		private:
			Core::JIdentifier* LoadAssetData(Core::JDITypeDataBase* data) final;
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
			Core::JOwnerPtr<Core::JIdentifier> Create(std::unique_ptr<Core::JDITypeDataBase>&& initData) final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		};
		class CompileInterface final
		{
		private:
			friend class Graphic::JGraphic;
			friend class Graphic::JHZBOccCulling;
		private:
			static void RecompileGraphicShader(JShader* shader)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JResourceObjectPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}