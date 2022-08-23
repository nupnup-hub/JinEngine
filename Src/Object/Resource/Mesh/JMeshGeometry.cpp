#include"JMeshGeometry.h" 
#include"../JResourceObjectFactory.h"
#include"../../Directory/JDirectory.h"
#include"../../../Application/JApplicationVariable.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/Exception/JExceptionMacro.h" 
#include"../../../Utility/JMathHelper.h" 
#include"../../../Utility/JD3DUtility.h" 
#include"../../../Graphic/JGraphic.h"

namespace JinEngine
{
	using namespace DirectX;
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

	// We can free this memory after we finish upload to the GPU.
	void JMeshGeometry::DisposeUploaders()
	{
		vertexBufferUploader = nullptr;
		indexBufferUploader = nullptr;
	}

	uint JMeshGeometry::GetVertexByteStride()const noexcept
	{
		return vertexByteStride;
	}
	uint JMeshGeometry::GetVertexBufferByteSize()const noexcept
	{
		return vertexBufferByteSize;
	}
	uint JMeshGeometry::GetMeshVertexCount()const noexcept
	{
		return vertexCount;
	}
	uint JMeshGeometry::GetIndexBufferByteSize()const noexcept
	{
		return indexBufferByteSize;
	}
	DXGI_FORMAT JMeshGeometry::GetIndexFormat()const noexcept
	{
		return indexFormat;
	}
	uint JMeshGeometry::GetMeshIndexCount()const noexcept
	{
		return indexCount;
	}
	uint JMeshGeometry::GetSubmeshIndexCount(int i)const noexcept
	{
		return submeshes[i].IndexCount;
	}
	uint JMeshGeometry::GetSubmeshStartIndexLocation(int i)const noexcept
	{
		return submeshes[i].StartIndexLocation;
	}
	int JMeshGeometry::GetSubmeshBaseVertexLocation(int i)const noexcept
	{
		return submeshes[i].BaseVertexLocation;
	}
	DirectX::XMFLOAT3 JMeshGeometry::GetBoundingBoxCenter()const noexcept
	{
		return boundingBox.Center;
	}
	DirectX::XMFLOAT3 JMeshGeometry::GetBoundingBoxExtent()const noexcept
	{
		return boundingBox.Extents;
	}
	DirectX::XMFLOAT3 JMeshGeometry::GetBoundingSphereCenter()const noexcept
	{
		return boundingSphere.Center;
	}
	float JMeshGeometry::GetBoundingSphereRadius()const noexcept
	{
		return boundingSphere.Radius;
	}
	J_RESOURCE_TYPE JMeshGeometry::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	} 
	std::string JMeshGeometry::GetFormat()const noexcept
	{
		return GetAvailableFormat()[formatIndex];
	}
	std::vector<std::string> JMeshGeometry::GetAvailableFormat()noexcept
	{
		static std::vector<std::string> format{ ".mesh", ".obj", ".fbx" };
		return format;
	}
	J_MESHGEOMETRY_TYPE JMeshGeometry::GetMeshGeometryType()const noexcept
	{
		return meshType;
	}
	bool JMeshGeometry::HasUV()const noexcept
	{
		return hasUV;
	}
	bool JMeshGeometry::HasNormal()const noexcept
	{
		return hasNormal;
	}
	void JMeshGeometry::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
		StuffResource();
	}
	void JMeshGeometry::DoDeActivate()noexcept
	{
		JResourceObject::DoDeActivate();
		ClearResource();
	}
	void JMeshGeometry::StuffResource()
	{
		//구현필요
		// 0 == mesh 
		if (!IsValid())
		{
			if (formatIndex == 0)
			{
				;//if(Load Mesh());
				if(ReadMeshData())
					SetValid(true);
			}
		}
	}
	void JMeshGeometry::ClearResource()
	{
		//구현필요
		// 0 == mesh 
		if (IsValid())
		{
			if (formatIndex == 0)
			{
				vertexBufferCPU.Reset();
				indexBufferCPU.Reset();
				vertexBufferGPU.Reset();
				indexBufferGPU.Reset();
				vertexBufferUploader.Reset();
				indexBufferUploader.Reset();

				SetValid(false);
				;//Clear Mesh;
			}
		}
	}
	bool JMeshGeometry::ReadMeshData()
	{
		const JResourcePathData pathData{ GetWPath() };
		std::wifstream stream;
		stream.open(pathData.wstrPath, std::ios::in | std::ios::binary);
		if (stream.is_open())
		{
			int vertexCount;
			int indexCount;
			int meshType;

			stream >> vertexCount;
			stream >> indexCount;
			stream >> meshType;
			if (meshType == (int)J_MESHGEOMETRY_TYPE::STATIC)
			{
				JStaticMeshData staticMeshdata;
				BoundingBox boundingBox;
				BoundingSphere boundingSphere;

				const XMFLOAT3 vMinf3(+JMathHelper::Infinity, +JMathHelper::Infinity, +JMathHelper::Infinity);
				const XMFLOAT3 vMaxf3(-JMathHelper::Infinity, -JMathHelper::Infinity, -JMathHelper::Infinity);
				XMVECTOR vMin = XMLoadFloat3(&vMinf3);
				XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

				for (int i = 0; i < vertexCount; ++i)
				{
					XMFLOAT3 position;
					XMFLOAT3 normal;
					XMFLOAT2 texC;
					XMFLOAT3 tangentU;

					stream >> position.x >> position.y >> position.z;
					stream >> normal.x >> normal.y >> normal.z;
					stream >> texC.x >> texC.y;
					stream >> tangentU.x >> tangentU.y >> tangentU.z;

					XMVECTOR P = XMLoadFloat3(&position);
					vMin = XMVectorMin(vMin, P);
					vMax = XMVectorMax(vMax, P);

					staticMeshdata.vertices.emplace_back(JStaticMeshVertex(position, normal, texC, tangentU));
				}

				XMStoreFloat3(&boundingBox.Center, 0.5f * (vMin + vMax));
				XMStoreFloat3(&boundingBox.Extents, 0.5f * (vMax - vMin));
				XMStoreFloat3(&boundingSphere.Center, 0.5f * (vMin + vMax));
				XMFLOAT3 dis;
				XMStoreFloat3(&dis, XMVector3Length((0.5f * (vMin + vMax)) - vMax));
				boundingSphere.Radius = (float)sqrt(pow(dis.x, 2) + pow(dis.y, 2) + pow(dis.z, 2));

				for (int i = 0; i < indexCount; ++i)
				{
					int index;
					stream >> index;
					staticMeshdata.indices32.emplace_back(index);
				}

				StuffStaticMesh(staticMeshdata, boundingBox, boundingSphere);
				stream.close();				 
				return true;
			}
			else
			{
				//수정필요
				//skinned 미구현 engine내부에서 skeleton 생성이 전제조건
				stream.close();
				return false;
			}
		}
		else
			return false;
	}
	bool JMeshGeometry::StuffStaticMesh(JStaticMeshData& meshData, const DirectX::BoundingBox& boundingBox, const DirectX::BoundingSphere& boundingSphere)
	{
		if (vertexCount == 0 && meshData.vertices.size() < 0)
		{
			ID3D12Device* device = JGraphic::Instance().DeviceInterface()->GetDevice();
			ID3D12CommandQueue* mCommandQueue = JGraphic::Instance().CommandInterface()->GetCommandQueue();
			ID3D12CommandAllocator* mDirectCmdListAlloc = JGraphic::Instance().CommandInterface()->GetCommandAllocator();
			ID3D12GraphicsCommandList* mCommandList = JGraphic::Instance().CommandInterface()->GetCommandList();

			uint vbByteSize = (uint)meshData.vertices.size() * sizeof(JStaticMeshVertex);
			if (meshData.indices32.size() > 65535)
			{
				const uint ibByteSize = (uint)meshData.indices32.size() * sizeof(uint32_t);
				ThrowIfFailedHr(D3DCreateBlob(vbByteSize, &vertexBufferCPU));
				CopyMemory(vertexBufferCPU->GetBufferPointer(), meshData.vertices.data(), vbByteSize);

				ThrowIfFailedHr(D3DCreateBlob(ibByteSize, &indexBufferCPU));
				CopyMemory(indexBufferCPU->GetBufferPointer(), meshData.indices32.data(), ibByteSize);

				vertexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, meshData.vertices.data(), vbByteSize, vertexBufferUploader);
				indexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, meshData.indices32.data(), ibByteSize, indexBufferUploader);

				vertexByteStride = sizeof(JStaticMeshVertex);
				vertexBufferByteSize = vbByteSize;
				vertexCount = (uint)meshData.vertices.size();
				indexFormat = DXGI_FORMAT_R32_UINT;
				indexBufferByteSize = ibByteSize;
				indexCount = (uint)meshData.indices32.size();
			}
			else
			{
				if (meshData.IsIndices16Empty())
					meshData.Stuff8ByteDataTo4Byte();

				const uint ibByteSize = (uint)meshData.indices16.size() * sizeof(std::uint16_t);
				ThrowIfFailedHr(D3DCreateBlob(vbByteSize, &vertexBufferCPU));
				CopyMemory(vertexBufferCPU->GetBufferPointer(), meshData.vertices.data(), vbByteSize);

				ThrowIfFailedHr(D3DCreateBlob(ibByteSize, &indexBufferCPU));
				CopyMemory(indexBufferCPU->GetBufferPointer(), meshData.indices16.data(), ibByteSize);

				vertexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, meshData.vertices.data(), vbByteSize, vertexBufferUploader);
				indexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, meshData.indices16.data(), ibByteSize, indexBufferUploader);

				vertexByteStride = sizeof(JStaticMeshVertex);
				vertexBufferByteSize = vbByteSize;
				vertexCount = (uint)meshData.vertices.size();
				indexFormat = DXGI_FORMAT_R16_UINT;
				indexBufferByteSize = ibByteSize;
				indexCount = (uint)meshData.indices16.size();
			}
			JMeshGeometry::boundingBox = boundingBox;
			JMeshGeometry::boundingSphere = boundingSphere;

			SubmeshGeometry submesh;
			submesh.IndexCount = indexCount;
			submesh.BaseVertexLocation = 0;
			submesh.StartIndexLocation = 0;
			submeshes.push_back(submesh);
			meshType = J_MESHGEOMETRY_TYPE::STATIC;
			hasNormal = meshData.hasNormal;
			hasUV = meshData.hasUV;
			return true;
		}
		else
			return false;
	}
	bool JMeshGeometry::StuffSkinnedMesh(JSkinnedMeshData& meshData, const DirectX::BoundingBox& boundingBox, const DirectX::BoundingSphere& boundingSphere)
	{
		if (vertexCount == 0 && meshData.vertices.size() > 0)
		{
			ID3D12Device* device = JGraphic::Instance().DeviceInterface()->GetDevice();
			ID3D12CommandQueue* mCommandQueue = JGraphic::Instance().CommandInterface()->GetCommandQueue();
			ID3D12CommandAllocator* mDirectCmdListAlloc = JGraphic::Instance().CommandInterface()->GetCommandAllocator();
			ID3D12GraphicsCommandList* mCommandList = JGraphic::Instance().CommandInterface()->GetCommandList();

			uint vbByteSize = (uint)meshData.vertices.size() * sizeof(JSkinnedMeshVertex);
			if (meshData.indices32.size() > 65535)
			{
				const uint ibByteSize = (uint)meshData.indices32.size() * sizeof(uint32);
				ThrowIfFailedHr(D3DCreateBlob(vbByteSize, &vertexBufferCPU));
				CopyMemory(vertexBufferCPU->GetBufferPointer(), meshData.vertices.data(), vbByteSize);

				ThrowIfFailedHr(D3DCreateBlob(ibByteSize, &indexBufferCPU));
				CopyMemory(indexBufferCPU->GetBufferPointer(), meshData.indices32.data(), ibByteSize);

				vertexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, meshData.vertices.data(), vbByteSize, vertexBufferUploader);
				indexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, meshData.indices32.data(), ibByteSize, indexBufferUploader);

				vertexByteStride = sizeof(JSkinnedMeshVertex);
				vertexBufferByteSize = vbByteSize;
				vertexCount = (uint)meshData.vertices.size();
				indexFormat = DXGI_FORMAT_R32_UINT;
				indexBufferByteSize = ibByteSize;
				indexCount = (uint)meshData.indices32.size();
			}
			else
			{
				meshData.Stuff8ByteDataTo4Byte();
				const uint ibByteSize = (uint)meshData.indices16.size() * sizeof(std::uint16);
				ThrowIfFailedHr(D3DCreateBlob(vbByteSize, &vertexBufferCPU));
				CopyMemory(vertexBufferCPU->GetBufferPointer(), meshData.vertices.data(), vbByteSize);

				ThrowIfFailedHr(D3DCreateBlob(ibByteSize, &indexBufferCPU));
				CopyMemory(indexBufferCPU->GetBufferPointer(), meshData.indices16.data(), ibByteSize);

				vertexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, meshData.vertices.data(), vbByteSize, vertexBufferUploader);
				indexBufferGPU = JD3DUtility::CreateDefaultBuffer(device, mCommandList, meshData.indices16.data(), ibByteSize, indexBufferUploader);

				vertexByteStride = sizeof(JSkinnedMeshVertex);
				vertexBufferByteSize = vbByteSize;
				vertexCount = (uint)meshData.vertices.size();
				indexFormat = DXGI_FORMAT_R16_UINT;
				indexBufferByteSize = ibByteSize;
				indexCount = (uint)meshData.indices16.size();
			}
			JMeshGeometry::boundingBox = boundingBox;
			JMeshGeometry::boundingSphere = boundingSphere;

			SubmeshGeometry submesh;
			submesh.IndexCount = indexCount;
			submesh.BaseVertexLocation = 0;
			submesh.StartIndexLocation = 0;
			submeshes.push_back(submesh);
			meshType = J_MESHGEOMETRY_TYPE::SKINNED;
			hasNormal = meshData.hasNormal;
			hasUV = meshData.hasUV;
			return true;
		}
		else
			return false;
	}
	void JMeshGeometry::MakeBoundingCollider(const std::vector<JStaticMeshVertex>& vertices)noexcept
	{
		const XMFLOAT3 vMinf3(+JMathHelper::Infinity, +JMathHelper::Infinity, +JMathHelper::Infinity);
		const XMFLOAT3 vMaxf3(-JMathHelper::Infinity, -JMathHelper::Infinity, -JMathHelper::Infinity);
		XMVECTOR vMin = XMLoadFloat3(&vMinf3);
		XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

		for (int i = 0; i < vertices.size(); ++i)
		{
			const XMVECTOR position = XMLoadFloat3(&vertices[i].position);
			vMin = XMVectorMin(vMin, position);
			vMax = XMVectorMax(vMax, position);
		}

		XMStoreFloat3(&boundingBox.Center, 0.5f * (vMin + vMax));
		XMStoreFloat3(&boundingBox.Extents, 0.5f * (vMax - vMin));

		XMStoreFloat3(&boundingSphere.Center, 0.5f * (vMin + vMax));
		XMFLOAT3 dis;
		XMStoreFloat3(&dis, XMVector3Length((0.5f * (vMin + vMax)) - vMax));
		boundingSphere.Radius = (float)sqrt(pow(dis.x, 2) + pow(dis.y, 2) + pow(dis.z, 2));
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
		Core::J_FILE_IO_RESULT storeMetaRes = JResourceObject::StoreMetadata(stream, mesh);
		stream.close();
		return storeMetaRes;
	}
	JMeshGeometry* JMeshGeometry::LoadObject(JDirectory* directory, const JResourcePathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;
		 
		if (!JResourceObject::IsResourceFormat<JMeshGeometry>(pathData.format))
			return nullptr;

		std::wifstream stream;
		stream.open(ConvertMetafilePath(pathData.wstrPath), std::ios::in | std::ios::binary);
		ObjectMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);
		stream.close();

		JMeshGeometry* newMesh = nullptr;
		if (directory->HasFile(pathData.fullName))
			newMesh = JResourceManager::Instance().GetResourceByPath<JMeshGeometry>(pathData.strPath);

		if (newMesh == nullptr)
		{
			if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
			{
				newMesh = new JMeshGeometry(pathData.name, metadata.guid, metadata.flag, directory,
					JResourceObject::GetFormatIndex<JMeshGeometry>(pathData.format));
			}
			else
			{
				newMesh = new JMeshGeometry(pathData.name, Core::MakeGuid(), OBJECT_FLAG_NONE, directory,
					JResourceObject::GetFormatIndex<JMeshGeometry>(pathData.format));
			}
		}

		if (newMesh->IsValid())
			return newMesh;
		else if (newMesh->ReadMeshData())
		{
			newMesh->SetValid(true);
			return newMesh;
		}
		else
		{
			delete newMesh;
			return nullptr;
		}
	}
	void JMeshGeometry::RegisterJFunc()
	{
		auto defaultC = [](JDirectory* owner) ->JResourceObject*
		{
			return new JMeshGeometry(owner->MakeUniqueFileName(GetDefaultName<JMeshGeometry>()),
				Core::MakeGuid(),
				OBJECT_FLAG_NONE,
				owner,
				JResourceObject::GetDefaultFormatIndex());
		};
		auto initC = [](const std::string& name, const size_t guid, const J_OBJECT_FLAG objFlag, JDirectory* directory, const uint8 formatIndex)-> JResourceObject*
		{
			return  new JMeshGeometry(name, guid, objFlag, directory, formatIndex);
		};
		auto loadC = [](JDirectory* directory, const JResourcePathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori)->JResourceObject*
		{
			return static_cast<JMeshGeometry*>(ori)->CopyResource();
		};

		JRFI<JMeshGeometry>::Register(defaultC, initC, loadC, copyC);

		auto getFormatIndexLam = [](const std::string& format) {return JResourceObject::GetFormatIndex<JMeshGeometry>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JMeshGeometry::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JMeshGeometry::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };
		 
		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, true, false, false };
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{});
	}
	JMeshGeometry::JMeshGeometry(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		: JMeshInterface(name, guid, flag, directory, formatIndex)
	{}

	JMeshGeometry::~JMeshGeometry()
	{

	}
}