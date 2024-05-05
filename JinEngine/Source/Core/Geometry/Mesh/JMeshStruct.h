#pragma once
#include"JMeshType.h" 
#include"../../Pointer/JOwnerPtr.h"
#include"../../Math/JVector.h" 
#include"../../Reflection/JReflection.h" 
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
		struct J1BytePosVertex
		{
		public:
			JVector4<int8> position = JVector4<int8>(0, 0, 0, 0);
		public:
			J1BytePosVertex() = default;
			J1BytePosVertex(const JVector4<int8>& position);
		public:
			static J1BytePosVertex Encode(const JStaticMeshVertex& vertex);
			static JStaticMeshVertex Decode(const J1BytePosVertex& vertex);
		};

		class JIdentifier;
		class JMeshGeometry;
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
			JMeshData() = default;
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
			void AddPosition(const JVector3<float>& offset)noexcept;
			/**
			* @brief position을 뒤집는 과정에서 positio을 제외한 다른 데이터들에 유효성이 없어지므로 가급적 사용하지말것
			*/ 		
			void ReversePositionY()noexcept; 		
		private:
			void ReversePosition(const JVector3<float>& offset)noexcept;
			virtual void _AddPosition(const uint index, const JVector3<float>& offset)noexcept = 0;
			virtual void _MulPosition(const uint index, const JVector3<float>& offset)noexcept = 0;
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
			JVector3<float> GetPosition(const uint index)const noexcept final;
			JStaticMeshVertex GetVertex(const uint index)const noexcept; 
		public:
			void SetVertex(const uint index, const JStaticMeshVertex& vertex)const noexcept;
			void SetVertexPositionScale(const float rate)noexcept final;
		public:
			void AddVertex(const JStaticMeshVertex& vertex)noexcept;
		private:
			void _AddPosition(const uint index, const JVector3<float>& offset)noexcept final;
			void _MulPosition(const uint index, const JVector3<float>& offset)noexcept final;
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
			uint GetVertexCount()const noexcept final;
			JVector3<float> GetPosition(const uint index)const noexcept final;
			JSkinnedMeshVertex GetVertex(const uint index)const noexcept; 
		public:
			void SetVertex(const uint index, const JSkinnedMeshVertex& vertex)const noexcept;
			void SetVertexPositionScale(const float rate)noexcept final;
		private:
			void _AddPosition(const uint index, const JVector3<float>& offset)noexcept final;
			void _MulPosition(const uint index, const JVector3<float>& offset)noexcept final;
		}; 
		struct JLowMeshData : public JMeshData
		{
		private:
			mutable std::vector<J1BytePosVertex> vertices;
		public:
			JLowMeshData() = default;
			JLowMeshData(const std::wstring& name,
				const size_t guid,
				std::vector<uint>&& indices,
				const bool hasUV,
				const bool hasNormal,
				std::vector<J1BytePosVertex>&& vertices);
			JLowMeshData(const std::wstring& name,
				std::vector<uint>&& indices,
				const bool hasUV,
				const bool hasNormal,
				std::vector<J1BytePosVertex>&& vertices);
			JLowMeshData(const std::wstring& name,
				std::vector<uint>&& indices,
				const bool hasUV,
				const bool hasNormal,
				const std::vector<JStaticMeshVertex>& vertices);
		public:
			J_MESHGEOMETRY_TYPE GetMeshType()const noexcept final;
			uint GetVertexCount()const noexcept final;
			JVector3<float> GetPosition(const uint index)const noexcept final;
			J1BytePosVertex GetVertex(const uint index)const noexcept; 
		public:
			void SetVertex(const uint index, const J1BytePosVertex& vertex)const noexcept;
			void SetVertexPositionScale(const float rate)noexcept final;
		private: 
			void _AddPosition(const uint index, const JVector3<float>& offset)noexcept final;
			void _MulPosition(const uint index, const JVector3<float>& offset)noexcept final;
		public:
			static std::vector<J1BytePosVertex> ConvertVertex(const std::vector<JStaticMeshVertex>& staticVertex);
		};

		struct JMeshGroup
		{ 
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JMeshGroup)
		public:
			JMeshGroup() = default;
			virtual ~JMeshGroup() = default;
			JMeshGroup(JMeshGroup&& rhs) = default;
			JMeshGroup& operator=(JMeshGroup&& rhs) = default;
		public:
			virtual uint GetMeshDataCount()const noexcept = 0;
			virtual JMeshData* GetMeshData(const uint index)noexcept = 0;
			virtual J_MESHGEOMETRY_TYPE GetMeshGroupType()const noexcept = 0;
			virtual uint GetVertexSize()const noexcept = 0; 
			uint GetTotalVertexCount() noexcept;
			uint GetTotalIndexCount() noexcept;
			DirectX::BoundingBox GetGroupBBox() noexcept;
		public:
			void SetVertexPositionScale(const float rate)noexcept; 
		};
		struct JStaticMeshGroup final : public JMeshGroup
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JStaticMeshGroup)
		private:
			std::vector<std::unique_ptr<JStaticMeshData>> staticMeshData;
		public:
			JStaticMeshGroup() = default;
			~JStaticMeshGroup() = default;
			JStaticMeshGroup(JStaticMeshGroup&& rhs) = default;
			JStaticMeshGroup& operator=(JStaticMeshGroup&& rhs) = default;
		public:
			uint GetMeshDataCount()const noexcept final;
			JMeshData* GetMeshData(const uint index)noexcept final;
			J_MESHGEOMETRY_TYPE GetMeshGroupType()const noexcept final;
			uint GetVertexSize()const noexcept final; 
			void AddMeshData(std::unique_ptr<JStaticMeshData>&& meshData) noexcept;
			std::unique_ptr<JStaticMeshData> PopMeshData(const uint index)noexcept;
		};		 
		struct JSkinnedMeshGroup final : public JMeshGroup
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JSkinnedMeshGroup)
		private:
			std::vector<std::unique_ptr<JSkinnedMeshData>> skinnedMeshData;
			JUserPtr<JIdentifier> skeletonAsset;
		public:
			JSkinnedMeshGroup() = default;
			~JSkinnedMeshGroup() = default;
			JSkinnedMeshGroup(JSkinnedMeshGroup&& rhs) = default;
			JSkinnedMeshGroup& operator=(JSkinnedMeshGroup&& rhs) = default;
		public:
			uint GetMeshDataCount()const noexcept final;
			JMeshData* GetMeshData(const uint index)noexcept final;
			J_MESHGEOMETRY_TYPE GetMeshGroupType()const noexcept final;
			uint GetVertexSize()const noexcept final;
			JUserPtr<JIdentifier> GetSkeletonAsset()const noexcept;
		public:
			void SetSkeletonAsset(JUserPtr<JIdentifier> newSkeletonAsset)noexcept;
		public:
			void AddMeshData(std::unique_ptr<JSkinnedMeshData>&& meshData) noexcept;
			std::unique_ptr<JSkinnedMeshData> PopMeshData(const uint index)noexcept;
		};
		struct JLowMeshGroup final : public JMeshGroup
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JLowMeshGroup)
		private:
			std::vector<std::unique_ptr<JLowMeshData>> lowMeshData; 
		public:
			JLowMeshGroup() = default;
			~JLowMeshGroup() = default;
			JLowMeshGroup(JLowMeshGroup&& rhs) = default;
			JLowMeshGroup& operator=(JLowMeshGroup&& rhs) = default;
		public:
			uint GetMeshDataCount()const noexcept final;
			JMeshData* GetMeshData(const uint index)noexcept final;
			J_MESHGEOMETRY_TYPE GetMeshGroupType()const noexcept final;
			uint GetVertexSize()const noexcept final;
			void AddMeshData(std::unique_ptr<JLowMeshData> && meshData) noexcept;
			std::unique_ptr<JLowMeshData> PopMeshData(const uint index)noexcept;
		};
	}
}