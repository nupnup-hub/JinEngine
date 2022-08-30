#pragma once  
#include"JRenderItemInterface.h"
#include"../../../Utility/JMathHelper.h" 
#include"../../../Core/JDataType.h"
#include"JRenderLayer.h"
#include"JRenderVisibility.h"
#include<DirectXCollision.h>
#include<d3d12.h>

namespace JinEngine
{ 
	class JMeshGeometry;
	class JMaterial;

	class JRenderItem : public JRenderItemInterface
	{
		REGISTER_CLASS(JRenderItem)
	private:
		JMeshGeometry* meshGeo = nullptr;
		JMaterial* material = nullptr; 
		DirectX::XMFLOAT4X4 textureTransform;
		D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		J_RENDER_LAYER renderLayer = J_RENDER_LAYER::OPAQUE_OBJECT;
		J_RENDER_VISIBILITY renderVisibility = J_RENDER_VISIBILITY::VISIBLE;
		 
		uint startIndexLocation = 0;
		int baseVertexLocation = 0;
		// Only applicable to skinned render-items.
		//uint aniCBIndex = -1;
		// nullptr if this render-item is not animated by skinned mesh.
		//SkinnedModelInstance* SkinnedModelInst = nullptr;
	public:
		JMeshGeometry* GetMesh()const noexcept;
		JMaterial* GetMaterial()const noexcept;
		DirectX::XMFLOAT4X4 GetTextransform()const noexcept;
		D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveType()const noexcept;
		J_RENDER_LAYER GetRenderLayer()const noexcept;  
		uint GetIndexCount()const noexcept;
		uint GetStartIndexLocation()const noexcept;
		int GetBaseVertexLocation()const noexcept;
		DirectX::BoundingBox GetBoundingBox()noexcept;
		DirectX::BoundingSphere GetBoundingSphere()noexcept;

		void SetMeshGeometry(JMeshGeometry* meshGeo)noexcept;
		void SetMaterial(JMaterial* material)noexcept;
		void SetTextureTransform(const DirectX::XMFLOAT4X4& textureTransform)noexcept;
		void SetPrimitiveType(const D3D12_PRIMITIVE_TOPOLOGY primitiveType)noexcept;
		void SetRenderLayer(const J_RENDER_LAYER renderLayer)noexcept;
		void SetRenderVisibility(const J_RENDER_VISIBILITY renderVisibility)noexcept; 

		bool HasMaterial()const noexcept; 		 
		bool IsVisible()const noexcept;
		 
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final; 
	public:
		bool Copy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final; 
	private:
		bool UpdateFrame(Graphic::JObjectConstants& constant)final;
	private:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
	private:
		Core::J_FILE_IO_RESULT CallStoreComponent(std::wofstream& stream)final;
		static Core::J_FILE_IO_RESULT StoreObject(std::wofstream& stream, JRenderItem* renderItem);
		static JRenderItem* LoadObject(std::wifstream& stream, JGameObject* owner);
		static void RegisterJFunc();
	private:
		JRenderItem(const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
		~JRenderItem();
	};
}
