#pragma once  
#include"../JComponent.h"
#include"JRenderLayer.h"
#include"JRenderVisibility.h"
#include"JRenderItemSpaceSpatialMask.h"   
#include"../../JFrameUpdateUserAccess.h"
#include<DirectXCollision.h>
#include<d3d12.h>

namespace JinEngine
{
	class JMeshGeometry;
	class JMaterial;
	class JRenderItemPrivate;
	class JRenderItem final : public JComponent, 
		public JFrameUpdateUserAccessInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JRenderItem)
	public: 
		class InitData final : public JComponent::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(JGameObject* owner);
			InitData(const size_t guid, const J_OBJECT_FLAG flag, JGameObject* owner);
		};
	private:
		friend class JRenderItemPrivate;
		class JRenderItemImpl;
	private:
		std::unique_ptr<JRenderItemImpl> impl;
	public:
		Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final;
		JFrameUpdateUserAccess GetFrameUserInterface() noexcept final;
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM;
		}
	public:
		Core::JUserPtr<JMeshGeometry> GetMesh()const noexcept;
		Core::JUserPtr<JMaterial> GetValidMaterial(int index)const noexcept;
		std::vector<Core::JUserPtr<JMaterial>> GetMaterialVec()const noexcept;
		DirectX::XMFLOAT4X4 GetTextransform()const noexcept;
		D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveType()const noexcept;
		J_RENDER_LAYER GetRenderLayer()const noexcept;
		J_RENDERITEM_SPACE_SPATIAL_MASK GetSpaceSpatialMask()const noexcept; 
		uint GetTotalVertexCount()const noexcept;
		uint GetTotalIndexCount()const noexcept;
		uint GetSubmeshCount()const noexcept;
		//apply scale rotation tranlation 
		DirectX::BoundingBox GetBoundingBox()noexcept;
		//apply scale tranlation  
		DirectX::BoundingSphere GetBoundingSphere()noexcept;
	public:
		void SetMesh(Core::JUserPtr<JMeshGeometry> newMesh)noexcept;
		void SetMaterial(int index, Core::JUserPtr<JMaterial> newMaterial)noexcept;
		void SetMaterialVec(const std::vector<Core::JUserPtr<JMaterial>> newVec)noexcept;
		void SetTextureTransform(const DirectX::XMFLOAT4X4& textureTransform)noexcept;
		void SetPrimitiveType(const D3D12_PRIMITIVE_TOPOLOGY primitiveType)noexcept;
		void SetRenderLayer(const J_RENDER_LAYER renderLayer)noexcept;
		void SetRenderVisibility(const J_RENDER_VISIBILITY renderVisibility)noexcept;
		void SetSpaceSpatialMask(const J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask)noexcept;
	public:
		bool IsVisible()const noexcept;
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final; 
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		JRenderItem(const InitData& initData);
		~JRenderItem();
	};
}
