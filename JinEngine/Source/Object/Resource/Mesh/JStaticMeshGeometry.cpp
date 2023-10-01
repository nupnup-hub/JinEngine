#include"JStaticMeshGeometry.h" 
#include"JStaticMeshGeometryPrivate.h" 
#include"../JResourceManager.h"
#include"../Material/JMaterial.h"
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
		static JStaticMeshGeometryPrivate sPrivate;
	}
 
	class JStaticMeshGeometry::JStaticMeshGeometryImpl : public Core::JTypeImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JStaticMeshGeometryImpl)
	public:
		JWeakPtr<JStaticMeshGeometry> thisPointer = nullptr;
	public:
		JStaticMeshGeometryImpl(const InitData& initData)
		{}
		~JStaticMeshGeometryImpl(){} 
	public:
		static std::unique_ptr<Core::JMeshGroup> ReadAssetData(const std::wstring& path)
		{
			std::wifstream stream;
			stream.open(path, std::ios::in | std::ios::binary);
			if (!stream.is_open())
				return nullptr;
			 
			std::unique_ptr<Core::JStaticMeshGroup> meshGroup = std::make_unique<Core::JStaticMeshGroup>();

			uint meshCount = 0;
			uint totalVertexCount = 0;
			uint totalIndexCount = 0;
			JObjectFileIOHelper::LoadAtomicData(stream, meshCount);
			JObjectFileIOHelper::LoadAtomicData(stream, totalVertexCount);
			JObjectFileIOHelper::LoadAtomicData(stream, totalIndexCount);

			for (uint i = 0; i < meshCount; ++i)
			{
				std::wstring name;
				size_t guid = 0;
				uint vertexCount = 0;
				uint indexCount = 0;
				Core::J_MESHGEOMETRY_TYPE meshType;

				JObjectFileIOHelper::LoadJString(stream, name);
				JObjectFileIOHelper::LoadAtomicData(stream, guid);
				JObjectFileIOHelper::LoadAtomicData(stream, vertexCount);
				JObjectFileIOHelper::LoadAtomicData(stream, indexCount);
				JObjectFileIOHelper::LoadEnumData(stream, meshType);

				std::vector<Core::JStaticMeshVertex> vertices(vertexCount);
				std::vector<uint> indices;

				for (uint i = 0; i < vertexCount; ++i)
				{
					JObjectFileIOHelper::LoadVector3(stream, vertices[i].position);
					JObjectFileIOHelper::LoadVector3(stream, vertices[i].normal);
					JObjectFileIOHelper::LoadVector2(stream, vertices[i].texC);
					JObjectFileIOHelper::LoadVector3(stream, vertices[i].tangentU);
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

				meshGroup->AddMeshData(Core::JStaticMeshData{ name , guid, std::move(indices),hasUV, hasNormal, std::move(vertices) });
			} 

			for (uint i = 0; i < meshCount; ++i)
				meshGroup->GetMeshData(i)->SetMaterial(JObjectFileIOHelper::_LoadHasIden<JMaterial>(stream));
			stream.close();
			return std::move(meshGroup);
		}
		static bool WriteAssetData(const std::wstring& path, Core::JMeshGroup* meshGroup)
		{
			std::wofstream stream;
			stream.open(path, std::ios::out | std::ios::binary);
			if (!stream.is_open())
				return false;

			if(meshGroup == nullptr)
				return false;

			Core::JStaticMeshGroup* staticMeshs = static_cast<Core::JStaticMeshGroup*>(meshGroup);
			const uint meshCount = staticMeshs->GetMeshDataCount();
			JObjectFileIOHelper::StoreAtomicData(stream, L"MeshCount:", meshCount);
			JObjectFileIOHelper::StoreAtomicData(stream, L"TotalVertexCount:", staticMeshs->GetTotalVertexCount());
			JObjectFileIOHelper::StoreAtomicData(stream, L"TotalIndexCount:", staticMeshs->GetTotalIndexCount());

			for (uint i = 0; i < meshCount; ++i)
			{
				Core::JStaticMeshData* staticData = static_cast<Core::JStaticMeshData*>(staticMeshs->GetMeshData(i));

				JObjectFileIOHelper::StoreJString(stream, L"Name:", staticData->GetName());
				JObjectFileIOHelper::StoreAtomicData(stream, Core::JFileConstant::StreamHasObjGuidSymbol(), staticData->GetGuid());

				const uint vertexCount = staticData->GetVertexCount();
				const uint indexCount = staticData->GetIndexCount();

				JObjectFileIOHelper::StoreAtomicData(stream, L"VertexCount:", vertexCount);
				JObjectFileIOHelper::StoreAtomicData(stream, L"IndexCount:", indexCount);
				JObjectFileIOHelper::StoreEnumData(stream, L"MeshType:", Core::J_MESHGEOMETRY_TYPE::STATIC);

				for (uint i = 0; i < vertexCount; ++i)
				{
					Core::JStaticMeshVertex vertices = staticData->GetVertex(i);
					JObjectFileIOHelper::StoreVector3(stream, L"P:", vertices.position);
					JObjectFileIOHelper::StoreVector3(stream, L"N:", vertices.normal);
					JObjectFileIOHelper::StoreVector2(stream, L"U:", vertices.texC);
					JObjectFileIOHelper::StoreVector3(stream, L"T:", vertices.tangentU);
				}
				JObjectFileIOHelper::StoreAtomicDataVec(stream, L"Index:", staticData->GetIndexVector(), 6);

				const DirectX::BoundingBox boundingBox = staticData->GetBBox();
				const DirectX::BoundingSphere boundingSphere = staticData->GetBSphere();

				JObjectFileIOHelper::StoreXMFloat3(stream, L"BBoxCenter:", boundingBox.Center);
				JObjectFileIOHelper::StoreXMFloat3(stream, L"BBoxExtents:", boundingBox.Extents);
				JObjectFileIOHelper::StoreXMFloat3(stream, L"SphereCenter:", boundingSphere.Center);
				JObjectFileIOHelper::StoreAtomicData(stream, L"SphereRadius:", boundingSphere.Radius);
				JObjectFileIOHelper::StoreAtomicData(stream, L"HasUV:", staticData->HasUV());
				JObjectFileIOHelper::StoreAtomicData(stream, L"HasNormal:", staticData->HasNormal());
			}
			  
			for (uint i = 0; i < meshCount; ++i)
				JObjectFileIOHelper::_StoreHasIden(stream,  Core::ConnectChildUserPtr<JMaterial>(meshGroup->GetMeshData(i)->GetMaterial()).Get());
			
			stream.close();
			return true;
		} 
	public:
		static std::unique_ptr<InitData> CreateInitData(const std::wstring& name, const std::wstring& path, LoadMetaData* meta)
		{
			return std::make_unique<InitData>(name, meta->guid, meta->flag, meta->formatIndex, meta->directory, ReadAssetData(path));
		}
	public:
		void RegisterThisPointer(JStaticMeshGeometry* mesh)
		{
			thisPointer = Core::GetWeakPtr(mesh);
		}
		static void RegisterTypeData()
		{
			Core::JIdentifier::RegisterPrivateInterface(JStaticMeshGeometry::StaticTypeInfo(), sPrivate);
			IMPL_REALLOC_BIND(JStaticMeshGeometry::JStaticMeshGeometryImpl, thisPointer)
		}
	};

	JStaticMeshGeometry::InitData::InitData(const uint8 formatIndex, const JUserPtr<JDirectory>& directory, std::unique_ptr<Core::JMeshGroup>&& meshGroup)
		: JMeshGeometry::InitData(JStaticMeshGeometry::StaticTypeInfo(), formatIndex, directory, std::move(meshGroup))
	{}
	JStaticMeshGeometry::InitData::InitData(const size_t guid,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		std::unique_ptr<Core::JMeshGroup>&& meshGroup)
		: JMeshGeometry::InitData(JStaticMeshGeometry::StaticTypeInfo(), guid, formatIndex, directory, std::move(meshGroup))
	{ }
	JStaticMeshGeometry::InitData::InitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		std::unique_ptr<Core::JMeshGroup>&& meshGroup)
		: JMeshGeometry::InitData(JStaticMeshGeometry::StaticTypeInfo(), name, guid, flag, formatIndex, directory, std::move(meshGroup))
	{ }

	JStaticMeshGeometry::LoadMetaData::LoadMetaData(const JUserPtr<JDirectory>& directory)
		: JMeshGeometry::LoadMetaData(JStaticMeshGeometry::StaticTypeInfo(), directory)
	{}

	Core::JIdentifierPrivate& JStaticMeshGeometry::PrivateInterface()const noexcept
	{
		return sPrivate;
	}
	Core::J_MESHGEOMETRY_TYPE JStaticMeshGeometry::GetMeshGeometryType()const noexcept
	{
		return Core::J_MESHGEOMETRY_TYPE::STATIC;
	}
	JStaticMeshGeometry::JStaticMeshGeometry(InitData& initData)
		: JMeshGeometry(initData), impl(std::make_unique<JStaticMeshGeometryImpl>(initData))
	{ }
	JStaticMeshGeometry::~JStaticMeshGeometry()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JStaticMeshGeometryPrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JStaticMeshGeometryPrivate::AssetDataIOInterface; 

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{ 
		return Core::JPtrUtil::MakeOwnerPtr<JStaticMeshGeometry>(*static_cast<JStaticMeshGeometry::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JMeshGeometryPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JStaticMeshGeometry* mesh = static_cast<JStaticMeshGeometry*>(createdPtr);
		mesh->impl->RegisterThisPointer(mesh);
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JStaticMeshGeometry::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JStaticMeshGeometry::LoadData::StaticTypeInfo()))
			return nullptr;
  
		auto loadData = static_cast<JStaticMeshGeometry::LoadData*>(data);
		auto pathData = loadData->pathData;
		JUserPtr<JDirectory> directory = loadData->directory;

		auto metaData = std::make_unique<JStaticMeshGeometry::LoadMetaData>(directory);	//for load metadata
		if (LoadMetaData(pathData.engineMetaFileWPath, metaData.get()) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;

		JUserPtr<JStaticMeshGeometry> newMesh = nullptr;
		if (directory->HasFile(metaData->guid))
			newMesh = Core::GetUserPtr<JStaticMeshGeometry>(JStaticMeshGeometry::StaticTypeInfo().TypeGuid(), metaData->guid);

		if (newMesh == nullptr)
		{
			using Impl = JStaticMeshGeometry::JStaticMeshGeometryImpl;
			auto idenUser = sPrivate.GetCreateInstanceInterface().BeginCreate(Impl::CreateInitData(pathData.name, pathData.engineFileWPath, metaData.get()), &sPrivate);
			newMesh.ConnnectChild(idenUser);
		} 
		   
		return newMesh;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JStaticMeshGeometry::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JStaticMeshGeometry::StoreData*>(data);
		if (!storeData->HasCorrectType(JStaticMeshGeometry::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JUserPtr<JStaticMeshGeometry>newMesh;
		newMesh.ConnnectChild(storeData->obj);
		return newMesh->impl->WriteAssetData(newMesh->GetPath(), newMesh->GetMeshGroupData()) ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JStaticMeshGeometry::LoadMetaData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		std::wifstream stream;
		stream.open(path, std::ios::in | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto loadMetaData = static_cast<JStaticMeshGeometry::LoadMetaData*>(data);
		if (LoadCommonMetaData(stream, loadMetaData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JObjectFileIOHelper::LoadEnumData(stream, loadMetaData->meshType);
		stream.close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JStaticMeshGeometry::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JStaticMeshGeometry::StoreData*>(data); 
		JUserPtr<JStaticMeshGeometry>mesh;
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
		return JStaticMeshGeometry::JStaticMeshGeometryImpl::ReadAssetData(path);
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JStaticMeshGeometryPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	JResourceObjectPrivate::AssetDataIOInterface& JStaticMeshGeometryPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}