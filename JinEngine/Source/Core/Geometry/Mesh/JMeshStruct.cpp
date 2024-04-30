#include"JMeshStruct.h" 
#include"../JDirectXCollisionEx.h"
#include"../../Guid/JGuidCreator.h"
#include"../../Utility/JCommonUtility.h" 
#include"../../Math/JMathHelper.h"
#include"../../Math/JVectorExtend.h"

namespace JinEngine
{
	using namespace DirectX;
	namespace Core
	{
		JBlendingIndexWeightPair::JBlendingIndexWeightPair(const JBlendingIndexWeightPair& data)
			:blendingIndex(data.blendingIndex), blendingWeight(data.blendingWeight)
		{}
		JStaticMeshVertex::JStaticMeshVertex(const JVector3<float>& p, const JVector3<float>& n, const JVector2<float>& uv, const JVector3<float>& t)
			: position(p), normal(n), texC(uv), tangentU(t)
		{}
		JStaticMeshVertex::JStaticMeshVertex(const JVector3<float>& p, const JVector3<float>& n, const JVector2<float>& uv, const JVector4<float>& t)
			: position(p), normal(n), texC(uv), tangentU(t.x, t.y, t.z)
		{}
		JStaticMeshVertex::JStaticMeshVertex(const JVector3<float>& p, const JVector3<float>& n, const JVector2<float>& uv)
			: position(p), normal(n), texC(uv)
		{}
		JStaticMeshVertex::JStaticMeshVertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v, float tx, float ty, float tz)
			: position(px, py, pz), normal(nx, ny, nz), texC(u, v), tangentU(tx, ty, tz)
		{}
		JStaticMeshVertex::JStaticMeshVertex(float px, float py, float pz, float nx, float ny, float nz)
			: position(px, py, pz), normal(nx, ny, nz)
		{}
		JStaticMeshVertex::JStaticMeshVertex(float px, float py, float pz)
			: position(px, py, pz)
		{}

		JSkinnedMeshVertex::JSkinnedMeshVertex(const JVector3<float>& p,
			const JVector3<float>& n,
			const JVector2<float>& uv,
			const JVector4<float>& t,
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
		JSkinnedMeshVertex::JSkinnedMeshVertex(const JVector3<float> position,
			const JVector3<float> normal,
			const JVector2<float> texC,
			const JVector3<float> tangentU,
			const JVector3<float> jointWeight,
			const uint(&jointIndex)[4])
			:position(position), normal(normal), texC(texC), tangentU(tangentU), jointWeight(jointWeight)
		{
			for (int i = 0; i < 4; ++i)
				JSkinnedMeshVertex::jointIndex[i] = jointIndex[i];
		}
		JSkinnedMeshVertex::JSkinnedMeshVertex(const JVector3<float>& p, const JVector3<float>& n, const JVector2<float>& uv, const JVector3<float>& t)
			: position(p), normal(n), texC(uv), tangentU(t)
		{}
		JSkinnedMeshVertex::JSkinnedMeshVertex(const JVector3<float>& p, const JVector3<float>& n, const JVector2<float>& uv)
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
		J1BytePosVertex::J1BytePosVertex(const JVector4<int8>& position)
			:position(position)
		{}
		J1BytePosVertex J1BytePosVertex::Encode(const JStaticMeshVertex& vertex)
		{
			J1BytePosVertex v;
			v.position.x = (vertex.position.x == 1.0f) ? 127 : (int)(vertex.position.x * 128.0f);
			v.position.y = (vertex.position.y == 1.0f) ? 127 : (int)(vertex.position.y * 128.0f);
			v.position.z = (vertex.position.z == 1.0f) ? 127 : (int)(vertex.position.z * 128.0f);
			v.position.w = 0;
			return v;
		}
		JStaticMeshVertex J1BytePosVertex::Decode(const J1BytePosVertex& vertex)
		{
			JStaticMeshVertex v;
			v.position.x = (vertex.position.x == 127) ? 1.0f : (int)(vertex.position.x / 128.0f);
			v.position.y = (vertex.position.y == 127) ? 1.0f : (int)(vertex.position.y / 128.0f);
			v.position.z = (vertex.position.z == 127) ? 1.0f : (int)(vertex.position.z / 128.0f);
			return v;
		}
		 
		JMeshData::JMeshData(const std::wstring& name,
			const size_t guid,
			std::vector<uint>&& indices,
			const bool hasUV,
			const bool hasNormal)
			: name(name), guid(guid), indices(std::move(indices)), hasUV(hasUV), hasNormal(hasNormal)
		{}
		void JMeshData::InverseIndex()noexcept
		{
			uint indicesCount = GetIndexCount(); ;
			std::vector<uint> oldIndices;
			std::copy(indices.begin(), indices.end(), std::back_inserter(oldIndices));
			for (uint i = 0; i < indicesCount; ++i)
				indices[indicesCount - 1 - i] = oldIndices[i];
		}
		std::wstring JMeshData::GetName()const noexcept
		{
			return name;
		}
		size_t JMeshData::GetGuid()const noexcept
		{
			return guid;
		}
		JUserPtr<JIdentifier> JMeshData::GetMaterial()const noexcept
		{
			return material;
		}
		uint JMeshData::GetIndexCount()const noexcept
		{
			return (uint)indices.size();
		}
		uint JMeshData::GetIndex(const uint index)const noexcept
		{
			return indices[index];
		}
		const std::vector<uint>& JMeshData::GetIndexVector()const noexcept
		{
			return indices;
		}
		DirectX::BoundingBox JMeshData::GetBBox()const noexcept
		{
			return boundingBox;
		}
		DirectX::BoundingSphere JMeshData::GetBSphere()const noexcept
		{
			return boundingSphere;
		}
		void JMeshData::SetName(const std::wstring& newName)noexcept
		{
			name = newName;
		}
		void JMeshData::SetMaterial(JUserPtr<JIdentifier> material)noexcept
		{
			JMeshData::material = material;
		}
		void JMeshData::AddIndex(const uint index)noexcept
		{
			indices.push_back(index);
		} 
		void JMeshData::AddPosition(const JVector3<float>& offset)noexcept
		{
			const uint vCount = GetVertexCount();
			for (uint i = 0; i < vCount; ++i)
				_AddPosition(i, offset);
			CreateBoundingObject();
		} 
		void JMeshData::ReversePositionY()noexcept
		{
			ReversePosition(JVector3F(1.0f, -1.0f, 1.0f));
		} 
		void JMeshData::ReversePosition(const JVector3<float>& offset)noexcept
		{
			const uint vCount = GetVertexCount();
			for (uint i = 0; i < vCount; ++i)
				_MulPosition(i, offset);
			CreateBoundingObject();
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
			return indices.size() < (1 << 16);
		}
		bool JMeshData::IsValid()const noexcept
		{
			return GetVertexCount() > 0 && GetIndexCount() > 0;
		}
		void JMeshData::CreateBoundingObject()noexcept
		{
			XMVECTOR vMin = JVector3<float>::PositiveInfV().ToXmV();
			XMVECTOR vMax = JVector3<float>::NegativeInfV().ToXmV();

			const uint vertexCount = GetVertexCount();
			for (uint i = 0; i < vertexCount; ++i)
			{
				XMVECTOR P = GetPosition(i).ToXmV();
				vMin = XMVectorMin(vMin, P);
				vMax = XMVectorMax(vMax, P);
			}
			boundingBox = Core::JDirectXCollisionEx::CreateBoundingBox(vMin, vMax);
			boundingSphere = Core::JDirectXCollisionEx::CreateBoundingSphere(vMin, vMax);
		}
		void JMeshData::Merge(const JMeshData& meshData)
		{
			if (meshData.GetMeshType() == J_MESHGEOMETRY_TYPE::SKINNED)
				return;

			const uint addedCount = meshData.GetIndexCount();
			const uint vertexOffset = GetVertexCount();
			const uint indexOffset = GetIndexCount();
			const uint newTotalIndexCount = indexOffset + addedCount;
			const bool isOver16Bit = newTotalIndexCount >= (1 << 16);

			indices.resize(newTotalIndexCount);
			for (uint i = 0; i < addedCount; ++i)
				indices[i + indexOffset] = meshData.indices[i] + vertexOffset;
		}

		JStaticMeshData::JStaticMeshData() {}
		JStaticMeshData::JStaticMeshData(const std::wstring& name,
			const size_t guid,
			std::vector<uint>&& indices,
			const bool hasUV,
			const bool hasNormal,
			std::vector<JStaticMeshVertex>&& vertices)
			:JMeshData(name, guid, std::move(indices), hasUV, hasNormal), vertices(std::move(vertices))
		{
			CreateBoundingObject();
		}
		JStaticMeshData::JStaticMeshData(const std::wstring& name,
			std::vector<uint>&& indices,
			const bool hasUV,
			const bool hasNormal,
			std::vector<JStaticMeshVertex>&& vertices)
			: JMeshData(name, Core::MakeGuid(), std::move(indices), hasUV, hasNormal), vertices(std::move(vertices))
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
		JVector3<float> JStaticMeshData::GetPosition(const uint index)const noexcept
		{
			return vertices[index].position;
		}
		JStaticMeshVertex JStaticMeshData::GetVertex(const uint index)const noexcept
		{
			return vertices[index];
		}
		void JStaticMeshData::SetVertex(const uint index, const JStaticMeshVertex& vertex)const noexcept
		{
			vertices[index] = vertex;
		}
		void JStaticMeshData::SetVertexPositionScale(const float rate)noexcept
		{
			for (auto& data : vertices)
			{
				data.position.x *= rate;
				data.position.y *= rate;
				data.position.z *= rate;
			}
			CreateBoundingObject();
		}
		void JStaticMeshData::AddVertex(const JStaticMeshVertex& vertex)noexcept
		{
			vertices.push_back(vertex);
		}
		void JStaticMeshData::_AddPosition(const uint index, const JVector3<float>& offset)noexcept
		{
			vertices[index].position += offset;
		}
		void JStaticMeshData::_MulPosition(const uint index, const JVector3<float>& offset)noexcept
		{
			vertices[index].position *= offset;
			vertices[index].normal *= offset;
			vertices[index].tangentU *= offset;
		}
		void JStaticMeshData::Merge(const JStaticMeshData& mesh)noexcept
		{
			//vertices.insert(vertices.end(), std::move(*mesh.vertices.data())); 
			JMeshData::Merge(mesh);
			vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
			CreateBoundingObject();
		}

		JSkinnedMeshData::JSkinnedMeshData(const std::wstring& name,
			const size_t guid,
			std::vector<uint>&& indices,
			const bool hasUV,
			const bool hasNormal,
			std::vector<JSkinnedMeshVertex>&& vertices)
			:JMeshData(name, guid, std::move(indices), hasUV, hasNormal), vertices(std::move(vertices))
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
		JVector3<float> JSkinnedMeshData::GetPosition(const uint index)const noexcept
		{
			return vertices[index].position;
		}
		JSkinnedMeshVertex JSkinnedMeshData::GetVertex(const uint index)const noexcept
		{
			return vertices[index];
		}
		void JSkinnedMeshData::SetVertex(const uint index, const JSkinnedMeshVertex& vertex)const noexcept
		{
			vertices[index] = vertex;
		}
		void JSkinnedMeshData::SetVertexPositionScale(const float rate)noexcept
		{
			for (auto& data : vertices)
			{
				data.position.x *= rate;
				data.position.y *= rate;
				data.position.z *= rate;
			}
		}
		void JSkinnedMeshData::_AddPosition(const uint index, const JVector3<float>& offset)noexcept
		{
			vertices[index].position += offset;
		}
		void JSkinnedMeshData::_MulPosition(const uint index, const JVector3<float>& offset)noexcept
		{
			vertices[index].position *= offset;
			vertices[index].normal *= offset; 
			vertices[index].tangentU *= offset;
		}

		JLowMeshData::JLowMeshData(const std::wstring& name,
			const size_t guid,
			std::vector<uint>&& indices,
			const bool hasUV,
			const bool hasNormal,
			std::vector<J1BytePosVertex>&& vertices)
			:JMeshData(name, guid, std::move(indices), hasUV, hasNormal), vertices(std::move(vertices))
		{}
		JLowMeshData::JLowMeshData(const std::wstring& name,
			std::vector<uint>&& indices,
			const bool hasUV,
			const bool hasNormal,
			std::vector<J1BytePosVertex>&& vertices)
			: JMeshData(name, Core::MakeGuid(), std::move(indices), hasUV, hasNormal), vertices(std::move(vertices))
		{}
		JLowMeshData::JLowMeshData(const std::wstring& name,
			std::vector<uint>&& indices,
			const bool hasUV,
			const bool hasNormal,
			const std::vector<JStaticMeshVertex>& vertices)
			: JMeshData(name, Core::MakeGuid(), std::move(indices), hasUV, hasNormal), vertices(ConvertVertex(vertices))
		{}
		J_MESHGEOMETRY_TYPE JLowMeshData::GetMeshType()const noexcept
		{
			return J_MESHGEOMETRY_TYPE::STATIC;
		}
		uint JLowMeshData::GetVertexCount()const noexcept
		{
			return (uint)vertices.size();
		}
		JVector3<float> JLowMeshData::GetPosition(const uint index)const noexcept
		{
			return vertices[index].position.XYZ();
		}
		J1BytePosVertex JLowMeshData::GetVertex(const uint index)const noexcept
		{
			return vertices[index];
		} 
		void JLowMeshData::SetVertex(const uint index, const J1BytePosVertex& vertex)const noexcept
		{
			vertices[index] = vertex;
		}
		void JLowMeshData::SetVertexPositionScale(const float rate)noexcept
		{
			for (auto& data : vertices)
			{
				data.position.x *= rate;
				data.position.y *= rate;
				data.position.z *= rate;
			}
		} 
		void JLowMeshData::_AddPosition(const uint index, const JVector3<float>& offset)noexcept
		{
			vertices[index].position += JVector4<int8>(offset, 0.0f);
		}
		void JLowMeshData::_MulPosition(const uint index, const JVector3<float>& offset)noexcept
		{
			vertices[index].position *= JVector4<int8>(offset, 1.0f);
		}
		std::vector<J1BytePosVertex> JLowMeshData::ConvertVertex(const std::vector<JStaticMeshVertex>& staticVertex)
		{
			const uint vertexCount = (uint)staticVertex.size();
			std::vector<J1BytePosVertex> lowMeshData(vertexCount);
			for(uint i = 0; i < vertexCount; ++i)
				lowMeshData[i] = J1BytePosVertex::Encode(staticVertex[i]);
			return lowMeshData;
		}

		uint JMeshGroup::GetTotalVertexCount()noexcept
		{
			const uint meshCount = GetMeshDataCount();
			uint totalVertex = 0;
			for (uint i = 0; i < meshCount; ++i)
				totalVertex += GetMeshData(i)->GetVertexCount();
			return totalVertex;
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
			return totalIndex;
		}
		DirectX::BoundingBox JMeshGroup::GetGroupBBox()noexcept
		{
			Core::JBBox bbox;
			const uint meshCount = GetMeshDataCount();
			for (uint i = 0; i < meshCount; ++i)
				bbox = Core::JBBox::Union(bbox, GetMeshData(i)->GetBBox());
			return bbox.Convert();
		}
		void JMeshGroup::SetVertexPositionScale(const float rate)noexcept
		{
			const uint meshCount = GetMeshDataCount();
			for (uint i = 0; i < meshCount; ++i)
				GetMeshData(i)->SetVertexPositionScale(rate);
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
				return staticMeshData[index].get();
		}
		J_MESHGEOMETRY_TYPE JStaticMeshGroup::GetMeshGroupType()const noexcept
		{
			return J_MESHGEOMETRY_TYPE::STATIC;
		}
		uint JStaticMeshGroup::GetVertexSize()const noexcept
		{
			return sizeof(JStaticMeshVertex);
		} 
		void JStaticMeshGroup::AddMeshData(std::unique_ptr<JStaticMeshData>&& meshData) noexcept
		{
			staticMeshData.push_back(std::move(meshData));
		}
		std::unique_ptr<JStaticMeshData> JStaticMeshGroup::PopMeshData(const uint index)noexcept
		{
			std::unique_ptr<JStaticMeshData> mesh = std::move(staticMeshData[index]);
			staticMeshData.erase(staticMeshData.begin() + index);
			return std::move(mesh);
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
				return skinnedMeshData[index].get();
		}
		J_MESHGEOMETRY_TYPE JSkinnedMeshGroup::GetMeshGroupType()const noexcept
		{
			return J_MESHGEOMETRY_TYPE::SKINNED;
		}
		uint JSkinnedMeshGroup::GetVertexSize()const noexcept
		{
			return sizeof(JSkinnedMeshVertex);
		}
		JUserPtr<JIdentifier> JSkinnedMeshGroup::GetSkeletonAsset()const noexcept
		{
			return skeletonAsset;
		}
		void JSkinnedMeshGroup::SetSkeletonAsset(JUserPtr<JIdentifier> newSkeletonAsset)noexcept
		{
			skeletonAsset = newSkeletonAsset;
		}
		void JSkinnedMeshGroup::AddMeshData(std::unique_ptr<JSkinnedMeshData>&& meshData) noexcept
		{
			skinnedMeshData.push_back(std::move(meshData));
		}
		std::unique_ptr<JSkinnedMeshData> JSkinnedMeshGroup::PopMeshData(const uint index)noexcept
		{
			std::unique_ptr<JSkinnedMeshData> mesh = std::move(skinnedMeshData[index]);
			skinnedMeshData.erase(skinnedMeshData.begin() + index);
			return std::move(mesh);
		}

		uint JLowMeshGroup::GetMeshDataCount()const noexcept
		{
			return (uint)lowMeshData.size();
		}
		JMeshData* JLowMeshGroup::GetMeshData(const uint index)noexcept
		{
			return lowMeshData[index].get();
		}
		J_MESHGEOMETRY_TYPE JLowMeshGroup::GetMeshGroupType()const noexcept
		{
			return J_MESHGEOMETRY_TYPE::STATIC;
		}
		uint JLowMeshGroup::GetVertexSize()const noexcept
		{
			return sizeof(J1BytePosVertex);
		}
		void JLowMeshGroup::AddMeshData(std::unique_ptr<JLowMeshData>&& meshData) noexcept
		{
			lowMeshData.push_back(std::move(meshData));
		}
		std::unique_ptr<JLowMeshData> JLowMeshGroup::PopMeshData(const uint index)noexcept
		{
			std::unique_ptr<JLowMeshData> mesh = std::move(lowMeshData[index]);
			lowMeshData.erase(lowMeshData.begin() + index);
			return std::move(mesh);
		}
	}
}