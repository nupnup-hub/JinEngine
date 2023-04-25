#pragma once
#include"../JResourceObject.h" 
#include"JMeshType.h" 
#include<DirectXCollision.h> 

namespace JinEngine
{
	struct JMeshGroup; 
	class JMeshGeometryPrivate;
	class JMaterial;
	class JMeshGeometry : public JResourceObject
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JMeshGeometry)
	public: 
		class InitData : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			std::unique_ptr<JMeshGroup> meshGroup;
		public:
			InitData(const Core::JTypeInfo& type,
				const uint8 formatIndex, 
				JDirectory* directory,
				std::unique_ptr<JMeshGroup>&& meshGroup);
			InitData(const Core::JTypeInfo& type, 
				const size_t guid,
				const uint8 formatIndex,
				JDirectory* directory,
				std::unique_ptr<JMeshGroup>&& meshGroup);
			InitData(const Core::JTypeInfo& type, 
				const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex,
				JDirectory* directory,
				std::unique_ptr<JMeshGroup>&& meshGroup);
		public:
			bool IsValidData()const noexcept override;
		};
	protected: 
		class LoadMetaData : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadMetaData)
		public:
			J_MESHGEOMETRY_TYPE meshType;
		public:
			LoadMetaData(const Core::JTypeInfo& type, JDirectory* directory);
		};
	private:
		friend class JMeshGeometryPrivate;
		class JMeshGeometryImpl;
	private:
		std::unique_ptr<JMeshGeometryImpl> impl;
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
	protected:
		JMeshGroup* GetMeshGroupData()const noexcept;
	protected:
		void DoActivate()noexcept override;
		void DoDeActivate()noexcept override; 
	protected:
		JMeshGeometry(InitData& initData);
		~JMeshGeometry();
	};
}
  