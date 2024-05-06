/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once  
#include"../JComponent.h"
#include"JRenderLayer.h" 
#include"JRenderItemAcceleratorMask.h"   
#include"JRenderItemPrimitive.h"
#include"../../../Graphic/Frameresource/JFrameUpdateUserAccess.h"
#include"../../../Core/Math/JMatrix.h"
#include<DirectXCollision.h> 

namespace JinEngine
{
	class JMeshGeometry;
	class JMaterial;
	class JRenderItemPrivate;
	class JRenderItem final : public JComponent, 
		public Graphic::JFrameUpdateUserAccess
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JRenderItem)
	public: 
		class InitData final : public JComponent::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			J_RENDER_LAYER layer = J_RENDER_LAYER::OPAQUE_OBJECT;
			J_RENDERITEM_ACCELERATOR_MASK acceleratorMask = J_RENDERITEM_ACCELERATOR_MASK::ACCELERATOR_ALLOW_ALL;
		public:
			InitData(const JUserPtr<JGameObject>& owner);
			InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner);
			InitData(const size_t guid, 
				const J_OBJECT_FLAG flag,
				const JUserPtr<JGameObject>& owner,
				const J_RENDER_LAYER layer,
				const J_RENDERITEM_ACCELERATOR_MASK acceleratorMask);
		};
	private:
		friend class JRenderItemPrivate;
		class JRenderItemImpl;
	private:
		std::unique_ptr<JRenderItemImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final; 
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM;
		}
	public:
		JUserPtr<JMeshGeometry> GetMesh()const noexcept;
		JUserPtr<JMaterial> GetValidMaterial(int index)const noexcept;
		std::vector<JUserPtr<JMaterial>> GetMaterialVec()const noexcept;
		JMatrix4x4 GetTextransform()const noexcept;
		J_RENDER_PRIMITIVE GetPrimitiveType()const noexcept;
		J_RENDER_LAYER GetRenderLayer()const noexcept;
		J_RENDERITEM_ACCELERATOR_MASK GetAcceleratorMask()const noexcept; 
		uint GetTotalVertexCount()const noexcept;
		uint GetTotalIndexCount()const noexcept;
		uint GetSubmeshCount()const noexcept;
		//apply scale rotation tranlation 
		DirectX::BoundingBox GetBoundingBox()const noexcept;
		//for debug
		DirectX::BoundingOrientedBox GetDetphTestBoundingBox()const noexcept;
		DirectX::BoundingOrientedBox GetOrientedBoundingBox()const noexcept;
		//apply scale tranlation  
		DirectX::BoundingSphere GetBoundingSphere()const noexcept;
	public:
		void SetMesh(JUserPtr<JMeshGeometry> newMesh)noexcept;
		void SetMaterial(int index, JUserPtr<JMaterial> newMaterial)noexcept;
		void SetMaterialVec(const std::vector<JUserPtr<JMaterial>> newVec)noexcept;
		void SetTextureTransform(const JMatrix4x4& textureTransform)noexcept;
		void SetPrimitiveType(const J_RENDER_PRIMITIVE primitiveType)noexcept;
		void SetRenderLayer(const J_RENDER_LAYER renderLayer)noexcept; 
		void SetAcceleratorMask(const J_RENDERITEM_ACCELERATOR_MASK acceleratorMask)noexcept;
		void SetOccluder(const bool value)noexcept;
	public: 
		bool IsFrameDirted()const noexcept final;
		bool IsAvailableOverlap()const noexcept final;
		/**
		* @brief it is valid in hzb occ & hardware occ
		*/
		bool IsOccluder()const noexcept;
		bool IsIgnoreCullingResult()const noexcept;
		bool PassDefectInspection()const noexcept final; 
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		JRenderItem(const InitData& initData);
		~JRenderItem();
	};
}
