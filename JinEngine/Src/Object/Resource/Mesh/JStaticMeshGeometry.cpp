#include"JStaticMeshGeometry.h" 
#include"JStaticMeshGeometryPrivate.h"
#include"JMeshStruct.h"
#include"../JResourceManager.h"
#include"../Material/JMaterial.h"
#include"../../Directory/JDirectory.h" 
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/File/JFileConstant.h"
#include"../../../Core/File/JFileIOHelper.h"
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
		static std::unique_ptr<JMeshGroup> ReadAssetData(const std::wstring& path)
		{
			std::wifstream stream;
			stream.open(path, std::ios::in | std::ios::binary);
			if (!stream.is_open())
				return nullptr;
			 
			std::unique_ptr<JStaticMeshGroup> meshGroup = std::make_unique<JStaticMeshGroup>();

			uint meshCount = 0;
			uint totalVertexCount = 0;
			uint totalIndexCount = 0;
			JFileIOHelper::LoadAtomicData(stream, meshCount);
			JFileIOHelper::LoadAtomicData(stream, totalVertexCount);
			JFileIOHelper::LoadAtomicData(stream, totalIndexCount);

			for (uint i = 0; i < meshCount; ++i)
			{
				std::wstring name;
				size_t guid = 0;
				uint vertexCount = 0;
				uint indexCount = 0;
				J_MESHGEOMETRY_TYPE meshType;

				JFileIOHelper::LoadJString(stream, name);
				JFileIOHelper::LoadAtomicData(stream, guid);
				JFileIOHelper::LoadAtomicData(stream, vertexCount);
				JFileIOHelper::LoadAtomicData(stream, indexCount);
				JFileIOHelper::LoadEnumData(stream, meshType);

				std::vector<JStaticMeshVertex> vertices(vertexCount);
				std::vector<uint> indices;

				for (uint i = 0; i < vertexCount; ++i)
				{
					JFileIOHelper::LoadXMFloat3(stream, vertices[i].position);
					JFileIOHelper::LoadXMFloat3(stream, vertices[i].normal);
					JFileIOHelper::LoadXMFloat2(stream, vertices[i].texC);
					JFileIOHelper::LoadXMFloat3(stream, vertices[i].tangentU);
				}

				JFileIOHelper::LoadAtomicDataVec(stream, indices);

				DirectX::BoundingBox boundingBox;
				DirectX::BoundingSphere boundingSphere;
				bool hasUV;
				bool hasNormal;

				JFileIOHelper::LoadXMFloat3(stream, boundingBox.Center);
				JFileIOHelper::LoadXMFloat3(stream, boundingBox.Extents);
				JFileIOHelper::LoadXMFloat3(stream, boundingSphere.Center);
				JFileIOHelper::LoadAtomicData(stream, boundingSphere.Radius);
				JFileIOHelper::LoadAtomicData(stream, hasUV);
				JFileIOHelper::LoadAtomicData(stream, hasNormal);

				meshGroup->AddMeshData(JStaticMeshData{ name , guid, std::move(indices),hasUV, hasNormal, std::move(vertices) });
			} 

			for (uint i = 0; i < meshCount; ++i)
				meshGroup->GetMeshData(i)->SetMaterial(JFileIOHelper::LoadHasObjectIden<JMaterial>(stream));
			stream.close();
			return std::move(meshGroup);
		}
		static bool WriteAssetData(const std::wstring& path, JMeshGroup* meshGroup)
		{
			std::wofstream stream;
			stream.open(path, std::ios::out | std::ios::binary);
			if (!stream.is_open())
				return false;

			if(meshGroup == nullptr)
				return false;

			JStaticMeshGroup* staticMeshs = static_cast<JStaticMeshGroup*>(meshGroup);
			const uint meshCount = staticMeshs->GetMeshDataCount();
			JFileIOHelper::StoreAtomicData(stream, L"MeshCount:", meshCount);
			JFileIOHelper::StoreAtomicData(stream, L"TotalVertexCount:", staticMeshs->GetTotalVertexCount());
			JFileIOHelper::StoreAtomicData(stream, L"TotalIndexCount:", staticMeshs->GetTotalIndexCount());

			for (uint i = 0; i < meshCount; ++i)
			{
				JStaticMeshData* staticData = static_cast<JStaticMeshData*>(staticMeshs->GetMeshData(i));

				JFileIOHelper::StoreJString(stream, L"Name:", staticData->GetName());
				JFileIOHelper::StoreAtomicData(stream, Core::JFileConstant::StreamHasObjGuidSymbol(), staticData->GetGuid());

				const uint vertexCount = staticData->GetVertexCount();
				const uint indexCount = staticData->GetIndexCount();

				JFileIOHelper::StoreAtomicData(stream, L"VertexCount:", vertexCount);
				JFileIOHelper::StoreAtomicData(stream, L"IndexCount:", indexCount);
				JFileIOHelper::StoreEnumData(stream, L"MeshType:", J_MESHGEOMETRY_TYPE::STATIC);

				for (uint i = 0; i < vertexCount; ++i)
				{
					JStaticMeshVertex vertices = staticData->GetVertex(i);
					JFileIOHelper::StoreXMFloat3(stream, L"P:", vertices.position);
					JFileIOHelper::StoreXMFloat3(stream, L"N:", vertices.normal);
					JFileIOHelper::StoreXMFloat2(stream, L"U:", vertices.texC);
					JFileIOHelper::StoreXMFloat3(stream, L"T:", vertices.tangentU);
				}
				JFileIOHelper::StoreAtomicDataVec(stream, L"Index:", staticData->GetIndexVector(), 6);

				const DirectX::BoundingBox boundingBox = staticData->GetBBox();
				const DirectX::BoundingSphere boundingSphere = staticData->GetBSphere();

				JFileIOHelper::StoreXMFloat3(stream, L"BBoxCenter:", boundingBox.Center);
				JFileIOHelper::StoreXMFloat3(stream, L"BBoxExtents:", boundingBox.Extents);
				JFileIOHelper::StoreXMFloat3(stream, L"SphereCenter:", boundingSphere.Center);
				JFileIOHelper::StoreAtomicData(stream, L"SphereRadius:", boundingSphere.Radius);
				JFileIOHelper::StoreAtomicData(stream, L"HasUV:", staticData->HasUV());
				JFileIOHelper::StoreAtomicData(stream, L"HasNormal:", staticData->HasNormal());
			}
			 
			for (uint i = 0; i < meshCount; ++i)
				JFileIOHelper::StoreHasObjectIden(stream, meshGroup->GetMeshData(i)->GetMaterial().Get());
			
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

	JStaticMeshGeometry::InitData::InitData(const uint8 formatIndex, const JUserPtr<JDirectory>& directory, std::unique_ptr<JMeshGroup>&& meshGroup)
		: JMeshGeometry::InitData(JStaticMeshGeometry::StaticTypeInfo(), formatIndex, directory, std::move(meshGroup))
	{}
	JStaticMeshGeometry::InitData::InitData(const size_t guid,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		std::unique_ptr<JMeshGroup>&& meshGroup)
		: JMeshGeometry::InitData(JStaticMeshGeometry::StaticTypeInfo(), guid, formatIndex, directory, std::move(meshGroup))
	{ }
	JStaticMeshGeometry::InitData::InitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		std::unique_ptr<JMeshGroup>&& meshGroup)
		: JMeshGeometry::InitData(JStaticMeshGeometry::StaticTypeInfo(), name, guid, flag, formatIndex, directory, std::move(meshGroup))
	{ }

	JStaticMeshGeometry::LoadMetaData::LoadMetaData(const JUserPtr<JDirectory>& directory)
		: JMeshGeometry::LoadMetaData(JStaticMeshGeometry::StaticTypeInfo(), directory)
	{}

	Core::JIdentifierPrivate& JStaticMeshGeometry::GetPrivateInterface()const noexcept
	{
		return sPrivate;
	}
	J_MESHGEOMETRY_TYPE JStaticMeshGeometry::GetMeshGeometryType()const noexcept
	{
		return J_MESHGEOMETRY_TYPE::STATIC;
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

		JFileIOHelper::LoadEnumData(stream, loadMetaData->meshType);
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

		JFileIOHelper::StoreEnumData(stream, L"MeshType:", mesh->GetMeshGeometryType());
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	std::unique_ptr<JMeshGroup> AssetDataIOInterface::ReadMeshGroupData(const std::wstring& path)
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