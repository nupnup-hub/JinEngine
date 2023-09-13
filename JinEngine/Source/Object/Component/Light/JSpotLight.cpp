#include"JSpotLight.h"   
#include"JSpotLightPrivate.h"
#include"JLightConstants.h"
#include"../Transform/JTransform.h" 
#include"../JComponentHint.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h" 
#include"../../Resource/Scene/JScenePrivate.h"
#include"../../../Core/Guid/JGuidCreator.h" 
#include"../../../Core/File/JFileConstant.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Math/JMathHelper.h"
#include"../../../Graphic/JGraphic.h"  
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
		using LitFrameUpdate = Graphic::JFrameUpdate<Graphic::JFrameUpdateInterfaceHolder2<
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, Graphic::JSpotLightConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW, Graphic::JShadowMapDrawConstants&>>,
			Graphic::JFrameDirty>;
	}
	namespace
	{
		static auto isAvailableoverlapLam = []() {return true; };
		static JSpotLightPrivate lPrivate;
 
		static constexpr float minSpotPower = 0.1f;
		static constexpr float maxSpotPower = 1.0f;
		static constexpr float minSpotAngle = 15.0f * JMathHelper::DegToRad;
		static constexpr float maxSpotAngle = 90.0f * JMathHelper::DegToRad;
		static constexpr float minSpotAspect = 0.1f;
		static constexpr float maxSpotAspect = 16.0f;

		static constexpr float InitSpotAngle()noexcept
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
			const XMVECTOR worldPos = CalLightWorldPos(transform);
			const XMVECTOR worldDir = CalLightWorldDir(transform);

			return XMMatrixLookAtLH(worldPos,
				worldDir,
				XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
		}
		static XMMATRIX CalProj(const float fallEnd,
			const float angle,
			const float aspect)noexcept
		{
			return XMMatrixPerspectiveFovLH(angle, aspect, Constants::lightNear, fallEnd);
		}
	}

	class JSpotLight::JSpotLightImpl : public Core::JTypeImplBase,
		public LitFrameUpdate,
		public Graphic::JGraphicTypePerSingleResourceHolder,
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
		REGISTER_PROPERTY_EX(falloffStart, GetFalloffStart, SetFalloffStart, GUI_SLIDER(1, 2000, true, false))
		float falloffStart = 1.0f;
		REGISTER_PROPERTY_EX(falloffEnd, GetFalloffEnd, SetFalloffEnd, GUI_SLIDER(1, 2000, true, false))
		float falloffEnd = 100.0f;
		REGISTER_PROPERTY_EX(spotPower, GetSpotPower, SetSpotPower, GUI_SLIDER(minSpotPower, maxSpotPower, true, false))
		float spotPower = minSpotPower;
		REGISTER_PROPERTY_EX(spotAngle, GetSpotAngle, SetSpotAngle, GUI_SLIDER(minSpotAngle, maxSpotAngle, true, false))
		float spotAngle = InitSpotAngle(); 
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
		float GetNear()const noexcept
		{
			return Constants::lightNear;
		}
		float GetFar()const noexcept
		{
			return falloffEnd;
		}
		float GetFalloffStart()const noexcept
		{
			return falloffStart;
		}
		float GetFalloffEnd()const noexcept
		{
			return falloffEnd;
		}
		float GetSpotPower()const noexcept
		{
			return spotPower;
		}
		float GetSpotAngle()const noexcept
		{
			return spotAngle;
		} 
		DirectX::BoundingBox GetBBox()const
		{
			auto t = GetTransform(); 
			auto litFar = GetFar();
			auto radius = litFar * tan(spotAngle);
			auto centerV = XMVectorAdd(CalLightWorldPos(t), XMVectorScale(CalLightWorldDir(t), litFar * 0.5f));
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
		void SetFalloffStart(const float newFalloffStart)noexcept
		{
			falloffStart = newFalloffStart;
			SetFrameDirty();
		}
		void SetFalloffEnd(const float newFalloffEnd)noexcept
		{
			falloffEnd = newFalloffEnd;
			SetFrameDirty();
		}
		void SetSpotPower(const float newSpotPower)noexcept
		{
			spotPower = std::clamp(newSpotPower, minSpotPower, maxSpotPower);
			SetFrameDirty();
		}
		void SetSpotAngle(const float newspotAngle)noexcept
		{
			spotAngle = std::clamp(newspotAngle, minSpotAngle, maxSpotAngle);
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
			CreateShadowMapCubeTexture(thisPointer->GetShadowMapSize());
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
			constant.falloffStart = falloffStart;
			constant.position = CalLightWorldPos(GetTransform());
			constant.falloffEnd = falloffEnd;
			constant.direction = CalLightWorldDir(GetTransform());
			constant.power = spotPower;
			constant.angle = spotAngle;
			constant.shadowMapIndex = IsShadowActivated() ? GetResourceArrayIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, 0) : 0;
			constant.hasShadowMap = IsShadowActivated();
			SpotLitFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JShadowMapDrawConstants& constant)noexcept final
		{
			constant.shadowMapTransform.StoreXM(XMMatrixTranspose(view.LoadXM() * proj.LoadXM()));
			ShadowMapDrawFrame::MinusMovedDirty();
		}
		void UpdateLightTransform()noexcept
		{
			view.StoreXM(CalView(GetTransform()));
			proj.StoreXM(CalProj(falloffEnd, spotAngle, 1));
		}
	public:
		static bool DoCopy(JSpotLight* from, JSpotLight* to)
		{ 
			from->impl->SetAllowFrustumCulling(to->impl->AllowFrustumCulling());
			from->impl->SetFalloffStart(to->impl->GetFalloffStart());
			from->impl->SetFalloffEnd(to->impl->GetFalloffEnd());
			from->impl->SetSpotPower(to->impl->GetSpotPower());
			from->impl->SetSpotAngle(to->impl->GetSpotAngle()); 

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
	float JSpotLight::GetNear()const noexcept
	{
		return impl->GetNear();
	}
	float JSpotLight::GetFar()const noexcept
	{
		return impl->GetFar();
	}
	float JSpotLight::GetFalloffStart()const noexcept
	{
		return impl->GetFalloffStart();
	}
	float JSpotLight::GetFalloffEnd()const noexcept
	{
		return impl->GetFalloffEnd();
	}
	float JSpotLight::GetSpotPower()const noexcept
	{
		return impl->GetSpotPower();
	}
	float JSpotLight::GetSpotAngle()const noexcept
	{
		return impl->GetSpotAngle();
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
		if (sQuality == GetShadowResolution())
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
	void JSpotLight::SetFalloffStart(const float falloffStart)noexcept
	{
		impl->SetFalloffStart(falloffStart);
	}
	void JSpotLight::SetFalloffEnd(const float falloffEnd)noexcept
	{
		impl->SetFalloffEnd(falloffEnd);
	}
	void JSpotLight::SetSpotPower(const float spotPower)noexcept
	{
		impl->SetSpotPower(spotPower);
	}
	void JSpotLight::SetSpotAngle(const float spotAngle)noexcept
	{
		impl->SetSpotAngle(spotAngle);
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
		float sFalloffStart;
		float sFalloffEnd;
		float sSpotPower;
		float sSpotAngle; 

		auto loadData = static_cast<JSpotLight::LoadData*>(data);
		std::wifstream& stream = loadData->stream;
		JUserPtr<JGameObject> owner = loadData->owner;

		JFileIOHelper::LoadObjectIden(stream, guid, flag);
		auto idenUser = lPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JSpotLight::InitData>(guid, flag, owner), &lPrivate);
		JUserPtr<JSpotLight> litUser;
		litUser.ConnnectChild(idenUser);

		JLightPrivate::AssetDataIOInterface::LoadLightData(stream, litUser);
		JFileIOHelper::LoadAtomicData(stream, sFalloffStart);
		JFileIOHelper::LoadAtomicData(stream, sFalloffEnd);
		JFileIOHelper::LoadAtomicData(stream, sSpotPower);
		JFileIOHelper::LoadAtomicData(stream, sSpotAngle); 

		litUser->SetFalloffStart(sFalloffStart);
		litUser->SetFalloffEnd(sFalloffEnd);
		litUser->SetSpotPower(sSpotPower);
		litUser->SetSpotAngle(sSpotAngle); 
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

		JFileIOHelper::StoreObjectIden(stream, lit.Get());
		JLightPrivate::AssetDataIOInterface::StoreLightData(stream, lit);
		JFileIOHelper::StoreAtomicData(stream, L"FallOffStart:", impl->falloffStart);
		JFileIOHelper::StoreAtomicData(stream, L"FallOffEnd:", impl->falloffEnd);
		JFileIOHelper::StoreAtomicData(stream, L"SpotPower:", impl->spotPower);
		JFileIOHelper::StoreAtomicData(stream, L"SpotAngle:", impl->spotAngle); 

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
		if (dLit->impl->IsFrameDirted())
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

		return static_cast<JSpotLight*>(lit)->impl->SpotLitFrame::GetUploadIndex();
	}
	int FrameUpdateInterface::GetShadowMapFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::SPOT)
			return -1;

		return static_cast<JSpotLight*>(lit)->impl->ShadowMapDrawFrame::GetUploadIndex();
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

		return static_cast<JSpotLight*>(lit)->impl->SpotLitFrame::GetUploadIndex();
	}
	int FrameIndexInterface::GetShadowMapFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::SPOT)
			return -1;

		return static_cast<JSpotLight*>(lit)->impl->ShadowMapDrawFrame::GetUploadIndex();
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