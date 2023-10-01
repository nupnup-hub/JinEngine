#pragma once
#include"JMeshType.h" 
#include"../../Pointer/JOwnerPtr.h"
#include"../../Math/JVector.h" 
#include<vector>
#include<DirectXMath.h>
#include<DirectXCollision.h>

namespace JinEngine
{
	namespace Core
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
			JVector3<float> position = JVector3<float>(0, 0, 0);
			JVector3<float> normal = JVector3<float>(0, 0, 0);
			JVector2<float> texC = JVector2<float>(0, 0);
			JVector3<float> tangentU = JVector3<float>(0, 0, 0);
		public:
			JStaticMeshVertex() = default;
			JStaticMeshVertex(const JVector3<float>& p, const JVector3<float>& n, const JVector2<float>& uv, const JVector3<float>& t);
			JStaticMeshVertex(const JVector3<float>& p, const JVector3<float>& n, const JVector2<float>& uv, const JVector4<float>& t);
			JStaticMeshVertex(const JVector3<float>& p, const JVector3<float>& n, const JVector2<float>& uv);
			JStaticMeshVertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v, float tx, float ty, float tz);
			JStaticMeshVertex(float px, float py, float pz, float nx, float ny, float nz);
			JStaticMeshVertex(float px, float py, float pz);
		};
		struct JSkinnedMeshVertex
		{
		public:
			JVector3<float> position = JVector3<float>(0, 0, 0);
			JVector3<float> normal = JVector3<float>(0, 0, 0);
			JVector2<float> texC = JVector2<float>(0, 0);
			JVector3<float> tangentU = JVector3<float>(0, 0, 0);
			JVector3<float> jointWeight = JVector3<float>(0, 0, 0);
			uint8 jointIndex[4]{ 0,0,0,0 };
		public:
			JSkinnedMeshVertex() = default;
			JSkinnedMeshVertex(const JVector3<float>& p,
				const JVector3<float>& n,
				const JVector2<float>& uv,
				const JVector4<float>& t,
				const std::vector<JBlendingIndexWeightPair>& blendWeightPair);
			JSkinnedMeshVertex(const JVector3<float> position,
				const JVector3<float> normal,
				const JVector2<float> texC,
				const JVector3<float> tangentU,
				const JVector3<float> jointWeight,
				const uint(&jointIndex)[4]);
			JSkinnedMeshVertex(const JVector3<float>& p, const JVector3<float>& n, const JVector2<float>& uv, const JVector3<float>& t);
			JSkinnedMeshVertex(const JVector3<float>& p, const JVector3<float>& n, const JVector2<float>& uv);
			JSkinnedMeshVertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v, float tx, float ty, float tz);
		};

		class JIdentifier;

		struct JMeshData
		{
		private:
			std::wstring name;
			size_t guid;
			JUserPtr<JIdentifier> material;
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
			JUserPtr<JIdentifier> GetMaterial()const noexcept;
			uint GetIndexCount()const noexcept;
			uint GetIndex(const uint index)const noexcept;
			const std::vector<uint>& GetIndexVector()const noexcept;
			DirectX::BoundingBox GetBBox()const noexcept;
			DirectX::BoundingSphere GetBSphere()const noexcept;
			virtual J_MESHGEOMETRY_TYPE GetMeshType()const noexcept = 0;
			virtual uint GetVertexCount()const noexcept = 0;
			virtual JVector3<float> GetPosition(uint i)const noexcept = 0;
		public:
			void SetName(const std::wstring& newName)noexcept;
			void SetMaterial(JUserPtr<JIdentifier> material)noexcept;
			virtual void SetVertexPositionScale(const float rate)noexcept = 0;
		public:
			void AddIndex(const uint index)noexcept;
		protected:
			virtual void AddPositionOffset(const JVector3<float>& offsetPos)noexcept;
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
			JVector3<float> GetPosition(const uint index)const noexcept;
			JStaticMeshVertex GetVertex(const uint index)const noexcept;
		public:
			void SetVertex(const uint index, const JStaticMeshVertex& vertex)const noexcept;
			void SetVertexPositionScale(const float rate)noexcept final;
		public:
			void AddVertex(const JStaticMeshVertex& vertex)noexcept;
			void AddPositionOffset(const JVector3<float>& offsetPos)noexcept final;
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
			JVector3<float> GetPosition(const uint index)const noexcept;
			JSkinnedMeshVertex GetVertex(const uint index)const noexcept;
		public:
			void SetVertex(const uint index, const JSkinnedMeshVertex& vertex)const noexcept;
			void SetVertexPositionScale(const float rate)noexcept final;
		public:
			void AddPositionOffset(const JVector3<float>& offsetPos)noexcept final;
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
		 
		struct JSkinnedMeshGroup final : public JMeshGroup
		{
		private:
			std::vector<JSkinnedMeshData> skinnedMeshData;
			JUserPtr<JIdentifier> skeletonAsset;
		public:
			uint GetMeshDataCount()const noexcept final;
			JMeshData* GetMeshData(const uint index)noexcept final;
			J_MESHGEOMETRY_TYPE GetMeshGroupType()const noexcept final;
			JUserPtr<JIdentifier> GetSkeletonAsset()const noexcept;
		public:
			void SetSkeletonAsset(JUserPtr<JIdentifier> newSkeletonAsset)noexcept;
		public:
			void AddMeshData(JSkinnedMeshData&& meshData) noexcept;
		};
	}
}