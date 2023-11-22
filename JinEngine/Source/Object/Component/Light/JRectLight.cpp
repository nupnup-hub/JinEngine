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

using namespace DirectX;
namespace JinEngine
{
	namespace
	{
		using LitFrameUpdate = Graphic::JFrameUpdate<Graphic::JFrameUpdateInterfaceHolder2<
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT, Graphic::JRectLightConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW, Graphic::JShadowMapDrawConstants&>>,
			Graphic::JFrameDirty>;
	}
	namespace
	{ 
		static JRectLightPrivate lPrivate;
	}
	namespace Private
	{ 
		static constexpr float minPower = 0.1f;
		static constexpr float maxPower = 8.0f;
		static constexpr float frustumNear = 1.0f;	 
		static constexpr float InitPower()noexcept
		{
			return 1.0f;
		}
		static JVector2F InitSize()noexcept
		{
			return JVector2F(2, 2);
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
	}

	class JRectLight::JRectLightImpl : public Core::JTypeImplBase,
		public LitFrameUpdate,
		public JResourceObjectUserInterface,
		public Graphic::JGraphicWideSingleResourceHolder<2>,	//shadowMap, debug
		//public Graphic::JGraphicTypePerSingleResourceHolder,
		public Graphic::JGraphicDrawListCompInterface,
		public Graphic::JCullingInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JRectLightImpl)
	public:
		using RectLitFrame = JFrameInterface1;
		using ShadowMapDrawFrame = JFrameInterface2;
	public:
		JWeakPtr<JRectLight> thisPointer; 
	public:
		REGISTER_PROPERTY_EX(areaSize, GetAreaSize, SetAreaSize, GUI_INPUT(false))
		JVector2F areaSize = Private::InitSize();
	public:
		REGISTER_PROPERTY_EX(power, GetPower, SetPower, GUI_SLIDER(Private::minPower, Private::maxPower, true, false))
		float power = Private::InitPower();
		REGISTER_PROPERTY_EX(range, GetRange, SetRange, GUI_SLIDER(Constants::lightNear, Constants::lightMaxFar, true, false))
		float range = 100.0f;
	public:
		REGISTER_PROPERTY_EX(isTwoSide, IsTwoSide, SetTwoSide, GUI_CHECKBOX())
		bool isTwoSide = false;
		bool allowFrustumCulling = false;
	public:
		REGISTER_PROPERTY_EX(sourceTexture, GetSourceTexture, SetSourceTexture, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, false))
		JUserPtr<JTexture> sourceTexture;
	public:
		JVector4F worldQ;
		JVector3F axis[3];
		JMatrix4x4 view;
		JMatrix4x4 proj;
	public:
		JRectLightImpl(const InitData& initData, JRectLight* thisLitRaw) {}
		~JRectLightImpl() {}
	public:
		J_LIGHT_TYPE GetLightType()const noexcept
		{ 
			sizeof(JRectLightImpl); 
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
		float GetPower()const noexcept
		{
			return power;
		} 
		float GetRange()const noexcept
		{
			return range;
		}
		DirectX::BoundingBox GetBBox()const noexcept
		{
			DirectX::BoundingBox bbox;
			bbox.Center = GetTransform()->GetWorldPosition().ToSimilar<XMFLOAT3>();
			bbox.Extents = XMFLOAT3(areaSize.x, areaSize.y, range);
			return bbox;
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
		void SetAreaSize(const JVector2F newAreaSize) noexcept
		{
			if (areaSize == newAreaSize)
				return;

			areaSize.Clamp(newAreaSize, 0, Constants::lightMaxDistance);
			SetFrameDirty();
		}
		void SetTwoSide(const bool value)noexcept
		{
			if (value == isTwoSide)
				return;

			isTwoSide = value;
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
		void SetSourceTexture(const JUserPtr<JTexture>& newSourceTexture)noexcept
		{
			CallOffResourceReference(sourceTexture.Get());
			sourceTexture = newSourceTexture;
			CallOnResourceReference(sourceTexture.Get());
			SetFrameDirty();
		}
	public:
		bool IsShadowActivated()const noexcept
		{
			return thisPointer->IsShadowActivated();
		}
		bool IsTwoSide()const noexcept
		{
			return isTwoSide;
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
		void UpdateFrame(Graphic::JRectLightConstants& constant)noexcept final
		{
			const XMMATRIX projM = proj.LoadXM();
			const XMMATRIX ndcM = JMatrix4x4::NdcToTextureSpaceXM();
			 
			constant.shadowMapTransform.StoreXM(XMMatrixTranspose(XMMatrixMultiply(XMMatrixMultiply(view.LoadXM(), projM), ndcM)));
			constant.origin = GetTransform()->GetWorldPosition();
			constant.extents = JVector3F(areaSize * 0.5f, 0.0f);
			constant.axis[0] = axis[0];
			constant.axis[1] = axis[1];
			constant.axis[2] = axis[2];
			constant.color = thisPointer->GetColor();
			constant.power = power; 
			constant.frustumNear = GetFrustumNear();
			constant.frustumFar = GetFrustumFar();
			constant.isTwoSide = isTwoSide;
			constant.shadowMapIndex = IsShadowActivated() ? GetResourceArrayIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, 0) : 0;
			constant.hasShadowMap = IsShadowActivated(); 
			constant.sourceTextureIndex = sourceTexture != nullptr ? sourceTexture->GraphicResourceUserInterface().GetFirstResourceArrayIndex() : invalidIndex;
			RectLitFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JShadowMapDrawConstants& constant)noexcept final
		{ 
			constant.shadowMapTransform.StoreXM(XMMatrixTranspose(XMMatrixMultiply(view.LoadXM(), proj.LoadXM())));
			ShadowMapDrawFrame::MinusMovedDirty();
		}
		void UpdateLightTransform()
		{
			auto t = GetTransform();
			view.StoreXM(Private::CalView(t));
			proj.StoreXM(Private::CalProj(GetFrustumNear(), GetFrustumFar())); 
			worldQ = t->GetWorldQuaternion();
			axis[0] = t->GetWorldRight();
			axis[1] = t->GetWorldUp();
			axis[2] = t->GetWorldFront();
		}
	public:
		static bool DoCopy(JRectLight* from, JRectLight* to)
		{
			from->impl->SetAllowFrustumCulling(to->impl->AllowFrustumCulling());
			from->impl->SetPower(to->impl->GetPower());
			from->impl->SetRange(to->impl->GetRange()); 

			to->impl->SetFrameDirty();
			return true;
		}
	public:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj, JResourceEventDesc* desc)
		{
			if (iden == thisPointer->GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (sourceTexture != nullptr && sourceTexture->GetGuid() == jRobj->GetGuid())
					SetSourceTexture(nullptr);
			}
			else if (eventType == J_RESOURCE_EVENT_TYPE::UPDATE_RESOURCE)
			{
				if (sourceTexture != nullptr && sourceTexture->GetGuid() == jRobj->GetGuid())
					SetFrameDirty();
			}
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

			JFrameUpdateData::ReRegisterFrameData(JLightType::LitToFrameR(GetLightType()), (RectLitFrame*)this);
			JFrameUpdateData::ReRegisterFrameData(JLightType::SmToFrameR(GetLightType(), false), (ShadowMapDrawFrame*)this);
			ResetEventListenerPointer(*JResourceObject::EvInterface(), thisPointer->GetGuid());
		}
	public:
		void RegisterThisPointer(JRectLight* lit)
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
			AddEventListener(*JResourceObject::EvInterface(), thisPointer->GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
			AddEventListener(*JResourceObject::EvInterface(), thisPointer->GetGuid(), J_RESOURCE_EVENT_TYPE::UPDATE_RESOURCE);
			JLightPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(thisPointer.Get(), this, thisPointer->GetGuid());
		}
		void RegisterLightFrameData(const Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE type)
		{
			if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT)
				RectLitFrame::RegisterFrameData(type, (RectLitFrame*)this, thisPointer->GetOwner()->GetOwnerGuid(), 1);
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW)
				ShadowMapDrawFrame::RegisterFrameData(type, (ShadowMapDrawFrame*)this, thisPointer->GetOwner()->GetOwnerGuid(), 1);
		}
		void DeRegisterPreDestruction()
		{
			RemoveListener(*JResourceObject::EvInterface(), thisPointer->GetGuid());
			if (thisPointer != nullptr)
				JLightPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(thisPointer.Get(), thisPointer->GetGuid());
		}
		void DeRegisterLightFrameData(const Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE type)
		{
			if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT)
				RectLitFrame::DeRegisterFrameData(type, (RectLitFrame*)this);
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW)
				ShadowMapDrawFrame::DeRegisterFrameData(type, (ShadowMapDrawFrame*)this);
		}
		static void RegisterTypeData()
		{
			Core::JIdentifier::RegisterPrivateInterface(JRectLight::StaticTypeInfo(), lPrivate);
			IMPL_REALLOC_BIND(JRectLight::JRectLightImpl, thisPointer)
			SET_GUI_FLAG(Core::J_GUI_OPTION_FLAG::J_GUI_OPTION_DISPLAY_PARENT);
		}
	};

	Core::JIdentifierPrivate& JRectLight::PrivateInterface()const noexcept
	{
		return lPrivate;
	}
	const Graphic::JGraphicResourceUserInterface JRectLight::GraphicResourceUserInterface()const noexcept
	{
		return Graphic::JGraphicResourceUserInterface(impl.get());
	}
	const Graphic::JCullingUserInterface JRectLight::CullingUserInterface()const noexcept
	{
		return Graphic::JCullingUserInterface(impl.get());
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
	float JRectLight::GetFrustumNear()const noexcept
	{
		return impl->GetFrustumNear();
	}
	float JRectLight::GetFrustumFar()const noexcept
	{
		return impl->GetFrustumFar();
	}
	float JRectLight::GetPower()const noexcept
	{
		return impl->GetPower();
	}
	float JRectLight::GetRange()const noexcept
	{
		return impl->GetRange();
	} 
	DirectX::BoundingBox JRectLight::GetBBox()const noexcept
	{
		return impl->GetBBox();
	}
	JVector3F JRectLight::GetDirection()const noexcept
	{
		return Private::CalLightWorldDir(impl->GetTransform());
	}
	JUserPtr<JTexture> JRectLight::GetSourceTexture()const noexcept
	{
		return impl->GetSourceTexture();
	}
	void JRectLight::SetShadow(const bool value)noexcept
	{
		if (value == IsShadowActivated())
			return;

		JLight::SetShadow(value);
		impl->SetShadow(value);
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

		JLight::SetAllowDisplayShadowMap(value);
		impl->SetAllowDisplayShadowMap(value);
	}
	void JRectLight::SetAreaSize(const JVector2F newSize)noexcept
	{
		impl->SetAreaSize(newSize);
	}
	void JRectLight::SetTwoSide(const bool value)noexcept
	{
		impl->SetTwoSide(value);
	}
	void JRectLight::SetPower(const float power)noexcept
	{
		impl->SetPower(power);
	}
	void JRectLight::SetRange(const float range)noexcept
	{
		impl->SetRange(range);
	} 
	void JRectLight::SetSourceTexture(const JUserPtr<JTexture>& srcTexture)noexcept
	{
		impl->SetSourceTexture(srcTexture);
	}
	bool JRectLight::IsFrameDirted()const noexcept
	{
		return impl->IsFrameDirted();
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
		JLight::DoActivate();
		impl->Activate();
		impl->SetFrameDirty();
	}
	void JRectLight::DoDeActivate()noexcept
	{
		impl->DeActivate();
		impl->OffFrameDirty();
		JLight::DoDeActivate();
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
	using FrameUpdateInterface = JRectLightPrivate::FrameUpdateInterface;
	using FrameIndexInterface = JRectLightPrivate::FrameIndexInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JRectLight>(*static_cast<JRectLight::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JLightPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JRectLight* lit = static_cast<JRectLight*>(createdPtr);
		lit->impl->RegisterThisPointer(lit);
		lit->impl->RegisterInterfacePointer();
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
		float sPower;
		float sRange; 
		bool sIsTwoSize;

		auto loadData = static_cast<JRectLight::LoadData*>(data);
		JFileIOTool& tool = loadData->tool;
		JUserPtr<JGameObject> owner = loadData->owner;

		JObjectFileIOHelper::LoadComponentIden(tool, guid, flag, isActivated);
		auto idenUser = lPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JRectLight::InitData>(guid, flag, owner), &lPrivate);
		JUserPtr<JRectLight> litUser;
		litUser.ConnnectChild(idenUser);

		JLightPrivate::AssetDataIOInterface::LoadLightData(tool, litUser);
		JObjectFileIOHelper::LoadVector2(tool, sAreaSize, "AreaSize:");
		JObjectFileIOHelper::LoadAtomicData(tool, sPower, "Power:");
		JObjectFileIOHelper::LoadAtomicData(tool, sRange, "Range:"); 
		JObjectFileIOHelper::LoadAtomicData(tool, sIsTwoSize, "IsTwoSize:");
		JUserPtr<JTexture> sSourceTexture = JObjectFileIOHelper::_LoadHasIden<JTexture>(tool, "SourceTexture:");

		litUser->SetAreaSize(sAreaSize);
		litUser->SetPower(sPower);
		litUser->SetRange(sRange); 
		litUser->SetTwoSide(sIsTwoSize);
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
		JObjectFileIOHelper::StoreAtomicData(tool, impl->power, "Power:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->range, "Range:"); 
		JObjectFileIOHelper::StoreAtomicData(tool, impl->isTwoSide, "IsTwoSize:");
		JObjectFileIOHelper::_StoreHasIden(tool, impl->sourceTexture.Get(), "SourceTexture:");

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	bool FrameUpdateInterface::UpdateStart(JLight* lit, const bool isUpdateForced)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::RECT)
			return false;

		JRectLight* dLit = static_cast<JRectLight*>(lit);
		if (isUpdateForced)
			dLit->impl->SetFrameDirty();

		dLit->impl->SetLastFrameUpdatedTrigger(false);
		dLit->impl->SetLastFrameHotUpdatedTrigger(false);
		if (dLit->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources)
			dLit->impl->UpdateLightTransform();
		return dLit->impl->IsFrameDirted();
	}
	void FrameUpdateInterface::UpdateFrame(JRectLight* lit, Graphic::JRectLightConstants& constant)noexcept
	{
		lit->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateFrame(JRectLight* lit, Graphic::JShadowMapDrawConstants& constant)noexcept
	{
		lit->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateEnd(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::RECT)
			return;

		JRectLight* dLit = static_cast<JRectLight*>(lit);
		if (dLit->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources)
			dLit->impl->SetLastFrameHotUpdatedTrigger(true);
		dLit->impl->SetLastFrameUpdatedTrigger(true);
		dLit->impl->UpdateFrameEnd();
	}
	int FrameUpdateInterface::GetLitFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::RECT)
			return -1;

		return static_cast<JRectLight*>(lit)->impl->RectLitFrame::GetFrameIndex();
	}
	int FrameUpdateInterface::GetShadowMapFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::RECT)
			return -1;

		return static_cast<JRectLight*>(lit)->impl->ShadowMapDrawFrame::GetFrameIndex();
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
		if (lit->GetLightType() != J_LIGHT_TYPE::RECT)
			return false;

		return static_cast<JRectLight*>(lit)->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources;
	}
	bool FrameUpdateInterface::IsLastFrameHotUpdated(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::RECT)
			return false;

		return static_cast<JRectLight*>(lit)->impl->IsLastFrameHotUpdated();
	}
	bool FrameUpdateInterface::IsLastUpdated(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::RECT)
			return false;

		return static_cast<JRectLight*>(lit)->impl->IsLastFrameUpdated();
	}
	bool FrameUpdateInterface::HasLitRecopyRequest(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::RECT)
			return false;

		return static_cast<JRectLight*>(lit)->impl->ShadowMapDrawFrame::HasMovedDirty();
	}
	bool FrameUpdateInterface::HasShadowMapRecopyRequest(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::RECT)
			return false;

		return static_cast<JRectLight*>(lit)->impl->ShadowMapDrawFrame::HasMovedDirty();
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
		if (lit->GetLightType() != J_LIGHT_TYPE::RECT)
			return -1;

		return static_cast<JRectLight*>(lit)->impl->RectLitFrame::GetFrameIndex();
	}
	int FrameIndexInterface::GetShadowMapFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::RECT)
			return -1;

		return static_cast<JRectLight*>(lit)->impl->ShadowMapDrawFrame::GetFrameIndex();
	}
	int FrameIndexInterface::GetDepthTestPassFrameIndex(JLight* lit)noexcept
	{
		return -1;
	}
	int FrameIndexInterface::GetHzbOccComputeFrameIndex(JLight* lit)noexcept
	{
		return -1;
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
	JLightPrivate::FrameUpdateInterface& JRectLightPrivate::GetFrameUpdateInterface()const noexcept
	{
		static FrameUpdateInterface pI;
		return pI;
	}
	JLightPrivate::FrameIndexInterface& JRectLightPrivate::GetFrameIndexInterface()const noexcept
	{
		static FrameIndexInterface pI;
		return pI;
	}
}