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
#include"../../../Graphic/Frameresource/JFrameUpdateUserAccess.h"
#include"../../../Graphic/Culling/JCullingUserAccess.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceUserAccess.h"
#include"JLightType.h" 
#include<DirectXCollision.h>

namespace JinEngine
{
	class JMeshGeometry;
	class JLight :public JComponent,
		public Graphic::JFrameUpdateUserAccess,
		public Graphic::JGraphicResourceUserAccess,
		public Graphic::JCullingUserAccess
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JLight)
	public: 
		class InitData : public JComponent::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(const Core::JTypeInfo& typeInfo, const JUserPtr<JGameObject>& owner);
			InitData(const Core::JTypeInfo& typeInfo, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner);
		}; 
	private:
		friend class JLightPrivate;
		class JLightImpl;
	private:
		std::unique_ptr<JLightImpl> impl;
	public: 
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT;
		} 
		JVector3<float> GetColor()const noexcept; 
		uint GetShadowResolution()const noexcept;
		J_SHADOW_RESOLUTION GetShadowResolutionType()const noexcept;
		uint GetShadowMapSize()const noexcept;
		float GetPower()const noexcept;
		float GetBias()const noexcept;
		float GetPenumbraWidth()const noexcept;
		float GetPenumbraBlockerWidth()const noexcept; 
		JComponent::UserCompComparePtr GetLitTypeComparePtr()const noexcept;
		virtual J_LIGHT_TYPE GetLightType()const noexcept = 0;
		virtual J_SHADOW_MAP_TYPE GetShadowMapType()const noexcept = 0;
		virtual float GetMinPower()const noexcept = 0;
		virtual float GetMaxPower()const noexcept = 0;
		virtual float GetFrustumNear()const noexcept = 0;
		virtual float GetFrustumFar()const noexcept = 0;
		virtual DirectX::BoundingBox GetBBox()const noexcept = 0;
		virtual JUserPtr<JMeshGeometry> GetMesh()const noexcept = 0;
		virtual DirectX::XMMATRIX GetMeshWorldM(const bool restrictScaledZ = false)const noexcept = 0;
	public: 
		void SetColor(const JVector3<float>& color)noexcept;  
		virtual void SetShadow(const bool value)noexcept;
		virtual void SetShadowResolution(const J_SHADOW_RESOLUTION sQuality)noexcept;
		virtual void SetAllowDisplayShadowMap(const bool value)noexcept;
		void SetPower(const float value)noexcept;
		void SetBias(const float value)noexcept;
		void SetPenumbraWidth(const float value)noexcept;
		void SetPenumbraBlockerWidth(const float value)noexcept;
	public: 
		bool IsShadowActivated()const noexcept;
		bool IsAvailableOverlap()const noexcept final; 
		bool AllowDisplayShadowMap()const noexcept; 
		bool AllowLightCulling()const noexcept final;  
	protected:
		void DoActivate()noexcept override;
		void DoDeActivate()noexcept override;
	protected:
		JLight(const InitData& initData);
		~JLight();
	};
}