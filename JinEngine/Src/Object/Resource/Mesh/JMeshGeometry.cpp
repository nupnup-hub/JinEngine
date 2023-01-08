#include"JMeshGeometry.h" 
#include"JStaticMeshGeometry.h"
#include"JSkinnedMeshGeometry.h"

#include"../JResourceManager.h"
#include"../JResourceObjectFactory.h"
#include"../JResourceImporter.h"
#include"../Material/JMaterial.h"
#include"../../Directory/JDirectory.h"
#include"../../../Application/JApplicationVariable.h" 
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
#include<fstream>

namespace JinEngine
{
	using namespace DirectX;

	JMeshGeometry::JMeshInitData::JMeshInitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		JDirectory* directory,
		const std::wstring oridataPath,
		Core::JOwnerPtr<JMeshGroup> meshGroup)
		:JResourceInitData(name, guid, flag, directory, JResourceObject::GetFormatIndex<JMeshGeometry>(JCUtil::DecomposeFileFormat(oridataPath))),
		meshGroup(std::move(meshGroup))
	{}
	JMeshGeometry::JMeshInitData::JMeshInitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		JDirectory* directory,
		const uint8 formatIndex,
		Core::JOwnerPtr<JMeshGroup> meshGroup)
		: JResourceInitData(name, guid, flag, directory, formatIndex),
		meshGroup(std::move(meshGroup))
	{}
	JMeshGeometry::JMeshInitData::JMeshInitData(const std::wstring& name,
		JDirectory* directory,
		const std::wstring oridataPath,
		Core::JOwnerPtr<JMeshGroup> meshGroup)
		: JResourceInitData(name, directory, JResourceObject::GetFormatIndex<JMeshGeometry>(JCUtil::DecomposeFileFormat(oridataPath))),
		meshGroup(std::move(meshGroup))
	{}
	bool JMeshGeometry::JMeshInitData::IsValidCreateData()
	{
		if (JResourceInitData::IsValidCreateData() && meshGroup.IsValid())
			return true;
		else
			return false;
	}
	J_RESOURCE_TYPE JMeshGeometry::JMeshInitData::GetResourceType() const noexcept
	{
		return J_RESOURCE_TYPE::MESH;
	}

	JMeshGeometry::SubmeshGeometry::SubmeshGeometry(const size_t guid)
		:guid(guid)
	{}
	JMeshGeometry::SubmeshGeometry::~SubmeshGeometry()
	{
	}
	JMaterial* JMeshGeometry::SubmeshGeometry::GetMaterial()const noexcept
	{
		return material;
	}
	uint JMeshGeometry::SubmeshGeometry::GetVertexCount()const noexcept
	{
		return vertexCount;
	}
	uint JMeshGeometry::SubmeshGeometry::GetVertexStart()const noexcept
	{
		return vertexStrat;
	}
	uint JMeshGeometry::SubmeshGeometry::GetIndexStart()const noexcept
	{
		return indexStart;
	}
	uint JMeshGeometry::SubmeshGeometry::GetIndexCount()const noexcept
	{
		return indexCount;
	}
	DirectX::XMFLOAT3 JMeshGeometry::SubmeshGeometry::GetBoundingBoxCenter()const noexcept
	{
		return boundingBox.Center;
	}
	DirectX::XMVECTOR JMeshGeometry::SubmeshGeometry::GetBoundingBoxCenterV()const noexcept
	{
		return XMLoadFloat3(&boundingBox.Center);
	}
	DirectX::XMFLOAT3 JMeshGeometry::SubmeshGeometry::GetBoundingBoxExtent()const noexcept
	{
		return boundingBox.Extents;
	}
	DirectX::XMVECTOR JMeshGeometry::SubmeshGeometry::GetBoundingBoxExtentV()const noexcept
	{
		return XMLoadFloat3(&boundingBox.Extents);
	}
	DirectX::XMFLOAT3 JMeshGeometry::SubmeshGeometry::GetBoundingSphereCenter()const noexcept
	{
		return boundingSphere.Center;
	}
	float JMeshGeometry::SubmeshGeometry::GetBoundingSphereRadius()const noexcept
	{
		return boundingSphere.Radius;
	}
	void JMeshGeometry::SubmeshGeometry::SetMesh(const JMeshData& meshData, const uint vertexSt, const uint indexSt)
	{
		Core::JUserPtr<JMaterial> meshMaterial = meshData.GetMaterial();
		if (meshMaterial.IsValid())
			material = meshMaterial.Get();
		else
			material = JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_STANDARD);

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
	void JMeshGeometry::SubmeshGeometry::SetMaterial(JMaterial* newMaterial)noexcept
	{
		material = newMaterial;
	}
	bool JMeshGeometry::SubmeshGeometry::HasUV()const noexcept
	{
		return hasUV;
	}
	bool JMeshGeometry::SubmeshGeometry::HasNormal()const noexcept
	{
		return hasNormal;
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
		return vertexCount;
	}
	uint JMeshGeometry::GetTotalIndexCount()const noexcept
	{
		return indexCount;
	}
	uint JMeshGeometry::GetTotalSubmeshCount()const noexcept
	{
		return (uint)submeshes.size();
	}
	uint JMeshGeometry::GetSubmeshVertexCount(const uint index)const noexcept
	{
		return submeshes.size() > index ? submeshes[index].GetVertexCount() : 0;
	}
	uint JMeshGeometry::GetSubmeshIndexCount(const uint index)const noexcept
	{
		return submeshes.size() > index ? submeshes[index].GetIndexCount() : 0;
	}
	uint JMeshGeometry::GetSubmeshBaseVertexLocation(const uint index)const noexcept
	{
		return submeshes.size() > index ? submeshes[index].GetVertexStart() : 0;
	}
	uint JMeshGeometry::GetSubmeshStartIndexLocation(const uint index)const noexcept
	{
		return submeshes.size() > index ? submeshes[index].GetIndexStart() : 0;
	}
	JMaterial* JMeshGeometry::GetSubmeshMaterial(const uint index)const noexcept
	{
		return submeshes.size() > index ? submeshes[index].GetMaterial() : nullptr;
	}
	DirectX::BoundingBox JMeshGeometry::GetBoundingBox()const noexcept
	{
		return boundingBox;
	}
	DirectX::XMFLOAT3 JMeshGeometry::GetBoundingBoxCenter()const noexcept
	{
		return boundingBox.Center;
	}
	DirectX::XMFLOAT3 JMeshGeometry::GetBoundingBoxExtent()const noexcept
	{
		return boundingBox.Extents;
	}
	DirectX::BoundingSphere JMeshGeometry::GetBoundingSphere()const noexcept
	{
		return boundingSphere;
	}
	DirectX::XMFLOAT3 JMeshGeometry::GetBoundingSphereCenter()const noexcept
	{
		return boundingSphere.Center;
	}
	float JMeshGeometry::GetBoundingSphereRadius()const noexcept
	{
		return boundingSphere.Radius;
	}
	D3D12_VERTEX_BUFFER_VIEW JMeshGeometry::VertexBufferView()const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = vertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = vertexByteStride;
		vbv.SizeInBytes = vertexBufferByteSize;
		return vbv;
	}
	D3D12_INDEX_BUFFER_VIEW JMeshGeometry::IndexBufferView()const
	{ 
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = indexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = indexFormat;
		ibv.SizeInBytes = indexBufferByteSize;
		return ibv;
	}
	void JMeshGeometry::CalculateMeshBound()noexcept
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
	JMeshInterface* JMeshGeometry::Interface()noexcept
	{
		return this;
	}
	void JMeshGeometry::DoCopy(JObject* ori)
	{
		JMeshGeometry* oriM = static_cast<JMeshGeometry*>(ori);
		CopyRFile(*oriM, true);
		ClearResource();
		StuffResource();
		for (uint i = 0; i < oriM->submeshes.size(); ++i)
			submeshes[i].SetMaterial(oriM->submeshes[i].GetMaterial());
	}
	void JMeshGeometry::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
		StuffResource();
	}
	void JMeshGeometry::DoDeActivate()noexcept
	{
		std::wofstream stream;
		stream.open(GetMetafilePath(), std::ios::out | std::ios::binary);
		StoreMetadata(stream, this);
		stream.close();

		JResourceObject::DoDeActivate();
		ClearResource();
	}
	bool JMeshGeometry::StuffSubMesh(JMeshGroup& meshGroup)
	{
		//SutffSubMesh는 두가지 경우 호출된다
		//1) Activated시
		//2) Import시
		//Import시에는 원본데이터를 엔진에서 활용할수있게 변환한뒤 저장하고 메모리에서 지우므로
		//Clear에서 Activated을 조건문으로 설정하면 안된다.

		Clear();
		const uint submeshCount = (uint)meshGroup.GetMeshDataCount();
		uint vertexCount = 0;
		uint indexCount = 0;
		if (submeshes.size() != submeshCount)
		{
			submeshes.clear();
			for (uint i = 0; i < submeshCount; ++i)
				submeshes.emplace_back(meshGroup.GetMeshData(i)->GetGuid());
		}

		for (uint i = 0; i < submeshCount; ++i)
		{
			if (meshGroup.GetMeshData(i)->GetMeshType() == J_MESHGEOMETRY_TYPE::STATIC)
				submeshes[i].SetMesh(*static_cast<JStaticMeshData*>(meshGroup.GetMeshData(i)), vertexCount, indexCount);
			else
				submeshes[i].SetMesh(*static_cast<JSkinnedMeshData*>(meshGroup.GetMeshData(i)), vertexCount, indexCount);

			vertexCount += submeshes[i].GetVertexCount();
			indexCount += submeshes[i].GetIndexCount();
		}
		JMeshGeometry::vertexCount = vertexCount;
		JMeshGeometry::indexCount = indexCount;

		ID3D12Device* device = JGraphic::Instance().DeviceInterface()->GetDevice();
		ID3D12CommandQueue* mCommandQueue = JGraphic::Instance().CommandInterface()->GetCommandQueue();
		ID3D12CommandAllocator* mDirectCmdListAlloc = JGraphic::Instance().CommandInterface()->GetCommandAllocator();
		ID3D12GraphicsCommandList* mCommandList = JGraphic::Instance().CommandInterface()->GetCommandList();

		if (meshGroup.GetMeshGroupType() == J_MESHGEOMETRY_TYPE::STATIC)
		{
			vertexByteStride = sizeof(JStaticMeshVertex);
			vertexBufferByteSize = vertexCount * vertexByteStride;

			std::vector<JStaticMeshVertex> vertices(vertexCount);
			uint vertexOffset = 0;
			for (uint i = 0; i < submeshCount; ++i)
			{
				JStaticMeshData* meshdata = static_cast<JStaticMeshData*>(meshGroup.GetMeshData(i));
				const uint subMeshVertexCount = meshdata->GetVertexCount();
				for (uint j = 0; j < subMeshVertexCount; ++j)
					vertices[vertexOffset + j] = meshdata->GetVertex(j);
				vertexOffset += subMeshVertexCount;
			}

			JGraphic::Instance().CommandInterface()->FlushCommandQueue();
			JGraphic::Instance().CommandInterface()->StartCommand();
			vertexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, vertices.data(), vertexBufferByteSize, vertexBufferUploader);
			JGraphic::Instance().CommandInterface()->EndCommand();
			JGraphic::Instance().CommandInterface()->FlushCommandQueue();
		}
		else
		{
			vertexByteStride = sizeof(JSkinnedMeshVertex);
			vertexBufferByteSize = vertexCount * vertexByteStride;

			std::vector<JSkinnedMeshVertex> vertices(vertexCount);
			uint vertexOffset = 0;
			for (uint i = 0; i < submeshCount; ++i)
			{
				JSkinnedMeshData* meshdata = static_cast<JSkinnedMeshData*>(meshGroup.GetMeshData(i));
				const uint subMeshVertexCount = meshdata->GetVertexCount();
				for (uint j = 0; j < subMeshVertexCount; ++j)
					vertices[vertexOffset + j] = meshdata->GetVertex(j);
				vertexOffset += subMeshVertexCount;
			}
			JGraphic::Instance().CommandInterface()->FlushCommandQueue();
			JGraphic::Instance().CommandInterface()->StartCommand();
			vertexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, vertices.data(), vertexBufferByteSize, vertexBufferUploader);
			JGraphic::Instance().CommandInterface()->EndCommand();
			JGraphic::Instance().CommandInterface()->FlushCommandQueue();
		}

		if (indexCount >= 1 << 16)
		{
			indexFormat = DXGI_FORMAT_R32_UINT;
			indexBufferByteSize = indexCount * sizeof(uint32);
			std::vector<uint32> indices32(indexCount);

			uint indicesOffset = 0;
			for (uint i = 0; i < submeshCount; ++i)
			{
				const JMeshData* meshdata = meshGroup.GetMeshData(i);
				if (meshdata->Is16bit())
				{
					const uint subMeshIndexCount = meshdata->GetIndexCount();
					for (uint j = 0; j < subMeshIndexCount; ++j)
						indices32[indicesOffset + j] = meshdata->GetIndex(j);
					indicesOffset += subMeshIndexCount;
				}
				else
				{
					const uint subMeshIndexCount = meshdata->GetIndexCount();
					for (uint j = 0; j < subMeshIndexCount; ++j)
						indices32[indicesOffset + j] = meshdata->GetIndex(j);
					indicesOffset += subMeshIndexCount;
				}
			}
			JGraphic::Instance().CommandInterface()->FlushCommandQueue();
			JGraphic::Instance().CommandInterface()->StartCommand();
			indexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, indices32.data(), indexBufferByteSize, indexBufferUploader);
			JGraphic::Instance().CommandInterface()->EndCommand();
			JGraphic::Instance().CommandInterface()->FlushCommandQueue();
		}
		else
		{
			indexFormat = DXGI_FORMAT_R16_UINT;
			indexBufferByteSize = indexCount * sizeof(uint16);
			std::vector<uint16> indices16(indexCount);
			uint indicesOffset = 0;

			for (uint i = 0; i < submeshCount; ++i)
			{
				const JMeshData* meshdata = meshGroup.GetMeshData(i);
				if (meshdata->Is16bit())
				{
					const uint subMeshIndexCount = meshdata->GetIndexCount();
					for (uint j = 0; j < subMeshIndexCount; ++j)
						indices16[indicesOffset + j] = meshdata->GetIndex(j);
					indicesOffset += subMeshIndexCount;
				}
				else
				{
					const uint subMeshIndexCount = meshdata->GetIndexCount();
					for (uint j = 0; j < subMeshIndexCount; ++j)
						indices16[indicesOffset + j] = meshdata->GetIndex(j);
					indicesOffset += subMeshIndexCount;
				}
			}
			JGraphic::Instance().CommandInterface()->FlushCommandQueue();
			JGraphic::Instance().CommandInterface()->StartCommand();
			indexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, indices16.data(), indexBufferByteSize, indexBufferUploader);
			JGraphic::Instance().CommandInterface()->EndCommand();
			JGraphic::Instance().CommandInterface()->FlushCommandQueue();
		}
		for (uint i = 0; i < submeshCount; ++i)
			CallOnResourceReference(submeshes[i].GetMaterial());
		CalculateMeshBound();

		vertexBufferUploader.Reset();
		indexBufferUploader.Reset();
		return true;
	}
	void JMeshGeometry::StuffResource()
	{
		if (!IsValid())
		{
			if (GetFormatIndex() != GetInvalidFormatIndex())
			{
				if (ReadMeshData())
					SetValid(true);
			}
		}
	}
	void JMeshGeometry::ClearResource()
	{
		if (IsValid())
		{
			// vertexBufferCPU.Reset();
			//indexBufferCPU.Reset(); 
			Clear();
			SetValid(false);
		}
	}
	void JMeshGeometry::Clear()
	{ 
		const uint subMeshCount = (uint)submeshes.size();
		for (uint i = 0; i < subMeshCount; ++i)
		{
			if(submeshes[i].GetMaterial() != nullptr)
				CallOffResourceReference(submeshes[i].GetMaterial());
		}
		vertexBufferGPU.Reset();
		indexBufferGPU.Reset();
		vertexBufferUploader.Reset();
		indexBufferUploader.Reset();
	}
	void JMeshGeometry::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
	{
		if (iden == GetGuid())
			return;

		if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE && jRobj->GetResourceType() == J_RESOURCE_TYPE::MATERIAL)
		{
			JMaterial* defaultMat = JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_STANDARD);

			const size_t tarGuid = jRobj->GetGuid();
			const uint subMeshCount = (uint)submeshes.size();
			for (uint i = 0; i < subMeshCount; ++i)
			{
				JMaterial* preMat = submeshes[i].GetMaterial();
				if (preMat->GetGuid() == tarGuid)
				{ 
					if (defaultMat != nullptr)
					{
						submeshes[i].SetMaterial(defaultMat);
						if (IsActivated())
							CallOnResourceReference(defaultMat);
					}
					else
						submeshes[i].SetMaterial(nullptr);
				}
			}
		}
	}
	Core::J_FILE_IO_RESULT JMeshGeometry::CallStoreResource()
	{
		return StoreObject(this);
	}
	Core::J_FILE_IO_RESULT JMeshGeometry::StoreObject(JMeshGeometry* mesh)
	{ 
		if (mesh == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (((int)mesh->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		std::wofstream stream;
		stream.open(mesh->GetMetafilePath(), std::ios::out | std::ios::binary);
		Core::J_FILE_IO_RESULT storeMetaRes = StoreMetadata(stream, mesh);
		stream.close();
		return storeMetaRes;
	}
	Core::J_FILE_IO_RESULT JMeshGeometry::StoreMetadata(std::wofstream& stream, JMeshGeometry* mesh)
	{
		if (stream.is_open())
		{
			Core::J_FILE_IO_RESULT res = JResourceObject::StoreMetadata(stream, mesh);
			if (res != Core::J_FILE_IO_RESULT::SUCCESS)
				return res;

			JFileIOHelper::StoreEnumData(stream, L"MeshType:", mesh->GetMeshGeometryType());
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JMeshGeometry* JMeshGeometry::LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;
		 
		std::wifstream stream;
		stream.open(pathData.engineMetaFileWPath, std::ios::in | std::ios::binary);
		JMeshMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);
		stream.close();

		JMeshGeometry* newMesh = nullptr;
		if (directory->HasFile(pathData.name))
			newMesh = JResourceManager::Instance().GetResourceByPath<JMeshGeometry>(pathData.engineFileWPath);

		if (newMesh == nullptr && loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
		{
			JMeshInitData initdata{ pathData.name, metadata.guid,metadata.flag, directory, (uint8)metadata.formatIndex };
			if (initdata.IsValidLoadData())
			{
				if (metadata.meshType == J_MESHGEOMETRY_TYPE::STATIC)
				{
					Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JStaticMeshGeometry>(initdata);
					newMesh = ownerPtr.Get();
					if (!AddInstance(std::move(ownerPtr)))
						return nullptr;
				}
				else
				{
					Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JSkinnedMeshGeometry>(initdata);
					newMesh = ownerPtr.Get();
					if (!AddInstance(std::move(ownerPtr)))
						return nullptr;
				}
			}
		}
		return newMesh;
	}
	Core::J_FILE_IO_RESULT JMeshGeometry::LoadMetadata(std::wifstream& stream, JMeshMetadata& metadata)
	{
		if (stream.is_open())
		{
			Core::J_FILE_IO_RESULT res = JResourceObject::LoadMetadata(stream, metadata);
			if (res != Core::J_FILE_IO_RESULT::SUCCESS)
				return res;

			JFileIOHelper::LoadEnumData(stream, metadata.meshType);
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	void JMeshGeometry::RegisterJFunc()
	{
		auto loadC = [](JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		JRFI<JMeshGeometry>::Register(nullptr, loadC, nullptr);

		auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex<JMeshGeometry>(format); };
		static GetTypeNameCallable getTypeNameCallable{ &JMeshGeometry::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JMeshGeometry::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{J_RESOURCE_TYPE::MATERIAL, J_RESOURCE_TYPE::SKELETON}, true, false};
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{});

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
			if (HasSQValueEnum(info.typeInfo, Core::J_FBXRESULT::HAS_SKELETON))
			{
				JSkinnedMeshGroup skinnedGroup;
				std::vector<Joint> joint;
				Core::J_FBXRESULT loadRes = Core::JFbxFileLoader::Instance().LoadFbxMeshFile(importPathData.oriFilePath, skinnedGroup, joint);
				if (loadRes == Core::J_FBXRESULT::FAIL)
					return { nullptr };

				if (HasSQValueEnum(info.typeInfo, Core::J_FBXRESULT::HAS_MESH))
				{
					res.push_back(JRFI<JSkinnedMeshGeometry>::Create(Core::JPtrUtil::MakeOwnerPtr<InitData>(importPathData.name,
						Core::MakeGuid(),
						importPathData.flag,
						dir,
						importPathData.oriFileWPath,
						Core::JPtrUtil::MakeOwnerPtr<JSkinnedMeshGroup>(std::move(skinnedGroup)))));
				}
				if (HasSQValueEnum(info.typeInfo, Core::J_FBXRESULT::HAS_SKELETON))
				{
					res.push_back(JRFI<JSkeletonAsset>::Create(Core::JPtrUtil::MakeOwnerPtr<JSkeletonAsset::InitData>(importPathData.name + L"Skel",
						Core::MakeGuid(),
						importPathData.flag,
						dir,
						importPathData.oriFileWPath,
						std::make_unique<JSkeleton>(std::move(joint)))));
				}
			}
			else
			{
				JStaticMeshGroup staticMeshGroup;
				Core::J_FBXRESULT loadRes = Core::JFbxFileLoader::Instance().LoadFbxMeshFile(importPathData.oriFilePath, staticMeshGroup);
				if (loadRes == Core::J_FBXRESULT::FAIL)
					return { nullptr };

				if (HasSQValueEnum(info.typeInfo, Core::J_FBXRESULT::HAS_MESH))
				{
					res.push_back(JRFI<JStaticMeshGeometry>::Create(Core::JPtrUtil::MakeOwnerPtr<InitData>(importPathData.name,
						Core::MakeGuid(),
						importPathData.flag,
						dir,
						importPathData.oriFileWPath,
						Core::JPtrUtil::MakeOwnerPtr<JStaticMeshGroup>(std::move(staticMeshGroup)))));
				}
			}
			return res;
		};
		auto objMeshImportC = [](JDirectory* dir, const Core::JFileImportHelpData importPathData) -> std::vector<JResourceObject*>
		{
			std::vector<JResourceObject*> res;

			Core::JObjFileMeshData objMeshData;
			std::vector<Core::JObjFileMatData> objMatData;

			if (JObjFileLoader::Instance().LoadObjFile(importPathData, objMeshData, objMatData))
			{
				res.push_back(JRFI<JStaticMeshGeometry>::Create(Core::JPtrUtil::MakeOwnerPtr<InitData>(importPathData.name,
					Core::MakeGuid(),
					importPathData.flag,
					dir,
					importPathData.oriFileWPath,
					Core::JPtrUtil::MakeOwnerPtr<JStaticMeshGroup>(std::move(objMeshData.meshGroup)))));
			}
			else
				return { nullptr };
			return res;
		};

		JResourceImporter::Instance().AddFormatInfo(L".fbx", J_RESOURCE_TYPE::MESH, fbxMeshImportC, fbxClassifyC);
		JResourceImporter::Instance().AddFormatInfo(L".obj", J_RESOURCE_TYPE::MESH, objMeshImportC);
	}
	JMeshGeometry::JMeshGeometry(const JMeshInitData& initdata)
		: JMeshInterface(initdata)
	{}
	JMeshGeometry::~JMeshGeometry()
	{

	}
}