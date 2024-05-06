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


#include"JSpotLight.h"   
#include"JSpotLightPrivate.h"
#include"JLightConstants.h"
#include"../Transform/JTransform.h" 
#include"../JComponentHint.h"
#include"../../JObjectFileIOHelper.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h"  
#include"../../Resource/JResourceManager.h"
#include"../../../Core/Guid/JGuidCreator.h" 
#include"../../../Core/File/JFileConstant.h" 
#include"../../../Core/Func/JFuncList.h"
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Math/JMathHelper.h"
#include"../../../Graphic/JGraphic.h"  
#include"../../../Graphic/Frameresource/JLightConstants.h"    
#include"../../../Graphic/Frameresource/JFrameUpdate.h"
#include"../../../Graphic/Culling/JCullingInterface.h"
#include"../../../Graphic/JGraphicPrivate.h"
#include"../../../Graphic/JGraphic.h"
#include"../../../Graphic/JGraphicDrawListInterface.h"    
#include"../../../Graphic/GraphicResource/JGraphicResourceInterface.h" 
#include<Windows.h>
#include<fstream>

using namespace DirectX;
namespace JinEngine
{
	namespace
	{
		using LitFrameUpdate = Graphic::JFrameUpdate<Graphic::JFrameUpdateInterfaceHolder2<
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, Graphic::JSpotLightConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW, Graphic::JShadowMapDrawConstants&>>,
			Graphic::JFrameDirty>;
		using JCullingSingleHolder = Graphic::JCullingSingleHolder<Graphic::J_CULLING_TYPE::FRUSTUM, Graphic::J_CULLING_TARGET::RENDERITEM>;
	}
	namespace
	{
		static auto isAvailableoverlapLam = []() {return true; };
		static JSpotLightPrivate lPrivate;
	}
	namespace Private
	{
		static constexpr float minPower = 0.1f;
		static constexpr float maxPower = 4.0f;
		static constexpr float minConeDegAngle = 7.5f;
		static constexpr float maxConeDegAngle = 60.0f;
		static constexpr float maxOuterConeDegAngle = 75.0f;
		static constexpr float minConeAngle = minConeDegAngle * JMathHelper::DegToRad;
		static constexpr float maxConeAngle = maxConeDegAngle * JMathHelper::DegToRad;
		static constexpr float maxOuterConeAngle = maxOuterConeDegAngle * JMathHelper::DegToRad;
		static constexpr float minAspect = 0.1f;
		static constexpr float maxAspect = 16.0f;

		using GetFrameDataPtr = Graphic::JFrameUpdateData* (*)(JLight*);
		static GetFrameDataPtr getFrameDataPtr[Graphic::LightFrameLayer::setCount];

		static constexpr float InitPower()noexcept
		{
			return 1.0f;
		}
		static constexpr float InitInnerConeAngle()noexcept
		{
			return 30.0f * JMathHelper::DegToRad;
		}
		static constexpr float InitOuterConeAngle()noexcept
		{
			return 31.0f * JMathHelper::DegToRad;
		}
		static constexpr float InitSpotAspect()noexcept
		{
			return 1.0f;
		}
		static XMVECTOR CalLightWorldDir(const JUserPtr<JTransform>& transform, const JVector3<float>& initDir = JVector3<float>(0, -1, 0)) noexcept
		{
			return XMVector3Normalize(XMVector3Rotate(initDir.ToXmV(), transform->GetWorldQuaternion().ToXmV()));
		}
		static XMVECTOR CalLightWorldPos(const JUserPtr<JTransform>& transform) noexcept
		{
			return transform->GetWorldPosition().ToXmV();
		}
		static XMMATRIX CalView(const JUserPtr<JTransform>& transform) noexcept
		{
			JMatrix4x4 m;
			JTransform::CalTransformMatrix(m,
				transform,
				transform->GetPosition(),
				transform->GetRight(),
				transform->GetFront(),
				transform->GetUp() * JVector3F::NegativeOne());
			return m.LoadXM();
		}
		static XMMATRIX CalProj(const float fNear, const float fFar, const float angle, const float aspect)noexcept
		{
			//Caution!
			//Near값은 1보다 작을시 shadow map에 그려지는 물체들의 깊이값이 비정확해진다.
			return XMMatrixPerspectiveFovLH(angle, aspect, fNear, fFar);
		}
	}

	class JSpotLight::JSpotLightImpl : public Core::JTypeImplBase,
		public LitFrameUpdate,
		public Graphic::JGraphicWideSingleResourceHolder<2>,	//shadowMap, debug,
		public Graphic::JGraphicDrawListCompInterface,
		public JCullingSingleHolder
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JSpotLightImpl)
	public:
		using SpotLitFrame = JFrameInterface1;
		using ShadowMapDrawFrame = JFrameInterface2;
	private:
		enum MANAGED_SET
		{
			MANAGED_SET_SHADOW_MAP = 0,
			MANAGED_SET_DISPLAY_SHADOW_MAP,		//for debugging
			MANAGED_SET_FRUSTUM_CULLING,
			MANAGED_SET_COUNT
		};
		using ManageFuncList = Core::JFuncList<MANAGED_SET_COUNT, JSpotLight::JSpotLightImpl, const bool>;
	public:
		JMatrix4x4 view;
		JMatrix4x4 proj;
		JVector3F direction;
	public:
		JWeakPtr<JSpotLight> thisPointer;
	public:
		REGISTER_PROPERTY_EX(range, GetRange, SetRange, GUI_SLIDER(Constants::localLightMinDistance, Constants::localLightMaxDistance, true, false))
		float range = 32.0f;
		REGISTER_PROPERTY_EX(innerConeAngle, GetInnerConeDegAngle, SetInnerConeDegAngle, GUI_SLIDER(Private::minConeDegAngle, Private::maxConeDegAngle, true, false))
		float innerConeAngle = Private::InitInnerConeAngle();
		REGISTER_PROPERTY_EX(outerConeAngle, GetOuterConeDegAngle, SetOuterConeDegAngle, GUI_SLIDER(Private::minConeDegAngle, Private::maxOuterConeDegAngle, true, false))
		float outerConeAngle = Private::InitOuterConeAngle();
	public:
		bool allowFrustumCulling = false;
	public:
		JSpotLightImpl(const InitData& initData, JSpotLight* thisLitRaw)
		{
		}
		~JSpotLightImpl()
		{ }
	public:
		J_LIGHT_TYPE GetLightType()const noexcept
		{
			return J_LIGHT_TYPE::SPOT;
		}
		J_SHADOW_MAP_TYPE GetShadowMapType()const noexcept
		{
			if (!thisPointer->IsShadowActivated())
				return J_SHADOW_MAP_TYPE::NONE;

			return J_SHADOW_MAP_TYPE::NORMAL;
		}
		float GetFrustumNear()const noexcept
		{
			return Constants::localLightMinDistance;
		}
		float GetFrustumFar()const noexcept
		{
			return range;
		}
		float GetRange()const noexcept
		{
			return range;
		}
		float GetInnerConeAngle()const noexcept
		{
			return innerConeAngle;
		}
		float GetInnerConeDegAngle()const noexcept
		{
			return innerConeAngle * JMathHelper::RadToDeg;
		}
		float GetOuterConeAngle()const noexcept
		{
			return outerConeAngle;
		}
		float GetOuterConeDegAngle()const noexcept
		{
			return outerConeAngle * JMathHelper::RadToDeg;
		}
		DirectX::BoundingBox GetBBox()const
		{
			auto t = GetTransform();
			auto litFar = GetFrustumFar();
			auto radius = litFar * tan(outerConeAngle);
			auto centerV = XMVectorAdd(Private::CalLightWorldPos(t), XMVectorScale(Private::CalLightWorldDir(t), litFar * 0.5f));
			//spot init dir is (0, -1, 0)
			auto extentsV = XMVector3Rotate(XMVectorSet(radius, -litFar * 0.5f, radius, 1.0f), t->GetQuaternion().ToXmV());

			DirectX::BoundingBox bbox;
			XMStoreFloat3(&bbox.Center, centerV);
			XMStoreFloat3(&bbox.Extents, XMVectorAbs(extentsV));
			return bbox;
		} 
		JUserPtr<JMeshGeometry> GetMesh()const noexcept
		{
			return _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::LOW_CONE);
		}
		DirectX::XMMATRIX GetMeshWorldM(const bool restrictScaledZ)const noexcept
		{
			JUserPtr<JTransform> t = GetTransform();
			JVector3F p;
			JVector4F q;
			JVector3F s;
			t->GetWorldPQS(p, q, s);

			float bottomRadius = range * tan(outerConeAngle);
			s = JVector3F(bottomRadius, bottomRadius, restrictScaledZ ? 1 : range);
			 
			return DirectX::XMMatrixAffineTransformation(s.ToXmV(), JVector4F::Zero().ToXmV(), q.ToXmV(), p.ToXmV());
		}
		XMMATRIX GetShadowMapTransform()const noexcept
		{
			return XMMatrixMultiply(XMMatrixMultiply(view.LoadXM(), proj.LoadXM()), JMatrix4x4::NdcToTextureSpaceXM());
		}
		JUserPtr<JTransform> GetTransform()const noexcept
		{
			return thisPointer->GetOwner()->GetTransform();
		}
		int GetResourceDataIndex(const Graphic::J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_TASK_TYPE taskType)const noexcept
		{
			switch (rType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
				return 0;
			default:
				return invalidIndex;
			}
		}
	public:
		void SetShadow(bool value)noexcept
		{
			SetFuncList().InvokePassCondition(MANAGED_SET_SHADOW_MAP, this, std::forward<bool>(value));
		}
		void SetShadowResolution(const J_SHADOW_RESOLUTION newShadowResolution)noexcept
		{
			if (thisPointer->IsActivated() && thisPointer->IsShadowActivated())
			{
				DestroyShadowMapResource();
				CreateShadowMapResource();
			}
			SetFrameDirty();
		}
		void SetAllowDisplayShadowMap(bool value)
		{
			SetFuncList().InvokePassCondition(MANAGED_SET_DISPLAY_SHADOW_MAP, this, std::forward<bool>(value));
		}
		void SetAllowFrustumCulling(bool value)noexcept
		{
			if (allowFrustumCulling == value)
				return;

			allowFrustumCulling = value;
			SetFuncList().InvokePassCondition(MANAGED_SET_FRUSTUM_CULLING, this, std::forward<bool>(value));
		}
		void SetRange(const float newRange)noexcept
		{
			range = std::clamp(newRange, Constants::localLightMinDistance, Constants::localLightMaxDistance);
			SetFrameDirty();

			JLightPrivate::ChildInterface::UpdateLightShape(thisPointer); 
		}
		void SetInnerConeAngle(const float newAngle)noexcept
		{
			innerConeAngle = std::clamp(newAngle, Private::minConeAngle, outerConeAngle);
			SetFrameDirty();
		}
		void SetInnerConeDegAngle(const float newAngle)noexcept
		{
			SetInnerConeAngle(newAngle * JMathHelper::DegToRad);
		}
		void SetOuterConeAngle(const float newAngle)noexcept
		{
			outerConeAngle = std::clamp(newAngle, innerConeAngle + 0.1f, Private::maxConeAngle);
			SetFrameDirty();

			JLightPrivate::ChildInterface::UpdateLightShape(thisPointer);
		}
		void SetOuterConeDegAngle(const float newAngle)noexcept
		{
			SetOuterConeAngle(newAngle * JMathHelper::DegToRad);
		}
		static ManageFuncList& SetFuncList()
		{
			static ManageFuncList setFuncList;
			return setFuncList;
		}
	public:
		bool IsShadowActivated()const noexcept
		{
			return thisPointer->IsShadowActivated();
		}
		bool AllowDisplayShadowMap()const noexcept
		{
			return thisPointer->AllowDisplayShadowMap();
		}
		bool AllowFrustumCulling()const noexcept
		{
			return allowFrustumCulling;
		}
		bool AllowHzbOcclusionCulling()const noexcept
		{
			return false;
		}
		bool AllowHdOcclusionCulling()const noexcept
		{
			return false;
		}
		bool AllowDisplayOccCullingDepthMap()const noexcept
		{
			return false;
		} 
	public:
		void CreateShadowMapResource()noexcept
		{
			CreateResource(JVector2F(thisPointer->GetShadowMapSize()), Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP);
			CreateFrustumCullingData();

			if (thisPointer->AllowDisplayShadowMap())
				CreateShadowMapDebugResource();

			RegisterLightFrameData(JLightType::SmToFrameR(GetLightType(), false));
			AddDrawShadowRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
			AddFrustumCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
		}
		void DestroyShadowMapResource()noexcept
		{
			DeRegisterLightFrameData(JLightType::SmToFrameR(GetLightType(), false));
			DestroyShadowMapDebugResource();
			DestroyAllTexture();

			DestroyCullingData(Graphic::J_CULLING_TYPE::FRUSTUM);
			PopDrawRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
			PopFrustumCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
		};
		void CreateShadowMapDebugResource()
		{
			CreateResource(JVector2F(thisPointer->GetShadowMapSize()), Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP);
		}
		void DestroyShadowMapDebugResource()
		{
			DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP);
		}
	public:
		void Activate()noexcept
		{
			RegisterLightFrameData(JLightType::LitToFrameR(GetLightType()));
			SetFuncList().InvokeAll(this, true, true);
		}
		void DeActivate()noexcept
		{
			//has order dependency
			DeRegisterLightFrameData(JLightType::LitToFrameR(GetLightType()));
			SetFuncList().InvokeAll(this, true, false);
			DestroyAllCullingData();
			DestroyAllTexture();
		}
	public:
		void UpdateFrame(Graphic::JSpotLightConstants& constant)noexcept final
		{
			if (IsShadowActivated())
				constant.shadowMapTransform.StoreXM(XMMatrixTranspose(GetShadowMapTransform()));

			constant.color = thisPointer->GetColor();
			constant.power = thisPointer->GetPower();
			constant.position = Private::CalLightWorldPos(GetTransform());
			constant.frustumNear = GetFrustumNear();
			constant.direction = direction;
			constant.frustumFar = GetFrustumFar();
			constant.innerConeCosAngle = cos(innerConeAngle);
			constant.outerConeCosAngle = cos(outerConeAngle);
			constant.outerConeAngle = outerConeAngle;
			constant.penumbraScale = thisPointer->GetPenumbraWidth();
			constant.penumbraBlockerScale = thisPointer->GetPenumbraBlockerWidth();
			constant.shadowMapIndex = IsShadowActivated() ? GetResourceArrayIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, 0) : 0;
			constant.hasShadowMap = IsShadowActivated();
			constant.shadowMapSize = thisPointer->GetShadowMapSize();
			constant.shadowMapInvSize = 1.0f / constant.shadowMapSize;
			constant.bias = thisPointer->GetBias();
			SpotLitFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JShadowMapDrawConstants& constant)noexcept final
		{
			constant.shadowMapTransform.StoreXM(XMMatrixTranspose(XMMatrixMultiply(view.LoadXM(), proj.LoadXM())));
			ShadowMapDrawFrame::MinusMovedDirty();
		}
		void UpdateLightTransform()noexcept
		{
			view.StoreXM(Private::CalView(GetTransform()));
			proj.StoreXM(Private::CalProj(GetFrustumNear(), GetFrustumFar(), outerConeAngle * 2, 1));
			direction = Private::CalLightWorldDir(GetTransform());
		}
	public:
		static bool DoCopy(JSpotLight* from, JSpotLight* to)
		{
			from->impl->SetAllowFrustumCulling(to->impl->AllowFrustumCulling());
			from->impl->SetRange(to->impl->GetRange());
			from->impl->SetInnerConeAngle(to->impl->GetInnerConeAngle());
			from->impl->SetOuterConeAngle(to->impl->GetOuterConeAngle());

			to->impl->SetFrameDirty();
			return true;
		}
	public:
		void NotifyReAlloc()
		{
			if (thisPointer.IsValid())
			{
				JLightPrivate::ChildInterface::DeRegisterFrameDirtyListener(thisPointer.Get(), thisPointer->GetGuid());
				JLightPrivate::ChildInterface::RegisterFrameDirtyListener(thisPointer.Get(), this, thisPointer->GetGuid());
			}
			RegisterInterfacePointer();
			JFrameUpdateData::ReRegisterFrameData(JLightType::LitToFrameR(GetLightType()), (SpotLitFrame*)this);
			JFrameUpdateData::ReRegisterFrameData(JLightType::SmToFrameR(GetLightType(), false), (ShadowMapDrawFrame*)this);
		}
	public:
		void RegisterThisPointer(JSpotLight* lit)
		{
			thisPointer = Core::GetWeakPtr(lit);
		}
		void RegisterInterfacePointer()
		{
			Graphic::JGraphicResourceInterface::SetInterfacePointer(this);
			Graphic::JCullingInterface::SetInterfacePointer(this);
		}
		void RegisterPostCreation()
		{
			JLightPrivate::ChildInterface::RegisterFrameDirtyListener(thisPointer.Get(), this, thisPointer->GetGuid());
		}
		void RegisterLightFrameData(const Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE type)
		{
			if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT)
				SpotLitFrame::RegisterFrameData(type, (SpotLitFrame*)this, thisPointer->GetOwner()->GetOwnerGuid(), 1);
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW)
				ShadowMapDrawFrame::RegisterFrameData(type, (ShadowMapDrawFrame*)this, thisPointer->GetOwner()->GetOwnerGuid(), 1);
		}
		void DeRegisterPreDestruction()
		{
			if (thisPointer != nullptr)
				JLightPrivate::ChildInterface::DeRegisterFrameDirtyListener(thisPointer.Get(), thisPointer->GetGuid());
		}
		void DeRegisterLightFrameData(const Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE type)
		{
			if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT)
				SpotLitFrame::DeRegisterFrameData(type, (SpotLitFrame*)this);
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW)
				ShadowMapDrawFrame::DeRegisterFrameData(type, (ShadowMapDrawFrame*)this);
		}
		static void RegisterTypeData()
		{
			Core::JIdentifier::RegisterPrivateInterface(JSpotLight::StaticTypeInfo(), lPrivate);
			IMPL_REALLOC_BIND(JSpotLight::JSpotLightImpl, thisPointer)
				SET_GUI_FLAG(Core::J_GUI_OPTION_FLAG::J_GUI_OPTION_DISPLAY_PARENT);

			auto setShadowMapLam = [](JSpotLightImpl* impl, const bool value)
			{
				if (value)
					impl->CreateShadowMapResource();
				else
					impl->DestroyShadowMapResource();
				impl->SetFrameDirty();
			};
			auto setDisplayShadowMapLam = [](JSpotLightImpl* impl, const bool value)
			{
				if (value)
					impl->CreateShadowMapDebugResource();
				else
					impl->DestroyShadowMapDebugResource();
				impl->SetFrameDirty();
			};
			auto setFrustumCullingLam = [](JSpotLightImpl* impl, const bool value)
			{
				if (value)
				{
					impl->CreateFrustumCullingData();
					impl->AddFrustumCullingRequest(impl->thisPointer->GetOwner()->GetOwnerScene(), impl->thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
				}
				else
				{
					impl->DestroyCullingData(Graphic::J_CULLING_TYPE::FRUSTUM);
					impl->PopFrustumCullingRequest(impl->thisPointer->GetOwner()->GetOwnerScene(), impl->thisPointer);
				}
				impl->SetFrameDirty();
			};

			using SetCallable = Core::JStaticCallable<void, JSpotLightImpl*, const bool>;
			using CondCallable = Core::JMemberCNCallable<JSpotLightImpl, bool>;

			SetFuncList().Register(std::make_unique<SetCallable>(setShadowMapLam), std::make_unique<CondCallable>(&JSpotLightImpl::IsShadowActivated), MANAGED_SET_SHADOW_MAP);
			SetFuncList().Register(std::make_unique<SetCallable>(setDisplayShadowMapLam), std::make_unique<CondCallable>(&JSpotLightImpl::AllowDisplayShadowMap), MANAGED_SET_DISPLAY_SHADOW_MAP);
			SetFuncList().Register(std::make_unique<SetCallable>(setFrustumCullingLam), std::make_unique<CondCallable>(&JSpotLightImpl::AllowFrustumCulling), MANAGED_SET_FRUSTUM_CULLING);
		
			auto getSpotFrameLam = [](JLight* lit)->JFrameUpdateData* {return (SpotLitFrame*)(static_cast<JSpotLight*>(lit)->impl.get()); };
			auto getShadowMapFrame = [](JLight* lit)->JFrameUpdateData* {return (ShadowMapDrawFrame*)(static_cast<JSpotLight*>(lit)->impl.get()); };
			 
			Private::getFrameDataPtr[Graphic::LightFrameLayer::light] = getSpotFrameLam;
			Private::getFrameDataPtr[Graphic::LightFrameLayer::shadowMap] = getShadowMapFrame;
		}
	};

	JSpotLight::InitData::InitData(const JUserPtr<JGameObject>& owner)
		:JLight::InitData(JSpotLight::StaticTypeInfo(), owner)
	{}
	JSpotLight::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner)
		: JLight::InitData(JSpotLight::StaticTypeInfo(), guid, flag, owner)
	{}

	Core::JIdentifierPrivate& JSpotLight::PrivateInterface()const noexcept
	{
		return lPrivate;
	}
	const Graphic::JGraphicResourceUserInterface JSpotLight::GraphicResourceUserInterface()const noexcept
	{
		return Graphic::JGraphicResourceUserInterface(impl.get());
	}
	const Graphic::JCullingUserInterface JSpotLight::CullingUserInterface()const noexcept
	{
		return Graphic::JCullingUserInterface(impl.get());
	}
	J_LIGHT_TYPE JSpotLight::GetLightType()const noexcept
	{
		return impl->GetLightType();
	}
	J_SHADOW_MAP_TYPE JSpotLight::GetShadowMapType()const noexcept
	{
		return impl->GetShadowMapType();
	}
	float JSpotLight::GetMinPower()const noexcept
	{
		return Private::minPower;
	}
	float JSpotLight::GetMaxPower()const noexcept
	{
		return Private::maxPower;
	}
	float JSpotLight::GetFrustumNear()const noexcept
	{
		return impl->GetFrustumNear();
	}
	float JSpotLight::GetFrustumFar()const noexcept
	{
		return impl->GetFrustumFar();
	}
	float JSpotLight::GetRange()const noexcept
	{
		return impl->GetRange();
	}
	float JSpotLight::GetInnerConeAngle()const noexcept
	{
		return impl->GetInnerConeAngle();
	}
	float JSpotLight::GetOuterConeAngle()const noexcept
	{
		return impl->GetOuterConeAngle();
	}
	DirectX::BoundingBox JSpotLight::GetBBox()const noexcept
	{
		return impl->GetBBox();
	}
	JUserPtr<JMeshGeometry> JSpotLight::GetMesh()const noexcept
	{
		return impl->GetMesh();
	}
	DirectX::XMMATRIX JSpotLight::GetMeshWorldM(const bool restrictScaledZ)const noexcept
	{
		return impl->GetMeshWorldM(restrictScaledZ);
	}
	JVector3F JSpotLight::GetDirection()const noexcept
	{
		return Private::CalLightWorldDir(impl->GetTransform());
	}
	void JSpotLight::SetShadow(const bool value)noexcept
	{
		if (value == IsShadowActivated())
			return;

		JLight::SetShadow(value);
		impl->SetShadow(value);
	}
	void JSpotLight::SetShadowResolution(const J_SHADOW_RESOLUTION sQuality)noexcept
	{
		if (sQuality == GetShadowResolutionType())
			return;

		JLight::SetShadowResolution(sQuality);
		impl->SetShadowResolution(sQuality);
	}
	void JSpotLight::SetAllowDisplayShadowMap(const bool value)noexcept
	{
		if (value == AllowDisplayShadowMap())
			return;

		JLight::SetAllowDisplayShadowMap(value);
		impl->SetAllowDisplayShadowMap(value);
	}
	void JSpotLight::SetRange(const float range)noexcept
	{
		impl->SetRange(range);
	}
	void JSpotLight::SetInnerConeAngle(const float spotAngle)noexcept
	{
		impl->SetInnerConeAngle(spotAngle);
	}
	void JSpotLight::SetOuterConeAngle(const float spotAngle)noexcept
	{
		impl->SetOuterConeAngle(spotAngle);
	}
	bool JSpotLight::IsFrameDirted()const noexcept
	{
		return impl->IsFrameDirted();
	}
	bool JSpotLight::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection())
			return true;
		else
			return false;
	}
	bool JSpotLight::AllowFrustumCulling()const noexcept
	{
		return impl->AllowFrustumCulling();
	}
	bool JSpotLight::AllowHzbOcclusionCulling()const noexcept
	{
		return impl->AllowHzbOcclusionCulling();
	}
	bool JSpotLight::AllowHdOcclusionCulling()const noexcept
	{
		return impl->AllowHdOcclusionCulling();
	}
	bool JSpotLight::AllowDisplayOccCullingDepthMap()const noexcept
	{
		return impl->AllowDisplayOccCullingDepthMap();
	} 
	void JSpotLight::DoActivate()noexcept
	{		
		//Caution 
		//Activate와 RegisterComponent는 순서에 종속성을 가진다.
		//RegisterComponent는 Scene과 가속구조에 Component에 대한 정보를 추가하는 작업으로
		//Activate Process중에 자기자신과 관련된 Scene component vector, Scene As관련 data에 대한 호출은 에러를 일으킬 수 있다.
		JLight::DoActivate();
		impl->Activate();
		impl->SetFrameDirty();
		RegisterComponent(impl->thisPointer, GetLitTypeComparePtr());
	}
	void JSpotLight::DoDeActivate()noexcept
	{
		DeRegisterComponent(impl->thisPointer);
		impl->DeActivate();
		impl->OffFrameDirty();
		JLight::DoDeActivate();
	}
	JSpotLight::JSpotLight(const InitData& initData)
		:JLight(initData), impl(std::make_unique<JSpotLightImpl>(initData, this))
	{ }
	JSpotLight::~JSpotLight()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JSpotLightPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JSpotLightPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JSpotLightPrivate::AssetDataIOInterface;
	using FrameUpdateInterface = JSpotLightPrivate::FrameUpdateInterface;
	using FrameIndexInterface = JSpotLightPrivate::FrameIndexInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JSpotLight>(*static_cast<JSpotLight::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JLightPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JSpotLight* lit = static_cast<JSpotLight*>(createdPtr);
		lit->impl->RegisterThisPointer(lit);
		lit->impl->RegisterInterfacePointer();
		lit->impl->RegisterPostCreation();
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JSpotLight::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JSpotLight::StaticTypeInfo());
		if (!canCopy)
			return false;

		if (!JLightPrivate::CreateInstanceInterface::Copy(from, to))
			return false;

		return JSpotLight::JSpotLightImpl::DoCopy(static_cast<JSpotLight*>(from.Get()), static_cast<JSpotLight*>(to.Get()));
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		static_cast<JSpotLight*>(ptr)->impl->DeRegisterPreDestruction();
		JLightPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JSpotLight::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;
		bool isActivated;

		float sRange;
		float sInnerAngle;
		float sOuterConeAngle;

		auto loadData = static_cast<JSpotLight::LoadData*>(data);
		JFileIOTool& tool = loadData->tool;
		JUserPtr<JGameObject> owner = loadData->owner;

		JObjectFileIOHelper::LoadComponentIden(tool, guid, flag, isActivated);
		auto idenUser = lPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JSpotLight::InitData>(guid, flag, owner), &lPrivate);
		JUserPtr<JSpotLight> litUser;
		litUser.ConnnectChild(idenUser);

		JLightPrivate::AssetDataIOInterface::LoadLightData(tool, litUser);
		JObjectFileIOHelper::LoadAtomicData(tool, sRange, "Range:");
		JObjectFileIOHelper::LoadAtomicData(tool, sInnerAngle, "InnerConeAngle:");
		JObjectFileIOHelper::LoadAtomicData(tool, sOuterConeAngle, "OuterConeAngle:");

		litUser->SetRange(sRange);
		litUser->impl->innerConeAngle = sInnerAngle;
		litUser->impl->outerConeAngle = sOuterConeAngle;
		litUser->impl->SetInnerConeAngle(sInnerAngle);
		litUser->impl->SetOuterConeAngle(sOuterConeAngle);
		if (!isActivated)
			litUser->DeActivate();
		return litUser;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JSpotLight::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JSpotLight::StoreData*>(data);
		if (!storeData->HasCorrectType(JSpotLight::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JUserPtr<JSpotLight> lit;
		lit.ConnnectChild(storeData->obj);

		JSpotLight::JSpotLightImpl* impl = lit->impl.get();
		JFileIOTool& tool = storeData->tool;

		JObjectFileIOHelper::StoreComponentIden(tool, lit.Get());
		JLightPrivate::AssetDataIOInterface::StoreLightData(tool, lit);
		JObjectFileIOHelper::StoreAtomicData(tool, impl->range, "Range:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->innerConeAngle, "InnerConeAngle:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->outerConeAngle, "OuterConeAngle:");

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	bool FrameUpdateInterface::UpdateStart(JLight* lit, const bool isUpdateForced)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::SPOT)
			return false;

		JSpotLight* dLit = static_cast<JSpotLight*>(lit);
		if (isUpdateForced)
			dLit->impl->SetFrameDirty();

		dLit->impl->SetLastFrameUpdatedTrigger(false);
		dLit->impl->SetLastFrameHotUpdatedTrigger(false);
		if (dLit->impl->IsFrameHotDirted())
			dLit->impl->UpdateLightTransform();
		return dLit->impl->IsFrameDirted();
	}
	void FrameUpdateInterface::UpdateFrame(JLight* lit, Graphic::JLightConstantsSet& set)noexcept
	{
		auto impl = static_cast<JSpotLight*>(lit)->impl.get();
		if (set.updateStart)
		{
			impl->UpdateFrame(set.spotLight);
			set.SetUpdated(Graphic::LightFrameLayer::light, impl->SpotLitFrame::GetFrameIndex());

			if (impl->ShadowMapDrawFrame::HasValidFrameIndex())
			{
				impl->UpdateFrame(set.shadowMap);
				set.SetUpdated(Graphic::LightFrameLayer::shadowMap, impl->ShadowMapDrawFrame::GetFrameIndex());
			} 
		}
		else
		{ 
			if (impl->ShadowMapDrawFrame::HasMovedDirty())
			{
				impl->UpdateFrame(set.shadowMap);
				set.SetUpdated(Graphic::LightFrameLayer::shadowMap, impl->ShadowMapDrawFrame::GetFrameIndex());
			} 
			if (impl->SpotLitFrame::HasMovedDirty() || set.isUpdated[Graphic::LightFrameLayer::shadowMap])
			{
				/*
				* 	light constants에는 shadow map index에 대한 변수가 있으므로
				*	shadow map update시 light constants와 shadow constants를 동시에
				*	Update해줄 필요가 있다. ex)graphic resource destroy인한 index변경 반영
				*/
				impl->UpdateFrame(set.spotLight);
				set.SetUpdated(Graphic::LightFrameLayer::light, impl->SpotLitFrame::GetFrameIndex());
			}
		}
	}
	void FrameUpdateInterface::UpdateEnd(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::SPOT)
			return;

		JSpotLight* dLit = static_cast<JSpotLight*>(lit);
		if (dLit->impl->IsFrameHotDirted())
			dLit->impl->SetLastFrameHotUpdatedTrigger(true);
		dLit->impl->SetLastFrameUpdatedTrigger(true);
		dLit->impl->UpdateFrameEnd();
	}
	int FrameUpdateInterface::GetFrameIndex(JLight* lit, const uint layerIndex)noexcept
	{
		return Private::getFrameDataPtr[layerIndex](lit)->GetFrameIndex();
	}
	int FrameUpdateInterface::GetFrameIndexSize(JLight* lit, const uint layerIndex)noexcept
	{
		return Private::getFrameDataPtr[layerIndex](lit)->GetFrameIndexSize();
	}
	int FrameUpdateInterface::GetShadowFrameLayerIndex(JLight* lit)noexcept
	{
		return Graphic::LightFrameLayer::shadowMap;
	}
	bool FrameUpdateInterface::IsFrameHotDirted(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::SPOT)
			return false;

		return static_cast<JSpotLight*>(lit)->impl->IsFrameHotDirted();
	}
	bool FrameUpdateInterface::IsLastFrameHotUpdated(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::SPOT)
			return false;

		return static_cast<JSpotLight*>(lit)->impl->IsLastFrameHotUpdated();
	}
	bool FrameUpdateInterface::IsLastUpdated(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::SPOT)
			return false;

		return static_cast<JSpotLight*>(lit)->impl->IsLastFrameUpdated();
	}

	int FrameIndexInterface::GetFrameIndex(JLight* lit, const uint layerIndex)noexcept
	{
		return Private::getFrameDataPtr[layerIndex](lit)->GetFrameIndex();
	}
	int FrameIndexInterface::GetShadowFrameLayerIndex(JLight* lit)noexcept
	{
		return Graphic::LightFrameLayer::shadowMap;
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JSpotLightPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JSpotLightPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JSpotLightPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
	JLightPrivate::FrameUpdateInterface& JSpotLightPrivate::GetFrameUpdateInterface()const noexcept
	{
		static FrameUpdateInterface pI;
		return pI;
	}
	JLightPrivate::FrameIndexInterface& JSpotLightPrivate::GetFrameIndexInterface()const noexcept
	{
		static FrameIndexInterface pI;
		return pI;
	}
}