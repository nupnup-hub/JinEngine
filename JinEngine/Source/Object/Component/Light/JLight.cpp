#include"JLight.h"   
#include"JLightPrivate.h"
#include"../Transform/JTransform.h"
#include"../Transform/JTransformPrivate.h"
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
#include"../../../Graphic/Frameresource/JOcclusionConstants.h"
#include"../../../Graphic/Frameresource/JDepthTestConstants.h" 
#include"../../../Graphic/Frameresource/JFrameUpdate.h"
#include"../../../Graphic/Culling/JCullingInterface.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../../Graphic/JGraphicDrawListInterface.h" 
#include"../../../Graphic/ShadowMap/JCsmHandlerInterface.h" 
#include<Windows.h>
#include<fstream>

using namespace DirectX;
namespace JinEngine
{
	namespace
	{
		static auto isAvailableoverlapLam = []() {return true; }; 
		 
		static JComponent::UserCompComparePtr GetLitTypeComparePtr()
		{
			return [](const JUserPtr<JComponent>& a, const JUserPtr<JComponent>& b)
			{
				return (int)static_cast<JLight*>(a.Get())->GetLightType() < (int)static_cast<JLight*>(b.Get())->GetLightType();
			};
		} 
	}
	 
	class JLight::JLightImpl : public Core::JTypeImplBase,
		public Graphic::JFrameDirtyChain<Graphic::JFrameDirtyTrigger>
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JLightImpl)
	public:
		JWeakPtr<JLight> thisPointer;
	public:  
	public:
		REGISTER_PROPERTY_EX(color, GetColor, SetColor, GUI_COLOR_PICKER(false))
		JVector3<float> color = { 0.8f, 0.8f, 0.8f };
	public:
		//type별 크기제한 필요
		REGISTER_PROPERTY_EX(shadowResolution, GetShadowResolution, TryCallSetShadowResolution, GUI_ENUM_COMBO(J_SHADOW_RESOLUTION, "-a (v\" X \"v)"))
		J_SHADOW_RESOLUTION shadowResolution = J_SHADOW_RESOLUTION::MEDIUM;
	public:
		REGISTER_PROPERTY_EX(onShadow, IsShadowActivated, TryCallSetShadow, GUI_CHECKBOX())
		bool onShadow = false;
	public:
		//managed by light type 
		REGISTER_PROPERTY_EX(allowDisplayShadowMap, AllowDisplayShadowMap, TryCallSetAllowDisplayShadowMap, GUI_CHECKBOX())
		bool allowDisplayShadowMap = false;
	public:
		JLightImpl(const InitData& initData, JLight* thisLitRaw)
		{}
		~JLightImpl()
		{ }
	public: 
		J_SHADOW_RESOLUTION GetShadowResolution()const noexcept
		{
			return shadowResolution;
		} 
		uint GetShadowMapSize()const noexcept
		{
			return (uint)shadowResolution;
		}
		JVector3<float> GetColor()const noexcept
		{
			return color;
		}
	public: 
		void SetColor(const JVector3<float>& newColor)noexcept
		{
			color = newColor;
			SetFrameDirty();
		}
		void SetShadow(const bool value)noexcept
		{
			onShadow = value;
			SetFrameDirty();
		}
		void SetShadowResolution(const J_SHADOW_RESOLUTION newShadowResolution)noexcept
		{
			shadowResolution = newShadowResolution;
			SetFrameDirty();
		}
		void SetAllowDisplayShadowMap(const bool value)noexcept
		{
			allowDisplayShadowMap = value;
			SetFrameDirty();
		} 
	public:
		bool IsShadowActivated()const noexcept
		{
			return onShadow;
		}
		bool AllowDisplayShadowMap()const noexcept
		{
			return allowDisplayShadowMap;	
		}
	public:
		//for call child class set func by gui
		void TryCallSetShadow(const bool value)noexcept
		{
			thisPointer->SetShadow(value);
		}
		void TryCallSetShadowResolution(const J_SHADOW_RESOLUTION newShadowResolution)noexcept
		{
			thisPointer->SetShadowResolution(newShadowResolution);
		}
		void TryCallSetAllowDisplayShadowMap(const bool value)noexcept
		{
			thisPointer->SetAllowDisplayShadowMap(value);
		}
	public:
		void Activate()noexcept
		{ 
			SetFrameDirty();
		}
		void DeActivate()noexcept
		{}
	public:
		static bool DoCopy(JLight* from, JLight* to)
		{ 
			from->SetColor(to->GetColor()); 	
			from->SetShadowResolution(to->GetShadowResolution());
			from->SetAllowDisplayShadowMap(to->AllowDisplayShadowMap());
			from->SetShadow(to->IsShadowActivated());

			to->impl->SetFrameDirty();
			return true;
		}
	public:
		void NotifyReAlloc()
		{
			auto transform = thisPointer->GetOwner()->GetTransform();
			if (transform.IsValid())
			{
				JTransformPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(transform.Get(), thisPointer->GetGuid());
				JTransformPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(transform.Get(), this, thisPointer->GetGuid());
			}
		}
	public:
		void RegisterThisPointer(JLight* lit)
		{
			thisPointer = Core::GetWeakPtr(lit);
		}
		void RegisterPostCreation()
		{
			JTransformPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(thisPointer->GetOwner()->GetTransform().Get(), this, thisPointer->GetGuid());
		} 
		void DeRegisterPreDestruction()
		{
			if (thisPointer->GetOwner()->GetTransform() != nullptr)
				JTransformPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(thisPointer->GetOwner()->GetTransform().Get(), thisPointer->GetGuid());
		}
		void RegisterFrameDirtyListener(Graphic::JFrameDirtyTriggerBase* newListener, const size_t guid)
		{
			AddFrameDirtyListener(newListener, guid);
		}
		void DeRegisterFrameDirtyListener(const size_t guid)
		{
			RemoveFrameDirtyListener(guid);
		}
		static void RegisterTypeData()
		{
			static GetCTypeInfoCallable getTypeInfoCallable{ &JLight::StaticTypeInfo };
			static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };
			using InitUnq = std::unique_ptr<Core::JDITypeDataBase>;
			auto createInitDataLam = [](const Core::JTypeInfo& typeInfo, JUserPtr<JGameObject> parent, InitUnq&& parentClassInitData) -> InitUnq
			{
				using CorrectType = JComponent::ParentType::InitData;
				const bool isValidType = typeInfo.IsChildOf<JLight>() && !typeInfo.IsA<JLight>();
				const bool isValidUnq = parentClassInitData != nullptr && parentClassInitData->GetTypeInfo().IsChildOf(CorrectType::StaticTypeInfo());
				if(!isValidType)
					return std::make_unique<JLight::InitData>(typeInfo, nullptr);

				if (isValidUnq)
				{
					CorrectType* ptr = static_cast<CorrectType*>(parentClassInitData.get());
					return std::make_unique<JLight::InitData>(typeInfo, ptr->guid, ptr->flag, parent);
				}
				else
					return std::make_unique<JLight::InitData>(typeInfo, parent);
			};
			static CreateInitDataCallable createInitDataCallable{ createInitDataLam };

			static auto setFrameLam = [](JComponent* comp) {static_cast<JLight*>(comp)->impl->SetFrameDirty(); };
			static SetCFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };

			static CTypeHint cTypeHint{ GetStaticComponentType(), true };
			static CTypeCommonFunc cTypeCommonFunc{ getTypeInfoCallable,isAvailableOverlapCallable, createInitDataCallable };
			static CTypePrivateFunc cTypeInterfaceFunc{ &setFrameDirtyCallable };

			RegisterCTypeInfo(JLight::StaticTypeInfo(), cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);

			IMPL_REALLOC_BIND(JLight::JLightImpl, thisPointer)
		}
	};

	JLight::InitData::InitData(const Core::JTypeInfo& typeInfo, const JUserPtr<JGameObject>& owner)
		:JComponent::InitData(typeInfo, owner)
	{}
	JLight::InitData::InitData(const Core::JTypeInfo& typeInfo, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner)
		: JComponent::InitData(typeInfo, GetDefaultName(typeInfo), guid, flag, owner)
	{}
	  
	J_COMPONENT_TYPE JLight::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	} 
	JVector3<float> JLight::GetColor()const noexcept
	{
		return impl->GetColor();
	}
	J_SHADOW_RESOLUTION JLight::GetShadowResolution()const noexcept
	{
		return impl->GetShadowResolution();
	} 
	uint JLight::GetShadowMapSize()const noexcept
	{
		return impl->GetShadowMapSize();
	}
	void JLight::SetColor(const JVector3<float>& color)noexcept
	{
		impl->SetColor(color);
	}
	void JLight::SetShadow(const bool value)noexcept
	{
		impl->SetShadow(value);
	} 
	void JLight::SetShadowResolution(const J_SHADOW_RESOLUTION sQuality)noexcept
	{
		impl->SetShadowResolution(sQuality);
	}
	void JLight::SetAllowDisplayShadowMap(const bool value)noexcept
	{
		impl->SetAllowDisplayShadowMap(value);
	}
	bool JLight::IsShadowActivated()const noexcept
	{
		return impl->IsShadowActivated();
	} 
	bool JLight::IsAvailableOverlap()const noexcept
	{
		return isAvailableoverlapLam();
	}  
	bool JLight::AllowDisplayShadowMap()const noexcept
	{
		return impl->AllowDisplayShadowMap();
	}
	void JLight::DoActivate()noexcept
	{
		JComponent::DoActivate();
		RegisterComponent(impl->thisPointer, GetLitTypeComparePtr());
		impl->Activate(); 
	}
	void JLight::DoDeActivate()noexcept
	{
		DeRegisterComponent(impl->thisPointer);
		impl->DeActivate(); 
		JComponent::DoDeActivate();
	}
	JLight::JLight(const InitData& initData)
		:JComponent(initData), impl(std::make_unique<JLightImpl>(initData, this))
	{ }
	JLight::~JLight()
	{
		impl.reset();
	}


	using CreateInstanceInterface = JLightPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JLightPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JLightPrivate::AssetDataIOInterface; 
	using FrameIndexInterface = JLightPrivate::FrameIndexInterface;
	using FrameDirtyInterface = JLightPrivate::FrameDirtyInterface;
	 
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JComponentPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JLight* lit = static_cast<JLight*>(createdPtr);
		lit->impl->RegisterThisPointer(lit); 
		lit->impl->RegisterPostCreation();
	} 
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsChildOf(JLight::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JLight::JLightImpl::DoCopy(static_cast<JLight*>(from.Get()), static_cast<JLight*>(to.Get()));
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		static_cast<JLight*>(ptr)->impl->DeRegisterPreDestruction();
		JComponentPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadLightData(std::wifstream& stream, JUserPtr<JLight> user)
	{
		if (!stream.is_open())
			return 	Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide; 
		J_SHADOW_RESOLUTION sShadowResolutionType;
		JVector3<float> sColor;
		bool sOnShadow;
		bool sAllowDisplayShadowMap;
		  
		JFileIOHelper::LoadVector3(stream, sColor);
		JFileIOHelper::LoadEnumData(stream, sShadowResolutionType);
		JFileIOHelper::LoadAtomicData(stream, sOnShadow);
		JFileIOHelper::LoadAtomicData(stream, sAllowDisplayShadowMap);
   
		user->SetColor(sColor);
		user->SetShadowResolution(sShadowResolutionType);
		user->SetAllowDisplayShadowMap(sAllowDisplayShadowMap);
		user->SetShadow(sOnShadow);
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreLightData(std::wofstream& stream, const JUserPtr<JLight>& user)
	{
		if (!stream.is_open())
			return 	Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
		 
		JFileIOHelper::StoreVector3(stream, L"Color:", user->GetColor());
		JFileIOHelper::StoreEnumData(stream, L"ShadowResolution: ", user->GetShadowResolution());
		JFileIOHelper::StoreAtomicData(stream, L"OnShadow:", user->IsShadowActivated()); 
		JFileIOHelper::StoreAtomicData(stream, L"AllowDisplayShadowMap:", user->AllowDisplayShadowMap());
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
 
	void FrameDirtyInterface::RegisterFrameDirtyListener(JLight* lit, Graphic::JFrameDirty* listener, const size_t guid)noexcept
	{
		lit->impl->RegisterFrameDirtyListener(listener, guid);
	}
	void FrameDirtyInterface::DeRegisterFrameDirtyListener(JLight* lit, const size_t guid)noexcept
	{
		lit->impl->DeRegisterFrameDirtyListener(guid);
	}
	 
	Core::JIdentifierPrivate::DestroyInstanceInterface& JLightPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
}