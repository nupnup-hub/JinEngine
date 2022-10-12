#include"JMeshStruct.h"
#include"../../../Utility/JCommonUtility.h" 
#include"../../../Utility/JMathHelper.h"
#include"../../../Core/DirectXEx/JDirectXCollisionEx.h"
#include"../../../Core/Guid/GuidCreator.h"
 
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
	JStaticMeshVertex::JStaticMeshVertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v,  float tx, float ty, float tz)
		: position(px, py, pz), normal(nx, ny, nz), tangentU(tx, ty, tz), texC(u, v)
	{}
	JStaticMeshVertex::JStaticMeshVertex(float px, float py, float pz, float nx, float ny, float nz)
		: position(px, py, pz), normal(nx, ny, nz)
	{}
	JStaticMeshVertex::JStaticMeshVertex(float px, float py, float pz)
		: position(px, py, pz)
	{}
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
	JSkinnedMeshVertex::JSkinnedMeshVertex(const DirectX::XMFLOAT3 position,
		const DirectX::XMFLOAT3 normal,
		const DirectX::XMFLOAT2 texC,
		const DirectX::XMFLOAT3 tangentU,
		const DirectX::XMFLOAT3 jointWeight,
		const uint(&jointIndex)[4])
		:position(position), normal(normal), texC(texC), tangentU(tangentU), jointWeight(jointWeight) 
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

	JMeshData::JMeshData(){}
	JMeshData::JMeshData(const std::wstring& name,
		const size_t guid,
		std::vector<uint>&& indices32,
		const bool hasUV,
		const bool hasNormal)
		:name(name), guid(guid), indices32(std::move(indices32)), hasUV(hasUV), hasNormal(hasNormal)
	{
		is16bit = false;
	}
	JMeshData::JMeshData(const std::wstring& name,
		const size_t guid,
		std::vector<uint16>&& indices16,
		const bool hasUV,
		const bool hasNormal)
		: name(name), guid(guid), indices16(std::move(indices16)), hasUV(hasUV), hasNormal(hasNormal)
	{
		is16bit = true;
	}

	void JMeshData::Stuff8ByteDataTo4Byte()noexcept
	{
		const uint indexCount = (uint)indices32.size();
		if (indexCount == 0 && indexCount < 1 << 16)
			return;

		indices16.resize(indexCount);
		for (size_t i = 0; i < indexCount; ++i)
			indices16[i] = indices32[i];

		indices32.clear();
		is16bit = true;
	}
	void JMeshData::Stuff4ByteDataTo8Byte()noexcept
	{
		const uint indexCount = (uint)indices16.size();
		if (indexCount == 0)
			return;

		indices32.resize(indexCount);
		for (size_t i = 0; i < indexCount; ++i)
			indices32[i] = indices16[i];

		indices16.clear();
		is16bit = false;
	}
	void JMeshData::InverseIndex()noexcept
	{
		uint indices16Size = (uint)indices16.size();
		uint indices32Size = (uint)indices32.size();
		if (indices32Size != 0)
		{
			std::vector<uint> oldIndices32;
			std::copy(indices32.begin(), indices32.end(), std::back_inserter(oldIndices32));
			for (uint i = 0; i < indices32Size; ++i)
				indices32[indices32Size - 1 - i] = oldIndices32[i];
		}
		else if(indices16Size != 0)
		{
			std::vector<uint16> oldIndices16;
			std::copy(indices16.begin(), indices16.end(), std::back_inserter(oldIndices16));
			for (uint i = 0; i < indices16Size; ++i)
				indices16[indices16Size - 1 - i] = oldIndices16[i];
		}
	}
	std::wstring JMeshData::GetName()const noexcept
	{
		return name;
	}
	size_t JMeshData::GetGuid()const noexcept
	{
		return guid;
	}
	Core::JUserPtr<JMaterial> JMeshData::GetMaterial()const noexcept
	{
		return material;
	}
	uint JMeshData::GetIndexCount()const noexcept
	{
		if (is16bit)
			return (uint)indices16.size();
		else
			return (uint)indices32.size();
	}
	uint16 JMeshData::GetU16Index(const uint index)const noexcept
	{
		if (is16bit)
			return indices16[index];
		else
			return 0;
	}
	uint32 JMeshData::GetU32Index(const uint index)const noexcept
	{
		if (is16bit)
			return 0;
		else
			return indices32[index];
	}
	const std::vector<uint16>& JMeshData::GetU16Vector()const noexcept
	{
		return indices16;
	}
	const std::vector<uint32>& JMeshData::GetU32Vector()const noexcept
	{
		return indices32;
	}
	DirectX::BoundingBox JMeshData::GetBBox()const noexcept
	{
		return boundingBox;
	}
	DirectX::BoundingSphere JMeshData::GetBSphere()const noexcept
	{
		return boundingSphere;
	}
	void JMeshData::SetMaterial(Core::JUserPtr<JMaterial> material)noexcept
	{
		JMeshData::material = material;
	}
	void JMeshData::AddIndex(const uint index)noexcept
	{
		if (Is16bit())
			indices16.push_back(index);
		else
			indices32.push_back(index);
	}
	bool JMeshData::HasUV()const noexcept
	{
		return hasUV;
	}
	bool JMeshData::HasNormal()const noexcept
	{
		return hasNormal;
	}
	bool JMeshData::Is16bit()const noexcept
	{
		return is16bit;
	}
	bool JMeshData::IsValid()const noexcept
	{
		return GetVertexCount() > 0 && GetIndexCount() > 0;
	}
	void JMeshData::CreateBoundingObject()noexcept
	{
		XMFLOAT3 vMinf3(+JMathHelper::Infinity, +JMathHelper::Infinity, +JMathHelper::Infinity);
		XMFLOAT3 vMaxf3(-JMathHelper::Infinity, -JMathHelper::Infinity, -JMathHelper::Infinity);

		XMVECTOR vMin = XMLoadFloat3(&vMinf3);
		XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

		const uint vertexCount = GetVertexCount();
		for (uint i = 0; i < vertexCount; ++i)
		{
			XMVECTOR P = GetPosition(i);
			vMin = XMVectorMin(vMin, P);
			vMax = XMVectorMax(vMax, P);
		}
		boundingBox = Core::JDirectXCollisionEx::CreateBoundingBox(vMin, vMax);
		boundingSphere = Core::JDirectXCollisionEx::CreateBoundingSphere(vMin, vMax);
	}
	JStaticMeshData::JStaticMeshData(){}
	JStaticMeshData::JStaticMeshData(const std::wstring& name,
		const size_t guid,
		std::vector<uint32>&& indices32,
		const bool hasUV,
		const bool hasNormal,
		std::vector<JStaticMeshVertex>&& vertices)
		:JMeshData(name ,guid, std::move(indices32), hasUV, hasNormal), vertices(std::move(vertices))
	{
		CreateBoundingObject();
	}
	JStaticMeshData::JStaticMeshData(const std::wstring& name,
		const size_t guid,
		std::vector<uint16>&& indices16,
		const bool hasUV,
		const bool hasNormal,
		std::vector<JStaticMeshVertex>&& vertices)
		: JMeshData(name, guid, std::move(indices16), hasUV, hasNormal), vertices(std::move(vertices))
	{
		CreateBoundingObject();
	}
	JStaticMeshData::JStaticMeshData(const std::wstring& name,
		std::vector<uint16>&& indices16,
		const bool hasUV,
		const bool hasNormal,
		std::vector<JStaticMeshVertex>&& vertices)
		: JMeshData(name, Core::MakeGuid(), std::move(indices16), hasUV, hasNormal), vertices(std::move(vertices))
	{
		CreateBoundingObject();
	}
	JStaticMeshData::JStaticMeshData(const std::wstring& name,
		std::vector<uint32>&& indices32,
		const bool hasUV,
		const bool hasNormal,
		std::vector<JStaticMeshVertex>&& vertices)
		: JMeshData(name, Core::MakeGuid(), std::move(indices32), hasUV, hasNormal), vertices(std::move(vertices))
	{
		CreateBoundingObject();
	}

	J_MESHGEOMETRY_TYPE JStaticMeshData::GetMeshType()const noexcept
	{
		return J_MESHGEOMETRY_TYPE::STATIC;
	}
	uint JStaticMeshData::GetVertexCount()const noexcept
	{
		return (uint)vertices.size();
	}
	DirectX::XMVECTOR JStaticMeshData::GetPosition(const uint index)const noexcept
	{
		return XMLoadFloat3(&vertices[index].position);
	}
	JStaticMeshVertex JStaticMeshData::GetVertex(const uint index)const noexcept
	{
		return vertices[index];
	}
	void JStaticMeshData::SetVertex(const uint index, const JStaticMeshVertex& vertex)const noexcept
	{
		vertices[index] = vertex;
	}
	void JStaticMeshData::AddVertex(const JStaticMeshVertex& vertex)noexcept
	{
		vertices.push_back(vertex);
	}
	JSkinnedMeshData::JSkinnedMeshData(const std::wstring& name,
		const size_t guid,
		std::vector<uint32>&& indices32,
		const bool hasUV,
		const bool hasNormal,
		std::vector<JSkinnedMeshVertex>&& vertices)
		:JMeshData(name, guid, std::move(indices32), hasUV, hasNormal), vertices(std::move(vertices))
	{
		CreateBoundingObject();
	}
	JSkinnedMeshData::JSkinnedMeshData(const std::wstring& name,
		const size_t guid,
		std::vector<uint16>&& indices16,
		const bool hasUV,
		const bool hasNormal,
		std::vector<JSkinnedMeshVertex>&& vertices)
		: JMeshData(name, guid, std::move(indices16), hasUV, hasNormal), vertices(std::move(vertices))
	{
		CreateBoundingObject();
	}
	J_MESHGEOMETRY_TYPE JSkinnedMeshData::GetMeshType()const noexcept
	{ 
		return J_MESHGEOMETRY_TYPE::SKINNED;
	}
	uint JSkinnedMeshData::GetVertexCount()const noexcept
	{
		return (uint)vertices.size();
	}
	DirectX::XMVECTOR JSkinnedMeshData::GetPosition(const uint index)const noexcept
	{
		return XMLoadFloat3(&vertices[index].position);
	}
	JSkinnedMeshVertex JSkinnedMeshData::GetVertex(const uint index)const noexcept
	{
		return vertices[index];
	}
	void JSkinnedMeshData::SetVertex(const uint index, const JSkinnedMeshVertex& vertex)const noexcept
	{
		vertices[index] = vertex;
	}

	uint JMeshGroup::GetTotalVertexCount()noexcept
	{
		const uint meshCount = GetMeshDataCount();
		uint totalVertex = 0;
		for (uint i = 0; i < meshCount; ++i)
			totalVertex += GetMeshData(i)->GetVertexCount();
	}
	uint JMeshGroup::GetTotalIndexCount()noexcept
	{
		const uint meshCount = GetMeshDataCount();
		uint totalIndex = 0;
		for (uint i = 0; i < meshCount; ++i)
		{
			const JMeshData* meshdata = GetMeshData(i);
			totalIndex += meshdata->GetIndexCount();
		}
	}
	uint JStaticMeshGroup::GetMeshDataCount()const noexcept
	{
		return (uint)staticMeshData.size();
	}
	JMeshData* JStaticMeshGroup::GetMeshData(const uint index) noexcept
	{
		if (staticMeshData.size() <= index)
			return nullptr;
		else
			return &staticMeshData[index];
	}
	J_MESHGEOMETRY_TYPE JStaticMeshGroup::GetMeshGroupType()const noexcept
	{
		return J_MESHGEOMETRY_TYPE::STATIC;
	} 
	void JStaticMeshGroup::AddMeshData(JStaticMeshData&& meshData) noexcept
	{
		staticMeshData.push_back(std::move(meshData));
	}

	uint JSkinnedMeshGroup::GetMeshDataCount()const noexcept
	{
		return (uint)skinnedMeshData.size();
	}
	JMeshData* JSkinnedMeshGroup::GetMeshData(const uint index) noexcept
	{
		if (skinnedMeshData.size() <= index)
			return nullptr;
		else
			return &skinnedMeshData[index];
	}
	J_MESHGEOMETRY_TYPE JSkinnedMeshGroup::GetMeshGroupType()const noexcept
	{
		return J_MESHGEOMETRY_TYPE::SKINNED;
	}
	Core::JUserPtr<JSkeletonAsset> JSkinnedMeshGroup::GetSkeletonAsset()const noexcept
	{
		return skeletonAsset;
	}
	void JSkinnedMeshGroup::AddMeshData(JSkinnedMeshData&& meshData) noexcept
	{
		skinnedMeshData.push_back(std::move(meshData));
	}
}