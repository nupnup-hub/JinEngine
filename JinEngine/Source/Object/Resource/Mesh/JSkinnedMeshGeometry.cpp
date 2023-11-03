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
			JFileIOTool tool;
			if (!tool.Begin(path, JFileIOTool::TYPE::INPUT_STREAM))
				return nullptr;

			std::unique_ptr<Core::JSkinnedMeshGroup> meshGroup = std::make_unique<Core::JSkinnedMeshGroup>();

			uint meshCount = 0;
			uint totalVertexCount = 0;
			uint totalIndexCount = 0;
			JObjectFileIOHelper::LoadAtomicData(tool, meshCount, "MeshCount:");
			JObjectFileIOHelper::LoadAtomicData(tool, totalVertexCount, "TotalVertexCount:");
			JObjectFileIOHelper::LoadAtomicData(tool, totalIndexCount, "TotalIndexCount:");
 
			tool.PushExistStack("MeshData");
			for (uint i = 0; i < meshCount; ++i)
			{
				std::wstring name;
				size_t guid;
				uint vertexCount;
				uint indexCount;
				Core::J_MESHGEOMETRY_TYPE meshType;

				tool.PushExistStack();
				JObjectFileIOHelper::LoadJString(tool, name, "Name:");
				JObjectFileIOHelper::LoadAtomicData(tool, guid, Core::JFileConstant::GetHasObjGuidSymbol());
				JObjectFileIOHelper::LoadAtomicData(tool, vertexCount, "VertexCount:");
				JObjectFileIOHelper::LoadAtomicData(tool, indexCount, "IndexCount:");
				JObjectFileIOHelper::LoadEnumData(tool, meshType, "MeshType:");
				 
				std::vector<Core::JSkinnedMeshVertex> vertices(vertexCount);
				std::vector<uint> indices;
				JVector4<int> jointIndex;

				tool.PushExistStack("VerticesData");
				for (uint i = 0; i < vertexCount; ++i)
				{
					tool.PushExistStack();
					JObjectFileIOHelper::LoadVector3(tool, vertices[i].position, "P:");
					JObjectFileIOHelper::LoadVector3(tool, vertices[i].normal, "N:");
					JObjectFileIOHelper::LoadVector2(tool, vertices[i].texC, "U:");
					JObjectFileIOHelper::LoadVector3(tool, vertices[i].tangentU, "T:");
					JObjectFileIOHelper::LoadVector3(tool, vertices[i].jointWeight, "W:");
					JObjectFileIOHelper::LoadVector4(tool, jointIndex, "I");
					tool.PopStack();
					
					vertices[i].jointIndex[0] = jointIndex.x;
					vertices[i].jointIndex[1] = jointIndex.y;
					vertices[i].jointIndex[2] = jointIndex.z;
					vertices[i].jointIndex[3] = jointIndex.w;
				}
				tool.PopStack();

				JObjectFileIOHelper::LoadAtomicDataVec(tool, indices, "Index");
				DirectX::BoundingBox boundingBox;
				DirectX::BoundingSphere boundingSphere;
				bool hasUV;
				bool hasNormal;

				JObjectFileIOHelper::LoadXMFloat3(tool, boundingBox.Center, "BBoxCenter:");
				JObjectFileIOHelper::LoadXMFloat3(tool, boundingBox.Extents, "BBoxExtents:");
				JObjectFileIOHelper::LoadXMFloat3(tool, boundingSphere.Center, "SphereCenter:");
				JObjectFileIOHelper::LoadAtomicData(tool, boundingSphere.Radius, "SphereRadius:");
				JObjectFileIOHelper::LoadAtomicData(tool, hasUV, "HasUV:");
				JObjectFileIOHelper::LoadAtomicData(tool, hasNormal, "HasNormal:");
				tool.PopStack();
				meshGroup->AddMeshData(Core::JSkinnedMeshData{ name , guid, std::move(indices), hasUV, hasNormal, std::move(vertices) });
			}
			tool.PopStack();
			tool.PushExistStack("MaterialData");
			for (uint i = 0; i < meshCount; ++i)
			{
				//tool.PushExistStack();
				meshGroup->GetMeshData(i)->SetMaterial(JObjectFileIOHelper::_LoadHasIden<JMaterial>(tool, std::to_string(i)));
				//tool.PopStack();
			}
			meshGroup->SetSkeletonAsset(JObjectFileIOHelper::_LoadHasIden<JSkeletonAsset>(tool, "SkeletonAsset"));
			tool.Close();
			return std::move(meshGroup);
		}
		bool WriteAssetData(const std::wstring& path, Core::JMeshGroup* meshGroup)
		{
			if (meshGroup == nullptr)
				return false;

			if (meshGroup->GetMeshGroupType() != Core::J_MESHGEOMETRY_TYPE::SKINNED)
				return false;

			JFileIOTool tool;
			if (!tool.Begin(thisPointer->GetPath(), JFileIOTool::TYPE::OUTPUT_STREAM))
				return false;

			Core::JSkinnedMeshGroup* skinnedMeshs = static_cast<Core::JSkinnedMeshGroup*>(meshGroup);
			const uint meshCount = skinnedMeshs->GetMeshDataCount();
			JObjectFileIOHelper::StoreAtomicData(tool, meshCount, "MeshCount:");
			JObjectFileIOHelper::StoreAtomicData(tool, skinnedMeshs->GetTotalVertexCount(), "TotalVertexCount:");
			JObjectFileIOHelper::StoreAtomicData(tool, skinnedMeshs->GetTotalIndexCount(), "TotalIndexCount:");

			tool.PushArrayOwner("MeshData");
			for (uint i = 0; i < meshCount; ++i)
			{
				Core::JSkinnedMeshData* skinnedData = static_cast<Core::JSkinnedMeshData*>(skinnedMeshs->GetMeshData(i));

				tool.PushArrayMember();
				JObjectFileIOHelper::StoreJString(tool, skinnedData->GetName(), "Name:");
				JObjectFileIOHelper::StoreAtomicData(tool, skinnedData->GetGuid(), Core::JFileConstant::GetHasObjGuidSymbol());

				const uint vertexCount = skinnedData->GetVertexCount();
				const uint indexCount = skinnedData->GetIndexCount();

				JObjectFileIOHelper::StoreAtomicData(tool, vertexCount, "VertexCount:");
				JObjectFileIOHelper::StoreAtomicData(tool, indexCount, "IndexCount:");
				JObjectFileIOHelper::StoreEnumData(tool, Core::J_MESHGEOMETRY_TYPE::SKINNED, "MeshType:");

				tool.PushArrayOwner("VerticesData");
				for (uint i = 0; i < vertexCount; ++i)
				{
					tool.PushArrayMember();
					Core::JSkinnedMeshVertex vertices = skinnedData->GetVertex(i);
					JObjectFileIOHelper::StoreVector3(tool, vertices.position, "P:");
					JObjectFileIOHelper::StoreVector3(tool, vertices.normal, "N:");
					JObjectFileIOHelper::StoreVector2(tool, vertices.texC, "U:");
					JObjectFileIOHelper::StoreVector3(tool, vertices.tangentU, "T:");
					JObjectFileIOHelper::StoreVector3(tool, vertices.jointWeight, "W:");
					JObjectFileIOHelper::StoreXMFloat4(tool, XMFLOAT4(vertices.jointIndex[0],
						vertices.jointIndex[1],
						vertices.jointIndex[2],
						vertices.jointIndex[3]),
						"I");
					tool.PopStack();
				}
				tool.PopStack();
				JObjectFileIOHelper::StoreAtomicDataVec(tool, skinnedData->GetIndexVector(), 8, "Index:");

				const DirectX::BoundingBox boundingBox = skinnedData->GetBBox();
				const DirectX::BoundingSphere boundingSphere = skinnedData->GetBSphere();

				JObjectFileIOHelper::StoreXMFloat3(tool, boundingBox.Center, "BBoxCenter:");
				JObjectFileIOHelper::StoreXMFloat3(tool, boundingBox.Extents, "BBoxExtents:");
				JObjectFileIOHelper::StoreXMFloat3(tool, boundingSphere.Center, "SphereCenter:");
				JObjectFileIOHelper::StoreAtomicData(tool, boundingSphere.Radius, "SphereRadius:");
				JObjectFileIOHelper::StoreAtomicData(tool, skinnedData->HasUV(), "HasUV:");
				JObjectFileIOHelper::StoreAtomicData(tool, skinnedData->HasNormal(), "HasNormal:");
				tool.PopStack();
			}
			tool.PopStack();
			tool.PushArrayOwner("MaterialData");
			for (uint i = 0; i < meshCount; ++i)
			{
				//tool.PushArrayMember();
				JObjectFileIOHelper::_StoreHasIden(tool, Core::ConnectChildUserPtr<JMaterial>(skinnedMeshs->GetMeshData(i)->GetMaterial()).Get(), std::to_string(i));
				//tool.PopStack();
			}
			tool.PopStack();
			JObjectFileIOHelper::_StoreHasIden(tool, Core::ConnectChildUserPtr<JSkeletonAsset>(skinnedMeshs->GetSkeletonAsset()).Get(), "SkeletonAsset");
			tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
			return true;
		}
	public:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj, JResourceEventDesc* desc)
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

		JFileIOTool tool;
		if (!tool.Begin(path, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto loadMetaData = static_cast<JSkinnedMeshGeometry::LoadMetaData*>(data);
		if (LoadCommonMetaData(tool, loadMetaData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JObjectFileIOHelper::LoadEnumData(tool, loadMetaData->meshType, "MeshType");
		tool.Close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JSkinnedMeshGeometry::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JSkinnedMeshGeometry::StoreData*>(data);
		JUserPtr<JSkinnedMeshGeometry>mesh;
		mesh.ConnnectChild(storeData->obj);

		JFileIOTool tool;
		if (!tool.Begin(mesh->GetMetaFilePath(), JFileIOTool::TYPE::JSON))
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (StoreCommonMetaData(tool, storeData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JObjectFileIOHelper::StoreEnumData(tool, mesh->GetMeshGeometryType(), "MeshType");
		tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
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