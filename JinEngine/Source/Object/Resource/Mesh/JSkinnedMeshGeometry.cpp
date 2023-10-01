#include"JSkinnedMeshGeometry.h"
#include"JSkinnedMeshGeometryPrivate.h"
#include"../JResourceObjectUserInterface.h" 
#include"../JResourceManager.h" 
#include"../Skeleton/JSkeletonAsset.h"
#include"../Skeleton/JSkeleton.h"
#include"../Material/JMaterial.h"
#include"../Material/JDefaultMaterialSetting.h"
#include"../../JObjectFileIOHelper.h"
#include"../../Directory/JDirectory.h"  
#include"../../../Core/Guid/JGuidCreator.h"
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/File/JFileConstant.h"  
#include"../../../Core/Geometry/Mesh/JMeshStruct.h"
#include<fstream>

namespace JinEngine
{
	using namespace DirectX; 
	namespace
	{
		const static std::wstring skeletonSymbol = L"--SkeletonData--";
		static JSkinnedMeshGeometryPrivate sPrivate;
	}
 
	class JSkinnedMeshGeometry::JSkinnedMeshGeometryImpl : public Core::JTypeImplBase,
		public JResourceObjectUserInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JSkinnedMeshGeometryImpl)
	public:
		JWeakPtr<JSkinnedMeshGeometry> thisPointer = nullptr;
	private:
		const size_t privateGuid = Core::MakeGuid();
	public:
		JUserPtr<JSkeletonAsset> skeletonAsset = nullptr;
	public:
		JSkinnedMeshGeometryImpl(const InitData& initData, JSkinnedMeshGeometry* thisMeshRaw){}
		~JSkinnedMeshGeometryImpl(){}
	public:
		void SetSkeletonAsset(JUserPtr<JSkeletonAsset> newSkeletonAsset)noexcept
		{
			if (thisPointer->IsActivated())
				CallOffResourceReference(skeletonAsset.Get());
			skeletonAsset = newSkeletonAsset;
			if (thisPointer->IsActivated())
				CallOnResourceReference(skeletonAsset.Get());
		}
	public:
		void OnResourceRef()noexcept
		{
			CallOnResourceReference(skeletonAsset.Get());
		}
		void OffResourceRef()noexcept
		{
			CallOffResourceReference(skeletonAsset.Get());
		}
	public:
		static std::unique_ptr<Core::JMeshGroup> ReadAssetData(const std::wstring& path)
		{
			std::wifstream stream;
			stream.open(path, std::ios::in | std::ios::binary);
			if (!stream.is_open())
				return nullptr;

			std::unique_ptr<Core::JSkinnedMeshGroup> meshGroup = std::make_unique<Core::JSkinnedMeshGroup>();

			uint meshCount = 0;
			uint totalVertexCount = 0;
			uint totalIndexCount = 0;
			JObjectFileIOHelper::LoadAtomicData(stream, meshCount);
			JObjectFileIOHelper::LoadAtomicData(stream, totalVertexCount);
			JObjectFileIOHelper::LoadAtomicData(stream, totalIndexCount);

			for (uint i = 0; i < meshCount; ++i)
			{
				std::wstring name;
				size_t guid;
				uint vertexCount;
				uint indexCount;
				Core::J_MESHGEOMETRY_TYPE meshType;

				JObjectFileIOHelper::LoadJString(stream, name);
				JObjectFileIOHelper::LoadAtomicData(stream, guid);
				JObjectFileIOHelper::LoadAtomicData(stream, vertexCount);
				JObjectFileIOHelper::LoadAtomicData(stream, indexCount);
				JObjectFileIOHelper::LoadEnumData(stream, meshType);
				 
				std::vector<Core::JSkinnedMeshVertex> vertices(vertexCount);
				std::vector<uint> indices;
				JVector4<int> jointIndex;

				for (uint i = 0; i < vertexCount; ++i)
				{
					JObjectFileIOHelper::LoadVector3(stream, vertices[i].position);
					JObjectFileIOHelper::LoadVector3(stream, vertices[i].normal);
					JObjectFileIOHelper::LoadVector2(stream, vertices[i].texC);
					JObjectFileIOHelper::LoadVector3(stream, vertices[i].tangentU);
					JObjectFileIOHelper::LoadVector3(stream, vertices[i].jointWeight);
					JObjectFileIOHelper::LoadVector4(stream, jointIndex);

					vertices[i].jointIndex[0] = jointIndex.x;
					vertices[i].jointIndex[1] = jointIndex.y;
					vertices[i].jointIndex[2] = jointIndex.z;
					vertices[i].jointIndex[3] = jointIndex.w;
				}

				JObjectFileIOHelper::LoadAtomicDataVec(stream, indices);

				DirectX::BoundingBox boundingBox;
				DirectX::BoundingSphere boundingSphere;
				bool hasUV;
				bool hasNormal;

				JObjectFileIOHelper::LoadXMFloat3(stream, boundingBox.Center);
				JObjectFileIOHelper::LoadXMFloat3(stream, boundingBox.Extents);
				JObjectFileIOHelper::LoadXMFloat3(stream, boundingSphere.Center);
				JObjectFileIOHelper::LoadAtomicData(stream, boundingSphere.Radius);
				JObjectFileIOHelper::LoadAtomicData(stream, hasUV);
				JObjectFileIOHelper::LoadAtomicData(stream, hasNormal);

				meshGroup->AddMeshData(Core::JSkinnedMeshData{ name , guid, std::move(indices), hasUV, hasNormal, std::move(vertices) });
			}
 
			for (uint i = 0; i < meshCount; ++i)
				meshGroup->GetMeshData(i)->SetMaterial(JObjectFileIOHelper::_LoadHasIden<JMaterial>(stream));
			meshGroup->SetSkeletonAsset(JObjectFileIOHelper::_LoadHasIden<JSkeletonAsset>(stream));

			stream.close();
			return std::move(meshGroup);
		}
		bool WriteAssetData(const std::wstring& path, Core::JMeshGroup* meshGroup)
		{
			if (meshGroup == nullptr)
				return false;

			if (meshGroup->GetMeshGroupType() != Core::J_MESHGEOMETRY_TYPE::SKINNED)
				return false;

			std::wofstream stream;
			stream.open(path, std::ios::out | std::ios::binary);
			if (!stream.is_open())
				return false;

			Core::JSkinnedMeshGroup* skinnedMeshs = static_cast<Core::JSkinnedMeshGroup*>(meshGroup);
			const uint meshCount = skinnedMeshs->GetMeshDataCount();
			JObjectFileIOHelper::StoreAtomicData(stream, L"MeshCount:", meshCount);
			JObjectFileIOHelper::StoreAtomicData(stream, L"TotalVertexCount:", skinnedMeshs->GetTotalVertexCount());
			JObjectFileIOHelper::StoreAtomicData(stream, L"TotalIndexCount:", skinnedMeshs->GetTotalIndexCount());

			for (uint i = 0; i < meshCount; ++i)
			{
				Core::JSkinnedMeshData* skinnedData = static_cast<Core::JSkinnedMeshData*>(skinnedMeshs->GetMeshData(i));

				JObjectFileIOHelper::StoreJString(stream, L"Name:", skinnedData->GetName());
				JObjectFileIOHelper::StoreAtomicData(stream, Core::JFileConstant::StreamHasObjGuidSymbol(), skinnedData->GetGuid());

				const uint vertexCount = skinnedData->GetVertexCount();
				const uint indexCount = skinnedData->GetIndexCount();

				JObjectFileIOHelper::StoreAtomicData(stream, L"VertexCount:", vertexCount);
				JObjectFileIOHelper::StoreAtomicData(stream, L"IndexCount:", indexCount);
				JObjectFileIOHelper::StoreEnumData(stream, L"MeshType:", Core::J_MESHGEOMETRY_TYPE::SKINNED);

				for (uint i = 0; i < vertexCount; ++i)
				{
					Core::JSkinnedMeshVertex vertices = skinnedData->GetVertex(i);
					JObjectFileIOHelper::StoreVector3(stream, L"P:", vertices.position);
					JObjectFileIOHelper::StoreVector3(stream, L"N:", vertices.normal);
					JObjectFileIOHelper::StoreVector2(stream, L"U:", vertices.texC);
					JObjectFileIOHelper::StoreVector3(stream, L"T:", vertices.tangentU);
					JObjectFileIOHelper::StoreVector3(stream, L"W:", vertices.jointWeight);
					JObjectFileIOHelper::StoreXMFloat4(stream, L"I", XMFLOAT4(vertices.jointIndex[0],
						vertices.jointIndex[1],
						vertices.jointIndex[2],
						vertices.jointIndex[3]));
				}
				JObjectFileIOHelper::StoreAtomicDataVec(stream, L"Index:", skinnedData->GetIndexVector(), 6);

				const DirectX::BoundingBox boundingBox = skinnedData->GetBBox();
				const DirectX::BoundingSphere boundingSphere = skinnedData->GetBSphere();

				JObjectFileIOHelper::StoreXMFloat3(stream, L"BBoxCenter:", boundingBox.Center);
				JObjectFileIOHelper::StoreXMFloat3(stream, L"BBoxExtents:", boundingBox.Extents);
				JObjectFileIOHelper::StoreXMFloat3(stream, L"SphereCenter:", boundingSphere.Center);
				JObjectFileIOHelper::StoreAtomicData(stream, L"SphereRadius:", boundingSphere.Radius);
				JObjectFileIOHelper::StoreAtomicData(stream, L"HasUV:", skinnedData->HasUV());
				JObjectFileIOHelper::StoreAtomicData(stream, L"HasNormal:", skinnedData->HasNormal());
			}
			 
			for (uint i = 0; i < meshCount; ++i)
				JObjectFileIOHelper::_StoreHasIden(stream, Core::ConnectChildUserPtr<JMaterial>(skinnedMeshs->GetMeshData(i)->GetMaterial()).Get());
			JObjectFileIOHelper::_StoreHasIden(stream, Core::ConnectChildUserPtr<JSkeletonAsset>(skinnedMeshs->GetSkeletonAsset()).Get());
			stream.close();
			return true;
		}
	public:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{ 
			if (iden == thisPointer->GetGuid())
				return;
			 
			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (skeletonAsset != nullptr && skeletonAsset->GetGuid() == jRobj->GetGuid())
					SetSkeletonAsset(nullptr);
			}
		}
	public:
		static std::unique_ptr<InitData> CreateInitData(const std::wstring& name, const std::wstring& path, LoadMetaData* meta)
		{
			return std::make_unique<InitData>(name, meta->guid, meta->flag, meta->formatIndex, meta->directory, ReadAssetData(path));
		}
	public:
		void NotifyReAlloc()
		{
			ResetEventListenerPointer(*JResourceObject::EvInterface(), privateGuid);
		}
	public:
		void Initialize(InitData* initData)
		{
			SetSkeletonAsset(initData->skeletonAsset);
		}	 
		void RegisterThisPointer(JSkinnedMeshGeometry* mesh)
		{
			thisPointer = Core::GetWeakPtr(mesh);
		}
		void RegisterPostCreation()
		{
			AddEventListener(*JResourceObject::EvInterface(), privateGuid, J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		}
		void DeRegisterPreDestruction()
		{
			RemoveListener(*JResourceObject::EvInterface(), privateGuid);
		}
		static void RegisterTypeData()
		{
			Core::JIdentifier::RegisterPrivateInterface(JSkinnedMeshGeometry::StaticTypeInfo(), sPrivate);
			IMPL_REALLOC_BIND(JSkinnedMeshGeometry::JSkinnedMeshGeometryImpl, thisPointer)
		}
	};

	JSkinnedMeshGeometry::InitData::InitData(const uint8 formatIndex, const JUserPtr<JDirectory>& directory, std::unique_ptr<Core::JMeshGroup>&& skinnedMeshGroup)
		: JMeshGeometry::InitData(JSkinnedMeshGeometry::StaticTypeInfo(), formatIndex, directory, std::move(skinnedMeshGroup))
	{
		skeletonAsset = Core::ConvertChildUserPtr<JSkeletonAsset>(static_cast<Core::JSkinnedMeshGroup*>(meshGroup.get())->GetSkeletonAsset());
	}
	JSkinnedMeshGeometry::InitData::InitData(const size_t guid,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		std::unique_ptr<Core::JMeshGroup>&& skinnedMeshGroup)
		: JMeshGeometry::InitData(JSkinnedMeshGeometry::StaticTypeInfo(), guid, formatIndex, directory, std::move(skinnedMeshGroup))
	{
		skeletonAsset = Core::ConvertChildUserPtr<JSkeletonAsset>(static_cast<Core::JSkinnedMeshGroup*>(meshGroup.get())->GetSkeletonAsset());
	}
	JSkinnedMeshGeometry::InitData::InitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		std::unique_ptr<Core::JMeshGroup>&& skinnedMeshGroup)
		: JMeshGeometry::InitData(JSkinnedMeshGeometry::StaticTypeInfo(), name, guid, flag, formatIndex, directory, std::move(skinnedMeshGroup))
	{
		skeletonAsset = Core::ConvertChildUserPtr<JSkeletonAsset>(static_cast<Core::JSkinnedMeshGroup*>(meshGroup.get())->GetSkeletonAsset());
	}
	bool JSkinnedMeshGeometry::InitData::IsValidData()const noexcept
	{
		return JMeshGeometry::InitData::IsValidData() && skeletonAsset.IsValid() && skeletonAsset->GetTypeInfo().IsChildOf<JSkeletonAsset>();
	}


	JSkinnedMeshGeometry::LoadMetaData::LoadMetaData(const JUserPtr<JDirectory>& directory)
		: JMeshGeometry::LoadMetaData(JSkinnedMeshGeometry::StaticTypeInfo(), directory)
	{}

	Core::JIdentifierPrivate& JSkinnedMeshGeometry::PrivateInterface()const noexcept
	{
		return sPrivate;
	}
	Core::J_MESHGEOMETRY_TYPE JSkinnedMeshGeometry::GetMeshGeometryType()const noexcept
	{
		return Core::J_MESHGEOMETRY_TYPE::SKINNED;
	}
	JUserPtr<JSkeletonAsset> JSkinnedMeshGeometry::GetSkeletonAsset()const noexcept
	{
		return impl->skeletonAsset;
	}
	void JSkinnedMeshGeometry::DoActivate()noexcept
	{
		JMeshGeometry::DoActivate();
		impl->OnResourceRef();
	}
	void JSkinnedMeshGeometry::DoDeActivate()noexcept
	{
		impl->OffResourceRef();
		JMeshGeometry::DoDeActivate();
	}
	JSkinnedMeshGeometry::JSkinnedMeshGeometry(InitData& initData)
		:JMeshGeometry(initData), impl(std::make_unique<JSkinnedMeshGeometryImpl>(initData, this))
	{}
	JSkinnedMeshGeometry::~JSkinnedMeshGeometry()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JSkinnedMeshGeometryPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JSkinnedMeshGeometryPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JSkinnedMeshGeometryPrivate::AssetDataIOInterface; 

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JSkinnedMeshGeometry>(*static_cast<JSkinnedMeshGeometry::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JMeshGeometryPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JSkinnedMeshGeometry* mesh = static_cast<JSkinnedMeshGeometry*>(createdPtr);
		mesh->impl->RegisterThisPointer(mesh);
		mesh->impl->RegisterPostCreation();
		mesh->impl->Initialize(static_cast<JSkinnedMeshGeometry::InitData*>(initData));
	} 
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JSkinnedMeshGeometry::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr,const bool isForced)
	{
		static_cast<JSkinnedMeshGeometry*>(ptr)->impl->DeRegisterPreDestruction();
		JMeshGeometryPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JSkinnedMeshGeometry::LoadData::StaticTypeInfo()))
			return nullptr;
 
		auto loadData = static_cast<JSkinnedMeshGeometry::LoadData*>(data);
		auto pathData = loadData->pathData;
		JUserPtr<JDirectory> directory = loadData->directory;

		auto metaData = std::make_unique<JSkinnedMeshGeometry::LoadMetaData>(directory);	//for load metadata
		if (LoadMetaData(pathData.engineMetaFileWPath, metaData.get()) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;

		JUserPtr<JSkinnedMeshGeometry> newMesh = nullptr;
		if (directory->HasFile(metaData->guid))
			newMesh = Core::GetUserPtr<JSkinnedMeshGeometry>(JSkinnedMeshGeometry::StaticTypeInfo().TypeGuid(), metaData->guid);

		if (newMesh == nullptr)
		{
			using Impl = JSkinnedMeshGeometry::JSkinnedMeshGeometryImpl;
			auto idenUser = sPrivate.GetCreateInstanceInterface().BeginCreate(Impl::CreateInitData(pathData.name, pathData.engineFileWPath, metaData.get()), &sPrivate);
			newMesh.ConnnectChild(idenUser);
		}
		return newMesh;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JSkinnedMeshGeometry::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JSkinnedMeshGeometry::StoreData*>(data);
		if (!storeData->HasCorrectType(JSkinnedMeshGeometry::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JUserPtr<JSkinnedMeshGeometry>newMesh;
		newMesh.ConnnectChild(storeData->obj);
		return newMesh->impl->WriteAssetData(newMesh->GetPath(), newMesh->GetMeshGroupData()) ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JSkinnedMeshGeometry::LoadMetaData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		std::wifstream stream;
		stream.open(path, std::ios::in | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto loadMetaData = static_cast<JSkinnedMeshGeometry::LoadMetaData*>(data);
		if (LoadCommonMetaData(stream, loadMetaData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JObjectFileIOHelper::LoadEnumData(stream, loadMetaData->meshType);
		stream.close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JSkinnedMeshGeometry::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JSkinnedMeshGeometry::StoreData*>(data);
		JUserPtr<JSkinnedMeshGeometry>mesh;
		mesh.ConnnectChild(storeData->obj);

		std::wofstream stream;
		stream.open(mesh->GetMetaFilePath(), std::ios::out | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (StoreCommonMetaData(stream, storeData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JObjectFileIOHelper::StoreEnumData(stream, L"MeshType:", mesh->GetMeshGeometryType());
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	std::unique_ptr<Core::JMeshGroup> AssetDataIOInterface::ReadMeshGroupData(const std::wstring& path)
	{
		return JSkinnedMeshGeometry::JSkinnedMeshGeometryImpl::ReadAssetData(path);
	}
	 
	Core::JIdentifierPrivate::CreateInstanceInterface& JSkinnedMeshGeometryPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JSkinnedMeshGeometryPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
	JResourceObjectPrivate::AssetDataIOInterface& JSkinnedMeshGeometryPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}