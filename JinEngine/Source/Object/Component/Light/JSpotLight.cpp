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
#include"../../GraphicRule/JGraphicModuleInterfaceHolder.h"
#include"../../GraphicRule/JGraphicModuleUtility.h"
#include<Windows.h>
#include<fstream>

using namespace DirectX;
namespace JinEngine
{
	namespace Private
	{
		static JSpotLightPrivate instance;

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

	class JSpotLight::JSpotLightImpl : public Core::JTypeImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JSpotLightImpl)
	private:
		enum MANAGED_SET
		{
			MANAGED_SET_SHADOW_MAP = 0,
			MANAGED_SET_DISPLAY_SHADOW_MAP,		//for debugging
			MANAGED_SET_FRUSTUM_CULLING,
			MANAGED_SET_COUNT
		};
		using ManageFuncList = Core::JFuncList<MANAGED_SET_COUNT, JSpotLight::JSpotLightImpl, const bool>;
		using CONDTION_MASK = ManageFuncList::CONDITION_MASK;
	public:
		JMatrix4x4 view;
		JMatrix4x4 proj;
		JVector3F direction;
	public:
		JWeakPtr<JSpotLight> thisPointer;
		JUserPtr<JGraphicModuleManagedDataFrame> graphicData;
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
	public:
		void SetShadow(bool value)noexcept
		{
			SetFuncList().InvokePassLocalCondition(MANAGED_SET_SHADOW_MAP, this, std::forward<bool>(value));
		}
		void SetShadowResolution(const J_SHADOW_RESOLUTION newShadowResolution)noexcept
		{
			if (thisPointer->IsActivated() && thisPointer->IsShadowActivated())
			{
				DestroyShadowMapResource();
				CreateShadowMapResource();
			}
			JGMUtil::SetFrameDirty(graphicData.Get());
		}
		void SetAllowDisplayShadowMap(bool value)
		{
			SetFuncList().InvokePassLocalCondition(MANAGED_SET_DISPLAY_SHADOW_MAP, this, std::forward<bool>(value));
		}
		void SetAllowFrustumCulling(bool value)noexcept
		{
			if (allowFrustumCulling == value)
				return;

			allowFrustumCulling = value;
			SetFuncList().InvokePassLocalCondition(MANAGED_SET_FRUSTUM_CULLING, this, std::forward<bool>(value));
		}
		void SetRange(const float newRange)noexcept
		{
			range = std::clamp(newRange, Constants::localLightMinDistance, Constants::localLightMaxDistance);
			JGMUtil::SetFrameDirty(graphicData.Get());

			thisPointer->UpdateLightShape();
		}
		void SetInnerConeAngle(const float newAngle)noexcept
		{
			innerConeAngle = std::clamp(newAngle, Private::minConeAngle, outerConeAngle);
			JGMUtil::SetFrameDirty(graphicData.Get());
		}
		void SetInnerConeDegAngle(const float newAngle)noexcept
		{
			SetInnerConeAngle(newAngle * JMathHelper::DegToRad);
		}
		void SetOuterConeAngle(const float newAngle)noexcept
		{
			outerConeAngle = std::clamp(newAngle, innerConeAngle + 0.1f, Private::maxConeAngle);
			JGMUtil::SetFrameDirty(graphicData.Get());

			thisPointer->UpdateLightShape();
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
		bool IsActivated()const noexcept
		{
			return thisPointer->IsActivated();
		}
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
	private:
		void CreateShadowMapResource()noexcept
		{
			JGMUtil::CreateFrame(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW, thisPointer->GetAreaGuid());

			JGraphicResourceTypeSet gTypeSet(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW);
			GMI()->CreateGraphicResource(graphicData.Get(), gTypeSet);

			JCullingTypeSet cTypeSet(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM);
			GMI()->CreateCullingData(graphicData.Get(), cTypeSet);

			if (thisPointer->AllowDisplayShadowMap())
				CreateShadowMapDebugResource();

			JGraphicRequestCreationDesc requestDesc(J_GRAPHIC_REQUEST_TYPE::DRAW_SHADOW_MAP, J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY::UPDATED);
			GMI()->RequestExecutableGraphicFeature(graphicData.Get(), requestDesc);

			requestDesc.type = J_GRAPHIC_REQUEST_TYPE::FRUSTUM_CULLING;
			GMI()->RequestExecutableGraphicFeature(graphicData.Get(), requestDesc);	  
		}
		void DestroyShadowMapResource()noexcept
		{
			GMI()->CancelExecutableGraphicFeature(graphicData.Get(), J_GRAPHIC_REQUEST_TYPE::FRUSTUM_CULLING);
			GMI()->CancelExecutableGraphicFeature(graphicData.Get(), J_GRAPHIC_REQUEST_TYPE::DRAW_SHADOW_MAP);

			DestroyShadowMapDebugResource();

			JCullingTypeSet cTypeSet(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM);
			GMI()->DestroyCullingData(graphicData.Get(), cTypeSet);

			JGraphicResourceTypeSet gTypeSet(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW);
			GMI()->DestroyGraphicResource(graphicData.Get(), gTypeSet);

			GMI()->DestroyFrameUploadData(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW);
		};
		void CreateShadowMapDebugResource()
		{
			JGraphicResourceTypeSet gTypeSet(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW);
			GMI()->CreateGraphicResource(graphicData.Get(), gTypeSet);
		}
		void DestroyShadowMapDebugResource()
		{
			JGraphicResourceTypeSet gTypeSet(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW);
			GMI()->DestroyGraphicResource(graphicData.Get(), gTypeSet);
		}
	public:
		void Activate()noexcept
		{
			IMPL_REGISTER_FRAME_UPDATE_ACTION();
			JGMUtil::CreateFrame(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SPOT_LIGHT, thisPointer->GetAreaGuid());
			SetFuncList().InvokeAll(this, CONDTION_MASK::PASS_NONE, true);
			JGMUtil::SetFrameDirty(graphicData.Get());		    
		}
		void DeActivate()noexcept
		{ 
			SetFuncList().InvokeAll(this, CONDTION_MASK::PASS_NONE, false);
			GMI()->DestroyAllGraphicsResources(graphicData.Get());
			GMI()->DestroyAllCullingData(graphicData.Get());
			GMI()->DestroyFrameUploadData(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SPOT_LIGHT); 
			IMPL_DEREGISTER_FRAME_UPDATE_ACTION();
		}
	private:
		void Update()
		{
			UpdateLightTransform();
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

			JGMUtil::SetFrameDirty(to->impl->graphicData.Get());
			return true;
		}
	public:
		void NotifyReAlloc()
		{
		}
	public:
		void RegisterThisPointer(JSpotLight* lit)
		{
			thisPointer = Core::GetWeakPtr(lit);
		}
		void RegisterPostCreation()
		{ 
		}
		void DeRegisterPreDestruction()
		{ 
		} 
		static void RegisterTypeData()
		{
			Core::JIdentifier::RegisterPrivateInterface(JSpotLight::StaticTypeInfo(), Private::instance);
			IMPL_REALLOC_BIND()
			SET_GUI_FLAG(Core::J_GUI_OPTION_FLAG::J_GUI_OPTION_DISPLAY_PARENT);

			auto setShadowMapLam = [](JSpotLightImpl* impl, const bool value)
			{
				if (value)
					impl->CreateShadowMapResource();
				else
					impl->DestroyShadowMapResource();
				JGMUtil::SetFrameDirty(impl->graphicData.Get());
			};
			auto setDisplayShadowMapLam = [](JSpotLightImpl* impl, const bool value)
			{
				if (value)
					impl->CreateShadowMapDebugResource();
				else
					impl->DestroyShadowMapDebugResource();
				JGMUtil::SetFrameDirty(impl->graphicData.Get());
			};
			auto setFrustumCullingLam = [](JSpotLightImpl* impl, const bool value)
			{
				const JCullingTypeSet typeSet(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM);
				if (value)
				{
					JGraphicRequestCreationDesc desc(J_GRAPHIC_REQUEST_TYPE::FRUSTUM_CULLING, J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY::UPDATED);

					GMI()->CreateCullingData(impl->graphicData.Get(), typeSet);
					GMI()->RequestExecutableGraphicFeature(impl->graphicData.Get(), desc);
				}
				else
				{
					GMI()->CancelExecutableGraphicFeature(impl->graphicData.Get(), J_GRAPHIC_REQUEST_TYPE::FRUSTUM_CULLING);
					GMI()->DestroyAllCullingDataOfType(impl->graphicData.Get(), typeSet.type);
				}
				JGMUtil::SetFrameDirty(impl->graphicData.Get());
			};

			using SetCallable = Core::JStaticCallable<void, JSpotLightImpl*, const bool>;
			using CondCallable = Core::JMemberCNCallable<JSpotLightImpl, bool>;

			SetFuncList().Register(std::make_unique<SetCallable>(setShadowMapLam), std::make_unique<CondCallable>(&JSpotLightImpl::IsShadowActivated), MANAGED_SET_SHADOW_MAP);
			SetFuncList().Register(std::make_unique<SetCallable>(setDisplayShadowMapLam), std::make_unique<CondCallable>(&JSpotLightImpl::AllowDisplayShadowMap), MANAGED_SET_DISPLAY_SHADOW_MAP);
			SetFuncList().Register(std::make_unique<SetCallable>(setFrustumCullingLam), std::make_unique<CondCallable>(&JSpotLightImpl::AllowFrustumCulling), MANAGED_SET_FRUSTUM_CULLING);		 
			SetFuncList().RegisterGlobalCond(std::make_unique<CondCallable>(&JSpotLightImpl::IsActivated));
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
		return Private::instance;
	}
	JGraphicModuleManagedDataFrame* JSpotLight::ModuleManagedData()const noexcept
	{
		return impl->graphicData.Get();
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
	JVector3F JSpotLight::GetWorldPosition()const noexcept
	{
		return Private::CalLightWorldPos(impl->GetTransform());
	}
	JVector3F JSpotLight::GetWorldDirection()const noexcept
	{
		return Private::CalLightWorldDir(impl->GetTransform());
	}
	JVector3F JSpotLight::GetCachedWorldDirection()const noexcept
	{
		return impl->direction;
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
	JMatrix4x4 JSpotLight::GetView()const noexcept
	{
		return impl->proj;
	}
	JMatrix4x4 JSpotLight::GetProj()const noexcept
	{
		return impl->view;
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
		impl->graphicData = GraphicModuleInterface()->Allocate(impl->thisPointer);
		JLight::DoActivate();
		impl->Activate(); 
		RegisterComponent(impl->thisPointer, GetLitTypeComparePtr());
	}
	void JSpotLight::DoDeActivate()noexcept
	{
		DeRegisterComponent(impl->thisPointer);
		impl->DeActivate(); 
		JLight::DoDeActivate();
		GraphicModuleInterface()->DeAllocate(impl->graphicData);
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

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JSpotLight>(*static_cast<JSpotLight::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JLightPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JSpotLight* lit = static_cast<JSpotLight*>(createdPtr);
		lit->impl->RegisterThisPointer(lit); 
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
		auto idenUser = Private::instance.GetCreateInstanceInterface().BeginCreate(std::make_unique<JSpotLight::InitData>(guid, flag, owner), &Private::instance);
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
}