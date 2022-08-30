#pragma once
#include"JMeshInterface.h"
#include"JMeshStruct.h"
#include"JMeshType.h"
#include<d3d12.h> 
#include<wrl/client.h>

namespace JinEngine
{
	class JMeshGeometry : public JMeshInterface
	{
		REGISTER_CLASS(JMeshGeometry)
		// Give it a name so we can look it up by name.  
	private:
		// System memory copies.  Use Blobs because the vertex/index format can be generic.
		// It is up to the client to cast appropriately.  
		Microsoft::WRL::ComPtr<ID3DBlob> vertexBufferCPU = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> indexBufferCPU = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferGPU = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferGPU = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUploader = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUploader = nullptr;

		DirectX::BoundingBox boundingBox;
		DirectX::BoundingSphere boundingSphere;

		// Data about the buffers.
		uint vertexByteStride = 0;
		uint vertexBufferByteSize = 0;
		uint vertexCount = 0;
		DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
		uint indexBufferByteSize = 0;
		uint indexCount = 0; 
		std::vector<SubmeshGeometry> submeshes;
		J_MESHGEOMETRY_TYPE meshType; 
		bool hasNormal = false;
		bool hasUV = false;
	public: 
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const;
		D3D12_INDEX_BUFFER_VIEW IndexBufferView()const;
		void DisposeUploaders();
		uint GetVertexByteStride()const noexcept;
		uint GetVertexBufferByteSize()const noexcept;
		uint GetMeshVertexCount()const noexcept;
		uint GetIndexBufferByteSize()const noexcept;
		uint GetMeshIndexCount()const noexcept;
		DXGI_FORMAT GetIndexFormat()const noexcept;

		uint GetSubmeshIndexCount(int i)const noexcept;
		uint GetSubmeshStartIndexLocation(int i)const noexcept;
		int GetSubmeshBaseVertexLocation(int i)const noexcept;
		DirectX::XMFLOAT3 GetBoundingBoxCenter()const noexcept;
		DirectX::XMFLOAT3 GetBoundingBoxExtent()const noexcept;
		DirectX::XMFLOAT3 GetBoundingSphereCenter()const noexcept;
		float GetBoundingSphereRadius()const noexcept;

		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::MESH;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
		J_MESHGEOMETRY_TYPE GetMeshGeometryType()const noexcept;

		bool HasUV()const noexcept;
		bool HasNormal()const noexcept;
	public:
		bool Copy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void StuffResource() final;
		void ClearResource() final;
		//for .mesh file
		bool ReadMeshData();
	private:
		bool StuffStaticMesh(JStaticMeshData& meshData, const DirectX::BoundingBox& boundingBox, const DirectX::BoundingSphere& boundingSphere)final;
		bool StuffSkinnedMesh(JSkinnedMeshData& meshData, const DirectX::BoundingBox& boundingBox, const DirectX::BoundingSphere& boundingSphere)final;
	private:
		void MakeBoundingCollider(const std::vector<JStaticMeshVertex>& vertices)noexcept;
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JMeshGeometry* mesh);
		static JMeshGeometry* LoadObject(JDirectory* directory, const JResourcePathData& pathData);
		static void RegisterJFunc();
	private:
		JMeshGeometry(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
		~JMeshGeometry();
	};

}
