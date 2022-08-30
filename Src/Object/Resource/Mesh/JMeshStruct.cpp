#include"JMeshStruct.h"
#include"../../../Utility/JCommonUtility.h" 

namespace JinEngine
{
	using namespace DirectX;

	JBlendingIndexWeightPair::JBlendingIndexWeightPair(const JBlendingIndexWeightPair& data)
		:blendingIndex(data.blendingIndex), blendingWeight(data.blendingWeight)
	{}
	JStaticMeshVertex::JStaticMeshVertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT2& uv, const DirectX::XMFLOAT3& t)
		: position(p), normal(n), texC(uv), tangentU(t)
	{}
	JStaticMeshVertex::JStaticMeshVertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT2& uv, const DirectX::XMFLOAT4& t)
		: position(p), normal(n), texC(uv), tangentU(t.x, t.y, t.z)
	{}
	JStaticMeshVertex::JStaticMeshVertex(const JVector3<float>& p, const JVector3<float>& n, const JVector2<float>& uv, const JVector3<float>& t)
		: position(p.x, p.y, p.z), normal(n.x, n.y, n.z), tangentU(t.x, t.y, t.z), texC(uv.x, uv.y)
	{}
	JStaticMeshVertex::JStaticMeshVertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT2& uv)
		: position(p), normal(n), texC(uv)
	{}
	JStaticMeshVertex::JStaticMeshVertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v, float tx, float ty, float tz)
		: position(px, py, pz), normal(nx, ny, nz), tangentU(tx, ty, tz), texC(u, v)
	{}
	JStaticMeshVertex::JStaticMeshVertex(float px, float py, float pz, float nx, float ny, float nz)
		: position(px, py, pz), normal(nx, ny, nz)
	{}
	JStaticMeshVertex::JStaticMeshVertex(float px, float py, float pz)
		: position(px, py, pz)
	{}
	void JStaticMeshData::Stuff8ByteDataTo4Byte()noexcept
	{
		indices16.resize(indices32.size());
		for (size_t i = 0; i < indices32.size(); ++i)
			indices16[i] = indices32[i];
	}
	void JStaticMeshData::InverseIndex()noexcept
	{
		uint indicesSize = (uint)indices32.size();
		if (indicesSize == 0)
			return;
		std::vector<uint> oldIndices32;
		std::copy(indices32.begin(), indices32.end(), std::back_inserter(oldIndices32));
		for (uint i = 0; i < indicesSize; ++i)
			indices32[indicesSize - 1 - i] = oldIndices32[i];
	}
	bool JStaticMeshData::IsIndices32Empty()const noexcept
	{
		return indices32.size() == 0;
	}
	bool JStaticMeshData::IsIndices16Empty()const noexcept
	{
		return indices16.size() == 0;
	}
	JSkinnedMeshVertex::JSkinnedMeshVertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT2& uv, const DirectX::XMFLOAT3& t)
		:position(p), normal(n), texC(uv), tangentU(t)
	{}

	JSkinnedMeshVertex::JSkinnedMeshVertex(const DirectX::XMFLOAT3& p, 
		const DirectX::XMFLOAT3& n,
		const DirectX::XMFLOAT2& uv,
		const DirectX::XMFLOAT4& t,
		const std::vector<JBlendingIndexWeightPair>& blendWeightPair)
		: position(p),
		normal(n),
		texC(uv),
		tangentU(t.x, t.y, t.z)
	{
		int i = 0;
		for (; i < blendWeightPair.size() && i < 4; ++i)
		{
			jointIndex[i] = blendWeightPair[i].blendingIndex;
			if (i == 0)
				jointWeight.x = blendWeightPair[i].blendingWeight;
			else if (i == 1)
				jointWeight.y = blendWeightPair[i].blendingWeight;
			else if (i == 2)
				jointWeight.z = blendWeightPair[i].blendingWeight;
		}
	}
	JSkinnedMeshVertex::JSkinnedMeshVertex(const DirectX::XMFLOAT3& p,
		const DirectX::XMFLOAT3& n,
		const DirectX::XMFLOAT2& uv,
		const DirectX::XMFLOAT4& t,
		uint8(&jointIndex)[4])
		: position(p),
		normal(n),
		texC(uv),
		tangentU(t.x, t.y, t.z)
	{
		for (int i = 0; i < 4; ++i)
			JSkinnedMeshVertex::jointIndex[i] = jointIndex[i];
	}
	JSkinnedMeshVertex::JSkinnedMeshVertex(const JVector3<float>& p, const JVector3<float>& n, const JVector2<float>& uv, const JVector3<float>& t)
		:position(p.x, p.y, p.z), normal(n.x, n.y, n.z), tangentU(t.x, t.y, t.z), texC(uv.x, uv.y)
	{}

	JSkinnedMeshVertex::JSkinnedMeshVertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT2& uv)
		: position(p), normal(n), texC(uv)
	{}

	JSkinnedMeshVertex::JSkinnedMeshVertex(
		float px, float py, float pz,
		float nx, float ny, float nz,
		float u, float v,
		float tx, float ty, float tz) :
		position(px, py, pz),
		normal(nx, ny, nz),
		texC(u, v),
		tangentU(tx, ty, tz)
	{}

	void JSkinnedMeshData::Stuff8ByteDataTo4Byte()noexcept
	{
		indices16.resize(indices32.size());
		for (size_t i = 0; i < indices32.size(); ++i)
			indices16[i] = indices32[i];
	}
	void JSkinnedMeshData::InverseIndex()noexcept
	{
		uint indicesSize = (uint)indices32.size();
		if (indicesSize == 0)
			return;
		std::vector<uint> oldIndices32;
		std::copy(indices32.begin(), indices32.end(), std::back_inserter(oldIndices32));
		for (uint i = 0; i < indicesSize; ++i)
			indices32[indicesSize - 1 - i] = oldIndices32[i];
	}
}