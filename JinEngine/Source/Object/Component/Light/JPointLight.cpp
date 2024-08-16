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


#include"JPointLight.h"   
#include"JPointLightPrivate.h"  
#include"JLightConstants.h"
#include"../Transform/JTransform.h" 
#include"../JComponentHint.h"
#include"../../JObjectFileIOHelper.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h"  
#include"../../Resource/JResourceManager.h"
#include"../../GraphicRule/JGraphicModuleInterfaceHolder.h"
#include"../../GraphicRule/JGraphicModuleUtility.h"
#include"../../../Core/Guid/JGuidCreator.h" 
#include"../../../Core/File/JFileConstant.h" 
#include"../../../Core/Func/JFuncList.h" 
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Math/JMathHelper.h" 

#include<fstream> 
using namespace DirectX;
namespace JinEngine
{
	namespace
	{ 
		enum class POINT_LIT_DIR
		{
			RIGHT,
			LEFT,
			UP,
			DOWN,
			FORWARD,
			BACK,
			COUNT
		};
	}
	namespace Private
	{
		static JPointLightPrivate instance;

		static constexpr float minPower = 0.1f;
		static constexpr float maxPower = 4.0f;
		static constexpr float frustumNear = 1.0f;	//Constants::lightNear
 
		static constexpr float InitPower()noexcept
		{
			return 1.0f;
		}
		static void CalView(const JUserPtr<JTransform>& transform, const POINT_LIT_DIR type, const float fNear, _Out_ JMatrix4x4& m) noexcept
		{
			/**
			*  front축에 dir방향이 위치하게 회전한다고 생각하면 계산이 편하다.
			*/
			float zOffset = fNear;
			switch (type)
			{
			case JinEngine::POINT_LIT_DIR::UP:
			{
				JTransform::CalTransformMatrix(m,
					transform,
					transform->GetPosition(),
					//transform->GetPosition() + (transform->GetUp().Normalize() * -zOffset),
					transform->GetRight(),
					transform->GetFront() * JVector3F::NegativeOne(),
					transform->GetUp());
				break;
			}
			case JinEngine::POINT_LIT_DIR::DOWN:
			{
				JTransform::CalTransformMatrix(m,
					transform,
					transform->GetPosition(),
					//transform->GetPosition() + (transform->GetUp().Normalize() * zOffset),
					transform->GetRight(),
					transform->GetFront(),
					transform->GetUp() * JVector3F::NegativeOne());
				break;
			}
			case JinEngine::POINT_LIT_DIR::FORWARD:
			{
				JTransform::CalTransformMatrix(m,
					transform,
					transform->GetPosition(),
					//transform->GetPosition() + (transform->GetFront().Normalize() * -zOffset),
					transform->GetRight(),
					transform->GetUp(),
					transform->GetFront());
				break;
			}
			case JinEngine::POINT_LIT_DIR::BACK:
			{
				JTransform::CalTransformMatrix(m,
					transform,
					transform->GetPosition(),
					//transform->GetPosition() + (transform->GetFront().Normalize() * zOffset),
					transform->GetRight() * JVector3F::NegativeOne(),
					transform->GetUp(),
					transform->GetFront() * JVector3F::NegativeOne());
				break;
			}
			case JinEngine::POINT_LIT_DIR::RIGHT:
			{
				JTransform::CalTransformMatrix(m,
					transform,
					transform->GetPosition(),
					//transform->GetPosition() + (transform->GetRight().Normalize() * -zOffset),
					transform->GetFront() * JVector3F::NegativeOne(),
					transform->GetUp(),
					transform->GetRight());
				break;
			}
			case JinEngine::POINT_LIT_DIR::LEFT:
			{
				JTransform::CalTransformMatrix(m,
					transform,
					transform->GetPosition(),
					//transform->GetPosition() + (transform->GetRight().Normalize() * zOffset),
					transform->GetFront(),
					transform->GetUp(),
					transform->GetRight() * JVector3F::NegativeOne());
				break;
			}
			default:
				break;
			}
		}
		static XMMATRIX CalProj(const float fNear, const float fFar)noexcept
		{
			return XMMatrixPerspectiveFovLH(90.0f * JMathHelper::DegToRad, 1.0f, fNear, fFar);
			//return XMMatrixOrthographicOffCenterLH(-radius, radius, -radius, radius, frustumNear, frustumFar);
			//return XMMatrixOrthographicLH(radius * 2, radius * 2, frustumNear, frustumFar);
		}
	}

	class JPointLight::JPointLightImpl : public Core::JTypeImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JPointLightImpl) 
	private:
		enum MANAGED_SET
		{
			MANAGED_SET_SHADOW_MAP = 0,
			MANAGED_SET_DISPLAY_SHADOW_MAP,		//for debugging
			MANAGED_SET_FRUSTUM_CULLING,
			MANAGED_SET_COUNT
		};
		using ManageFuncList = Core::JFuncList<MANAGED_SET_COUNT, JPointLight::JPointLightImpl, const bool>;
		using CONDTION_MASK = ManageFuncList::CONDITION_MASK;
	public:
		JWeakPtr<JPointLight> thisPointer;
		JUserPtr<JGraphicModuleManagedDataFrame> graphicData;
	public:
		REGISTER_PROPERTY_EX(range, GetRange, SetRange, GUI_SLIDER(Constants::localLightMinDistance, Constants::localLightMaxDistance, true, false))
		float range = 32.0f;
		REGISTER_PROPERTY_EX(radius, GetRadius, SetRadius, GUI_SLIDER(0, Constants::localLightMaxDistance, true, false, 3))
		float radius = 0.0f;
	public:
		//managed by light type  
		bool allowFrustumCulling = false;
	public:
		//J_SIMPLE_GET_SET_EX(bool, allowVSM, AllowVSM, JGMUtil::SetFrameDirty(graphicData.Get()); )
		//REGISTER_PROPERTY_EX(allowVSM, GetAllowVSM, SetAllowVSM, GUI_CHECKBOX())
		//bool allowVSM = false;
	public:
		JMatrix4x4 view[(uint)POINT_LIT_DIR::COUNT];
		JMatrix4x4 proj;
		JVector3F sidePosition[2];
	public:
		JPointLightImpl(const InitData& initData, JPointLight* thisLitRaw) {}
		~JPointLightImpl() {}
	public:
		J_LIGHT_TYPE GetLightType()const noexcept
		{
			return J_LIGHT_TYPE::POINT;
		}
		J_SHADOW_MAP_TYPE GetShadowMapType()const noexcept
		{
			if (!thisPointer->IsShadowActivated())
				return J_SHADOW_MAP_TYPE::NONE;

			return J_SHADOW_MAP_TYPE::CUBE;
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
		float GetRadius()const noexcept
		{
			return radius;
		}
		DirectX::BoundingBox GetBBox()const noexcept
		{
			DirectX::BoundingBox bbox;
			bbox.Center = GetTransform()->GetWorldPosition().ToSimilar<XMFLOAT3>();
			bbox.Extents = XMFLOAT3(range, range, range);
			return bbox;
		}
		JUserPtr<JMeshGeometry> GetMesh()const noexcept
		{
			return _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::LOW_SPHERE);
		}
		DirectX::XMMATRIX GetMeshWorldM(const bool restrictScaledZ)const noexcept
		{
			return DirectX::XMMatrixAffineTransformation(XMVectorSet(range, range, restrictScaledZ ? 1 : range, 0.0f), JVector4F::Zero().ToXmV(), JVector4F::Zero().ToXmV(), GetTransform()->GetWorldPosition().ToXmV());
		}
		JUserPtr<JTransform> GetTransform()const noexcept
		{
			return thisPointer->GetOwner()->GetTransform();
		} 
	public:
		//value가 bool type일경우에만 justCallFunc을 사용할수있다
		//justCallFunc는 값을 변경하지않고 함수내에서 value per 기능을 수행한다
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
			JGMUtil::SetFrameDirty(graphicData.Get()); ;

			thisPointer->UpdateLightShape(); 
		}
		void SetRadius(const float newRadius)noexcept
		{
			radius = std::clamp(newRadius, 0.0f, Constants::localLightMaxDistance);
			JGMUtil::SetFrameDirty(graphicData.Get()); 

			thisPointer->UpdateLightShape();
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
			JGMUtil::CreateFrame(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_CUBE_DRAW, thisPointer->GetAreaGuid());
			
			JGraphicResourceTypeSet gTypeSet(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE, J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW);
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

			JGraphicResourceTypeSet gTypeSet(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE, J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW);
			GMI()->DestroyGraphicResource(graphicData.Get(), gTypeSet);
			
			GMI()->DestroyFrameUploadData(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_CUBE_DRAW);
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
			JGMUtil::CreateFrame(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::POINT_LIGHT, thisPointer->GetAreaGuid());
			SetFuncList().InvokeAll(this, CONDTION_MASK::PASS_NONE, true);
			JGMUtil::SetFrameDirty(graphicData.Get());
		}
		void DeActivate()noexcept
		{
			//has order dependency
			SetFuncList().InvokeAll(this, CONDTION_MASK::PASS_NONE, false);
			GMI()->DestroyAllGraphicsResources(graphicData.Get());
			GMI()->DestroyAllCullingData(graphicData.Get());
			GMI()->DestroyFrameUploadData(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::POINT_LIGHT);
			IMPL_DEREGISTER_FRAME_UPDATE_ACTION();
		}
	private:
		void Update()
		{
			UpdateLightTransform();
		}
		void UpdateLightTransform()
		{
			for (uint i = 0; i < (uint)POINT_LIT_DIR::COUNT; ++i)
				Private::CalView(GetTransform(), (POINT_LIT_DIR)i, GetFrustumNear(), view[i]);
			proj.StoreXM(Private::CalProj(GetFrustumNear(), GetFrustumFar()));

			auto t = GetTransform();
			const JVector3F pos = t->GetPosition();
			const JVector3F right = t->GetRight();
			const XMVECTOR worldQ = t->GetWorldQuaternion().ToXmV();

			sidePosition[0] = XMVector3Rotate((pos + right * radius).ToXmV(), worldQ);
			sidePosition[1] = XMVector3Rotate((pos - right * radius).ToXmV(), worldQ);
		}
	public:
		static bool DoCopy(JPointLight* from, JPointLight* to)
		{
			from->impl->SetAllowFrustumCulling(to->impl->AllowFrustumCulling());
			from->impl->SetRange(to->impl->GetRange());
			from->impl->SetRadius(to->impl->GetRadius());

			JGMUtil::SetFrameDirty(to->impl->graphicData.Get());
			return true;
		}
	public:
		void NotifyReAlloc()
		{ 
		}
	public:
		void RegisterThisPointer(JPointLight* lit)
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
			Core::JIdentifier::RegisterPrivateInterface(JPointLight::StaticTypeInfo(), Private::instance);
			IMPL_REALLOC_BIND()
			SET_GUI_FLAG(Core::J_GUI_OPTION_FLAG::J_GUI_OPTION_DISPLAY_PARENT);

			auto setShadowMapLam = [](JPointLightImpl* impl, const bool value)
			{
				if (value)
					impl->CreateShadowMapResource();
				else
					impl->DestroyShadowMapResource();
				JGMUtil::SetFrameDirty(impl->graphicData.Get());
			};
			auto setDisplayShadowMapLam = [](JPointLightImpl* impl, const bool value)
			{
				if (value)
					impl->CreateShadowMapDebugResource();
				else
					impl->DestroyShadowMapDebugResource();
				JGMUtil::SetFrameDirty(impl->graphicData.Get());
			};
			auto setFrustumCullingLam = [](JPointLightImpl* impl, const bool value)
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

			using SetCallable = Core::JStaticCallable<void, JPointLightImpl*, const bool>;
			using CondCallable = Core::JMemberCNCallable<JPointLightImpl, bool>;

			SetFuncList().Register(std::make_unique<SetCallable>(setShadowMapLam), std::make_unique<CondCallable>(&JPointLightImpl::IsShadowActivated), MANAGED_SET_SHADOW_MAP);
			SetFuncList().Register(std::make_unique<SetCallable>(setDisplayShadowMapLam), std::make_unique<CondCallable>(&JPointLightImpl::AllowDisplayShadowMap), MANAGED_SET_DISPLAY_SHADOW_MAP);
			SetFuncList().Register(std::make_unique<SetCallable>(setFrustumCullingLam), std::make_unique<CondCallable>(&JPointLightImpl::AllowFrustumCulling), MANAGED_SET_FRUSTUM_CULLING);;
			SetFuncList().RegisterGlobalCond(std::make_unique<CondCallable>(&JPointLightImpl::IsActivated));
		}
	};

	Core::JIdentifierPrivate& JPointLight::PrivateInterface()const noexcept
	{
		return Private::instance;
	}
	JGraphicModuleManagedDataFrame* JPointLight::ModuleManagedData()const noexcept
	{
		return impl->graphicData.Get();
	}
	J_LIGHT_TYPE JPointLight::GetLightType()const noexcept
	{
		return impl->GetLightType();
	}
	J_SHADOW_MAP_TYPE JPointLight::GetShadowMapType()const noexcept
	{
		return impl->GetShadowMapType();
	}
	float JPointLight::GetMinPower()const noexcept
	{
		return Private::minPower;
	}
	float JPointLight::GetMaxPower()const noexcept
	{
		return Private::maxPower;
	}
	float JPointLight::GetFrustumNear()const noexcept
	{
		return impl->GetFrustumNear();
	}
	float JPointLight::GetFrustumFar()const noexcept
	{
		return impl->GetFrustumFar();
	}
	float JPointLight::GetRange()const noexcept
	{
		return impl->GetRange();
	}
	float JPointLight::GetRadius()const noexcept
	{
		return impl->GetRadius();
	}
	void JPointLight::GetSidePosition(JVector3F& left, JVector3F& right)const noexcept
	{
		left = impl->sidePosition[0];
		right = impl->sidePosition[1];
	}
	DirectX::BoundingBox JPointLight::GetBBox()const noexcept
	{
		return impl->GetBBox();
	}
	JUserPtr<JMeshGeometry> JPointLight::GetMesh()const noexcept
	{
		return impl->GetMesh();
	}
	DirectX::XMMATRIX JPointLight::GetMeshWorldM(const bool restrictScaledZ)const noexcept
	{
		return impl->GetMeshWorldM(restrictScaledZ);
	} 
	JMatrix4x4 JPointLight::GetView(const uint index)const noexcept
	{
		return impl->view[index];
	}
	JMatrix4x4 JPointLight::GetProj()const noexcept
	{
		return impl->proj;
	}
	void JPointLight::SetShadow(const bool value)noexcept
	{
		if (value == IsShadowActivated())
			return;

		JLight::SetShadow(value);
		impl->SetShadow(value);
	}
	void JPointLight::SetShadowResolution(const J_SHADOW_RESOLUTION sQuality)noexcept
	{
		if (sQuality == GetShadowResolutionType())
			return;

		JLight::SetShadowResolution(sQuality);
		impl->SetShadowResolution(sQuality);
	}
	void JPointLight::SetAllowDisplayShadowMap(const bool value)noexcept
	{
		if (value == AllowDisplayShadowMap())
			return;

		JLight::SetAllowDisplayShadowMap(value);
		impl->SetAllowDisplayShadowMap(value);
	}
	void JPointLight::SetRange(const float range)noexcept
	{
		impl->SetRange(range);
	}
	void JPointLight::SetRadius(const float radius)noexcept
	{
		impl->SetRadius(radius);
	}
	bool JPointLight::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection())
			return true;
		else
			return false;
	}
	bool JPointLight::AllowFrustumCulling()const noexcept
	{
		return impl->AllowFrustumCulling();
	}
	bool JPointLight::AllowHzbOcclusionCulling()const noexcept
	{
		return impl->AllowHzbOcclusionCulling();
	}
	bool JPointLight::AllowHdOcclusionCulling()const noexcept
	{
		return impl->AllowHdOcclusionCulling();
	}
	bool JPointLight::AllowDisplayOccCullingDepthMap()const noexcept
	{
		return impl->AllowDisplayOccCullingDepthMap();
	}
	void JPointLight::DoActivate()noexcept
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
	void JPointLight::DoDeActivate()noexcept
	{
		DeRegisterComponent(impl->thisPointer);
		impl->DeActivate(); 
		JLight::DoDeActivate();
		GraphicModuleInterface()->DeAllocate(impl->graphicData);
	}
	JPointLight::JPointLight(const InitData& initData)
		:JLight(initData), impl(std::make_unique<JPointLightImpl>(initData, this))
	{ }
	JPointLight::~JPointLight()
	{
		impl.reset();
	}

	JPointLight::InitData::InitData(const JUserPtr<JGameObject>& owner)
		:JLight::InitData(JPointLight::StaticTypeInfo(), owner)
	{}
	JPointLight::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner)
		: JLight::InitData(JPointLight::StaticTypeInfo(), guid, flag, owner)
	{}

	using CreateInstanceInterface = JPointLightPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JPointLightPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JPointLightPrivate::AssetDataIOInterface; 

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JPointLight>(*static_cast<JPointLight::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JLightPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JPointLight* lit = static_cast<JPointLight*>(createdPtr);
		lit->impl->RegisterThisPointer(lit); 
		lit->impl->RegisterPostCreation();
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JPointLight::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JPointLight::StaticTypeInfo());
		if (!canCopy)
			return false;

		if (!JLightPrivate::CreateInstanceInterface::Copy(from, to))
			return false;

		return JPointLight::JPointLightImpl::DoCopy(static_cast<JPointLight*>(from.Get()), static_cast<JPointLight*>(to.Get()));
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		static_cast<JPointLight*>(ptr)->impl->DeRegisterPreDestruction();
		JLightPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JPointLight::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;
		bool isActivated;
		float sRange;
		float sRadius;

		auto loadData = static_cast<JPointLight::LoadData*>(data);
		JFileIOTool& tool = loadData->tool;
		JUserPtr<JGameObject> owner = loadData->owner;

		JObjectFileIOHelper::LoadComponentIden(tool, guid, flag, isActivated);
		auto idenUser = Private::instance.GetCreateInstanceInterface().BeginCreate(std::make_unique<JPointLight::InitData>(guid, flag, owner), &Private::instance);
		JUserPtr<JPointLight> litUser;
		litUser.ConnnectChild(idenUser);

		JLightPrivate::AssetDataIOInterface::LoadLightData(tool, litUser);
		JObjectFileIOHelper::LoadAtomicData(tool, sRange, "Range:");
		JObjectFileIOHelper::LoadAtomicData(tool, sRadius, "Radius:");
		litUser->SetRange(sRange);
		litUser->SetRadius(sRadius);
		if (!isActivated)
			litUser->DeActivate();
		return litUser;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JPointLight::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JPointLight::StoreData*>(data);
		if (!storeData->HasCorrectType(JPointLight::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JUserPtr<JPointLight> lit;
		lit.ConnnectChild(storeData->obj);

		JPointLight::JPointLightImpl* impl = lit->impl.get();
		JFileIOTool& tool = storeData->tool;

		JObjectFileIOHelper::StoreComponentIden(tool, lit.Get());
		JLightPrivate::AssetDataIOInterface::StoreLightData(tool, lit);
		JObjectFileIOHelper::StoreAtomicData(tool, impl->range, "Range:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->radius, "Radius:");

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
 
	Core::JIdentifierPrivate::CreateInstanceInterface& JPointLightPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JPointLightPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JPointLightPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	} 
}