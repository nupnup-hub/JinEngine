#include"JPointLight.h"   
#include"JPointLightPrivate.h"  
#include"JLightConstants.h"
#include"../Transform/JTransform.h" 
#include"../JComponentHint.h"
#include"../../JObjectFileIOHelper.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h" 
#include"../../Resource/Scene/JScenePrivate.h"
#include"../../../Core/Guid/JGuidCreator.h" 
#include"../../../Core/File/JFileConstant.h" 
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Math/JMathHelper.h"
#include"../../../Graphic/JGraphic.h"  
#include"../../../Graphic/JGraphicPrivate.h"
#include"../../../Graphic/Frameresource/JLightConstants.h"  
#include"../../../Graphic/Frameresource/JShadowMapConstants.h"   
#include"../../../Graphic/Frameresource/JFrameUpdate.h"
#include"../../../Graphic/Culling/JCullingInterface.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../../Graphic/JGraphicDrawListInterface.h"   
#include<Windows.h>
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
		using LitFrameUpdate = Graphic::JFrameUpdate<Graphic::JFrameUpdateInterfaceHolder2<
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT, Graphic::JPointLightConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW, Graphic::JShadowMapCubeDrawConstants&>>,
			Graphic::JFrameDirty>;
	}
	namespace
	{
		static auto isAvailableoverlapLam = []() {return true; };
		static JPointLightPrivate lPrivate;
	}
	namespace Private
	{
		static constexpr float minPower = 0.1f;
		static constexpr float maxPower = 2.0f;
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

	class JPointLight::JPointLightImpl : public Core::JTypeImplBase,
		public LitFrameUpdate,
		public Graphic::JGraphicWideSingleResourceHolder<2>,	//shadowMap, debug
		//public Graphic::JGraphicTypePerSingleResourceHolder,
		public Graphic::JGraphicDrawListCompInterface,
		public Graphic::JCullingInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JPointLightImpl)
	public:
		using PointLitFrame = JFrameInterface1;
		using ShadowMapCubeDrawFrame = JFrameInterface2;
	public:
		JWeakPtr<JPointLight> thisPointer;
	public:
		REGISTER_PROPERTY_EX(power, GetPower, SetPower, GUI_SLIDER(Private::minPower, Private::maxPower, true, false))
		float power = Private::InitPower();
		REGISTER_PROPERTY_EX(range, GetRange, SetRange, GUI_SLIDER(Constants::lightNear, Constants::lightMaxFar, true, false))
		float range = 32.0f;
		REGISTER_PROPERTY_EX(radius, GetRadius, SetRadius, GUI_SLIDER(0, Constants::lightMaxFar, true, false, 3))
		float radius = 0.0f;
	public:
		//managed by light type  
		bool allowFrustumCulling = false;
	public:
		//J_SIMPLE_GET_SET_EX(bool, allowVSM, AllowVSM, SetFrameDirty();)
		//REGISTER_PROPERTY_EX(allowVSM, GetAllowVSM, SetAllowVSM, GUI_CHECKBOX())
		//bool allowVSM = false;
	public:
		JMatrix4x4 view[(uint)POINT_LIT_DIR::COUNT];
		JMatrix4x4 proj;
		JVector3F worldRight;
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
		float GetPower()const noexcept
		{
			return power;
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
		JUserPtr<JTransform> GetTransform()const noexcept
		{
			return thisPointer->GetOwner()->GetTransform();
		}
	public:
		//value가 bool type일경우에만 justCallFunc을 사용할수있다
		//justCallFunc는 값을 변경하지않고 함수내에서 value per 기능을 수행한다
		void SetShadow(const bool value, const bool isManual = false)noexcept
		{
			if (thisPointer->IsActivated() || isManual)
			{
				if (value)
					CreateShadowMapResource();
				else
					DestroyShadowMapResource();
			}
			SetFrameDirty();
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
		void SetAllowDisplayShadowMap(const bool value, const bool isManual = false)
		{
			if (thisPointer->IsActivated() || isManual)
			{
				if (value)
					CreateShadowMapDebugResource();
				else
					DestroyShadowMapDebugResource();
			}
			SetFrameDirty();
		}
		void SetAllowFrustumCulling(const bool value, const bool justCallFunc = false)noexcept
		{
			if (!justCallFunc && allowFrustumCulling == value)
				return;

			if (!justCallFunc)
				allowFrustumCulling = value;
			if (justCallFunc || thisPointer->IsActivated())
			{
				if (value)
				{
					CreateFrustumCullingData();
					AddFrustumCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
				}
				else
				{
					DestroyCullingData(Graphic::J_CULLING_TYPE::FRUSTUM);
					PopFrustumCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
				}
			}
			SetFrameDirty();
		}
		void SetPower(const float newPower)noexcept
		{
			power = std::clamp(newPower, Private::minPower, Private::maxPower);
			SetFrameDirty();
		}
		void SetRange(const float newRange)noexcept
		{
			range = std::clamp(newRange, Constants::lightNear, Constants::lightMaxFar);
			SetFrameDirty();
		}
		void SetRadius(const float newRadius)noexcept
		{
			radius = std::clamp(newRadius, 0.0f, Constants::lightMaxFar);
			SetFrameDirty();
		}
	public:
		bool IsShadowActivated()const noexcept
		{
			return thisPointer->IsShadowActivated();
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
			CreateShadowMapTextureCube(thisPointer->GetShadowMapSize());
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
			CreateLayerDepthDebugResource(JVector2<uint>(thisPointer->GetShadowMapSize(), thisPointer->GetShadowMapSize()));
		}
		void DestroyShadowMapDebugResource()
		{
			DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG);
		}
	public:
		void Activate()noexcept
		{
			RegisterLightFrameData(JLightType::LitToFrameR(GetLightType()));
			if (thisPointer->IsShadowActivated())
				SetShadow(true, true);
			if (allowFrustumCulling)
				SetAllowFrustumCulling(true, true);
		}
		void DeActivate()noexcept
		{
			//has order dependency
			DeRegisterLightFrameData(JLightType::LitToFrameR(GetLightType()));
			if (thisPointer->IsShadowActivated())
				SetShadow(false, true);
			if (allowFrustumCulling)
				SetAllowFrustumCulling(false, true);
			DestroyAllCullingData();
			DestroyAllTexture();
		}
	public:
		void UpdateFrame(Graphic::JPointLightConstants& constant)noexcept final
		{
			const XMMATRIX projM = proj.LoadXM();
			const XMMATRIX ndcM = JMatrix4x4::NdcToTextureSpaceXM();
			for (uint i = 0; i < Graphic::Constants::cubeMapPlaneCount; ++i)		 
				constant.shadowMapTransform[i].StoreXM(XMMatrixTranspose(XMMatrixMultiply(XMMatrixMultiply(view[i].LoadXM(), projM), ndcM)));
 
			auto t = GetTransform();
			const JVector3F pos = t->GetPosition();
			const JVector3F right = t->GetRight();
			const XMVECTOR worldQ = t->GetWorldQuaternion().ToXmV();

			constant.midPosition = t->GetWorldPosition();
			constant.sidePosition[0] = XMVector3Rotate((pos + right * radius).ToXmV(), worldQ);
			constant.sidePosition[1] = XMVector3Rotate((pos - right * radius).ToXmV(), worldQ);

			constant.color = thisPointer->GetColor();
			constant.power = power; 
			constant.frustumNear = GetFrustumNear();
			constant.frustumFar = GetFrustumFar();
			constant.radius = radius;
			constant.penumbraScale = thisPointer->GetPenumbraWidth();
			constant.penumbraBlockerScale = thisPointer->GetPenumbraBlockerWidth();
			constant.shadowMapIndex = IsShadowActivated() ? GetResourceArrayIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE, 0) : 0;
			constant.hasShadowMap = IsShadowActivated();
			constant.shadowMapSize = thisPointer->GetShadowMapSize();
			constant.shadowMapInvSize = 1.0f / constant.shadowMapSize;
			constant.bias = thisPointer->GetBias(); 
			PointLitFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JShadowMapCubeDrawConstants& constant)noexcept final
		{
			const XMMATRIX projM = proj.LoadXM();
			for (uint i = 0; i < Graphic::Constants::cubeMapPlaneCount; ++i)
				constant.shadowMapTransform[i].StoreXM(XMMatrixTranspose(XMMatrixMultiply(view[i].LoadXM(), projM)));
			ShadowMapCubeDrawFrame::MinusMovedDirty();
		}
		void UpdateLightTransform()
		{
			for (uint i = 0; i < (uint)POINT_LIT_DIR::COUNT; ++i)
				Private::CalView(GetTransform(), (POINT_LIT_DIR)i, GetFrustumNear(), view[i]);
			proj.StoreXM(Private::CalProj(GetFrustumNear(), GetFrustumFar()));
			worldRight = GetTransform()->GetWorldRight();
		}
	public:
		static bool DoCopy(JPointLight* from, JPointLight* to)
		{
			from->impl->SetAllowFrustumCulling(to->impl->AllowFrustumCulling());
			from->impl->SetPower(to->impl->GetPower());
			from->impl->SetRange(to->impl->GetRange());
			from->impl->SetRadius(to->impl->GetRadius()); 

			to->impl->SetFrameDirty();
			return true;
		}
	public:
		void NotifyReAlloc()
		{
			if (thisPointer.IsValid())
			{
				JLightPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(thisPointer.Get(), thisPointer->GetGuid());
				JLightPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(thisPointer.Get(), this, thisPointer->GetGuid());
			}
			RegisterInterfacePointer();

			JFrameUpdateData::ReRegisterFrameData(JLightType::LitToFrameR(GetLightType()), (PointLitFrame*)this);
			JFrameUpdateData::ReRegisterFrameData(JLightType::SmToFrameR(GetLightType(), false), (ShadowMapCubeDrawFrame*)this);
		}
	public:
		void RegisterThisPointer(JPointLight* lit)
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
			JLightPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(thisPointer.Get(), this, thisPointer->GetGuid());
		}
		void RegisterLightFrameData(const Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE type)
		{
			if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT)
				PointLitFrame::RegisterFrameData(type, (PointLitFrame*)this, thisPointer->GetOwner()->GetOwnerGuid(), 1);
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW)
				ShadowMapCubeDrawFrame::RegisterFrameData(type, (ShadowMapCubeDrawFrame*)this, thisPointer->GetOwner()->GetOwnerGuid(), 1);
		}
		void DeRegisterPreDestruction()
		{
			if (thisPointer != nullptr)
				JLightPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(thisPointer.Get(), thisPointer->GetGuid());
		}
		void DeRegisterLightFrameData(const Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE type)
		{
			if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT)
				PointLitFrame::DeRegisterFrameData(type, (PointLitFrame*)this);
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW)
				ShadowMapCubeDrawFrame::DeRegisterFrameData(type, (ShadowMapCubeDrawFrame*)this);
		}
		static void RegisterTypeData()
		{
			Core::JIdentifier::RegisterPrivateInterface(JPointLight::StaticTypeInfo(), lPrivate);
			IMPL_REALLOC_BIND(JPointLight::JPointLightImpl, thisPointer)
				SET_GUI_FLAG(Core::J_GUI_OPTION_FLAG::J_GUI_OPTION_DISPLAY_PARENT);
		}
	};

	Core::JIdentifierPrivate& JPointLight::PrivateInterface()const noexcept
	{
		return lPrivate;
	}
	const Graphic::JGraphicResourceUserInterface JPointLight::GraphicResourceUserInterface()const noexcept
	{
		return Graphic::JGraphicResourceUserInterface(impl.get());
	}
	const Graphic::JCullingUserInterface JPointLight::CullingUserInterface()const noexcept
	{
		return Graphic::JCullingUserInterface(impl.get());
	}
	J_LIGHT_TYPE JPointLight::GetLightType()const noexcept
	{
		return impl->GetLightType();
	}
	J_SHADOW_MAP_TYPE JPointLight::GetShadowMapType()const noexcept
	{
		return impl->GetShadowMapType();
	}
	float JPointLight::GetFrustumNear()const noexcept
	{
		return impl->GetFrustumNear();
	}
	float JPointLight::GetFrustumFar()const noexcept
	{
		return impl->GetFrustumFar();
	}
	float JPointLight::GetPower()const noexcept
	{
		return impl->GetPower();
	}
	float JPointLight::GetRange()const noexcept
	{
		return impl->GetRange();
	}
	float JPointLight::GetRadius()const noexcept
	{
		return impl->GetRadius();
	}
	DirectX::BoundingBox JPointLight::GetBBox()const noexcept
	{
		return impl->GetBBox();
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
	void JPointLight::SetPower(const float power)noexcept
	{
		impl->SetPower(power);
	}
	void JPointLight::SetRange(const float range)noexcept
	{
		impl->SetRange(range);
	}
	void JPointLight::SetRadius(const float radius)noexcept
	{
		impl->SetRadius(radius);
	}
	bool JPointLight::IsFrameDirted()const noexcept
	{
		return impl->IsFrameDirted();
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
		JLight::DoActivate();
		impl->Activate();
		impl->SetFrameDirty();
	}
	void JPointLight::DoDeActivate()noexcept
	{
		impl->DeActivate();
		impl->OffFrameDirty();
		JLight::DoDeActivate();
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
	using FrameUpdateInterface = JPointLightPrivate::FrameUpdateInterface;
	using FrameIndexInterface = JPointLightPrivate::FrameIndexInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JPointLight>(*static_cast<JPointLight::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JLightPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JPointLight* lit = static_cast<JPointLight*>(createdPtr);
		lit->impl->RegisterThisPointer(lit);
		lit->impl->RegisterInterfacePointer();
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
		float sPower;
		float sRange;
		float sRadius;

		auto loadData = static_cast<JPointLight::LoadData*>(data);
		JFileIOTool& tool = loadData->tool;
		JUserPtr<JGameObject> owner = loadData->owner;

		JObjectFileIOHelper::LoadComponentIden(tool, guid, flag, isActivated);
		auto idenUser = lPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JPointLight::InitData>(guid, flag, owner), &lPrivate);
		JUserPtr<JPointLight> litUser;
		litUser.ConnnectChild(idenUser);

		JLightPrivate::AssetDataIOInterface::LoadLightData(tool, litUser);
		JObjectFileIOHelper::LoadAtomicData(tool, sPower, "Power:");
		JObjectFileIOHelper::LoadAtomicData(tool, sRange, "Range:");
		JObjectFileIOHelper::LoadAtomicData(tool, sRadius, "Radius:");
		litUser->SetPower(sPower);
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
		JObjectFileIOHelper::StoreAtomicData(tool, impl->power, "Power:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->range, "Range:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->radius, "Radius:");

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	bool FrameUpdateInterface::UpdateStart(JLight* lit, const bool isUpdateForced)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::POINT)
			return false;

		JPointLight* dLit = static_cast<JPointLight*>(lit);
		if (isUpdateForced)
			dLit->impl->SetFrameDirty();

		dLit->impl->SetLastFrameUpdatedTrigger(false);
		dLit->impl->SetLastFrameHotUpdatedTrigger(false);
		if (dLit->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources)
			dLit->impl->UpdateLightTransform();
		return dLit->impl->IsFrameDirted();
	}
	void FrameUpdateInterface::UpdateFrame(JPointLight* lit, Graphic::JPointLightConstants& constant)noexcept
	{
		lit->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateFrame(JPointLight* lit, Graphic::JShadowMapCubeDrawConstants& constant)noexcept
	{
		lit->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateEnd(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::POINT)
			return;

		JPointLight* dLit = static_cast<JPointLight*>(lit);
		if (dLit->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources)
			dLit->impl->SetLastFrameHotUpdatedTrigger(true);
		dLit->impl->SetLastFrameUpdatedTrigger(true);
		dLit->impl->UpdateFrameEnd();
	}
	int FrameUpdateInterface::GetLitFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::POINT)
			return -1;

		return static_cast<JPointLight*>(lit)->impl->PointLitFrame::GetFrameIndex();
	}
	int FrameUpdateInterface::GetShadowMapFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::POINT)
			return -1;

		return static_cast<JPointLight*>(lit)->impl->ShadowMapCubeDrawFrame::GetFrameIndex();
	}
	int FrameUpdateInterface::GetDepthTestPassFrameIndex(JLight* lit)noexcept
	{
		return -1;
	}
	int FrameUpdateInterface::GetHzbOccComputeFrameIndex(JLight* lit)noexcept
	{
		return -1;
	}
	bool FrameUpdateInterface::IsHotUpdate(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::POINT)
			return false;

		return static_cast<JPointLight*>(lit)->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources;
	}
	bool FrameUpdateInterface::IsLastFrameHotUpdated(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::POINT)
			return false;

		return static_cast<JPointLight*>(lit)->impl->IsLastFrameHotUpdated();
	}
	bool FrameUpdateInterface::IsLastUpdated(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::POINT)
			return false;

		return static_cast<JPointLight*>(lit)->impl->IsLastFrameUpdated();
	}
	bool FrameUpdateInterface::HasLitRecopyRequest(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::POINT)
			return false;

		return static_cast<JPointLight*>(lit)->impl->ShadowMapCubeDrawFrame::HasMovedDirty();
	}
	bool FrameUpdateInterface::HasShadowMapRecopyRequest(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::POINT)
			return false;

		return static_cast<JPointLight*>(lit)->impl->ShadowMapCubeDrawFrame::HasMovedDirty();
	}
	bool FrameUpdateInterface::HasDepthTestPassRecopyRequest(JLight* lit)noexcept
	{
		return false;
	}
	bool FrameUpdateInterface::HasHzbOccComputeRecopyRequest(JLight* lit)noexcept
	{
		return false;
	}

	int FrameIndexInterface::GetLitFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::POINT)
			return -1;

		return static_cast<JPointLight*>(lit)->impl->PointLitFrame::GetFrameIndex();
	}
	int FrameIndexInterface::GetShadowMapFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::POINT)
			return -1;

		return static_cast<JPointLight*>(lit)->impl->ShadowMapCubeDrawFrame::GetFrameIndex();
	}
	int FrameIndexInterface::GetDepthTestPassFrameIndex(JLight* lit)noexcept
	{
		return -1;
	}
	int FrameIndexInterface::GetHzbOccComputeFrameIndex(JLight* lit)noexcept
	{
		return -1;
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
	JLightPrivate::FrameUpdateInterface& JPointLightPrivate::GetFrameUpdateInterface()const noexcept
	{
		static FrameUpdateInterface pI;
		return pI;
	}
	JLightPrivate::FrameIndexInterface& JPointLightPrivate::GetFrameIndexInterface()const noexcept
	{
		static FrameIndexInterface pI;
		return pI;
	}
}