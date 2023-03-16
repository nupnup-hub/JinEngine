#pragma once
#include"JMeshInterface.h"
#include"JMeshStruct.h"
#include"JMeshType.h"
#include"../JResourceUserInterface.h"
#include<d3d12.h> 
#include<wrl/client.h>

namespace JinEngine
{
	class JMaterial;
	class JMeshGeometry : public JMeshInterface,
		public JResourceUserInterface
	{
		REGISTER_CLASS(JMeshGeometry)
	public:
		struct JMeshInitData : JResourceInitData
		{
		public:
			Core::JOwnerPtr<JMeshGroup> meshGroup;
		public:
			JMeshInitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				JDirectory* directory,
				const std::wstring oridataPath,
				Core::JOwnerPtr<JMeshGroup> meshGroup = nullptr);
			JMeshInitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				JDirectory* directory,
				const uint8 formatIndex,
				Core::JOwnerPtr<JMeshGroup> meshGroup = nullptr);
			JMeshInitData(const std::wstring& name,
				JDirectory* directory,
				const std::wstring oridataPath,
				Core::JOwnerPtr<JMeshGroup> meshGroup = nullptr);
		public:
			bool IsValidCreateData()final; 
		};
		using InitData = JMeshInitData;
	private:
		struct JMeshMetadata : public JResourceMetaData
		{
		public:
			J_MESHGEOMETRY_TYPE meshType; 
		};
	private:
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
			SubmeshGeometry(const std::wstring name, const size_t guid);
			~SubmeshGeometry();
		public:
			std::wstring GetName()const noexcept;
			Core::JUserPtr<JMaterial> GetMaterial()const noexcept;
			uint GetVertexCount()const noexcept;
			uint GetVertexStart()const noexcept; 
			uint GetIndexCount()const noexcept;
			uint GetIndexStart()const noexcept;  
			DirectX::XMFLOAT3 GetBoundingBoxCenter()const noexcept;
			DirectX::XMVECTOR GetBoundingBoxCenterV()const noexcept;
			DirectX::XMFLOAT3 GetBoundingBoxExtent()const noexcept;
			DirectX::XMVECTOR GetBoundingBoxExtentV()const noexcept;
			DirectX::XMFLOAT3 GetBoundingSphereCenter()const noexcept;
			float GetBoundingSphereRadius()const noexcept;
		public:
			void SetMesh(const JMeshData& meshData, const uint vertexSt, const uint indexSt);
			void SetMaterial(Core::JUserPtr<JMaterial> newMaterial)noexcept;
		public:
			bool HasUV()const noexcept;
			bool HasNormal()const noexcept;
		};
	private:
		// System memory copies.  Use Blobs because the vertex/index format can be generic.
		// It is up to the client to cast appropriately.  
		//Microsoft::WRL::ComPtr<ID3DBlob> vertexBufferCPU = nullptr;
		//Microsoft::WRL::ComPtr<ID3DBlob> indexBufferCPU = nullptr;
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferGPU = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferGPU = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUploader = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUploader = nullptr;

		DirectX::BoundingBox boundingBox;
		DirectX::BoundingSphere boundingSphere;

		// Data about the buffers.
		uint vertexCount = 0;
		uint indexCount = 0; 
		uint vertexByteStride = 0;
		uint vertexBufferByteSize = 0;
		DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
		uint indexBufferByteSize = 0;
		std::vector<SubmeshGeometry> submeshes; 
	public: 
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::MESH;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
		virtual J_MESHGEOMETRY_TYPE GetMeshGeometryType()const noexcept = 0;

		uint GetTotalVertexCount()const noexcept;
		uint GetTotalIndexCount()const noexcept;
		uint GetTotalSubmeshCount()const noexcept;
		uint GetSubmeshVertexCount(const uint index)const noexcept;
		uint GetSubmeshIndexCount(const uint index)const noexcept;
		uint GetSubmeshBaseVertexLocation(const uint index)const noexcept;
		uint GetSubmeshStartIndexLocation(const uint index)const noexcept;
		std::wstring GetSubMeshName(const uint index)const noexcept;
		Core::JUserPtr<JMaterial> GetSubmeshMaterial(const uint index)const noexcept;
		DirectX::BoundingBox GetBoundingBox()const noexcept;
		DirectX::XMFLOAT3 GetBoundingBoxCenter()const noexcept;
		DirectX::XMFLOAT3 GetBoundingBoxExtent()const noexcept;
		DirectX::BoundingSphere GetBoundingSphere()const noexcept;
		DirectX::XMFLOAT3 GetBoundingSphereCenter()const noexcept;
		float GetBoundingSphereRadius()const noexcept;
	public:
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const;
		D3D12_INDEX_BUFFER_VIEW IndexBufferView()const;
	private:
		void CalculateMeshBound()noexcept;
	public:
		JMeshInterface* Interface()noexcept;
	private:
		void DoCopy(JObject* ori) final; 
	protected:
		void DoActivate()noexcept override;
		void DoDeActivate()noexcept override;
	protected:
		//derivation class this func when doactivate
		bool StuffSubMesh(JMeshGroup& meshGroup);
	private:
		void StuffResource() final;
		void ClearResource() final;
		virtual bool WriteMeshData(JMeshGroup& meshGroup) = 0;
		virtual bool ReadMeshData() = 0;
		virtual bool ImportMesh(JMeshGroup& meshGroup) = 0;
	protected:
		void ClearGpuBuffer();
	protected:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)override;
	protected:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JMeshGeometry* mesh);
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JMeshGeometry* mesh);
		static JMeshGeometry* LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData);
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, JMeshMetadata& metadata);
	protected:
		static void RegisterImportFunc();
	private:
		static void RegisterJFunc();
	protected:
		JMeshGeometry(const JMeshInitData& initdata);
		~JMeshGeometry();
	};

}
