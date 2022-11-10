#pragma once  
#include"JRenderItemInterface.h"
#include"JRenderLayer.h"
#include"JRenderVisibility.h"
#include"../../../Core/JDataType.h"
#include"../../../Utility/JMathHelper.h" 
#include"../../Resource/Mesh/JMeshGeometry.h"
#include"../../Resource/Material/JMaterial.h"
#include<DirectXCollision.h>
#include<d3d12.h>

namespace JinEngine
{ 
	class JRenderItem final : public JRenderItemInterface
	{
		REGISTER_CLASS(JRenderItem)
	private:
		REGISTER_PROPERTY_EX(mesh, GetMesh, SetMesh, GUI_SELECTOR(true, true))
		JMeshGeometry* mesh = nullptr; 
		REGISTER_PROPERTY_EX(material, GetMaterialVec, SetMaterialVec, GUI_SELECTOR(true, true))
		std::vector<JMaterial*> material;
		DirectX::XMFLOAT4X4 textureTransform = JMathHelper::Identity4x4();
		D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		J_RENDER_LAYER renderLayer = J_RENDER_LAYER::OPAQUE_OBJECT;
		J_RENDER_VISIBILITY renderVisibility = J_RENDER_VISIBILITY::VISIBLE;
	public:
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM;
		}
	public:
		JMeshGeometry* GetMesh()const noexcept; 
		JMaterial* GetValidMaterial(int index)const noexcept;
		std::vector<JMaterial*> GetMaterialVec()const noexcept;
		DirectX::XMFLOAT4X4 GetTextransform()const noexcept;
		D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveType()const noexcept;
		J_RENDER_LAYER GetRenderLayer()const noexcept;  
		REGISTER_METHOD(GetVertexTotalCount, GUI_READONLY_TEXT())
		uint GetVertexTotalCount()const noexcept;
		REGISTER_METHOD(GetIndexTotalCount, GUI_READONLY_TEXT())
		uint GetIndexTotalCount()const noexcept;
		uint GetSubmeshCount()const noexcept; 
		//apply object transform
		DirectX::BoundingBox GetBoundingBox()noexcept;
		//apply object transform
		DirectX::BoundingSphere GetBoundingSphere()noexcept;

		void SetMesh(JMeshGeometry* newMesh)noexcept; 
		void SetMaterial(int index, JMaterial* newMaterial)noexcept;
		void SetMaterialVec(const std::vector<JMaterial*>& newVec)noexcept;
		void SetTextureTransform(const DirectX::XMFLOAT4X4& textureTransform)noexcept;
		void SetPrimitiveType(const D3D12_PRIMITIVE_TOPOLOGY primitiveType)noexcept;
		void SetRenderLayer(const J_RENDER_LAYER renderLayer)noexcept;
		void SetRenderVisibility(const J_RENDER_VISIBILITY renderVisibility)noexcept; 
		 		 
		bool IsVisible()const noexcept;
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final; 
	private:
		void DoCopy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final; 
	private:
		bool UpdateFrame(Graphic::JObjectConstants& constant, 
			Graphic::JBoundingObjectConstants&,
			const uint submeshIndex,
			const bool isUpdateBoundingObj)final;
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
