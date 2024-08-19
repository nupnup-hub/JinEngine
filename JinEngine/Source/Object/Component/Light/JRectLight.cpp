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


#include"JRectLight.h"
#include"JRectLightPrivate.h"  
#include"JLightConstants.h"
#include"../Transform/JTransform.h" 
#include"../JComponentHint.h"
#include"../../JObjectFileIOHelper.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h" 
#include"../../Resource/Scene/JScenePrivate.h"
#include"../../Resource/Texture/JTexture.h" 
#include"../../Resource/JResourceObjectUserInterface.h"
#include"../../Resource/JResourceManager.h"
#include"../../../Core/Guid/JGuidCreator.h" 
#include"../../../Core/File/JFileConstant.h" 
#include"../../../Core/Func/JFuncList.h" 
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Math/JMathHelper.h" 
#include"../../GraphicRule/JGraphicModuleInterfaceHolder.h"
#include"../../GraphicRule/JGraphicModuleUtility.h"

using namespace DirectX;
namespace JinEngine
{ 
	namespace Private
	{
		static JRectLightPrivate instance;

		static constexpr float minPower = 0.1f;
		static constexpr float maxPower = 8.0f;
		static constexpr float frustumNear = 1.0f;	 
		static constexpr float maxBarndoorLength = 64;
		 
		static constexpr float InitPower()noexcept
		{
			return 1.0f;
		}
		static JVector2F InitSize()noexcept
		{
			return JVector2F(4, 4);
		}
		static XMVECTOR GetInitDir()
		{
			return XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		}
		static XMVECTOR CalLightWorldDir(const JUserPtr<JTransform>& transform) noexcept
		{
			return XMVector3Normalize(XMVector3Rotate(GetInitDir(), transform->GetWorldQuaternion().ToXmV()));
		}
		static XMMATRIX CalView(const JUserPtr<JTransform>& transform) noexcept
		{
			JMatrix4x4 m;
			transform->CalTransformMatrix(m);
			return m.LoadXM();
		}
		static XMMATRIX CalProj(const float fNear, const float fFar)noexcept
		{
			return XMMatrixPerspectiveFovLH(90.0f * JMathHelper::DegToRad, 1.0f, fNear, fFar); 
		}
		static JGraphicResourceTypeSet ShadowMapTypeSet() noexcept
		{
			return JGraphicResourceTypeSet(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW);
		}
		static JGraphicResourceTypeSet DebugTypeSet() noexcept
		{
			return JGraphicResourceTypeSet(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE);
		}
		static JCullingTypeSet FrustumSet()noexcept
		{
			return JCullingTypeSet(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM);
		}
	}

	class JRectLight::JRectLightImpl : public Core::JTypeImplBase, public JResourceObjectUserInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JRectLightImpl)
	private:
		enum MANAGED_SET
		{
			MANAGED_SET_SHADOW_MAP = 0,
			MANAGED_SET_DISPLAY_SHADOW_MAP,		//for debugging
			MANAGED_SET_FRUSTUM_CULLING,
			MANAGED_SET_COUNT
		};
		using ManageFuncList = Core::JFuncList<MANAGED_SET_COUNT, JRectLight::JRectLightImpl, const bool>;
		using CONDTION_MASK = ManageFuncList::CONDITION_MASK;
	public:
		JWeakPtr<JRectLight> thisPointer; 
		JUserPtr<JGraphicModuleManagedDataFrame> graphicData;
	public:
		REGISTER_PROPERTY_EX(areaSize, GetAreaSize, SetAreaSize, GUI_INPUT(false))
		JVector2F areaSize = Private::InitSize();
	public: 
		REGISTER_PROPERTY_EX(range, GetRange, SetRange, GUI_SLIDER(Constants::localLightMinDistance, Constants::localLightMaxDistance, true, false))
		float range = 32.0f;
		REGISTER_PROPERTY_EX(barndoorLength, GetBarndoorLength, SetBarndoorLength, GUI_SLIDER(0, Private::maxBarndoorLength, true, false))
		float barndoorLength = 0;
		REGISTER_PROPERTY_EX(barndoorAngle, GetBarndoorAngle, SetBarndoorAngle, GUI_SLIDER(0, 90, true, false))
		float barndoorAngle = 90;
	public:
		//REGISTER_PROPERTY_EX(isTwoSide, IsTwoSide, SetTwoSide, GUI_CHECKBOX())
		//bool isTwoSide = false;
		bool allowFrustumCulling = false;
	public:
		REGISTER_PROPERTY_EX(sourceTexture, GetSourceTexture, SetSourceTexture, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, false, false))
		JUserPtr<JTexture> sourceTexture;
		JUserPtr<JTexture> ltcMat;
		JUserPtr<JTexture> ltcAmp;
	public:
		JVector3F worldAxis[3]; 
		JVector3F worldDir;
		JMatrix4x4 view;
		JMatrix4x4 proj;
	public:
		JRectLightImpl(const InitData& initData, JRectLight* thisLitRaw) {}
		~JRectLightImpl() {}
	public:
		J_LIGHT_TYPE GetLightType()const noexcept
		{   
			return J_LIGHT_TYPE::RECT;
		}
		J_SHADOW_MAP_TYPE GetShadowMapType()const noexcept
		{
			return J_SHADOW_MAP_TYPE::NORMAL;
		}
		JVector2F GetAreaSize()const noexcept
		{
			return areaSize;
		}
		float GetFrustumNear()const noexcept
		{
			return Private::frustumNear;
		}
		float GetFrustumFar()const noexcept
		{
			return range;
		} 
		float GetRange()const noexcept
		{
			return range;
		}
		float GetBarndoorLength()const noexcept
		{
			return barndoorLength;
		}
		float GetBarndoorAngle()const noexcept
		{
			return barndoorAngle;
		}
		DirectX::BoundingBox GetBBox()const noexcept
		{
			DirectX::BoundingBox bbox;
			bbox.Center = GetTransform()->GetWorldPosition().ToSimilar<XMFLOAT3>();
			bbox.Extents = XMFLOAT3(areaSize.x, areaSize.y, range);
			return bbox;
		}
		JUserPtr<JMeshGeometry> GetMesh()const noexcept
		{
			return _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::LOW_HEMI_SPHERE);
		}
		DirectX::XMMATRIX GetMeshWorldM(const bool restrictScaledZ)const noexcept
		{
			JUserPtr<JTransform> t = GetTransform();
			JVector3F p;
			JVector4F q;
			JVector3F s;
			t->GetWorldPQS(p, q, s);

			XMVECTOR qv = DirectX::XMQuaternionRotationAxis(JVector3F::Right().ToXmV(), JMathHelper::DegToRad * 90.0f);
			qv = XMQuaternionMultiply(qv, q.ToXmV());
			 
			//barndoorAngle에 대한 적용필요.
			s = JVector3F(areaSize.x * 1.025f, areaSize.y * 1.025f, restrictScaledZ ? 1 : range);
			return DirectX::XMMatrixAffineTransformation(s.ToXmV(), JVector4F::Zero().ToXmV(), qv, p.ToXmV());
		}
		JUserPtr<JTransform> GetTransform()const noexcept
		{
			return thisPointer->GetOwner()->GetTransform();
		}
		JUserPtr<JTexture> GetSourceTexture()const noexcept
		{
			return sourceTexture;
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
		void SetAreaSize(const JVector2F newAreaSize) noexcept
		{
			if (areaSize == newAreaSize)
				return;

			areaSize = JVector2F::Clamp(newAreaSize, 0, Constants::localLightMaxDistance);
			JGMUtil::SetFrameDirty(graphicData.Get());

			thisPointer->UpdateLightShape(); 
		}
		/*
		void SetTwoSide(const bool value)noexcept
		{
			if (value == isTwoSide)
				return;

			isTwoSide = value;
			JGMUtil::SetFrameDirty(graphicData.Get());
		}*/ 
		void SetRange(const float newRange)noexcept
		{
			range = std::clamp(newRange, Constants::localLightMinDistance, Constants::localLightMaxDistance);
			JGMUtil::SetFrameDirty(graphicData.Get());

			thisPointer->UpdateLightShape();
		} 
		void SetBarndoorLength(const float newLength)noexcept
		{ 
			if (barndoorLength == newLength)
				return;

			barndoorLength = std::clamp(newLength, 0.0f, Private::maxBarndoorLength);
			JGMUtil::SetFrameDirty(graphicData.Get());

			thisPointer->UpdateLightShape();
		}
		void SetBarndoorAngle(const float newAngle)noexcept
		{
			if (barndoorAngle == newAngle)
				return;

			barndoorAngle = std::clamp(newAngle, 0.0f, 90.0f);
			JGMUtil::SetFrameDirty(graphicData.Get());

			thisPointer->UpdateLightShape();
		}
		void SetSourceTexture(const JUserPtr<JTexture>& newSourceTexture)noexcept
		{
			CallOffResourceReference(sourceTexture.Get());
			sourceTexture = newSourceTexture;
			CallOnResourceReference(sourceTexture.Get());
			JGMUtil::SetFrameDirty(graphicData.Get());
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
			 
			JGraphicResourceCreationDesc desc(Private::ShadowMapTypeSet(), JVector2F(thisPointer->GetShadowMapSize()));
			GMI()->CreateGraphicResource(graphicData.Get(), desc);
			 
			GMI()->CreateCullingData(graphicData.Get(), Private::FrustumSet());

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
			 
			GMI()->DestroyCullingData(graphicData.Get(), Private::FrustumSet());		 
			GMI()->DestroyGraphicResource(graphicData.Get(), Private::ShadowMapTypeSet());
			GMI()->DestroyFrameUploadData(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW);
		};
		void CreateShadowMapDebugResource()
		{ 
			JGraphicResourceCreationDesc desc(Private::DebugTypeSet(), JVector2F(thisPointer->GetShadowMapSize()));
			GMI()->CreateGraphicResource(graphicData.Get(), desc);
		}
		void DestroyShadowMapDebugResource()
		{ 
			GMI()->DestroyGraphicResource(graphicData.Get(), Private::DebugTypeSet());
		}
	public:
		void Activate()noexcept
		{
			IMPL_REGISTER_FRAME_UPDATE_ACTION_HOT();
			ltcMat = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::LTC_MAT);
			ltcAmp = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::LTC_AMP);

			JGMUtil::CreateFrame(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::RECT_LIGHT, thisPointer->GetAreaGuid());
			SetFuncList().InvokeAll(this, CONDTION_MASK::PASS_NONE, true);
			
			JGMUtil::SetFrameDirty(graphicData.Get());
		}
		void DeActivate()noexcept
		{
			SetFuncList().InvokeAll(this, CONDTION_MASK::PASS_NONE, false);
			GMI()->DestroyAllGraphicsResources(graphicData.Get());
			GMI()->DestroyAllCullingData(graphicData.Get());
			GMI()->DestroyFrameUploadData(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SPOT_LIGHT); 
			ltcMat = nullptr;
			ltcAmp = nullptr;
			IMPL_DEREGISTER_FRAME_UPDATE_ACTION();
		}
	private:
		void HotUpdate()
		{
			UpdateLightTransform();
		}
		void UpdateLightTransform()
		{
			auto t = GetTransform();
			view.StoreXM(Private::CalView(t));
			proj.StoreXM(Private::CalProj(GetFrustumNear(), GetFrustumFar())); 
			worldAxis[0] = t->GetWorldRight();
			worldAxis[1] = t->GetWorldUp();
			worldAxis[2] = t->GetWorldFront();
			worldDir = Private::CalLightWorldDir(t);
		}
	public:
		static bool DoCopy(JRectLight* from, JRectLight* to)
		{
			from->impl->SetAllowFrustumCulling(to->impl->AllowFrustumCulling()); 
			from->impl->SetRange(to->impl->GetRange()); 

			JGMUtil::SetFrameDirty(to->impl->graphicData.Get());
			return true;
		}
	private:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj, JResourceEventDesc* desc)
		{
			if (iden == thisPointer->GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (sourceTexture != nullptr && sourceTexture->GetGuid() == jRobj->GetGuid())
					SetSourceTexture(nullptr);
			}
			else if (eventType == J_RESOURCE_EVENT_TYPE::UPDATE_NON_FRAME_RESOURCE)
			{
				if (sourceTexture != nullptr && sourceTexture->GetGuid() == jRobj->GetGuid())
					JGMUtil::SetFrameDirty(graphicData.Get());
			}
		}
	public:
		void NotifyReAlloc()
		{ 
			ResetEventListenerPointer(*JResourceObject::EvInterface(), thisPointer->GetGuid());
		}
	public:
		void RegisterThisPointer(JRectLight* lit)
		{
			thisPointer = Core::GetWeakPtr(lit);
		} 
		void RegisterPostCreation()
		{
			AddEventListener(*JResourceObject::EvInterface(), thisPointer->GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
			AddEventListener(*JResourceObject::EvInterface(), thisPointer->GetGuid(), J_RESOURCE_EVENT_TYPE::UPDATE_NON_FRAME_RESOURCE);
		} 
		void DeRegisterPreDestruction()
		{ 
			RemoveListener(*JResourceObject::EvInterface(), thisPointer->GetGuid()); 
		} 
		static void RegisterTypeData()
		{
			Core::JIdentifier::RegisterPrivateInterface(JRectLight::StaticTypeInfo(), Private::instance);
			IMPL_REALLOC_BIND()
			SET_GUI_FLAG(Core::J_GUI_OPTION_FLAG::J_GUI_OPTION_DISPLAY_PARENT);

			auto setShadowMapLam = [](JRectLightImpl* impl, const bool value)
			{
				if (value)
					impl->CreateShadowMapResource();
				else
					impl->DestroyShadowMapResource();
				JGMUtil::SetFrameDirty(impl->graphicData.Get());
			};
			auto setDisplayShadowMapLam = [](JRectLightImpl* impl, const bool value)
			{
				if (value)
					impl->CreateShadowMapDebugResource();
				else
					impl->DestroyShadowMapDebugResource();
				JGMUtil::SetFrameDirty(impl->graphicData.Get());
			};
			auto setFrustumCullingLam = [](JRectLightImpl* impl, const bool value)
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

			using SetCallable = Core::JStaticCallable<void, JRectLightImpl*, const bool>;
			using CondCallable = Core::JMemberCNCallable<JRectLightImpl, bool>;

			SetFuncList().Register(std::make_unique<SetCallable>(setShadowMapLam), std::make_unique<CondCallable>(&JRectLightImpl::IsShadowActivated), MANAGED_SET_SHADOW_MAP);
			SetFuncList().Register(std::make_unique<SetCallable>(setDisplayShadowMapLam), std::make_unique<CondCallable>(&JRectLightImpl::AllowDisplayShadowMap), MANAGED_SET_DISPLAY_SHADOW_MAP);
			SetFuncList().Register(std::make_unique<SetCallable>(setFrustumCullingLam), std::make_unique<CondCallable>(&JRectLightImpl::AllowFrustumCulling), MANAGED_SET_FRUSTUM_CULLING);
			SetFuncList().RegisterGlobalCond(std::make_unique<CondCallable>(&JRectLightImpl::IsActivated));
		}
	};

	Core::JIdentifierPrivate& JRectLight::PrivateInterface()const noexcept
	{
		return Private::instance;
	}
	JGraphicModuleManagedDataFrame* JRectLight::ModuleManagedData()const noexcept
	{
		return impl->graphicData.Get();
	}
	J_LIGHT_TYPE JRectLight::GetLightType()const noexcept
	{
		return impl->GetLightType();
	}
	J_SHADOW_MAP_TYPE JRectLight::GetShadowMapType()const noexcept
	{
		return impl->GetShadowMapType();
	}
	JVector2F JRectLight::GetAreaSize()const noexcept
	{
		return impl->GetAreaSize();
	}
	float JRectLight::GetMinPower()const noexcept
	{
		return Private::minPower;
	}
	float JRectLight::GetMaxPower()const noexcept
	{
		return Private::maxPower;
	}
	float JRectLight::GetFrustumNear()const noexcept
	{
		return impl->GetFrustumNear();
	}
	float JRectLight::GetFrustumFar()const noexcept
	{
		return impl->GetFrustumFar();
	}
	float JRectLight::GetRange()const noexcept
	{
		return impl->GetRange();
	} 
	float JRectLight::GetBarndoorLength()const noexcept
	{
		return impl->GetBarndoorLength();
	}
	float JRectLight::GetBarndoorAngle()const noexcept
	{
		return impl->GetBarndoorAngle();
	}
	JVector3F JRectLight::GetWorldDirection()const noexcept
	{
		return Private::CalLightWorldDir(impl->GetTransform());
	}
	JVector3F JRectLight::GetCachedWorldDirection()const noexcept
	{
		return impl->worldDir;
	}
	void JRectLight::GetWorldAxis(JVector3F& right, JVector3F& up, JVector3F& front)const noexcept
	{
		right = impl->worldAxis[0];
		up = impl->worldAxis[1];
		front = impl->worldAxis[2];
	}
	DirectX::BoundingBox JRectLight::GetBBox()const noexcept
	{
		return impl->GetBBox();
	}
	JUserPtr<JMeshGeometry> JRectLight::GetMesh()const noexcept
	{
		return impl->GetMesh();
	}
	DirectX::XMMATRIX JRectLight::GetMeshWorldM(const bool restrictScaledZ)const noexcept
	{
		return impl->GetMeshWorldM(restrictScaledZ);
	} 
	JUserPtr<JTexture> JRectLight::GetSourceTexture()const noexcept
	{
		return impl->GetSourceTexture();
	}
	JMatrix4x4 JRectLight::GetView()const noexcept
	{
		return impl->view;
	}
	JMatrix4x4 JRectLight::GetProj()const noexcept
	{
		return impl->proj;
	}
	void JRectLight::SetShadow(const bool value)noexcept
	{
		if (value == IsShadowActivated())
			return;

		//shadow map 미구현 이기때문에 pass
		JLight::SetShadow(false);
		//JLight::SetShadow(value);
	//	impl->SetShadow(value);
	}
	void JRectLight::SetShadowResolution(const J_SHADOW_RESOLUTION sQuality)noexcept
	{
		if (sQuality == GetShadowResolutionType())
			return;

		JLight::SetShadowResolution(sQuality);
		impl->SetShadowResolution(sQuality);
	}
	void JRectLight::SetAllowDisplayShadowMap(const bool value)noexcept
	{
		if (value == AllowDisplayShadowMap())
			return;
		 
		//shadow map 미구현 이기때문에 pass
		JLight::SetAllowDisplayShadowMap(false);
		//JLight::SetAllowDisplayShadowMap(value);
		//impl->SetAllowDisplayShadowMap(value);
	}
	void JRectLight::SetAreaSize(const JVector2F newSize)noexcept
	{
		impl->SetAreaSize(newSize);
	}
	/*
	void JRectLight::SetTwoSide(const bool value)noexcept
	{
		impl->SetTwoSide(value);
	}*/ 
	void JRectLight::SetRange(const float range)noexcept
	{
		impl->SetRange(range);
	} 
	void JRectLight::SetBarndoorLength(const float newLength)noexcept
	{
		impl->SetBarndoorLength(newLength);
	}
	void JRectLight::SetBarndoorAngle(const float newAngle)noexcept
	{
		impl->SetBarndoorAngle(newAngle);
	}
	void JRectLight::SetSourceTexture(const JUserPtr<JTexture>& srcTexture)noexcept
	{
		impl->SetSourceTexture(srcTexture);
	}
	bool JRectLight::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection())
			return true;
		else
			return false;
	}
	bool JRectLight::AllowFrustumCulling()const noexcept
	{
		return impl->AllowFrustumCulling();
	}
	bool JRectLight::AllowHzbOcclusionCulling()const noexcept
	{
		return impl->AllowHzbOcclusionCulling();
	}
	bool JRectLight::AllowHdOcclusionCulling()const noexcept
	{
		return impl->AllowHdOcclusionCulling();
	}
	bool JRectLight::AllowDisplayOccCullingDepthMap()const noexcept
	{
		return impl->AllowDisplayOccCullingDepthMap();
	} 
	void JRectLight::DoActivate()noexcept
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
	void JRectLight::DoDeActivate()noexcept
	{
		DeRegisterComponent(impl->thisPointer);
		impl->DeActivate(); 
		JLight::DoDeActivate();
		GraphicModuleInterface()->DeAllocate(impl->graphicData);
	}
	JRectLight::JRectLight(const InitData& initData)
		:JLight(initData), impl(std::make_unique<JRectLightImpl>(initData, this))
	{ }
	JRectLight::~JRectLight()
	{
		impl.reset();
	}

	JRectLight::InitData::InitData(const JUserPtr<JGameObject>& owner)
		:JLight::InitData(JRectLight::StaticTypeInfo(), owner)
	{}
	JRectLight::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner)
		: JLight::InitData(JRectLight::StaticTypeInfo(), guid, flag, owner)
	{}

	using CreateInstanceInterface = JRectLightPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JRectLightPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JRectLightPrivate::AssetDataIOInterface; 

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JRectLight>(*static_cast<JRectLight::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JLightPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JRectLight* lit = static_cast<JRectLight*>(createdPtr);
		lit->impl->RegisterThisPointer(lit); 
		lit->impl->RegisterPostCreation();
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JRectLight::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JRectLight::StaticTypeInfo());
		if (!canCopy)
			return false;

		if (!JLightPrivate::CreateInstanceInterface::Copy(from, to))
			return false;

		return JRectLight::JRectLightImpl::DoCopy(static_cast<JRectLight*>(from.Get()), static_cast<JRectLight*>(to.Get()));
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		static_cast<JRectLight*>(ptr)->impl->DeRegisterPreDestruction();
		JLightPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JRectLight::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;
		bool isActivated;
		JVector2F sAreaSize; 
		float sRange; 
		//bool sIsTwoSize;

		auto loadData = static_cast<JRectLight::LoadData*>(data);
		JFileIOTool& tool = loadData->tool;
		JUserPtr<JGameObject> owner = loadData->owner;

		JObjectFileIOHelper::LoadComponentIden(tool, guid, flag, isActivated);
		auto idenUser = Private::instance.GetCreateInstanceInterface().BeginCreate(std::make_unique<JRectLight::InitData>(guid, flag, owner), &Private::instance);
		JUserPtr<JRectLight> litUser;
		litUser.ConnnectChild(idenUser);

		JLightPrivate::AssetDataIOInterface::LoadLightData(tool, litUser);
		JObjectFileIOHelper::LoadVector2(tool, sAreaSize, "AreaSize:"); 
		JObjectFileIOHelper::LoadAtomicData(tool, sRange, "Range:"); 
		//JObjectFileIOHelper::LoadAtomicData(tool, sIsTwoSize, "IsTwoSize:");
		JUserPtr<JTexture> sSourceTexture = JObjectFileIOHelper::_LoadHasIden<JTexture>(tool, "SourceTexture:");

		litUser->SetAreaSize(sAreaSize); 
		litUser->SetRange(sRange); 
		//litUser->SetTwoSide(sIsTwoSize);
		litUser->SetSourceTexture(sSourceTexture);
		if (!isActivated)
			litUser->DeActivate();
		return litUser;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JRectLight::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JRectLight::StoreData*>(data);
		if (!storeData->HasCorrectType(JRectLight::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JUserPtr<JRectLight> lit;
		lit.ConnnectChild(storeData->obj);

		JRectLight::JRectLightImpl* impl = lit->impl.get();
		JFileIOTool& tool = storeData->tool;

		JObjectFileIOHelper::StoreComponentIden(tool, lit.Get());
		JLightPrivate::AssetDataIOInterface::StoreLightData(tool, lit);
		JObjectFileIOHelper::StoreVector2(tool, impl->areaSize, "AreaSize:"); 
		JObjectFileIOHelper::StoreAtomicData(tool, impl->range, "Range:"); 
		//JObjectFileIOHelper::StoreAtomicData(tool, impl->isTwoSide, "IsTwoSize:");
		JObjectFileIOHelper::_StoreHasIden(tool, impl->sourceTexture.Get(), "SourceTexture:");

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JRectLightPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JRectLightPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JRectLightPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	} 
}