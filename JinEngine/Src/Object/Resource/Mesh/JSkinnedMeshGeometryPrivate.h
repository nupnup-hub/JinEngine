#pragma once
#include"JMeshGeometryPrivate.h" 

namespace JinEngine
{
	class JSkinnedMeshGeometryPrivate final : public JMeshGeometryPrivate
	{
	public:
		class AssetDataIOInterface final : public JMeshGeometryPrivate::AssetDataIOInterface
		{
		private:
			Core::JIdentifier* LoadAssetData(Core::JDITypeDataBase* data) final;
			Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) final;////리소스 생성시 에셋파일이 없을 경우 한번 호출된다
		private:
			Core::J_FILE_IO_RESULT LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)final;	//use clipMetaData
			Core::J_FILE_IO_RESULT StoreMetaData(Core::JDITypeDataBase* data)final;	//use storeData	 
		private:
			std::unique_ptr<JMeshGroup> ReadMeshGroupData(const std::wstring& path) final;
		};
		class CreateInstanceInterface final : public JMeshGeometryPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface;
		private:
			Core::JOwnerPtr<Core::JIdentifier> Create(std::unique_ptr<Core::JDITypeDataBase>&& initData) final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JResourceObjectPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}