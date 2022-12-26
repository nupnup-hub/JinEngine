#include"JSkinnedMeshGeometry.h"
#include"../JResourceManager.h"
#include"../JResourceObjectFactory.h"
#include"../Skeleton/JSkeletonAsset.h"
#include"../Skeleton/JSkeleton.h"
#include"../Material/JMaterial.h"
#include"../../Directory/JDirectory.h" 
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/File/JFileConstant.h"  
#include"../../../Core/File/JFileIOHelper.h"
#include<fstream>

namespace JinEngine
{
	using namespace DirectX;
	const static std::wstring skeletonSymbol = L"--SkeletonData--";

	JSkeletonAsset* JSkinnedMeshGeometry::GetSkeletonAsset()const noexcept
	{
		return skeletonAsset;
	}
	J_MESHGEOMETRY_TYPE JSkinnedMeshGeometry::GetMeshGeometryType()const noexcept
	{
		return J_MESHGEOMETRY_TYPE::SKINNED;
	}
	void JSkinnedMeshGeometry::SetSkeletonAsset(JSkeletonAsset* skeletonAsset)noexcept
	{
		if (IsActivated())
			CallOffResourceReference(JSkinnedMeshGeometry::skeletonAsset);
		JSkinnedMeshGeometry::skeletonAsset = skeletonAsset;
		if (IsActivated())
			CallOnResourceReference(JSkinnedMeshGeometry::skeletonAsset);
	}
	void JSkinnedMeshGeometry::DoActivate()noexcept
	{
		JMeshGeometry::DoActivate();
		CallOnResourceReference(JSkinnedMeshGeometry::skeletonAsset);
	}
	void JSkinnedMeshGeometry::DoDeActivate()noexcept
	{
		JMeshGeometry::DoDeActivate();
		CallOffResourceReference(JSkinnedMeshGeometry::skeletonAsset);
	}
	bool JSkinnedMeshGeometry::WriteMeshData(JMeshGroup& meshGroup)
	{
		if (meshGroup.GetMeshGroupType() != J_MESHGEOMETRY_TYPE::SKINNED)
			return false;

		std::wofstream stream;
		stream.open(GetPath(), std::ios::out | std::ios::binary);
		if (stream.is_open())
		{
			JSkinnedMeshGroup* skinnedMeshs = static_cast<JSkinnedMeshGroup*>(&meshGroup);
			const uint meshCount = skinnedMeshs->GetMeshDataCount();
			JFileIOHelper::StoreAtomicData(stream, L"MeshCount:", meshCount);
			JFileIOHelper::StoreAtomicData(stream, L"TotalVertexCount:", skinnedMeshs->GetTotalVertexCount());
			JFileIOHelper::StoreAtomicData(stream, L"TotalIndexCount:", skinnedMeshs->GetTotalIndexCount());

			for (uint i = 0; i < meshCount; ++i)
			{
				JSkinnedMeshData* skinnedData = static_cast<JSkinnedMeshData*>(skinnedMeshs->GetMeshData(i));

				JFileIOHelper::StoreJString(stream, L"Name:", skinnedData->GetName());
				JFileIOHelper::StoreAtomicData(stream, Core::JFileConstant::StreamHasObjGuidSymbol(), skinnedData->GetGuid());

				const uint vertexCount = skinnedData->GetVertexCount();
				const uint indexCount = skinnedData->GetIndexCount();

				JFileIOHelper::StoreAtomicData(stream, L"VertexCount:", vertexCount);
				JFileIOHelper::StoreAtomicData(stream, L"IndexCount:", indexCount);
				JFileIOHelper::StoreEnumData(stream, L"MeshType:", J_MESHGEOMETRY_TYPE::SKINNED);

				for (uint i = 0; i < vertexCount; ++i)
				{
					JSkinnedMeshVertex vertices = skinnedData->GetVertex(i);
					JFileIOHelper::StoreXMFloat3(stream, L"P:", vertices.position);
					JFileIOHelper::StoreXMFloat3(stream, L"N:", vertices.normal);
					JFileIOHelper::StoreXMFloat2(stream, L"U:", vertices.texC);
					JFileIOHelper::StoreXMFloat3(stream, L"T:", vertices.tangentU);
					JFileIOHelper::StoreXMFloat3(stream, L"W:", vertices.jointWeight);
					JFileIOHelper::StoreXMFloat4(stream, L"I", XMFLOAT4(vertices.jointIndex[0],
						vertices.jointIndex[1], vertices.jointIndex[2], vertices.jointIndex[3]));
				}
				if (skinnedData->Is16bit())
					JFileIOHelper::StoreAtomicDataVec(stream, L"Index:", skinnedData->GetU16Vector(), 6);
				else
					JFileIOHelper::StoreAtomicDataVec(stream, L"Index:", skinnedData->GetU32Vector(), 6);

				const DirectX::BoundingBox boundingBox = skinnedData->GetBBox();
				const DirectX::BoundingSphere boundingSphere = skinnedData->GetBSphere();

				JFileIOHelper::StoreXMFloat3(stream, L"BBoxCenter:", boundingBox.Center);
				JFileIOHelper::StoreXMFloat3(stream, L"BBoxExtents:", boundingBox.Extents);
				JFileIOHelper::StoreXMFloat3(stream, L"SphereCenter:", boundingSphere.Center);
				JFileIOHelper::StoreAtomicData(stream, L"SphereRadius:", boundingSphere.Radius);
				JFileIOHelper::StoreAtomicData(stream, L"HasUV:", skinnedData->HasUV());
				JFileIOHelper::StoreAtomicData(stream, L"HasNormal:", skinnedData->HasNormal());
			}

			const uint submeshCount = GetTotalSubmeshCount();
			JFileIOHelper::StoreAtomicData(stream, L"SubmeshCount:", submeshCount);
			for (uint i = 0; i < submeshCount; ++i)
				JFileIOHelper::StoreHasObjectIden(stream, GetSubmeshMaterial(i));

			const Core::JUserPtr<JSkeletonAsset> sUser = skinnedMeshs->GetSkeletonAsset();
			JSkeleton* skeleton = sUser->GetSkeleton();
			const uint jointCount = skeleton->GetJointCount();

			JFileIOHelper::StoreHasObjectIden(stream, skinnedMeshs->GetSkeletonAsset().Get());
			stream << skeletonSymbol << '\n';
			JFileIOHelper::StoreAtomicData(stream, L"JointCount:", jointCount);

			for (uint i = 0; i < jointCount; ++i)
			{
				Joint joint = skeleton->GetJoint(i);
				JFileIOHelper::StoreJString(stream, L"Name:", joint.name);
				JFileIOHelper::StoreAtomicData(stream, L"ParentIndex:", joint.parentIndex);
				JFileIOHelper::StoreAtomicData(stream, L"Length:", joint.length);
				JFileIOHelper::StoreXMFloat4x4(stream, L"JointBindPose:", joint.inbindPose);
			}
			stream.close();
			return true;
		}
		else
			return false;
	}
	bool JSkinnedMeshGeometry::ReadMeshData()
	{
		std::wifstream stream;
		stream.open(GetPath(), std::ios::in | std::ios::binary);
		if (stream.is_open())
		{
			JSkinnedMeshGroup meshGroup;

			uint meshCount = 0;
			uint totalVertexCount = 0;
			uint totalIndexCount = 0;
			JFileIOHelper::LoadAtomicData(stream, meshCount);
			JFileIOHelper::LoadAtomicData(stream, totalVertexCount);
			JFileIOHelper::LoadAtomicData(stream, totalIndexCount);

			for (uint i = 0; i < meshCount; ++i)
			{
				std::wstring name;
				size_t guid;
				uint vertexCount;
				uint indexCount;
				J_MESHGEOMETRY_TYPE meshType;

				JFileIOHelper::LoadJString(stream, name);
				JFileIOHelper::LoadAtomicData(stream, guid);
				JFileIOHelper::LoadAtomicData(stream, vertexCount);
				JFileIOHelper::LoadAtomicData(stream, indexCount);
				JFileIOHelper::LoadEnumData(stream, meshType);

				std::vector<JSkinnedMeshVertex> vertices(vertexCount);
				std::vector<uint16> u16Indices;
				std::vector<uint32> u32Indices;
				XMFLOAT4 jointIndex;

				for (uint i = 0; i < vertexCount; ++i)
				{
					JFileIOHelper::LoadXMFloat3(stream, vertices[i].position);
					JFileIOHelper::LoadXMFloat3(stream, vertices[i].normal);
					JFileIOHelper::LoadXMFloat2(stream, vertices[i].texC);
					JFileIOHelper::LoadXMFloat3(stream, vertices[i].tangentU);
					JFileIOHelper::LoadXMFloat3(stream, vertices[i].jointWeight);
					JFileIOHelper::LoadXMFloat4(stream, jointIndex);

					vertices[i].jointIndex[0] = jointIndex.x;
					vertices[i].jointIndex[1] = jointIndex.y;
					vertices[i].jointIndex[2] = jointIndex.z;
					vertices[i].jointIndex[3] = jointIndex.w;
				}

				if (indexCount < (1 << 16))
					JFileIOHelper::LoadAtomicDataVec(stream, u16Indices);
				else
					JFileIOHelper::LoadAtomicDataVec(stream, u32Indices);

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

				if (u32Indices.size() > 0)
				{
					meshGroup.AddMeshData(JSkinnedMeshData{ name , guid, std::move(u32Indices),
						hasUV, hasNormal, std::move(vertices) });
				}
				else
				{
					meshGroup.AddMeshData(JSkinnedMeshData{ name , guid, std::move(u16Indices),
						hasUV, hasNormal, std::move(vertices) });
				}
			}
			uint submeshCount;
			JFileIOHelper::LoadAtomicData(stream, submeshCount);

			for (uint i = 0; i < submeshCount; ++i)
			{
				Core::JIdentifier* mat = JFileIOHelper::LoadHasObjectIden(stream);
				if(mat != nullptr && mat->GetTypeInfo().IsA(JMaterial::StaticTypeInfo()))
					meshGroup.GetMeshData(i)->SetMaterial(Core::GetUserPtr<JMaterial>(mat));
			}

			Core::JIdentifier* skeletonAsset = JFileIOHelper::LoadHasObjectIden(stream);
			if (skeletonAsset != nullptr && skeletonAsset->GetTypeInfo().IsA(JSkeletonAsset::StaticTypeInfo()))
				SetSkeletonAsset(static_cast<JSkeletonAsset*>(skeletonAsset));

			stream.close();
			StuffSubMesh(meshGroup);
			return true;
		}
		else
			return false;
	}
	Core::JOwnerPtr<JSkeleton> JSkinnedMeshGeometry::ReadSkeletonData()
	{
		std::wifstream stream;
		stream.open(GetPath(), std::ios::in | std::ios::binary);
		if (stream.is_open())
		{
			if (!JFileIOHelper::SkipLine(stream, skeletonSymbol))
				return Core::JOwnerPtr<JSkeleton>{};

			uint jointCount;
			JFileIOHelper::LoadAtomicData(stream, jointCount);

			std::vector<Joint> joint(jointCount);
			for (uint i = 0; i < jointCount; ++i)
			{
				int parentIndex = 0;
				JFileIOHelper::LoadJString(stream, joint[i].name);
				JFileIOHelper::LoadAtomicData(stream, parentIndex);
				JFileIOHelper::LoadAtomicData(stream, joint[i].length);
				JFileIOHelper::LoadXMFloat4x4(stream, joint[i].inbindPose);
				joint[i].parentIndex = (uint8)parentIndex;
			}
			stream.close();
			return Core::JPtrUtil::MakeOwnerPtr<JSkeleton>(std::move(joint));
		}
		else
			return Core::JOwnerPtr<JSkeleton>{};
	}
	bool JSkinnedMeshGeometry::ImportMesh(JMeshGroup& meshGroup)
	{
		if (meshGroup.GetMeshGroupType() != J_MESHGEOMETRY_TYPE::SKINNED)
			return false;

		const JSkinnedMeshGroup* skinnedGroup = static_cast<const JSkinnedMeshGroup*>(&meshGroup);
		Core::JUserPtr<JSkeletonAsset> user = skinnedGroup->GetSkeletonAsset();
		if (user.IsValid())
			SetSkeletonAsset(skinnedGroup->GetSkeletonAsset().Get());

		const uint meshCount = meshGroup.GetMeshDataCount();
		for (uint i = 0; i < meshCount; ++i)
		{
			const std::wstring materialName = L"m" + meshGroup.GetMeshData(i)->GetName();
			JMaterial* newMaterial = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>(materialName, GetDirectory()));
			newMaterial->SetLight(true);
			newMaterial->SetShadow(true);
			meshGroup.GetMeshData(i)->SetMaterial(Core::GetUserPtr(newMaterial));
		}

		if (StuffSubMesh(meshGroup) && StoreObject(this) == Core::J_FILE_IO_RESULT::SUCCESS)
		{
			bool res = WriteMeshData(meshGroup);
			Clear();
			//Resource 할당은 Activated상태에서 이루어진다
			//Import는 데이터 변환과 메타데이터 저장을 위함
			return res;
		}
		else
		{
			for (uint i = 0; i < meshCount; ++i)
				BegineForcedDestroy(meshGroup.GetMeshData(i)->GetMaterial().Get());
			if (skeletonAsset != nullptr)
				BegineForcedDestroy(skeletonAsset);

			DeleteRFile();
			return false;
		}
	}
	void JSkinnedMeshGeometry::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
	{
		JMeshGeometry::OnEvent(iden, eventType, jRobj);
		if (iden == GetGuid())
			return;

		if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
		{
			if (skeletonAsset != nullptr && skeletonAsset->GetGuid() == jRobj->GetGuid())
				SetSkeletonAsset(nullptr);
		}
	}
	void JSkinnedMeshGeometry::RegisterJFunc()
	{
		auto defaultC = [](Core::JOwnerPtr<JResourceInitData>initdata) ->JResourceObject*
		{
			if (initdata.IsValid() && initdata->GetResourceType() == J_RESOURCE_TYPE::MESH && initdata->IsValidCreateData())
			{
				JMeshInitData* mInitdata = static_cast<JMeshInitData*>(initdata.Get());
				if (mInitdata->meshGroup->GetMeshGroupType() != J_MESHGEOMETRY_TYPE::SKINNED)
					return nullptr;

				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JSkinnedMeshGeometry>(*mInitdata);
				JSkinnedMeshGeometry* newSkinnedMesh = ownerPtr.Get();
				if (AddInstance(std::move(ownerPtr)))
				{
					if (newSkinnedMesh->ImportMesh(*mInitdata->meshGroup.Get()))
						return newSkinnedMesh;
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
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JSkinnedMeshGeometry>(InitData(ori->GetName(),
				Core::MakeGuid(),
				ori->GetFlag(),
				directory,
				GetFormatIndex<JMeshGeometry>(ori->GetFormat())));

			JSkinnedMeshGeometry* newMesh = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				newMesh->Copy(ori);
				return newMesh;
			}
			else
				return nullptr;
		};
		JRFI<JSkinnedMeshGeometry>::Register(defaultC, loadC, copyC);
	}
	JSkinnedMeshGeometry::JSkinnedMeshGeometry(const JMeshInitData& initdata)
		:JMeshGeometry(initdata)
	{
		AddEventListener(*JResourceManager::Instance().EvInterface(), GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
	}
	JSkinnedMeshGeometry::~JSkinnedMeshGeometry()
	{
		RemoveListener(*JResourceManager::Instance().EvInterface(), GetGuid());
	}
}