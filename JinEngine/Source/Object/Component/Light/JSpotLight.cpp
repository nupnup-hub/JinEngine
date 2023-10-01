#include"JSpotLight.h"   
#include"JSpotLightPrivate.h"
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
#include"../../../Graphic/Frameresource/JLightConstants.h"  
#include"../../../Graphic/Frameresource/JShadowMapConstants.h"   
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
	}
	namespace
	{
		static auto isAvailableoverlapLam = []() {return true; };
		static JSpotLightPrivate lPrivate;
	}
	namespace Private
	{
		static constexpr float minPower = 0.1f;
		static constexpr float maxPower = 2.0f;
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
			return 30.0f * JMathHelper::DegToRad;
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
		static XMMATRIX CalProj(const float range, const float angle, const float aspect)noexcept
		{  
			//Caution!
			//Near값은 1보다 작을시 shadow map에 그려지는 물체들의 깊이값이 비정확해진다.
			return XMMatrixPerspectiveFovLH(angle, aspect, Constants::lightNear, range);
		} 
	}

	class JSpotLight::JSpotLightImpl : public Core::JTypeImplBase,
		public LitFrameUpdate,
		public Graphic::JGraphicWideSingleResourceHolder<2>,
		public Graphic::JGraphicDrawListCompInterface,
		public Graphic::JCullingInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JSpotLightImpl)
	public:
		using SpotLitFrame = JFrameInterface1;
		using ShadowMapDrawFrame = JFrameInterface2;
	public:
		JMatrix4x4 view;
		JMatrix4x4 proj;
	public:
		JWeakPtr<JSpotLight> thisPointer;
	public:
		REGISTER_PROPERTY_EX(power, GetPower, SetPower, GUI_SLIDER(Private::minPower, Private::maxPower, true, false))
		float power = Private::InitPower();
		REGISTER_PROPERTY_EX(range, GetRange, SetRange, GUI_SLIDER(Constants::lightNear, Constants::lightMaxFar, true, false))
		float range = 100.0f;
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
			return Constants::lightNear;
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
			auto extentsV = XMVector3Rotate(XMVectorSet(radius, -litFar * 0.5f, radius, 1.0f), t->GetQuaternion());

			DirectX::BoundingBox bbox;
			XMStoreFloat3(&bbox.Center, centerV);
			XMStoreFloat3(&bbox.Extents, XMVectorAbs(extentsV));
			return bbox;
		}
		JUserPtr<JTransform> GetTransform()const noexcept
		{
			return thisPointer->GetOwner()->GetTransform();
		}
		XMMATRIX GetShadowMapTransform()const noexcept
		{
			return XMMatrixMultiply(XMMatrixMultiply(view.LoadXM(), proj.LoadXM()), JMatrix4x4::NdcToTextureSpaceXM());
		}
	public:
		//value가 bool type일경우에만 justCallFunc을 사용할수있다
		//justCallFunc는 값을 변경하지않고 함수내에서 value per 기능을 수행한다
		void SetShadow(const bool value)
		{
			SetShadowEx(value, false);
		}
		void SetShadowEx(const bool value, const bool justCallFunc)noexcept
		{
			if (justCallFunc || thisPointer->IsActivated())
			{
				if (thisPointer->IsShadowActivated())
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
		void SetAllowDisplayShadowMap(const bool value)
		{
			SetAllowDisplayShadowMapEx(value, false);
		}
		void SetAllowDisplayShadowMapEx(const bool value, const bool justCallFunc = false)
		{
			if (justCallFunc || thisPointer->IsActivated())
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
		}
		void SetOuterConeDegAngle(const float newAngle)noexcept
		{
			SetOuterConeAngle(newAngle * JMathHelper::DegToRad);
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
			CreateShadowMapTexture(thisPointer->GetShadowMapSize());
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
			DestroyTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG);
		}
	public:
		void Activate()noexcept
		{
			RegisterLightFrameData(JLightType::LitToFrameR(GetLightType()));
			if (thisPointer->IsShadowActivated())
				SetShadowEx(true, true);
			if (allowFrustumCulling)
				SetAllowFrustumCulling(true, true);
		}
		void DeActivate()noexcept
		{
			//has order dependency
			DeRegisterLightFrameData(JLightType::LitToFrameR(GetLightType()));
			if (thisPointer->IsShadowActivated())
				SetShadowEx(false, true);
			if (allowFrustumCulling)
				SetAllowFrustumCulling(false, true);
			DestroyAllCullingData();
			DestroyAllTexture();
		}
	public:
		void UpdateFrame(Graphic::JSpotLightConstants& constant)noexcept final
		{
			if (IsShadowActivated())
				constant.shadowMapTransform.StoreXM(XMMatrixTranspose(GetShadowMapTransform()));

			constant.color = thisPointer->GetColor(); 
			constant.range = range;
			constant.position = Private::CalLightWorldPos(GetTransform());
			constant.direction = Private::CalLightWorldDir(GetTransform());
			constant.power = power;
			constant.innerConeAngle = cos(innerConeAngle);
			constant.outerConeAngle = cos(outerConeAngle);
			constant.shadowMapIndex = IsShadowActivated() ? GetResourceArrayIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, 0) : 0;
			constant.hasShadowMap = IsShadowActivated();
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
			proj.StoreXM(Private::CalProj(range, outerConeAngle * 2, 1));
		}
	public:
		static bool DoCopy(JSpotLight* from, JSpotLight* to)
		{
			from->impl->SetAllowFrustumCulling(to->impl->AllowFrustumCulling());
			from->impl->SetPower(to->impl->GetPower());
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
				JLightPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(thisPointer.Get(), thisPointer->GetGuid());
				JLightPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(thisPointer.Get(), this, thisPointer->GetGuid());
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
			JLightPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(thisPointer.Get(), this, thisPointer->GetGuid());
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
				JLightPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(thisPointer.Get(), thisPointer->GetGuid());
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
	float JSpotLight::GetFrustumNear()const noexcept
	{
		return impl->GetFrustumNear();
	}
	float JSpotLight::GetFrustumFar()const noexcept
	{
		return impl->GetFrustumFar();
	}
	float JSpotLight::GetPower()const noexcept
	{
		return impl->GetPower();
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
	void JSpotLight::SetPower(const float power)noexcept
	{
		impl->SetPower(power);
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
		JLight::DoActivate();
		impl->Activate();
		impl->SetFrameDirty();
	}
	void JSpotLight::DoDeActivate()noexcept
	{
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
		float sPower;
		float sInnerAngle;
		float sOuterConeAngle;

		auto loadData = static_cast<JSpotLight::LoadData*>(data);
		std::wifstream& stream = loadData->stream;
		JUserPtr<JGameObject> owner = loadData->owner;

		JObjectFileIOHelper::LoadComponentIden(stream, guid, flag, isActivated);
		auto idenUser = lPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JSpotLight::InitData>(guid, flag, owner), &lPrivate);
		JUserPtr<JSpotLight> litUser;
		litUser.ConnnectChild(idenUser);

		JLightPrivate::AssetDataIOInterface::LoadLightData(stream, litUser);
		JObjectFileIOHelper::LoadAtomicData(stream, sPower);
		JObjectFileIOHelper::LoadAtomicData(stream, sRange);
		JObjectFileIOHelper::LoadAtomicData(stream, sInnerAngle);
		JObjectFileIOHelper::LoadAtomicData(stream, sOuterConeAngle);

		litUser->SetPower(sPower);
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
		std::wofstream& stream = storeData->stream;

		JObjectFileIOHelper::StoreComponentIden(stream, lit.Get());
		JLightPrivate::AssetDataIOInterface::StoreLightData(stream, lit);
		JObjectFileIOHelper::StoreAtomicData(stream, L"Power:", impl->power);
		JObjectFileIOHelper::StoreAtomicData(stream, L"Range:", impl->range);
		JObjectFileIOHelper::StoreAtomicData(stream, L"InnerConeAngle:", impl->innerConeAngle);
		JObjectFileIOHelper::StoreAtomicData(stream, L"OuterConeAngle:", impl->outerConeAngle);

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
		if (dLit->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources)
			dLit->impl->UpdateLightTransform();
		return dLit->impl->IsFrameDirted();
	}
	void FrameUpdateInterface::UpdateFrame(JSpotLight* lit, Graphic::JSpotLightConstants& constant)noexcept
	{
		lit->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateFrame(JSpotLight* lit, Graphic::JShadowMapDrawConstants& constant)noexcept
	{
		lit->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateEnd(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::SPOT)
			return;

		JSpotLight* dLit = static_cast<JSpotLight*>(lit);
		if (dLit->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources)
			dLit->impl->SetLastFrameHotUpdatedTrigger(true);
		dLit->impl->SetLastFrameUpdatedTrigger(true);
		dLit->impl->UpdateFrameEnd();
	}
	int FrameUpdateInterface::GetLitFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::SPOT)
			return -1;

		return static_cast<JSpotLight*>(lit)->impl->SpotLitFrame::GetFrameIndex();
	}
	int FrameUpdateInterface::GetShadowMapFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::SPOT)
			return -1;

		return static_cast<JSpotLight*>(lit)->impl->ShadowMapDrawFrame::GetFrameIndex();
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
		if (lit->GetLightType() != J_LIGHT_TYPE::SPOT)
			return false;

		return static_cast<JSpotLight*>(lit)->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources;
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
	bool FrameUpdateInterface::HasLitRecopyRequest(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::SPOT)
			return false;

		return static_cast<JSpotLight*>(lit)->impl->ShadowMapDrawFrame::HasMovedDirty();
	}
	bool FrameUpdateInterface::HasShadowMapRecopyRequest(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::SPOT)
			return false;

		return static_cast<JSpotLight*>(lit)->impl->ShadowMapDrawFrame::HasMovedDirty();
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
		if (lit->GetLightType() != J_LIGHT_TYPE::SPOT)
			return -1;

		return static_cast<JSpotLight*>(lit)->impl->SpotLitFrame::GetFrameIndex();
	}
	int FrameIndexInterface::GetShadowMapFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::SPOT)
			return -1;

		return static_cast<JSpotLight*>(lit)->impl->ShadowMapDrawFrame::GetFrameIndex();
	}
	int FrameIndexInterface::GetDepthTestPassFrameIndex(JLight* lit)noexcept
	{
		return -1;
	}
	int FrameIndexInterface::GetHzbOccComputeFrameIndex(JLight* lit)noexcept
	{
		return -1;
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