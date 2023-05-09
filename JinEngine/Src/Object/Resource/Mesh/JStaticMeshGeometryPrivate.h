#pragma once
#include"JMeshGeometryPrivate.h" 

namespace JinEngine
{ 
	class JStaticMeshGeometryPrivate final : public JMeshGeometryPrivate
	{
	public:
		class AssetDataIOInterface final : public JMeshGeometryPrivate::AssetDataIOInterface
		{
		private:
			JUserPtr<Core::JIdentifier> LoadAssetData(Core::JDITypeDataBase* data) final;
			Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) final;////���ҽ� ������ ���������� ���� ��� �ѹ� ȣ��ȴ�
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
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData) final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JResourceObjectPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}