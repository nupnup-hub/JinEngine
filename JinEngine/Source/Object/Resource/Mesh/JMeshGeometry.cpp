/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JMeshGeometry.h"
#include"JMeshGeometryPrivate.h"
#include"JStaticMeshGeometry.h"
#include"JSkinnedMeshGeometry.h"
#include"../JResourceObjectHint.h"
#include"../JResourceObjectImporter.h"
#include"../JClearableInterface.h"
#include"../JResourceObjectUserInterface.h"
#include"../JResourceManager.h"
#include"../Skeleton/JSkeletonAsset.h"
#include"../Skeleton/JSkeleton.h" 
#include"../Material/JMaterial.h" 
#include"../Material/JDefaultMaterialSetting.h"
#include"../Texture/JTexture.h"
#include"../../Directory/JDirectory.h" 
#include"../../JObjectFileIOHelper.h"
#include"../../../Application/Project/JApplicationProject.h" 
#include"../../../Core/Guid/JGuidCreator.h"
#include"../../../Core/Identity/JIdenCreator.h"
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Exception/JExceptionMacro.h" 
#include"../../../Core/File/JFileConstant.h" 
#include"../../../Core/Geometry/JDirectXCollisionEx.h"
#include"../../../Core/Geometry/Mesh/JMeshStruct.h"
#include"../../../Core/Geometry/Mesh/Loader/FbxLoader/JFbxFileLoader.h"
#include"../../../Core/Geometry/Mesh/Loader/ObjLoader/JObjFileLoader.h"
#include"../../../Core/Animation/Joint.h" 
#include"../../../Core/Utility/JCommonUtility.h"  
#include"../../../Core/Math/JMathHelper.h"
#include"../../../Core/Log/JLogMacro.h"

#include"../../../Graphic/JGraphic.h"
#include"../../../Graphic/JGraphicPrivate.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include<fstream>
#include<wrl/client.h>

namespace JinEngine
{
	using namespace DirectX; 

	namespace Private
	{
		static constexpr uint useGraphicResourceCount = 2;
	}

	struct MaterialIntermediateData
	{
	public:
		template<typename T>
		static constexpr bool CanConvert()
		{
			return std::is_base_of_v<Core::JFbxMaterial, T> || std::is_base_of_v<Core::JObjFileMaterial, T>;
		}
	public:
		std::wstring name;
	public: 
		Core::JMaterialParameter mParam;
	public: 
		std::wstring albedoMapName;
		std::wstring normalMapName; 
		std::wstring heightMapName;
		std::wstring roughnessMapName; 
		std::wstring ambientMapName;
		std::wstring specularMapName;
	public:
		MaterialIntermediateData(const Core::JFbxMaterial& fbxmat)
			:name(JCUtil::U8StrToWstr(fbxmat.name)),
			mParam(fbxmat.mParam),
			albedoMapName(fbxmat.albedoMapName.empty() ? L"" : JCUtil::U8StrToWstr(fbxmat.albedoMapName)),
			normalMapName(fbxmat.normalMapName.empty() ? L"" : JCUtil::U8StrToWstr(fbxmat.normalMapName)),
			heightMapName(fbxmat.heightMapName.empty() ? L"" : JCUtil::U8StrToWstr(fbxmat.heightMapName)),
			roughnessMapName(fbxmat.roughnessMapName.empty() ? L"" : JCUtil::U8StrToWstr(fbxmat.roughnessMapName)),
			ambientMapName(fbxmat.ambientMapName.empty() ? L"" : JCUtil::U8StrToWstr(fbxmat.ambientMapName)),
			specularMapName(fbxmat.specularName.empty() ? L"" : JCUtil::U8StrToWstr(fbxmat.specularName))
		{}
		MaterialIntermediateData(const Core::JObjFileMaterial& objmat)
			:name(objmat.name),
			mParam(objmat.mParam), 
			albedoMapName(objmat.albedoMapName),
			normalMapName(objmat.normalMapName),
			heightMapName(objmat.heightMapName), 
			ambientMapName(objmat.ambientMapName),
			specularMapName(objmat.specularColorMapName)
		{} 
	};
	class SubmeshGeometry
	{
	private:
		std::wstring name;
		size_t guid;
		JUserPtr<JMaterial> material;
	private:
		uint vertexCount = 0;
		uint vertexStart = 0;
		uint indexCount = 0;
		uint indexStart = 0;
		DirectX::BoundingBox boundingBox;
		DirectX::BoundingSphere boundingSphere;
		bool hasNormal = false;
		bool hasUV = false;
		Core::J_MESHGEOMETRY_TYPE type;
	public:
		SubmeshGeometry(const std::wstring name, const size_t guid)
			:name(name), guid(guid)
		{}
		~SubmeshGeometry() {}
	public:
		std::wstring GetName()const noexcept
		{
			return name;
		}
		JUserPtr<JMaterial> GetMaterial()const noexcept
		{
			return material;
		}
		uint GetVertexCount()const noexcept
		{
			return vertexCount;
		}
		uint GetVertexStart()const noexcept
		{
			return vertexStart;
		}
		uint GetIndexStart()const noexcept
		{
			return indexStart;
		}
		uint GetIndexCount()const noexcept
		{
			return indexCount;
		}
		JVector3<float> GetBoundingBoxCenter()const noexcept
		{
			return boundingBox.Center;
		}
		DirectX::XMVECTOR GetBoundingBoxCenterV()const noexcept
		{
			return XMLoadFloat3(&boundingBox.Center);
		}
		JVector3<float> GetBoundingBoxExtent()const noexcept
		{
			return boundingBox.Extents;
		}
		DirectX::XMVECTOR GetBoundingBoxExtentV()const noexcept
		{
			return XMLoadFloat3(&boundingBox.Extents);
		}
		JVector3<float> GetBoundingSphereCenter()const noexcept
		{
			return boundingSphere.Center;
		}
		float GetBoundingSphereRadius()const noexcept
		{
			return boundingSphere.Radius;
		}
	public:
		void SetMesh(const Core::JMeshData& meshData, const uint vertexSt, const uint indexSt)
		{
			JUserPtr<JMaterial> meshMaterial = Core::ConvertChildUserPtr<JMaterial>(meshData.GetMaterial());
			if (meshMaterial.IsValid())
				material = meshMaterial;
			else
				material = _JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_STANDARD);

			vertexStart = vertexSt;
			vertexCount = meshData.GetVertexCount();
			indexStart = indexSt;
			indexCount = meshData.GetIndexCount();

			SubmeshGeometry::boundingBox = meshData.GetBBox();
			SubmeshGeometry::boundingSphere = meshData.GetBSphere();

			hasNormal = meshData.HasNormal();
			hasUV = meshData.HasUV();
			type = meshData.GetMeshType();
		}
		void SetMaterial(JUserPtr<JMaterial> newMaterial)noexcept
		{
			material = newMaterial;
		}
	public:
		bool HasUV()const noexcept
		{
			return hasUV;
		}
		bool HasNormal()const noexcept
		{
			return hasNormal;
		}
	};
 
	class JMeshGeometry::JMeshGeometryImpl : public Core::JTypeImplBase,
		public JResourceObjectUserInterface,
		public JClearableInterface,
		public Graphic::JGraphicWideSingleResourceHolder<Private::useGraphicResourceCount>
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JMeshGeometryImpl)
	public:
		JWeakPtr<JMeshGeometry> thisPointer = nullptr;
	public:
		std::unique_ptr<Core::JMeshGroup> meshGroupData = nullptr;	//for store group data 리소스 초기화후 nullptr 이 된다
	public:
		// System memory copies.  Use Blobs because the vertex/index format can be generic.
		// It is up to the client to cast appropriately.
		//Microsoft::WRL::ComPtr<ID3DBlob> vertexBufferCPU = nullptr;
		//Microsoft::WRL::ComPtr<ID3DBlob> indexBufferCPU = nullptr;
	public:
		DirectX::BoundingBox boundingBox;
		DirectX::BoundingSphere boundingSphere;
	public:
		// Data about the buffers.
		uint vertexCount = 0;
		uint indexCount = 0;
		uint vertexByteSize = 0;
		uint vertexBufferByteSize = 0;
		uint indexByteSize = 0; 
		uint indexBufferByteSize = 0;
		std::vector<SubmeshGeometry> submeshes;
	public:
		JMeshGeometryImpl(const InitData& initData, JMeshGeometry* thisMeshRaw){}
		~JMeshGeometryImpl(){}
	public:
		REGISTER_METHOD(GetTotalVertexCount)
		REGISTER_METHOD_READONLY_GUI_WIDGET(VertexCount, GetTotalVertexCount, GUI_READONLY_TEXT())
		uint GetTotalVertexCount()const noexcept
		{ 
			return vertexCount;
		}
		REGISTER_METHOD(GetTotalIndexCount)
		REGISTER_METHOD_READONLY_GUI_WIDGET(IndexCount, GetTotalIndexCount, GUI_READONLY_TEXT())
		uint GetTotalIndexCount()const noexcept
		{
			return indexCount;
		}
		REGISTER_METHOD(GetTotalSubmeshCount)
		REGISTER_METHOD_READONLY_GUI_WIDGET(SubMeshCount, GetTotalSubmeshCount, GUI_READONLY_TEXT())
		uint GetTotalSubmeshCount()const noexcept
		{
			return (uint)submeshes.size();
		}
		uint GetSubmeshVertexCount(const uint index)const noexcept
		{
			return submeshes.size() > index ? submeshes[index].GetVertexCount() : 0;
		}
		uint GetSubmeshIndexCount(const uint index)const noexcept
		{
			return submeshes.size() > index ? submeshes[index].GetIndexCount() : 0;
		}
		uint GetSubmeshBaseVertexLocation(const uint index)const noexcept
		{
			return submeshes.size() > index ? submeshes[index].GetVertexStart() : 0;
		}
		uint GetSubmeshStartIndexLocation(const uint index)const noexcept
		{
			return submeshes.size() > index ? submeshes[index].GetIndexStart() : 0;
		} 
		std::wstring GetSubMeshName(const uint index)const noexcept
		{
			return submeshes.size() > index ? submeshes[index].GetName() : L"InValidAccess";
		} 
		REGISTER_METHOD(GetSubMeshMaterialVec)
		REGISTER_METHOD_READONLY_GUI_WIDGET(SubMeshMaterial, GetSubMeshMaterialVec, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE,  true, false))
		std::vector<JUserPtr<JMaterial>> GetSubMeshMaterialVec()const noexcept
		{
			const uint count = (uint)submeshes.size();
			std::vector<JUserPtr<JMaterial>> matVec(count);
			for (uint i = 0; i < count; ++i)
				matVec[i] = submeshes[i].GetMaterial();
			return matVec;
		}
		JUserPtr<JMaterial> GetSubmeshMaterial(const uint index)const noexcept
		{
			return submeshes.size() > index ? submeshes[index].GetMaterial() : JUserPtr<JMaterial>{};
		}
		int GetResourceDataIndex(const Graphic::J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_TASK_TYPE taskType)const noexcept
		{
			return (rType == Graphic::J_GRAPHIC_RESOURCE_TYPE::VERTEX || rType == Graphic::J_GRAPHIC_RESOURCE_TYPE::INDEX) ? 0 : invalidIndex;
		} 
	public:
		void UpdateMeshBound()noexcept
		{
			const uint submeshCount = (uint)submeshes.size();
			 
			XMVECTOR minXmV = JVector3<float>::PositiveInfV().ToXmV();
			XMVECTOR maxXmV = JVector3<float>::NegativeInfV().ToXmV();

			for (uint i = 0; i < submeshCount; ++i)
			{
				const XMVECTOR minV = submeshes[i].GetBoundingBoxCenterV() - submeshes[i].GetBoundingBoxExtentV();
				const XMVECTOR maxV = submeshes[i].GetBoundingBoxCenterV() + submeshes[i].GetBoundingBoxExtentV();
				minXmV = XMVectorMin(minXmV, minV);
				maxXmV = XMVectorMax(maxXmV, maxV);
			}
			boundingBox = Core::JDirectXCollisionEx::CreateBoundingBox(minXmV, maxXmV);
			boundingSphere = Core::JDirectXCollisionEx::CreateBoundingSphere(minXmV, maxXmV);
		}
	public:
		void OnResourceRef()
		{
			const uint submeshCount = GetTotalSubmeshCount();
			for (uint i = 0; i < submeshCount; ++i)
				CallOnResourceReference(submeshes[i].GetMaterial().Get());
		}
		void OffResourceRef()
		{
			const uint subMeshCount = (uint)submeshes.size();
			for (uint i = 0; i < subMeshCount; ++i)
				CallOffResourceReference(submeshes[i].GetMaterial().Get());
		}
	public:
		template<typename T>
		static void ConvertJEMaterial(Core::JMeshGroup& group,
			std::unordered_map<size_t, std::vector<T>> matData,
			const J_OBJECT_FLAG flag,
			const JUserPtr<JDirectory>& parentDir)
		{
			if constexpr (!MaterialIntermediateData::CanConvert<T>())
				return;

			if (parentDir == nullptr)
				return;

			JUserPtr<JDirectory> matDir = JICI::Create<JDirectory>(L"Material", Core::MakeGuid(), flag, parentDir);
			JUserPtr<JDirectory> textureDir = nullptr;
			
			JUserPtr<JMaterial> defaultMaterial = _JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_STANDARD);

			std::unordered_map<std::wstring, JUserPtr<JMaterial>> newMatmap;
			std::unordered_map<std::wstring, JUserPtr<JTexture>> newTextureMap;

			const uint count = group.GetMeshDataCount();
			for (uint i = 0; i < count; ++i)
			{
				auto meshData = group.GetMeshData(i);
				auto mapData = matData.find(meshData->GetGuid());
				if (mapData == matData.end() || mapData->second.size() == 0)
					continue;

				MaterialIntermediateData interMat(mapData->second[0]); 
				auto newMatData = newMatmap.find(interMat.name);
				if (newMatData != newMatmap.end())
				{
					meshData->SetMaterial(newMatData->second);
					continue;
				}
				 
				constexpr uint funcCount = 6;
				using SetTexturePtr = void(JMaterial::*)(JUserPtr<JTexture> texture);

				JUserPtr<JTexture> texture[funcCount];
				std::wstring nameRef[funcCount]
				{
					interMat.albedoMapName,
					interMat.normalMapName,
					interMat.heightMapName,
					interMat.roughnessMapName,
					interMat.ambientMapName,
					interMat.specularMapName
				};
				SetTexturePtr func[funcCount]
				{
					&JMaterial::SetAlbedoMap,
					&JMaterial::SetNormalMap,
					&JMaterial::SetHeightMap,
					&JMaterial::SetRoughnessMap,
					&JMaterial::SetAmbientOcclusionMap,
					&JMaterial::SetSpecularMap,
				};

				bool hasValidTexture = false;
				for (uint j = 0; j < funcCount; ++j)
				{
					if (textureDir == nullptr)
						textureDir = JICI::Create<JDirectory>(L"Texture", Core::MakeGuid(), flag, parentDir);

					const JResourceObjectImportDesc importDesc(Core::JFileImportPathData{ nameRef[j] }, textureDir);
					auto existData = newTextureMap.find(importDesc.importPathData.oriFileWPath);
					if (existData != newTextureMap.end())
						texture[j] = existData->second;
					else
					{
						auto res = JResourceObjectImporter::Instance().ImportResource(&importDesc);
						if (res.size() == 0)
							continue;
						 
						texture[j] = Core::ConnectChildUserPtr<JTexture>(res[0]); 
						newTextureMap.emplace(importDesc.importPathData.oriFileWPath, texture[j]);
					}
					hasValidTexture |= (texture[j] != nullptr);
				} 
				 
				if (!hasValidTexture && defaultMaterial->IsSame(interMat.mParam))
				{
					meshData->SetMaterial(defaultMaterial);
					continue;
				}
				
				JUserPtr<JMaterial> newMaterial = JICI::Create<JMaterial>(interMat.name,
					Core::MakeGuid(),
					flag,
					GetDefaultFormatIndex(),
					matDir);

				newMaterial->SetAlbedoColor(interMat.mParam.albedoColor);
				newMaterial->SetMetallic(interMat.mParam.metallic);
				newMaterial->SetRoughness(interMat.mParam.roughness);
				newMaterial->SetSpecularFactor(interMat.mParam.specularFactor);
				newMaterial->SetLight(true);
				newMaterial->SetShadow(true);

				for (uint j = 0; j < funcCount; ++j)
					(newMaterial.Get()->*func[j])(texture[j]);

				meshData->SetMaterial(newMaterial);
				newMatmap.emplace(interMat.name, newMaterial);
			}
		} 
		template<typename T>
		static auto SplitMeshGroup(_Inout_ T* group)
			->std::vector<std::unique_ptr<typename Core::TypeCondition<T, std::is_base_of_v<Core::JMeshGroup, T>>::Type>>
			//->std::conditional_t<std::is_base_of_v<Core::JMeshGroup, T>, std::vector<T>, void>
		{ 
			const uint meshDataCount = group->GetMeshDataCount();
			std::vector<std::unique_ptr<T>> splitMeshGroup(meshDataCount);

			T* oriGroup = static_cast<T*>(group);
			for (uint i = 0; i < meshDataCount; ++i)
			{
				splitMeshGroup[i] = std::make_unique<T>();
				splitMeshGroup[i]->AddMeshData(oriGroup->PopMeshData(0));
			}
			return std::move(splitMeshGroup);
		}
		void TryCreateMeshGroupMaterial(Core::JMeshGroup* meshGroup)
		{
			JUserPtr<JDirectory> parentDir = thisPointer->GetDirectory()->GetParent();
			JUserPtr<JDirectory> matDir = parentDir->GetChildDirctoryByName(L"Material");
			if (matDir == nullptr)
			{
				JUserPtr<JDirectory> modelDir = parentDir->GetChildDirctoryByName(L"Model");
				if (modelDir != nullptr)
				{
					//imported
					matDir = JICI::Create<JDirectory>(L"Material", Core::MakeGuid(), OBJECT_FLAG_NONE, parentDir);
				}
				else
				{
					//engine default
					matDir = thisPointer->GetDirectory();
					//matDir = JICI::Create<JDirectory>(L"Material", Core::MakeGuid(), OBJECT_FLAG_NONE, thisPointer->GetDirectory());
				} 
			}

			const uint submeshCount = (uint)meshGroup->GetMeshDataCount();
			for (uint i = 0; i < submeshCount; ++i)
			{
				if (meshGroup->GetMeshData(i)->GetMaterial().IsValid())
					continue;

				const std::wstring materialName = L"m" + meshGroup->GetMeshData(i)->GetName();
				JUserPtr<JMaterial> newMaterial = JICI::Create<JMaterial>(materialName,
					Core::MakeGuid(),
					thisPointer->GetFlag(),
					GetDefaultFormatIndex(),
					matDir);
				JDefaultMaterialSetting::SetStandard(newMaterial);
				meshGroup->GetMeshData(i)->SetMaterial(newMaterial);
			} 
		}
		bool ImportMesh(Core::JMeshGroup* meshGroup)
		{
			auto creationLam = [](JMeshGeometry::JMeshGeometryImpl* impl, void* data, uint elementCount, uint elementSize, Graphic::J_GRAPHIC_RESOURCE_TYPE type)
			{
				Graphic::JGraphicResourceCreationDesc desc;
				desc.width = elementCount;
				desc.uploadBufferDesc = std::make_unique<Graphic::JUploadBufferCreationDesc>(data, elementCount * elementSize);
				desc.uploadBufferDesc->useEngineDefine = false;
				desc.formatHint = std::make_unique<Graphic::JGraphicFormatHint>();
				desc.formatHint->elementSize = elementSize;
				impl->CreateResource(desc, type);
			};
			//SutffSubMesh는  하위 메시 클래스에서
			//리소스 초기화시 호출된다
			//stuff material
			DestroyAllTexture();
			const uint submeshCount = (uint)meshGroup->GetMeshDataCount();
 
			uint vertexCount = 0;
			uint indexCount = 0;
			if (submeshes.size() != submeshCount)
			{
				submeshes.clear();
				for (uint i = 0; i < submeshCount; ++i)
				{
					submeshes.emplace_back(meshGroup->GetMeshData(i)->GetName(),
						meshGroup->GetMeshData(i)->GetGuid());
				}
			}
			
			for (uint i = 0; i < submeshCount; ++i)
			{
				if (meshGroup->GetMeshData(i)->GetMeshType() == Core::J_MESHGEOMETRY_TYPE::STATIC)
					submeshes[i].SetMesh(*static_cast<Core::JStaticMeshData*>(meshGroup->GetMeshData(i)), vertexCount, indexCount);
				else
					submeshes[i].SetMesh(*static_cast<Core::JSkinnedMeshData*>(meshGroup->GetMeshData(i)), vertexCount, indexCount);

				submeshes[i].SetMaterial(Core::ConnectChildUserPtr<JMaterial>(meshGroup->GetMeshData(i)->GetMaterial()));
				vertexCount += submeshes[i].GetVertexCount();
				indexCount += submeshes[i].GetIndexCount();
			}
			JMeshGeometryImpl::vertexCount = vertexCount;
			JMeshGeometryImpl::indexCount = indexCount;
			  
			if (meshGroup->GetMeshGroupType() == Core::J_MESHGEOMETRY_TYPE::STATIC)
			{
				vertexByteSize = sizeof(Core::JStaticMeshVertex);
				vertexBufferByteSize = vertexCount * vertexByteSize;

				std::vector<Core::JStaticMeshVertex> vertex(vertexCount);
				uint vertexOffset = 0;
				for (uint i = 0; i < submeshCount; ++i)
				{
					Core::JStaticMeshData* meshdata = static_cast<Core::JStaticMeshData*>(meshGroup->GetMeshData(i));
					const uint subMeshVertexCount = meshdata->GetVertexCount();
					for (uint j = 0; j < subMeshVertexCount; ++j)
						vertex[vertexOffset + j] = meshdata->GetVertex(j);
					vertexOffset += subMeshVertexCount;
				}

				creationLam(this, vertex.data(), vertexCount, vertexByteSize, Graphic::J_GRAPHIC_RESOURCE_TYPE::VERTEX);
			}
			else
			{
				vertexByteSize = sizeof(Core::JSkinnedMeshVertex);
				vertexBufferByteSize = vertexCount * vertexByteSize;

				std::vector<Core::JSkinnedMeshVertex> vertex(vertexCount);
				uint vertexOffset = 0;
				for (uint i = 0; i < submeshCount; ++i)
				{
					Core::JSkinnedMeshData* meshdata = static_cast<Core::JSkinnedMeshData*>(meshGroup->GetMeshData(i));
					const uint subMeshVertexCount = meshdata->GetVertexCount();
					for (uint j = 0; j < subMeshVertexCount; ++j)
						vertex[vertexOffset + j] = meshdata->GetVertex(j);
					vertexOffset += subMeshVertexCount;
				} 				 
				creationLam(this, vertex.data(), vertexCount, vertexByteSize, Graphic::J_GRAPHIC_RESOURCE_TYPE::VERTEX);
			}

			if (indexCount >= 1 << 16)
			{
				indexByteSize = sizeof(uint32); 
				indexBufferByteSize = indexCount * indexByteSize;
				std::vector<uint32> index32(indexCount);

				uint indicesOffset = 0;
				for (uint i = 0; i < submeshCount; ++i)
				{
					const Core::JMeshData* meshdata = meshGroup->GetMeshData(i);
					const uint subMeshIndexCount = meshdata->GetIndexCount();
					for (uint j = 0; j < subMeshIndexCount; ++j)
						index32[indicesOffset + j] = meshdata->GetIndex(j);
					indicesOffset += subMeshIndexCount;
				}
				 
				creationLam(this, index32.data(), indexCount, indexByteSize, Graphic::J_GRAPHIC_RESOURCE_TYPE::INDEX);
			}
			else
			{
				indexByteSize = sizeof(uint16);
				indexBufferByteSize = indexCount * indexByteSize;
				std::vector<uint16> index16(indexCount);
				uint indicesOffset = 0;

				for (uint i = 0; i < submeshCount; ++i)
				{
					const Core::JMeshData* meshdata = meshGroup->GetMeshData(i);
					const uint subMeshIndexCount = meshdata->GetIndexCount();
					for (uint j = 0; j < subMeshIndexCount; ++j)
						index16[indicesOffset + j] = meshdata->GetIndex(j);
					indicesOffset += subMeshIndexCount;
				} 

				creationLam(this, index16.data(), indexCount, indexByteSize, Graphic::J_GRAPHIC_RESOURCE_TYPE::INDEX);
			}


			if (thisPointer->IsActivated())
			{
				for (uint i = 0; i < submeshCount; ++i)
					CallOnResourceReference(submeshes[i].GetMaterial().Get());
			}
			UpdateMeshBound();
			return true;
		}
		void StuffResource()
		{
			if (!thisPointer->IsValid())
			{
				if (thisPointer->GetFormatIndex() != GetInvalidFormatIndex())
				{
					auto& p = static_cast<JMeshGeometryPrivate&>(thisPointer->PrivateInterface());
					auto& pA = static_cast<JMeshGeometryPrivate::AssetDataIOInterface&>(p.GetAssetDataIOInterface());
					auto groupData = pA.ReadMeshGroupData(thisPointer->GetPath());
					if (ImportMesh(groupData.get()))
						thisPointer->SetValid(true); 
				}
			}
		}
		void ClearResource()
		{
			if (thisPointer->IsValid())
			{
				// vertexBufferCPU.Reset();
				//indexBufferCPU.Reset() 
				DestroyAllTexture();
				thisPointer->SetValid(false);
			}
		}
	public:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj, JResourceEventDesc* desc)
		{
			if (iden == thisPointer->GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE && jRobj->GetResourceType() == J_RESOURCE_TYPE::MATERIAL)
			{
				JUserPtr<JMaterial> defaultMat = _JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_STANDARD);
				const size_t tarGuid = jRobj->GetGuid();
				const uint subMeshCount = (uint)submeshes.size();
				for (uint i = 0; i < subMeshCount; ++i)
				{
					JMaterial* preMat = submeshes[i].GetMaterial().Get();
					if (preMat->GetGuid() == tarGuid)
					{
						if (defaultMat != nullptr)
						{
							submeshes[i].SetMaterial(defaultMat);
							if (thisPointer->IsActivated())
								CallOnResourceReference(defaultMat.Get());
						}
						else
							submeshes[i].SetMaterial(JUserPtr<JMaterial>{});
					}
				}
			}
		}
	public:
		void NotifyReAlloc()
		{
			RegisterInterfacePointer();
			ResetEventListenerPointer(*JResourceObject::EvInterface(), thisPointer->GetGuid());
		}
	public:
		void Initialize(InitData* initData)
		{
			TryCreateMeshGroupMaterial(initData->meshGroup.get());
			ImportMesh(initData->meshGroup.get());
			thisPointer->SetValid(true);
			meshGroupData = std::move(initData->meshGroup);
		}
		void RegisterThisPointer(JMeshGeometry* mesh)
		{
			thisPointer = Core::GetWeakPtr(mesh);
		}
		void RegisterInterfacePointer()
		{
			Graphic::JGraphicResourceInterface::SetInterfacePointer(this);
		}
		void DeRegisterInterfacePointer()
		{
			Graphic::JGraphicResourceInterface::SetInterfacePointer(nullptr);
		}
		void RegisterPostCreation()
		{
			AddEventListener(*JResourceObject::EvInterface(), thisPointer->GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		}
		void DeRegisterPreDestruction()
		{
			RemoveListener(*JResourceObject::EvInterface(), thisPointer->GetGuid());
		}
		static void RegisterTypeData()
		{
			auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex(GetStaticResourceType(), format); };
			static GetRTypeInfoCallable getTypeInfoCallable{ &JMeshGeometry::StaticTypeInfo };
			static GetAvailableFormatCallable getAvailableFormatCallable{ &JMeshGeometry::GetAvailableFormat };
			static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

			static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{J_RESOURCE_TYPE::MATERIAL, J_RESOURCE_TYPE::SKELETON}, true, false, false, true };
			static RTypeCommonFunc rTypeCFunc{ getTypeInfoCallable, getAvailableFormatCallable, getFormatIndexCallable };

			RegisterRTypeInfo(rTypeHint, rTypeCFunc, RTypePrivateFunc{});

			//JResourceObject*, const std::wstring, JDirectory*, const std::wstring>
			auto fbxClassifyC = [](const Core::JFileImportPathData importPathData) -> std::vector<J_RESOURCE_TYPE>
			{
				using FbxFileTypeInfo = Core::JFbxFileLoader::FbxFileTypeInfo;
				FbxFileTypeInfo info = JFbxFileLoader::Instance().GetFileTypeInfo(importPathData.oriFilePath);
				if (info.typeInfo == Core::J_FBX_RESULT_FAIL)
					return {};

				std::vector<J_RESOURCE_TYPE> resVec;
				if (HasSQValueEnum(info.typeInfo, Core::J_FBX_RESULT_HAS_MESH))
					resVec.push_back(J_RESOURCE_TYPE::MESH);
				if (HasSQValueEnum(info.typeInfo, Core::J_FBX_RESULT_HAS_ANIMATION))
					resVec.push_back(J_RESOURCE_TYPE::ANIMATION_CLIP);

				return resVec;
			};
			auto fbxMeshImportC = [](const JResourceObjectImportDesc* desc) -> std::vector<JUserPtr<JResourceObject>>
			{
				const JMeshGeometryImportDesc* meshDesc = static_cast<const JMeshGeometryImportDesc*>(desc);
				const Core::JFileImportPathData& importPathData = meshDesc->importPathData;
				const J_OBJECT_FLAG flag = (J_OBJECT_FLAG)importPathData.flag;
				const JUserPtr<JDirectory>& dir = meshDesc->dir;

				std::vector<JUserPtr<JResourceObject>> res;
				using FbxFileTypeInfo = Core::JFbxFileLoader::FbxFileTypeInfo;
				FbxFileTypeInfo info = JFbxFileLoader::Instance().GetFileTypeInfo(importPathData.oriFilePath);
				 
				JUserPtr<JSkeletonAsset> newSkeleton = nullptr;
				if (HasSQValueEnum(info.typeInfo, Core::J_FBX_RESULT_HAS_SKELETON))
				{
					std::unique_ptr<Core::JSkinnedMeshGroup> skinnedGroup = std::make_unique<Core::JSkinnedMeshGroup>();
					std::vector<Core::Joint> joint;
					Core::JFbxMaterialMap materialMap;
					Core::J_FBX_RESULT loadRes = JFbxFileLoader::Instance().LoadFbxMeshFile(importPathData.oriFilePath, *skinnedGroup, joint, materialMap);
					if (loadRes == Core::J_FBX_RESULT_FAIL)
						return { nullptr };
					 
					JUserPtr<JDirectory> fileDir = dir->GetChildDirctoryByName(importPathData.name);
					if (fileDir == nullptr)
						fileDir = JICI::Create<JDirectory>(importPathData.name, Core::MakeGuid(), flag, dir);
					
					ConvertJEMaterial(*skinnedGroup, materialMap, flag, fileDir);
					
					const size_t skeletonGuid = Core::MakeGuid();
					const size_t skinnedMeshGuid = Core::MakeGuid();
					JUserPtr<JDirectory> modelDir = JICI::Create<JDirectory>(L"Model", Core::MakeGuid(), flag, fileDir);
					if (HasSQValueEnum(info.typeInfo, Core::J_FBX_RESULT_HAS_SKELETON))
					{
						Core::JTypeInstanceSearchHint skinendHint(JSkinnedMeshGeometry::StaticTypeInfo(), skinnedMeshGuid);
						newSkeleton = JICI::Create<JSkeletonAsset>(importPathData.name + L"Skel",
							skeletonGuid,
							flag,
							RTypeCommonCall::CallFormatIndex(JSkeletonAsset::GetStaticResourceType(), importPathData.oriFileFormat),
							modelDir,
							skinendHint,
							std::move(joint));

						skinnedGroup->SetSkeletonAsset(newSkeleton);
					}
					if (HasSQValueEnum(info.typeInfo, Core::J_FBX_RESULT_HAS_MESH))
					{
						JUserPtr<JMeshGeometry> newMesh = JICI::Create<JSkinnedMeshGeometry>(importPathData.name,
							skinnedMeshGuid,
							flag,
							RTypeCommonCall::CallFormatIndex(GetStaticResourceType(), importPathData.oriFileFormat),
							modelDir,
							std::move(skinnedGroup));
						res.push_back(newMesh);

						const uint subMeshCount = (uint)newMesh->GetTotalSubmeshCount();
						for (uint i = 0; i < subMeshCount; ++i)
							res.push_back(newMesh->GetSubmeshMaterial(i));
					}
					if (newSkeleton != nullptr)
						res.push_back(newSkeleton);
				}
				else
				{
					std::unique_ptr<Core::JStaticMeshGroup> staticMeshGroup = std::make_unique<Core::JStaticMeshGroup>();
					Core::JFbxMaterialMap materialMap;
					Core::J_FBX_RESULT loadRes = JFbxFileLoader::Instance().LoadFbxMeshFile(importPathData.oriFilePath, *staticMeshGroup, materialMap);
					if (loadRes == Core::J_FBX_RESULT_FAIL)
						return { nullptr };

					JUserPtr<JDirectory> fileDir = dir->GetChildDirctoryByName(importPathData.name);
					if (fileDir == nullptr)
						fileDir = JICI::Create<JDirectory>(importPathData.name, Core::MakeGuid(), flag, dir);

					ConvertJEMaterial(*staticMeshGroup, materialMap, flag, fileDir);
					JUserPtr<JDirectory> modelDir = JICI::Create<JDirectory>(L"Model", Core::MakeGuid(), flag, fileDir);
					if (HasSQValueEnum(info.typeInfo, Core::J_FBX_RESULT_HAS_MESH))
					{
						if (meshDesc->useSplitMesh)
						{ 
							auto splitGroup = SplitMeshGroup(staticMeshGroup.get());
							for (auto& data : splitGroup)
							{
								auto meshData = data->GetMeshData(0);
								if (meshData == nullptr)
									continue;
								 
								JUserPtr<JMeshGeometry> newMesh = JICI::Create<JStaticMeshGeometry>(meshData->GetName(),
									Core::MakeGuid(),
									flag,
									RTypeCommonCall::CallFormatIndex(GetStaticResourceType(), importPathData.oriFileFormat),
									modelDir,
									std::move(data));
								res.push_back(newMesh);

								const uint subMeshCount = (uint)newMesh->GetTotalSubmeshCount();
								for (uint i = 0; i < subMeshCount; ++i)
									res.push_back(newMesh->GetSubmeshMaterial(i));
							}
						}
						else
						{
							JUserPtr<JMeshGeometry> newMesh = JICI::Create<JStaticMeshGeometry>(importPathData.name,
								Core::MakeGuid(),
								flag,
								RTypeCommonCall::CallFormatIndex(GetStaticResourceType(), importPathData.oriFileFormat),
								modelDir,
								std::move(staticMeshGroup));
							res.push_back(newMesh);

							const uint subMeshCount = (uint)newMesh->GetTotalSubmeshCount();
							for (uint i = 0; i < subMeshCount; ++i)
								res.push_back(newMesh->GetSubmeshMaterial(i));
						}
					}
				}

				return res;
			};
			auto objMeshImportC = [](const JResourceObjectImportDesc* desc) -> std::vector<JUserPtr<JResourceObject>>
			{				
				const JMeshGeometryImportDesc* meshDesc = static_cast<const JMeshGeometryImportDesc*>(desc);
				const Core::JFileImportPathData& importPathData = meshDesc->importPathData;
				const J_OBJECT_FLAG flag = (J_OBJECT_FLAG)importPathData.flag;
				const JUserPtr<JDirectory>& dir = meshDesc->dir;

				std::unique_ptr<Core::JStaticMeshGroup> meshGroup = std::make_unique<Core::JStaticMeshGroup>();
				Core::ObjMaterialMap objMatData;
				JUserPtr<JMeshGeometry> newMesh = nullptr;
				if (JObjFileLoader::Instance().LoadObjFile(importPathData, *meshGroup, objMatData))
				{
					JUserPtr<JDirectory> fileDir = dir->GetChildDirctoryByName(importPathData.name);
					if (fileDir == nullptr)
						fileDir = JICI::Create<JDirectory>(importPathData.name, Core::MakeGuid(), flag, dir);

					JUserPtr<JDirectory> modelDir = JICI::Create<JDirectory>(L"Model", Core::MakeGuid(), flag, fileDir);
					newMesh = JICI::Create<JStaticMeshGeometry>(importPathData.name,
						Core::MakeGuid(),
						flag,
						RTypeCommonCall::CallFormatIndex(GetStaticResourceType(), importPathData.oriFileFormat),
						modelDir,
						std::move(meshGroup));
				}

				std::vector<JUserPtr<JResourceObject>> res;
				if (newMesh != nullptr)
				{
					res.push_back(newMesh);
					const uint subMeshCount = (uint)newMesh->GetTotalSubmeshCount();
					for (uint i = 0; i < subMeshCount; ++i)
						res.push_back(newMesh->GetSubmeshMaterial(i));
				}
				return res;
			};

			JResourceObjectImporter::Instance().AddFormatInfo(L".fbx", J_RESOURCE_TYPE::MESH, fbxMeshImportC, fbxClassifyC);
			JResourceObjectImporter::Instance().AddFormatInfo(L".obj", J_RESOURCE_TYPE::MESH, objMeshImportC);

			IMPL_REALLOC_BIND(JMeshGeometry::JMeshGeometryImpl, thisPointer)
		}
	};

	JMeshGeometryImportDesc::JMeshGeometryImportDesc(const Core::JFileImportPathData& importPathData)
		:JResourceObjectImportDesc(importPathData)
	{}

	JMeshGeometry::InitData::InitData(const Core::JTypeInfo& type, 
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		std::unique_ptr<Core::JMeshGroup>&& meshGroup)
		: JResourceObject::InitData(type, formatIndex, GetStaticResourceType(), directory),
		meshGroup(std::move(meshGroup))
	{}
	JMeshGeometry::InitData::InitData(const Core::JTypeInfo& type, 
		const size_t guid,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		std::unique_ptr<Core::JMeshGroup>&& meshGroup)
		: JResourceObject::InitData(type, guid, formatIndex, GetStaticResourceType(), directory),
		meshGroup(std::move(meshGroup))
	{ }
	JMeshGeometry::InitData::InitData(const Core::JTypeInfo& type, 
		const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		std::unique_ptr<Core::JMeshGroup>&& meshGroup)
		: JResourceObject::InitData(type, name, guid, flag, formatIndex, GetStaticResourceType(), directory), 
		meshGroup(std::move(meshGroup))
	{ }
	bool JMeshGeometry::InitData::IsValidData()const noexcept
	{
		return JResourceObject::InitData::IsValidData() && meshGroup != nullptr;
	}

	JMeshGeometry::LoadMetaData::LoadMetaData(const Core::JTypeInfo& type, const JUserPtr<JDirectory>& directory)
		:JResourceObject::InitData(type, GetDefaultFormatIndex(), J_RESOURCE_TYPE::MESH,  directory)
	{}

	const Graphic::JGraphicResourceUserInterface JMeshGeometry::GraphicResourceUserInterface()const noexcept
	{
		return Graphic::JGraphicResourceUserInterface{ impl.get() };
	}
	J_RESOURCE_TYPE JMeshGeometry::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	}
	std::wstring JMeshGeometry::GetFormat()const noexcept
	{
		return GetAvailableFormat()[GetFormatIndex()];
	}
	std::vector<std::wstring> JMeshGeometry::GetAvailableFormat()noexcept
	{
		static std::vector<std::wstring> format{ L".mesh", L".obj", L".fbx" }; 
		return format;
	}
	uint JMeshGeometry::GetTotalVertexCount()const noexcept
	{
		return impl->GetTotalVertexCount();
	}
	uint JMeshGeometry::GetTotalIndexCount()const noexcept
	{
		return impl->GetTotalIndexCount();
	}
	uint JMeshGeometry::GetTotalSubmeshCount()const noexcept
	{
		return impl->GetTotalSubmeshCount();
	}
	uint JMeshGeometry::GetSubmeshVertexCount(const uint index)const noexcept
	{
		return impl->GetSubmeshVertexCount(index);
	}
	uint JMeshGeometry::GetSubmeshIndexCount(const uint index)const noexcept
	{
		return impl->GetSubmeshIndexCount(index);
	}
	uint JMeshGeometry::GetSubmeshBaseVertexLocation(const uint index)const noexcept
	{
		return impl->GetSubmeshBaseVertexLocation(index);
	}
	uint JMeshGeometry::GetSubmeshStartIndexLocation(const uint index)const noexcept
	{
		return impl->GetSubmeshStartIndexLocation(index);
	}
	uint JMeshGeometry::GetVertexByteSize()const noexcept
	{
		return impl->vertexByteSize;
	}
	uint JMeshGeometry::GetVertexBufferByteSize()const noexcept
	{
		return impl->vertexBufferByteSize;
	}
	uint JMeshGeometry::GetIndexByteSize()const noexcept
	{
		return impl->indexByteSize;
	}
	uint JMeshGeometry::GetIndexBufferByteSize()const noexcept
	{
		return impl->indexBufferByteSize;
	}
	std::wstring JMeshGeometry::GetSubMeshName(const uint index)const noexcept
	{
		return impl->GetSubMeshName(index);
	}
	JUserPtr<JMaterial> JMeshGeometry::GetSubmeshMaterial(const uint index)const noexcept
	{
		return impl->GetSubmeshMaterial(index);
	}
	DirectX::BoundingBox JMeshGeometry::GetBoundingBox()const noexcept
	{
		return impl->boundingBox;
	}
	JVector3<float> JMeshGeometry::GetBoundingBoxCenter()const noexcept
	{
		return impl->boundingBox.Center;
	}
	JVector3<float> JMeshGeometry::GetBoundingBoxExtent()const noexcept
	{
		return impl->boundingBox.Extents;
	}
	DirectX::BoundingSphere JMeshGeometry::GetBoundingSphere()const noexcept
	{
		return impl->boundingSphere;
	}
	JVector3<float> JMeshGeometry::GetBoundingSphereCenter()const noexcept
	{
		return impl->boundingSphere.Center;
	}
	float JMeshGeometry::GetBoundingSphereRadius()const noexcept
	{
		return impl->boundingSphere.Radius;
	}
	Core::JMeshGroup* JMeshGeometry::GetMeshGroupData()const noexcept
	{
		return impl->meshGroupData.get();
	}
	void JMeshGeometry::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
		impl->StuffResource();
		impl->OnResourceRef();
	}
	void JMeshGeometry::DoDeActivate()noexcept
	{
		impl->ClearResource();
		impl->OffResourceRef();
		JResourceObject::DoDeActivate();
	}
	JMeshGeometry::JMeshGeometry(InitData& initData)
		: JResourceObject(initData), impl(std::make_unique<JMeshGeometryImpl>(initData, this))
	{ }
	JMeshGeometry::~JMeshGeometry()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JMeshGeometryPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JMeshGeometryPrivate::DestroyInstanceInterface;  

	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JResourceObjectPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JMeshGeometry* mesh = static_cast<JMeshGeometry*>(createdPtr);
		mesh->impl->RegisterThisPointer(mesh);
		mesh->impl->RegisterInterfacePointer();
		mesh->impl->RegisterPostCreation();
		mesh->impl->Initialize(static_cast<JMeshGeometry::InitData*>(initData));
	}
	void CreateInstanceInterface::TryDestroyUnUseData(Core::JIdentifier* createdPtr)noexcept
	{
		static_cast<JMeshGeometry*>(createdPtr)->impl->meshGroupData = nullptr;
	} 

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{ 
		static_cast<JMeshGeometry*>(ptr)->impl->DeRegisterPreDestruction();
		static_cast<JMeshGeometry*>(ptr)->impl->DeRegisterInterfacePointer();
		JResourceObjectPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}
	 
	Core::JIdentifierPrivate::DestroyInstanceInterface& JMeshGeometryPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
}