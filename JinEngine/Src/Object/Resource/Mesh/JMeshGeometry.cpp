#include"JMeshGeometry.h"
#include"JMeshGeometryPrivate.h"
#include"JMeshStruct.h"
#include"JStaticMeshGeometry.h"
#include"JSkinnedMeshGeometry.h"
#include"../JResourceObjectHint.h"
#include"../JResourceObjectImporter.h"
#include"../JClearableInterface.h"
#include"../JResourceObjectUserInterface.h"
#include"../JResourceManager.h"
#include"../Skeleton/JSkeletonAsset.h"
#include"../Skeleton/JSkeleton.h" 
#include"../Skeleton/Joint.h" 
#include"../Material/JMaterial.h" 
#include"../Material/JDefaultMaterialSetting.h"
#include"../../Directory/JDirectory.h" 
#include"../../../Application/JApplicationProject.h" 
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/Identity/JIdenCreator.h"
#include"../../../Core/Identity/JIdentifierImplBase.h"
#include"../../../Core/Exception/JExceptionMacro.h" 
#include"../../../Core/File/JFileConstant.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Geometry/JDirectXCollisionEx.h"
#include"../../../Core/Loader/FbxLoader/JFbxFileLoader.h"
#include"../../../Core/Loader/ObjLoader/JObjFileLoader.h"

#include"../../../Utility/JMathHelper.h" 
#include"../../../Utility/JD3DUtility.h" 
#include"../../../Utility/JCommonUtility.h"
#include"../../../Graphic/JGraphic.h"
#include"../../../Graphic/JGraphicPrivate.h"
#include<fstream>
#include<wrl/client.h>

namespace JinEngine
{
	using namespace DirectX;
	namespace
	{
		using GDeviceInterface = Graphic::JGraphicPrivate::DeviceInterface;
		using GCommandInterface = Graphic::JGraphicPrivate::CommandInterface;
	}
 
	class SubmeshGeometry
	{
	private:
		std::wstring name;
		size_t guid;
		Core::JUserPtr<JMaterial> material;
	private:
		uint vertexCount = 0;
		uint vertexStrat = 0;
		uint indexCount = 0;
		uint indexStart = 0;
		DirectX::BoundingBox boundingBox;
		DirectX::BoundingSphere boundingSphere;
		bool hasNormal = false;
		bool hasUV = false;
		J_MESHGEOMETRY_TYPE type;
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
		Core::JUserPtr<JMaterial> GetMaterial()const noexcept
		{
			return material;
		}
		uint GetVertexCount()const noexcept
		{
			return vertexCount;
		}
		uint GetVertexStart()const noexcept
		{
			return vertexStrat;
		}
		uint GetIndexStart()const noexcept
		{
			return indexStart;
		}
		uint GetIndexCount()const noexcept
		{
			return indexCount;
		}
		DirectX::XMFLOAT3 GetBoundingBoxCenter()const noexcept
		{
			return boundingBox.Center;
		}
		DirectX::XMVECTOR GetBoundingBoxCenterV()const noexcept
		{
			return XMLoadFloat3(&boundingBox.Center);
		}
		DirectX::XMFLOAT3 GetBoundingBoxExtent()const noexcept
		{
			return boundingBox.Extents;
		}
		DirectX::XMVECTOR GetBoundingBoxExtentV()const noexcept
		{
			return XMLoadFloat3(&boundingBox.Extents);
		}
		DirectX::XMFLOAT3 GetBoundingSphereCenter()const noexcept
		{
			return boundingSphere.Center;
		}
		float GetBoundingSphereRadius()const noexcept
		{
			return boundingSphere.Radius;
		}
	public:
		void SetMesh(const JMeshData& meshData, const uint vertexSt, const uint indexSt)
		{
			Core::JUserPtr<JMaterial> meshMaterial = meshData.GetMaterial();
			if (meshMaterial.IsValid())
				material = meshMaterial;
			else
				material = _JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_STANDARD);

			vertexStrat = vertexSt;
			vertexCount = meshData.GetVertexCount();
			indexStart = indexSt;
			indexCount = meshData.GetIndexCount();

			SubmeshGeometry::boundingBox = meshData.GetBBox();
			SubmeshGeometry::boundingSphere = meshData.GetBSphere();

			hasNormal = meshData.HasNormal();
			hasUV = meshData.HasUV();
			type = meshData.GetMeshType();
		}
		void SetMaterial(Core::JUserPtr<JMaterial> newMaterial)noexcept
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

	class JMeshGeometry::JMeshGeometryImpl : public Core::JIdentifierImplBase,
		public JResourceObjectUserInterface,
		public JClearableInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JMeshGeometryImpl)
	public:
		JMeshGeometry* thisMesh = nullptr;
	public:
		std::unique_ptr<JMeshGroup> meshGroupData = nullptr;	//for store group data 리소스 초기화후 nullptr 이 된다
	public:
		// System memory copies.  Use Blobs because the vertex/index format can be generic.
		// It is up to the client to cast appropriately.
		//Microsoft::WRL::ComPtr<ID3DBlob> vertexBufferCPU = nullptr;
		//Microsoft::WRL::ComPtr<ID3DBlob> indexBufferCPU = nullptr;
	public:
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferGPU = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferGPU = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUploader = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUploader = nullptr;
	public:
		DirectX::BoundingBox boundingBox;
		DirectX::BoundingSphere boundingSphere;
	public:
		// Data about the buffers.
		uint vertexCount = 0;
		uint indexCount = 0;
		uint vertexByteStride = 0;
		uint vertexBufferByteSize = 0;
		DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
		uint indexBufferByteSize = 0;
		std::vector<SubmeshGeometry> submeshes;
	public:
		JMeshGeometryImpl(const InitData& initData, JMeshGeometry* thisMesh)
			:JResourceObjectUserInterface(thisMesh->GetGuid()), thisMesh(thisMesh)
		{
		}
		~JMeshGeometryImpl(){}
	public:
		void Initialize(InitData& initData)
		{
			TryCreateMeshGroupMaterial(initData.meshGroup.get());
			ImportMesh(initData.meshGroup.get());
			thisMesh->SetValid(true);
			meshGroupData = std::move(initData.meshGroup);
		}
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
		Core::JUserPtr<JMaterial> GetSubmeshMaterial(const uint index)const noexcept
		{
			return submeshes.size() > index ? submeshes[index].GetMaterial() : Core::JUserPtr<JMaterial>{};
		}
	public:
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const noexcept
		{
			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = vertexBufferGPU->GetGPUVirtualAddress();
			vbv.StrideInBytes = vertexByteStride;
			vbv.SizeInBytes = vertexBufferByteSize;
			return vbv;
		}
		D3D12_INDEX_BUFFER_VIEW IndexBufferView()const noexcept
		{
			D3D12_INDEX_BUFFER_VIEW ibv;
			ibv.BufferLocation = indexBufferGPU->GetGPUVirtualAddress();
			ibv.Format = indexFormat;
			ibv.SizeInBytes = indexBufferByteSize;
			return ibv;
		}
	public:
		void UpdateMeshBound()noexcept
		{
			const uint submeshCount = (uint)submeshes.size();

			XMFLOAT3 minInit(+JMathHelper::Infinity, +JMathHelper::Infinity, +JMathHelper::Infinity);
			XMFLOAT3 maxInit(-JMathHelper::Infinity, -JMathHelper::Infinity, -JMathHelper::Infinity);
			XMVECTOR minXmV = XMLoadFloat3(&minInit);
			XMVECTOR maxXmV = XMLoadFloat3(&maxInit);

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
		void TryCreateMeshGroupMaterial(JMeshGroup* meshGroup)
		{
			const uint submeshCount = (uint)meshGroup->GetMeshDataCount();
			JDirectory* matDir = thisMesh->GetDirectory()->GetChildDirctoryByName(L"Material");
			if (matDir == nullptr)
				matDir = JICI::Create<JDirectory>(L"Material", Core::MakeGuid(), OBJECT_FLAG_NONE, thisMesh->GetDirectory());

			for (uint i = 0; i < submeshCount; ++i)
			{
				if (meshGroup->GetMeshData(i)->GetMaterial().IsValid())
					continue;

				const std::wstring materialName = L"m" + meshGroup->GetMeshData(i)->GetName();
				JMaterial* newMaterial = JICI::Create<JMaterial>(materialName,
					Core::MakeGuid(),
					thisMesh->GetFlag(),
					GetDefaultFormatIndex(),
					matDir);
				JDefaultMaterialSetting::SetStandard(newMaterial);
				meshGroup->GetMeshData(i)->SetMaterial(Core::GetUserPtr(newMaterial));
			} 
		}
		bool ImportMesh(JMeshGroup* meshGroup)
		{
			//SutffSubMesh는  하위 메시 클래스에서
			//리소스 초기화시 호출된다

			//stuff material
			ClearGpuBuffer();
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
				if (meshGroup->GetMeshData(i)->GetMeshType() == J_MESHGEOMETRY_TYPE::STATIC)
					submeshes[i].SetMesh(*static_cast<JStaticMeshData*>(meshGroup->GetMeshData(i)), vertexCount, indexCount);
				else
					submeshes[i].SetMesh(*static_cast<JSkinnedMeshData*>(meshGroup->GetMeshData(i)), vertexCount, indexCount);

				vertexCount += submeshes[i].GetVertexCount();
				indexCount += submeshes[i].GetIndexCount();
			}
			JMeshGeometryImpl::vertexCount = vertexCount;
			JMeshGeometryImpl::indexCount = indexCount;

			ID3D12Device* device = GDeviceInterface::GetDevice(); 
			ID3D12CommandQueue* mCommandQueue = GCommandInterface::GetCommandQueue();
			ID3D12CommandAllocator* mDirectCmdListAlloc = GCommandInterface::GetCommandAllocator();
			ID3D12GraphicsCommandList* mCommandList = GCommandInterface::GetCommandList();

			if (meshGroup->GetMeshGroupType() == J_MESHGEOMETRY_TYPE::STATIC)
			{
				vertexByteStride = sizeof(JStaticMeshVertex);
				vertexBufferByteSize = vertexCount * vertexByteStride;

				std::vector<JStaticMeshVertex> vertices(vertexCount);
				uint vertexOffset = 0;
				for (uint i = 0; i < submeshCount; ++i)
				{
					JStaticMeshData* meshdata = static_cast<JStaticMeshData*>(meshGroup->GetMeshData(i));
					const uint subMeshVertexCount = meshdata->GetVertexCount();
					for (uint j = 0; j < subMeshVertexCount; ++j)
						vertices[vertexOffset + j] = meshdata->GetVertex(j);
					vertexOffset += subMeshVertexCount;
				}

				GCommandInterface::FlushCommandQueue();
				GCommandInterface::StartCommand();
				vertexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, vertices.data(), vertexBufferByteSize, vertexBufferUploader);
				GCommandInterface::EndCommand();
				GCommandInterface::FlushCommandQueue();
			}
			else
			{
				vertexByteStride = sizeof(JSkinnedMeshVertex);
				vertexBufferByteSize = vertexCount * vertexByteStride;

				std::vector<JSkinnedMeshVertex> vertices(vertexCount);
				uint vertexOffset = 0;
				for (uint i = 0; i < submeshCount; ++i)
				{
					JSkinnedMeshData* meshdata = static_cast<JSkinnedMeshData*>(meshGroup->GetMeshData(i));
					const uint subMeshVertexCount = meshdata->GetVertexCount();
					for (uint j = 0; j < subMeshVertexCount; ++j)
						vertices[vertexOffset + j] = meshdata->GetVertex(j);
					vertexOffset += subMeshVertexCount;
				}
				GCommandInterface::FlushCommandQueue();
				GCommandInterface::StartCommand();
				vertexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, vertices.data(), vertexBufferByteSize, vertexBufferUploader);
				GCommandInterface::EndCommand();
				GCommandInterface::FlushCommandQueue();
			}

			if (indexCount >= 1 << 16)
			{
				indexFormat = DXGI_FORMAT_R32_UINT;
				indexBufferByteSize = indexCount * sizeof(uint32);
				std::vector<uint32> indices32(indexCount);

				uint indicesOffset = 0;
				for (uint i = 0; i < submeshCount; ++i)
				{
					const JMeshData* meshdata = meshGroup->GetMeshData(i);
					const uint subMeshIndexCount = meshdata->GetIndexCount();
					for (uint j = 0; j < subMeshIndexCount; ++j)
						indices32[indicesOffset + j] = meshdata->GetIndex(j);
					indicesOffset += subMeshIndexCount;
				}
				GCommandInterface::FlushCommandQueue();
				GCommandInterface::StartCommand();
				indexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, indices32.data(), indexBufferByteSize, indexBufferUploader);
				GCommandInterface::EndCommand();
				GCommandInterface::FlushCommandQueue();
			}
			else
			{
				indexFormat = DXGI_FORMAT_R16_UINT;
				indexBufferByteSize = indexCount * sizeof(uint16);
				std::vector<uint16> indices16(indexCount);
				uint indicesOffset = 0;

				for (uint i = 0; i < submeshCount; ++i)
				{
					const JMeshData* meshdata = meshGroup->GetMeshData(i);
					const uint subMeshIndexCount = meshdata->GetIndexCount();
					for (uint j = 0; j < subMeshIndexCount; ++j)
						indices16[indicesOffset + j] = meshdata->GetIndex(j);
					indicesOffset += subMeshIndexCount;
				}
				GCommandInterface::FlushCommandQueue();
				GCommandInterface::StartCommand();
				indexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, indices16.data(), indexBufferByteSize, indexBufferUploader);
				GCommandInterface::EndCommand();
				GCommandInterface::FlushCommandQueue();
			}
			if (thisMesh->IsActivated())
			{
				for (uint i = 0; i < submeshCount; ++i)
					CallOnResourceReference(submeshes[i].GetMaterial().Get());
			}
			UpdateMeshBound();
			return true;
		}
		void ClearGpuBuffer()
		{
			vertexBufferUploader.Reset();
			indexBufferUploader.Reset();
			vertexBufferGPU.Reset();
			indexBufferGPU.Reset();
		}
		void StuffResource()
		{
			if (!thisMesh->IsValid())
			{
				if (thisMesh->GetFormatIndex() != GetInvalidFormatIndex())
				{
					auto& p = static_cast<JMeshGeometryPrivate&>(thisMesh->GetPrivateInterface());
					auto& pA = static_cast<JMeshGeometryPrivate::AssetDataIOInterface&>(p.GetAssetDataIOInterface());
					auto groupData = pA.ReadMeshGroupData(thisMesh->GetPath());
					if (ImportMesh(groupData.get()))
						thisMesh->SetValid(true);
				}
			}
		}
		void ClearResource()
		{
			if (thisMesh->IsValid())
			{
				// vertexBufferCPU.Reset();
				//indexBufferCPU.Reset(); 
				ClearGpuBuffer();
				thisMesh->SetValid(false);
			}
		}
	public:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == thisMesh->GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE && jRobj->GetResourceType() == J_RESOURCE_TYPE::MATERIAL)
			{
				Core::JUserPtr<JMaterial> defaultMat = _JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_STANDARD);
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
							if (thisMesh->IsActivated())
								CallOnResourceReference(defaultMat.Get());
						}
						else
							submeshes[i].SetMaterial(Core::JUserPtr<JMaterial>{});
					}
				}
			}
		}
	public:
		static void RegisterCallOnce()
		{
			auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex(GetStaticResourceType(), format); };
			static GetRTypeInfoCallable getTypeInfoCallable{ &JMeshGeometry::StaticTypeInfo };
			static GetAvailableFormatCallable getAvailableFormatCallable{ &JMeshGeometry::GetAvailableFormat };
			static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

			static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{J_RESOURCE_TYPE::MATERIAL, J_RESOURCE_TYPE::SKELETON}, true, false, false, true };
			static RTypeCommonFunc rTypeCFunc{ getTypeInfoCallable, getAvailableFormatCallable, getFormatIndexCallable };

			RegisterRTypeInfo(rTypeHint, rTypeCFunc, RTypePrivateFunc{});

			//JResourceObject*, const std::wstring, JDirectory*, const std::wstring>
			auto fbxClassifyC = [](const Core::JFileImportHelpData importPathdata) -> std::vector<J_RESOURCE_TYPE>
			{
				using FbxFileTypeInfo = Core::JFbxFileLoaderImpl::FbxFileTypeInfo;
				FbxFileTypeInfo info = Core::JFbxFileLoader::Instance().GetFileTypeInfo(importPathdata.oriFilePath);
				if (info.typeInfo == Core::J_FBXRESULT::FAIL)
					return {};

				std::vector<J_RESOURCE_TYPE> resVec;
				if (HasSQValueEnum(info.typeInfo, Core::J_FBXRESULT::HAS_MESH))
					resVec.push_back(J_RESOURCE_TYPE::MESH);
				if (HasSQValueEnum(info.typeInfo, Core::J_FBXRESULT::HAS_ANIMATION))
					resVec.push_back(J_RESOURCE_TYPE::ANIMATION_CLIP);

				return resVec;
			};
			auto fbxMeshImportC = [](JDirectory* dir, const Core::JFileImportHelpData importPathData) -> std::vector<JResourceObject*>
			{
				std::vector<JResourceObject*> res;
				using FbxFileTypeInfo = Core::JFbxFileLoaderImpl::FbxFileTypeInfo;
				FbxFileTypeInfo info = Core::JFbxFileLoader::Instance().GetFileTypeInfo(importPathData.oriFilePath);

				JMeshGeometry* newMesh = nullptr;
				JSkeletonAsset* newSkeleton = nullptr;
				if (HasSQValueEnum(info.typeInfo, Core::J_FBXRESULT::HAS_SKELETON))
				{
					std::unique_ptr<JSkinnedMeshGroup> skinnedGroup = std::make_unique<JSkinnedMeshGroup>();
					std::vector<Joint> joint;
					Core::J_FBXRESULT loadRes = Core::JFbxFileLoader::Instance().LoadFbxMeshFile(importPathData.oriFilePath, *skinnedGroup, joint);
					if (loadRes == Core::J_FBXRESULT::FAIL)
						return { nullptr };
					 
					JDirectory* fileDir = dir->GetChildDirctoryByName(importPathData.name);
					if (fileDir == nullptr)
						fileDir = JICI::Create<JDirectory>(importPathData.name, Core::MakeGuid(), OBJECT_FLAG_NONE, dir);

					JDirectory* modelDir = JICI::Create<JDirectory>(L"Model", Core::MakeGuid(), OBJECT_FLAG_NONE, fileDir);
					if (HasSQValueEnum(info.typeInfo, Core::J_FBXRESULT::HAS_SKELETON))
					{
						newSkeleton = JICI::Create<JSkeletonAsset>(importPathData.name + L"Skel",
							Core::MakeGuid(),
							importPathData.flag,
							RTypeCommonCall::CallFormatIndex(JSkeletonAsset::GetStaticResourceType(), importPathData.format),
							modelDir,
							std::make_unique<JSkeleton>(std::move(joint)));
						skinnedGroup->SetSkeletonAsset(Core::GetUserPtr(newSkeleton));
					}
					if (HasSQValueEnum(info.typeInfo, Core::J_FBXRESULT::HAS_MESH))
					{
						newMesh = JICI::Create<JSkinnedMeshGeometry>(importPathData.name,
							Core::MakeGuid(),
							importPathData.flag,
							RTypeCommonCall::CallFormatIndex(GetStaticResourceType(), importPathData.format),
							modelDir,
							std::move(skinnedGroup));
						res.push_back(newMesh);
					}
					if (newSkeleton != nullptr)
						res.push_back(newSkeleton);
				}
				else
				{
					std::unique_ptr<JStaticMeshGroup> staticMeshGroup = std::make_unique<JStaticMeshGroup>();
					Core::J_FBXRESULT loadRes = Core::JFbxFileLoader::Instance().LoadFbxMeshFile(importPathData.oriFilePath, *staticMeshGroup);
					if (loadRes == Core::J_FBXRESULT::FAIL)
						return { nullptr };

					JDirectory* fileDir = dir->GetChildDirctoryByName(importPathData.name);
					if (fileDir == nullptr)
						fileDir = JICI::Create<JDirectory>(importPathData.name, Core::MakeGuid(), OBJECT_FLAG_NONE, dir);

					JDirectory* modelDir = JICI::Create<JDirectory>(L"Model", Core::MakeGuid(), OBJECT_FLAG_NONE, fileDir);
					if (HasSQValueEnum(info.typeInfo, Core::J_FBXRESULT::HAS_MESH))
					{
						newMesh = JICI::Create<JStaticMeshGeometry>(importPathData.name,
							Core::MakeGuid(),
							importPathData.flag,
							RTypeCommonCall::CallFormatIndex(GetStaticResourceType(), importPathData.format),
							modelDir,
							std::move(staticMeshGroup));
						res.push_back(newMesh);
					}
				}
				if (newMesh != nullptr)
				{
					const uint subMeshCount = (uint)newMesh->GetTotalSubmeshCount();
					for (uint i = 0; i < subMeshCount; ++i)
						res.push_back(newMesh->GetSubmeshMaterial(i).Get());
				}
				return res;
			};
			auto objMeshImportC = [](JDirectory* dir, const Core::JFileImportHelpData importPathData) -> std::vector<JResourceObject*>
			{
				std::vector<JResourceObject*> res;
				Core::JObjFileMeshData objMeshData;
				std::vector<Core::JObjFileMatData> objMatData;
				JMeshGeometry* newMesh = nullptr;
				if (JObjFileLoader::Instance().LoadObjFile(importPathData, objMeshData, objMatData))
				{
					JDirectory* fileDir = dir->GetChildDirctoryByName(importPathData.name);
					if (fileDir == nullptr)
						fileDir = JICI::Create<JDirectory>(importPathData.name, Core::MakeGuid(), OBJECT_FLAG_NONE, dir);

					JDirectory* modelDir = JICI::Create<JDirectory>(L"Model", Core::MakeGuid(), OBJECT_FLAG_NONE, fileDir);
					newMesh = JICI::Create<JStaticMeshGeometry>(importPathData.name,
						Core::MakeGuid(),
						importPathData.flag,
						RTypeCommonCall::CallFormatIndex(GetStaticResourceType(), importPathData.format),
						modelDir,
						std::make_unique<JStaticMeshGroup>(objMeshData.meshGroup));
				}
				if (newMesh != nullptr)
				{
					res.push_back(newMesh);
					const uint subMeshCount = (uint)newMesh->GetTotalSubmeshCount();
					for (uint i = 0; i < subMeshCount; ++i)
						res.push_back(newMesh->GetSubmeshMaterial(i).Get());
				}
				return res;
			};

			JResourceObjectImporter::Instance().AddFormatInfo(L".fbx", J_RESOURCE_TYPE::MESH, fbxMeshImportC, fbxClassifyC);
			JResourceObjectImporter::Instance().AddFormatInfo(L".obj", J_RESOURCE_TYPE::MESH, objMeshImportC);
		}
	};

	JMeshGeometry::InitData::InitData(const Core::JTypeInfo& type, 
		const uint8 formatIndex,
		JDirectory* directory,
		std::unique_ptr<JMeshGroup>&& meshGroup)
		: JResourceObject::InitData(type, formatIndex, GetStaticResourceType(), directory),
		meshGroup(std::move(meshGroup))
	{}
	JMeshGeometry::InitData::InitData(const Core::JTypeInfo& type, 
		const size_t guid,
		const uint8 formatIndex,
		JDirectory* directory,
		std::unique_ptr<JMeshGroup>&& meshGroup)
		: JResourceObject::InitData(type, guid, formatIndex, GetStaticResourceType(), directory),
		meshGroup(std::move(meshGroup))
	{ }
	JMeshGeometry::InitData::InitData(const Core::JTypeInfo& type, 
		const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex,
		JDirectory* directory,
		std::unique_ptr<JMeshGroup>&& meshGroup)
		: JResourceObject::InitData(type, name, guid, flag, formatIndex, GetStaticResourceType(), directory), 
		meshGroup(std::move(meshGroup))
	{ }
	bool JMeshGeometry::InitData::IsValidData()const noexcept
	{
		return JResourceObject::InitData::IsValidData() && meshGroup != nullptr;
	}

	JMeshGeometry::LoadMetaData::LoadMetaData(const Core::JTypeInfo& type, JDirectory* directory)
		:JResourceObject::InitData(type, GetDefaultFormatIndex(), J_RESOURCE_TYPE::MESH,  directory)
	{}

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
	std::wstring JMeshGeometry::GetSubMeshName(const uint index)const noexcept
	{
		return impl->GetSubMeshName(index);
	}
	Core::JUserPtr<JMaterial> JMeshGeometry::GetSubmeshMaterial(const uint index)const noexcept
	{
		return impl->GetSubmeshMaterial(index);
	}
	DirectX::BoundingBox JMeshGeometry::GetBoundingBox()const noexcept
	{
		return impl->boundingBox;
	}
	DirectX::XMFLOAT3 JMeshGeometry::GetBoundingBoxCenter()const noexcept
	{
		return impl->boundingBox.Center;
	}
	DirectX::XMFLOAT3 JMeshGeometry::GetBoundingBoxExtent()const noexcept
	{
		return impl->boundingBox.Extents;
	}
	DirectX::BoundingSphere JMeshGeometry::GetBoundingSphere()const noexcept
	{
		return impl->boundingSphere;
	}
	DirectX::XMFLOAT3 JMeshGeometry::GetBoundingSphereCenter()const noexcept
	{
		return impl->boundingSphere.Center;
	}
	float JMeshGeometry::GetBoundingSphereRadius()const noexcept
	{
		return impl->boundingSphere.Radius;
	}
	JMeshGroup* JMeshGeometry::GetMeshGroupData()const noexcept
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
		JResourceObject::DoDeActivate();
		impl->ClearResource();
		impl->OffResourceRef();
	}
	JMeshGeometry::JMeshGeometry(InitData& initData)
		: JResourceObject(initData), impl(std::make_unique<JMeshGeometryImpl>(initData, this))
	{ 
		impl->Initialize(initData);
	}
	JMeshGeometry::~JMeshGeometry()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JMeshGeometryPrivate::CreateInstanceInterface;
	using BufferViewInterface = JMeshGeometryPrivate::BufferViewInterface;

	void CreateInstanceInterface::TryDestroyUnUseData(Core::JIdentifier* createdPtr)noexcept
	{
		static_cast<JMeshGeometry*>(createdPtr)->impl->meshGroupData.reset();
	} 

	D3D12_VERTEX_BUFFER_VIEW BufferViewInterface::VertexBufferView(JMeshGeometry* mesh)noexcept
	{
		return mesh->impl->VertexBufferView();
	}
	D3D12_INDEX_BUFFER_VIEW BufferViewInterface::IndexBufferView(JMeshGeometry* mesh)noexcept
	{
		return mesh->impl->IndexBufferView();
	}
}