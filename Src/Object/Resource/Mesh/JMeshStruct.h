#pragma once
#include<vector>
#include<DirectXMath.h>
#include"../../../Core/JDataType.h"
#include"../../../Utility/JVector.h"  

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
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texC;
		DirectX::XMFLOAT3 tangentU;
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
	struct JStaticMeshData
	{
	public:
		std::vector<JStaticMeshVertex> vertices;
		std::vector<uint> indices32;
		std::vector<uint16> indices16;
		bool hasUV;
		bool hasNormal;
	public:
		void Stuff8ByteDataTo4Byte()noexcept;
		void InverseIndex()noexcept;
		bool IsIndices32Empty()const noexcept;
		bool IsIndices16Empty()const noexcept;
	};
	struct JSkinnedMeshVertex
	{
	public:
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texC;
		DirectX::XMFLOAT3 tangentU;
		DirectX::XMFLOAT3 jointWeight;
		uint8 jointIndex[4];
	public:
		JSkinnedMeshVertex() = default;
		JSkinnedMeshVertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT2& uv, const DirectX::XMFLOAT3& t);
		JSkinnedMeshVertex(const DirectX::XMFLOAT3& p, 
			const DirectX::XMFLOAT3& n,
			const DirectX::XMFLOAT2& uv, 
			const DirectX::XMFLOAT4& t, 
			const std::vector<JBlendingIndexWeightPair>& blendWeightPair);
		JSkinnedMeshVertex(const DirectX::XMFLOAT3& p, 
			const DirectX::XMFLOAT3& n,
			const DirectX::XMFLOAT2& uv, 
			const DirectX::XMFLOAT4& t, 
			uint8(&jointIndex)[4]);
		JSkinnedMeshVertex(const JVector3<float>& p, const JVector3<float>& n, const JVector2<float>& uv, const JVector3<float>& t);
		JSkinnedMeshVertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT2& uv);
		JSkinnedMeshVertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v, float tx, float ty, float tz);
	};

	struct JSkinnedMeshData
	{
	public:
		std::vector<JSkinnedMeshVertex> vertices;
		std::vector<uint> indices32;
		std::vector<uint16> indices16;
		bool hasUV;
		bool hasNormal;
	public:
		void Stuff8ByteDataTo4Byte()noexcept;
		void InverseIndex()noexcept;
	};
	struct SubmeshGeometry
	{
		uint IndexCount = 0;
		uint StartIndexLocation = 0;
		int BaseVertexLocation = 0;
	};
}
