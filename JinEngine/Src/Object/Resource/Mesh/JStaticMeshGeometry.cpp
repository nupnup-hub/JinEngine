#include"JStaticMeshGeometry.h"
#include"../JResourceObjectFactory.h" 
#include"../Material/JMaterial.h"
#include"../Material/JDefaultMaterialSetting.h"
#include"../../Directory/JDirectory.h"
#include"../../Directory/JDirectoryFactory.h" 
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/File/JFileConstant.h"
#include"../../../Core/File/JFileIOHelper.h"
#include<fstream>

namespace JinEngine
{
	using namespace DirectX;

	J_MESHGEOMETRY_TYPE JStaticMeshGeometry::GetMeshGeometryType()const noexcept
	{
		return J_MESHGEOMETRY_TYPE::STATIC;
	}
	bool JStaticMeshGeometry::WriteMeshData(JMeshGroup& meshGroup)
	{
		if (meshGroup.GetMeshGroupType() != J_MESHGEOMETRY_TYPE::STATIC)
			return false;

		std::wofstream stream;
		stream.open(GetPath(), std::ios::out | std::ios::binary);
		if (stream.is_open())
		{
			JStaticMeshGroup* staticMeshs = static_cast<JStaticMeshGroup*>(&meshGroup);
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
			 
			const uint submeshCount = GetTotalSubmeshCount();
			JFileIOHelper::StoreAtomicData(stream, L"SubmeshCount:", submeshCount);
			for (uint i = 0; i < submeshCount; ++i)
				JFileIOHelper::StoreHasObjectIden(stream, GetSubmeshMaterial(i).Get());

			stream.close();
			return true;
		}
		else
			return false;
	}
	bool JStaticMeshGeometry::ReadMeshData()
	{
		std::wifstream stream;
		stream.open(GetPath(), std::ios::in | std::ios::binary);
		if (stream.is_open())
		{
			JStaticMeshGroup meshGroup;

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

				meshGroup.AddMeshData(JStaticMeshData{ name , guid, std::move(indices),hasUV, hasNormal, std::move(vertices) });
			}
			uint submeshCount;
			JFileIOHelper::LoadAtomicData(stream, submeshCount); 

			for (uint i = 0; i < submeshCount; ++i)
			{
				Core::JIdentifier* mat = JFileIOHelper::LoadHasObjectIden(stream);
				if (mat != nullptr && mat->GetTypeInfo().IsA(JMaterial::StaticTypeInfo()))
					meshGroup.GetMeshData(i)->SetMaterial(Core::GetUserPtr<JMaterial>(mat));
			}
			stream.close();
			StuffSubMesh(meshGroup);
			return true;
		}
		else
		{
			MessageBox(0, L"Fail Load Mesh", 0, 0);
			return false;
		}
	}
	bool JStaticMeshGeometry::ImportMesh(JMeshGroup& meshGroup)
	{
		if (meshGroup.GetMeshGroupType() != J_MESHGEOMETRY_TYPE::STATIC)
			return false;

		JDirectory* dir = GetDirectory();
		JDirectory* matDir = JDFI::Create(L"Material", Core::MakeGuid(), OBJECT_FLAG_NONE, *dir);

		const uint meshCount = meshGroup.GetMeshDataCount();
		for (uint i = 0; i < meshCount; ++i)
		{
			const std::wstring materialName = L"m" + meshGroup.GetMeshData(i)->GetName();
			JMaterial* newMaterial = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>(materialName,
				Core::MakeGuid(),
				GetFlag(),
				matDir));
			JDefaultMaterialSetting::SetStandard(newMaterial); 
			meshGroup.GetMeshData(i)->SetMaterial(Core::GetUserPtr(newMaterial));
		}

		if (StuffSubMesh(meshGroup) && StoreObject(this) == Core::J_FILE_IO_RESULT::SUCCESS)
		{
			bool res = WriteMeshData(meshGroup);
			ClearGpuBuffer();
			//Resource 할당은 Activated상태에서 이루어진다
			//Import는 데이터 변환과 메타데이터 저장을 위함
			return res;
		}
		else
		{
			for (uint i = 0; i < meshCount; ++i)
				BegineForcedDestroy(meshGroup.GetMeshData(i)->GetMaterial().Get());

			DeleteRFile();
			BegineForcedDestroy(this);
			return false;
		}
	}
	void JStaticMeshGeometry::RegisterJFunc()
	{
		auto defaultC = [](Core::JOwnerPtr<JResourceInitData>initdata) ->JResourceObject*
		{ 
			if (initdata.IsValid() && initdata->GetResourceType() == J_RESOURCE_TYPE::MESH && initdata->IsValidCreateData())
			{
				JMeshInitData* mInitdata = static_cast<JMeshInitData*>(initdata.Get());
				if (mInitdata->meshGroup->GetMeshGroupType() != J_MESHGEOMETRY_TYPE::STATIC)
					return nullptr;

				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JStaticMeshGeometry>(*mInitdata);
				JStaticMeshGeometry* newStaticMesh = ownerPtr.Get();
				if (AddInstance(std::move(ownerPtr)))
				{
					if (newStaticMesh->ImportMesh(*mInitdata->meshGroup.Get()))
						return newStaticMesh;			 
				}
				//StoreObject(newSkinnedMesh);
			}
			return nullptr;
		};
		auto loadC = [](JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori, JDirectory* directory)->JResourceObject*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JStaticMeshGeometry>(InitData(ori->GetName(),
				Core::MakeGuid(),
				ori->GetFlag(),
				directory,
				GetFormatIndex<JMeshGeometry>(ori->GetFormat())));

			JStaticMeshGeometry* newMesh = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				newMesh->Copy(ori);
				return newMesh;
			}
			else
				return nullptr;
		};
		JRFI<JStaticMeshGeometry>::Register(defaultC, loadC, copyC);		
	}

	JStaticMeshGeometry::JStaticMeshGeometry(const JMeshInitData& initdata)
		:JMeshGeometry(initdata)
	{}
	JStaticMeshGeometry::~JStaticMeshGeometry()
	{  
	}
}