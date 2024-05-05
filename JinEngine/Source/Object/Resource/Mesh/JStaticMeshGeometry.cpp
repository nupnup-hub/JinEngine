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
			JFileIOTool tool;
			if (!tool.Begin(path, JFileIOTool::TYPE::INPUT_STREAM))
				return nullptr;
			 
			std::unique_ptr<Core::JStaticMeshGroup> meshGroup = std::make_unique<Core::JStaticMeshGroup>();

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
				size_t guid = 0;
				uint vertexCount = 0;
				uint indexCount = 0;
				Core::J_MESHGEOMETRY_TYPE meshType;

				tool.PushExistStack();
				JObjectFileIOHelper::LoadJString(tool, name, "Name:");
				JObjectFileIOHelper::LoadAtomicData(tool, guid, Core::JFileConstant::GetHasObjGuidSymbol());
				JObjectFileIOHelper::LoadAtomicData(tool, vertexCount, "VertexCount:");
				JObjectFileIOHelper::LoadAtomicData(tool, indexCount, "IndexCount:");
				JObjectFileIOHelper::LoadEnumData(tool, meshType, "MeshType:");

				std::vector<Core::JStaticMeshVertex> vertices(vertexCount);
				std::vector<uint> indices;

				tool.PushExistStack("VerticesData");
				for (uint i = 0; i < vertexCount; ++i)
				{
					tool.PushExistStack();
					JObjectFileIOHelper::LoadVector3(tool, vertices[i].position, "P:");
					JObjectFileIOHelper::LoadVector3(tool, vertices[i].normal, "N:");
					JObjectFileIOHelper::LoadVector2(tool, vertices[i].texC, "U:");
					JObjectFileIOHelper::LoadVector3(tool, vertices[i].tangentU, "T:"); 
					tool.PopStack();
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
				meshGroup->AddMeshData(std::make_unique< Core::JStaticMeshData>(name , guid, std::move(indices),hasUV, hasNormal, std::move(vertices)));
			} 
			tool.PopStack();
			tool.PushExistStack("MaterialData");
			for (uint i = 0; i < meshCount; ++i)
			{
				//tool.PushExistStack();
				meshGroup->GetMeshData(i)->SetMaterial(JObjectFileIOHelper::_LoadHasIden<JMaterial>(tool, std::to_string(i)));
				//tool.PopStack();
			}
			tool.Close();
			return std::move(meshGroup);
		}
		static bool WriteAssetData(const std::wstring& path, Core::JMeshGroup* meshGroup)
		{
			if(meshGroup == nullptr)
				return false;

			JFileIOTool tool;
			if (!tool.Begin(path, JFileIOTool::TYPE::OUTPUT_STREAM))
				return false;

			Core::JStaticMeshGroup* staticMeshs = static_cast<Core::JStaticMeshGroup*>(meshGroup);
			const uint meshCount = staticMeshs->GetMeshDataCount();
			JObjectFileIOHelper::StoreAtomicData(tool, meshCount, "MeshCount:");
			JObjectFileIOHelper::StoreAtomicData(tool, staticMeshs->GetTotalVertexCount(), "TotalVertexCount:");
			JObjectFileIOHelper::StoreAtomicData(tool, staticMeshs->GetTotalIndexCount(), "TotalIndexCount:");

			tool.PushArrayOwner("MeshData");
			for (uint i = 0; i < meshCount; ++i)
			{
				Core::JStaticMeshData* staticData = static_cast<Core::JStaticMeshData*>(staticMeshs->GetMeshData(i));

				tool.PushArrayMember();
				JObjectFileIOHelper::StoreJString(tool, staticData->GetName(), "Name:");
				JObjectFileIOHelper::StoreAtomicData(tool, staticData->GetGuid(), Core::JFileConstant::GetHasObjGuidSymbol());
 
				const uint vertexCount = staticData->GetVertexCount();
				const uint indexCount = staticData->GetIndexCount();

				JObjectFileIOHelper::StoreAtomicData(tool, vertexCount, "VertexCount:");
				JObjectFileIOHelper::StoreAtomicData(tool, indexCount, "IndexCount:");
				JObjectFileIOHelper::StoreEnumData(tool, Core::J_MESHGEOMETRY_TYPE::STATIC, "MeshType:");

				tool.PushArrayOwner("VerticesData");
				for (uint i = 0; i < vertexCount; ++i)
				{
					tool.PushArrayMember();
					Core::JStaticMeshVertex vertices = staticData->GetVertex(i);
					JObjectFileIOHelper::StoreVector3(tool, vertices.position, "P:");
					JObjectFileIOHelper::StoreVector3(tool, vertices.normal, "N:");
					JObjectFileIOHelper::StoreVector2(tool, vertices.texC, "U:");
					JObjectFileIOHelper::StoreVector3(tool, vertices.tangentU, "T:"); 
					tool.PopStack(); 
				}
				tool.PopStack();
				JObjectFileIOHelper::StoreAtomicDataVec(tool, staticData->GetIndexVector(), 8, "Index:");

				const DirectX::BoundingBox boundingBox = staticData->GetBBox();
				const DirectX::BoundingSphere boundingSphere = staticData->GetBSphere();

				JObjectFileIOHelper::StoreXMFloat3(tool, boundingBox.Center, "BBoxCenter:");
				JObjectFileIOHelper::StoreXMFloat3(tool, boundingBox.Extents, "BBoxExtents:");
				JObjectFileIOHelper::StoreXMFloat3(tool, boundingSphere.Center, "SphereCenter:");
				JObjectFileIOHelper::StoreAtomicData(tool, boundingSphere.Radius, "SphereRadius:");
				JObjectFileIOHelper::StoreAtomicData(tool, staticData->HasUV(), "HasUV:");
				JObjectFileIOHelper::StoreAtomicData(tool, staticData->HasNormal(), "HasNormal:");
				tool.PopStack();
			}
			tool.PopStack();

			tool.PushArrayOwner("MaterialData");
			for (uint i = 0; i < meshCount; ++i)
			{
				//tool.PushArrayMember();
				JObjectFileIOHelper::_StoreHasIden(tool, Core::ConnectChildUserPtr<JMaterial>(meshGroup->GetMeshData(i)->GetMaterial()).Get(), std::to_string(i));
				//tool.PopStack();
			}
			tool.PopStack(); 
			tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
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
			IMPL_REALLOC_BIND(JStaticMeshGeometry::JStaticMeshGeometryImpl, thisPointer);
			SET_GUI_FLAG(Core::J_GUI_OPTION_FLAG::J_GUI_OPTION_DISPLAY_PARENT_TO_CHILD);
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
		if (LoadMetaData(pathData.metaFilePath, metaData.get()) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;

		JUserPtr<JStaticMeshGeometry> newMesh = nullptr;
		if (directory->HasFile(metaData->guid))
			newMesh = Core::GetUserPtr<JStaticMeshGeometry>(JStaticMeshGeometry::StaticTypeInfo().TypeGuid(), metaData->guid);

		if (newMesh == nullptr)
		{
			using Impl = JStaticMeshGeometry::JStaticMeshGeometryImpl;
			auto idenUser = sPrivate.GetCreateInstanceInterface().BeginCreate(Impl::CreateInitData(pathData.name, pathData.path, metaData.get()), &sPrivate);
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

		JFileIOTool tool;
		if (!tool.Begin(path, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto loadMetaData = static_cast<JStaticMeshGeometry::LoadMetaData*>(data);
		if (LoadCommonMetaData(tool, loadMetaData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JObjectFileIOHelper::LoadEnumData(tool, loadMetaData->meshType, "MeshType");
		tool.Close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JStaticMeshGeometry::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JStaticMeshGeometry::StoreData*>(data); 
		JUserPtr<JStaticMeshGeometry>mesh;
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