#pragma once
#include"JMeshType.h"
#include"../../../Core/JDataType.h"
#include"../../../Core/Pointer/JOwnerPtr.h"
#include"../../../Utility/JVector.h"  
#include<vector>
#include<DirectXMath.h>
#include<DirectXCollision.h>

namespace JinEngine
{ 
	struct JBlendingIndexWeightPair
	{
	public:
		uint8 blendingIndex = 0;
		float blendingWeight = 0;
	public:
		JBlendingIndexWeightPair() = default;
		JBlendingIndexWeightPair(const JBlendingIndexWeightPair& data);
	};

	struct JStaticMeshVertex
	{
	public:
		DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0, 0, 0);
		DirectX::XMFLOAT3 normal = DirectX::XMFLOAT3(0, 0, 0);
		DirectX::XMFLOAT2 texC = DirectX::XMFLOAT2(0, 0);
		DirectX::XMFLOAT3 tangentU = DirectX::XMFLOAT3(0, 0, 0);
	public:
		JStaticMeshVertex() = default; 
		JStaticMeshVertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT2& uv, const DirectX::XMFLOAT3& t);
		JStaticMeshVertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT2& uv, const DirectX::XMFLOAT4& t);

		JStaticMeshVertex(const JVector3<float>& p, const JVector3<float>& n, const JVector2<float>& uv, const JVector3<float>& t);
		JStaticMeshVertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT2& uv);
		JStaticMeshVertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v, float tx, float ty, float tz);
		JStaticMeshVertex(float px, float py, float pz, float nx, float ny, float nz);
		JStaticMeshVertex(float px, float py, float pz);
	};
	struct JSkinnedMeshVertex
	{
	public:
		DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0, 0, 0);
		DirectX::XMFLOAT3 normal = DirectX::XMFLOAT3(0, 0, 0);
		DirectX::XMFLOAT2 texC = DirectX::XMFLOAT2(0, 0);
		DirectX::XMFLOAT3 tangentU = DirectX::XMFLOAT3(0, 0, 0);
		DirectX::XMFLOAT3 jointWeight = DirectX::XMFLOAT3(0, 0, 0);
		uint8 jointIndex[4]{ 0,0,0,0 };
	public:
		JSkinnedMeshVertex() = default;
		JSkinnedMeshVertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT2& uv, const DirectX::XMFLOAT3& t);
		JSkinnedMeshVertex(const DirectX::XMFLOAT3& p, 
			const DirectX::XMFLOAT3& n,
			const DirectX::XMFLOAT2& uv, 
			const DirectX::XMFLOAT4& t, 
			const std::vector<JBlendingIndexWeightPair>& blendWeightPair);
		JSkinnedMeshVertex(const DirectX::XMFLOAT3 position,
			const DirectX::XMFLOAT3 normal,
			const DirectX::XMFLOAT2 texC,
			const DirectX::XMFLOAT3 tangentU,
			const DirectX::XMFLOAT3 jointWeight,
			const uint(&jointIndex)[4]);
		JSkinnedMeshVertex(const JVector3<float>& p, const JVector3<float>& n, const JVector2<float>& uv, const JVector3<float>& t);
		JSkinnedMeshVertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT2& uv);
		JSkinnedMeshVertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v, float tx, float ty, float tz);
	};

	class JMaterial;

	struct JMeshData
	{
	private:
		std::wstring name;
		size_t guid; 
		Core::JUserPtr<JMaterial> material;
		std::vector<uint> indices; 
		DirectX::BoundingBox boundingBox;
		DirectX::BoundingSphere boundingSphere;
		bool hasUV = false;
		bool hasNormal = false; 
	public:
		JMeshData();
		JMeshData(const std::wstring& name,
			const size_t guid,
			std::vector<uint>&& indices,
			const bool hasUV,
			const bool hasNormal);
	public:
		void InverseIndex()noexcept;
	public:
		std::wstring GetName()const noexcept;
		size_t GetGuid()const noexcept;
		Core::JUserPtr<JMaterial> GetMaterial()const noexcept;
		uint GetIndexCount()const noexcept;
		uint GetIndex(const uint index)const noexcept; 
		const std::vector<uint>& GetIndexVector()const noexcept; 
		DirectX::BoundingBox GetBBox()const noexcept;
		DirectX::BoundingSphere GetBSphere()const noexcept;
		virtual J_MESHGEOMETRY_TYPE GetMeshType()const noexcept = 0;
		virtual uint GetVertexCount()const noexcept = 0;
		virtual DirectX::XMVECTOR GetPosition(uint i)const noexcept = 0;
	public:
		void SetName(const std::wstring& newName)noexcept;
		void SetMaterial(Core::JUserPtr<JMaterial> material)noexcept;
		virtual void SetVertexPositionScale(const float rate)noexcept = 0;
	public:
		void AddIndex(const uint index)noexcept;
	protected:
		virtual void AddPositionOffset(const DirectX::XMFLOAT3& offsetPos)noexcept;
	public:
		bool HasUV()const noexcept;
		bool HasNormal()const noexcept;
		bool Is16bit()const noexcept;
		bool IsValid()const noexcept;
	public:
		void CreateBoundingObject()noexcept;
	protected:
		void Merge(const JMeshData& mesh);  
	};

	class JMeshGeometry;
	struct JStaticMeshData : public JMeshData
	{ 
	private:
		mutable std::vector<JStaticMeshVertex> vertices;
	public:
		JStaticMeshData();
		JStaticMeshData(const std::wstring& name,
			const size_t guid,
			std::vector<uint>&& indices,
			const bool hasUV,
			const bool hasNormal,
			std::vector<JStaticMeshVertex>&& vertices);
		JStaticMeshData(const std::wstring& name,
			std::vector<uint>&& indices,
			const bool hasUV,
			const bool hasNormal,
			std::vector<JStaticMeshVertex>&& vertices);
	public:
		J_MESHGEOMETRY_TYPE GetMeshType()const noexcept final; 
		uint GetVertexCount()const noexcept;
		DirectX::XMVECTOR GetPosition(const uint index)const noexcept;
		JStaticMeshVertex GetVertex(const uint index)const noexcept;
	public:
		void SetVertex(const uint index, const JStaticMeshVertex& vertex)const noexcept;
		void SetVertexPositionScale(const float rate)noexcept final;
	public:
		void AddVertex(const JStaticMeshVertex& vertex)noexcept;
		void AddPositionOffset(const DirectX::XMFLOAT3& offsetPos)noexcept final;
	public:
		void Merge(const JStaticMeshData& mesh)noexcept; 
	};

	struct JSkinnedMeshData : public JMeshData
	{ 
	private:
		mutable std::vector<JSkinnedMeshVertex> vertices;
	public:
		JSkinnedMeshData() = default; 
		JSkinnedMeshData(const std::wstring& name,
			const size_t guid,
			std::vector<uint>&& indices,
			const bool hasUV,
			const bool hasNormal,
			std::vector<JSkinnedMeshVertex>&& vertices);
	public:
		J_MESHGEOMETRY_TYPE GetMeshType()const noexcept final; 
		uint GetVertexCount()const noexcept;
		DirectX::XMVECTOR GetPosition(const uint index)const noexcept;
		JSkinnedMeshVertex GetVertex(const uint index)const noexcept;
	public:
		void SetVertex(const uint index, const JSkinnedMeshVertex& vertex)const noexcept;
		void SetVertexPositionScale(const float rate)noexcept final;
	public:
		void AddPositionOffset(const DirectX::XMFLOAT3& offsetPos)noexcept final; 
	};

	struct JMeshGroup
	{
	public:
		virtual uint GetMeshDataCount()const noexcept = 0;
		virtual JMeshData* GetMeshData(const uint index)noexcept = 0;
		virtual J_MESHGEOMETRY_TYPE GetMeshGroupType()const noexcept = 0; 
		uint GetTotalVertexCount() noexcept;
		uint GetTotalIndexCount() noexcept; 
		DirectX::BoundingBox GetGroupBBox() noexcept;
	public:
		void SetVertexPositionScale(const float rate)noexcept;
	};

	struct JStaticMeshGroup final : public JMeshGroup
	{
	private:
		std::vector<JStaticMeshData> staticMeshData;
	public:
		uint GetMeshDataCount()const noexcept final;
		JMeshData* GetMeshData(const uint index)noexcept final;
		J_MESHGEOMETRY_TYPE GetMeshGroupType()const noexcept final;
		void AddMeshData(JStaticMeshData&& meshData) noexcept;
	};
	 
	class JSkeletonAsset;
	struct JSkinnedMeshGroup final : public JMeshGroup
	{
	private:  
		std::vector<JSkinnedMeshData> skinnedMeshData;
		Core::JUserPtr<JSkeletonAsset> skeletonAsset;
	public:
		uint GetMeshDataCount()const noexcept final;
		JMeshData* GetMeshData(const uint index)noexcept final;
		J_MESHGEOMETRY_TYPE GetMeshGroupType()const noexcept final;
		Core::JUserPtr<JSkeletonAsset> GetSkeletonAsset()const noexcept;
	public:
		void SetSkeletonAsset(Core::JUserPtr<JSkeletonAsset> newSkeletonAsset)noexcept;
	public:
		void AddMeshData(JSkinnedMeshData&& meshData) noexcept;
	};
}
