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


#include"JLight.h"   
#include"JLightPrivate.h"
#include"../Transform/JTransform.h"
#include"../Transform/JTransformPrivate.h"
#include"../JComponentHint.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h" 
#include"../../Resource/Scene/JScenePrivate.h"
#include"../../JObjectFileIOHelper.h"
#include"../../../Core/Guid/JGuidCreator.h" 
#include"../../../Core/File/JFileConstant.h" 
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Math/JMathHelper.h"
#include"../../../Graphic/JGraphic.h"  
#include"../../../Graphic/Frameresource/JLightConstants.h"    
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
		static auto isAvailableoverlapLam = []() {return false; };  

		static constexpr float minPenumbraWidth = 0;
		static constexpr float maxPenumbraWidth = 100000.0f; //Constants::lightMaxDistance * 0.5f;

		static constexpr float minPenumbraNearPlane = 0;
		static constexpr float maxPenumbraNearPlane = 100000.0f;

		static constexpr float minPower = 0.1f;
		static constexpr float maxPower = 16.0f;
	}
	 
	class JLight::JLightImpl : public Core::JTypeImplBase,
		public Graphic::JFrameDirtyChain<Graphic::JFrameDirtyTrigger>
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JLightImpl)
	public:
		JWeakPtr<JLight> thisPointer;
	public:
		REGISTER_PROPERTY_EX(color, GetColor, SetColor, GUI_COLOR_PICKER(false))
		JVector3<float> color = { 0.8f, 0.8f, 0.8f };
	public:
		REGISTER_PROPERTY_EX(power, GetPower, SetPower, GUI_SLIDER(minPower, maxPower, true, false))
		float power = 1.0f;
	public:
		//type별 크기제한 필요
		REGISTER_PROPERTY_EX(shadowResolution, GetShadowResolution, TryCallSetShadowResolution, GUI_ENUM_COMBO(J_SHADOW_RESOLUTION, "-a {v} x {v};"))
		J_SHADOW_RESOLUTION shadowResolution = J_SHADOW_RESOLUTION::MEDIUM;
	public:
		REGISTER_PROPERTY_EX(bias, GetBias, SetBias, GUI_SLIDER(-1.0f, 1.0f, true, false, 5))
		float bias = 0.0f;
		REGISTER_PROPERTY_EX(penumbraWidth, GetPenumbraWidth, SetPenumbraWidth, GUI_SLIDER(minPenumbraWidth, maxPenumbraWidth, true, false))
		float penumbraWidth = 1.0f;
		REGISTER_PROPERTY_EX(penumbraBlockerScale, GetPenumbraBlockerWidth, SetPenumbraBlockerWidth, GUI_SLIDER(minPenumbraWidth, maxPenumbraWidth, true, false))
		float penumbraBlockerScale = 1.0f;
	public:
		REGISTER_PROPERTY_EX(onShadow, IsShadowActivated, TryCallSetShadow, GUI_CHECKBOX())
		bool onShadow = false;
		//managed by light type 
		REGISTER_PROPERTY_EX(allowDisplayShadowMap, AllowDisplayShadowMap, TryCallSetAllowDisplayShadowMap, GUI_CHECKBOX())
		bool allowDisplayShadowMap = false;
	public:
		JLightImpl(const InitData& initData, JLight* thisLitRaw)
		{}
		~JLightImpl()
		{}
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
		float GetPower()const noexcept
		{
			return power;
		}
		float GetBias()const noexcept
		{
			return bias;
		}
		float GetPenumbraWidth()const noexcept
		{
			return penumbraWidth;
		}
		float GetPenumbraBlockerWidth()const noexcept
		{
			return penumbraBlockerScale;
		}
	public: 
		void SetColor(const JVector3<float>& newColor)noexcept
		{ 
			color = JVector3F::Clamp(newColor, JVector3F::Zero(), JVector3F::PositiveOne());
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
		void SetPower(const float value)noexcept
		{
			power = std::clamp(value, thisPointer->GetMinPower(), thisPointer->GetMaxPower());
			SetFrameDirty();
		}
		void SetBias(const float value)noexcept
		{
			bias = std::clamp(value, -1.0f, 1.0f);
			SetFrameDirty();
		}
		void SetPenumbraWidth(const float value)noexcept
		{
			penumbraWidth = std::clamp(value, minPenumbraWidth, maxPenumbraWidth);
			SetFrameDirty();
		}
		void SetPenumbraBlockerWidth(const float value)noexcept
		{
			penumbraBlockerScale = std::clamp(value, minPenumbraWidth, maxPenumbraWidth);
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
		void UpdateLightShape()const noexcept
		{
			JScenePrivate::CompSettingInterface::UpdateTransform(thisPointer);
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
		{ 
		}
	public:
		static bool DoCopy(JLight* from, JLight* to)
		{ 
			from->SetColor(to->GetColor()); 	
			from->SetShadowResolution(to->GetShadowResolutionType());
			from->SetAllowDisplayShadowMap(to->AllowDisplayShadowMap());
			from->SetShadow(to->IsShadowActivated());
			from->SetPower(to->GetPower());
			from->SetBias(to->GetBias());

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
	uint JLight::GetShadowResolution()const noexcept
	{
		return (uint)impl->GetShadowResolution();
	} 
	J_SHADOW_RESOLUTION JLight::GetShadowResolutionType()const noexcept
	{
		return impl->GetShadowResolution();
	}
	uint JLight::GetShadowMapSize()const noexcept
	{
		return impl->GetShadowMapSize();
	}
	float JLight::GetPower()const noexcept
	{
		return impl->GetPower();
	}
	float JLight::GetBias()const noexcept
	{
		return impl->GetBias();
	}
	float JLight::GetPenumbraWidth()const noexcept
	{
		return impl->GetPenumbraWidth();
	}
	float JLight::GetPenumbraBlockerWidth()const noexcept
	{
		return impl->GetPenumbraBlockerWidth();
	}
	JComponent::UserCompComparePtr JLight::GetLitTypeComparePtr()const noexcept
	{
		return [](const JUserPtr<JComponent>& a, const JUserPtr<JComponent>& b)
		{
			return (int)static_cast<JLight*>(a.Get())->GetLightType() < (int)static_cast<JLight*>(b.Get())->GetLightType();
		};
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
	void JLight::SetPower(const float value)noexcept
	{
		impl->SetPower(value);
	}
	void JLight::SetBias(const float value)noexcept
	{
		impl->SetBias(value);
	}
	void JLight::SetPenumbraWidth(const float value)noexcept
	{
		impl->SetPenumbraWidth(value);
	}
	void JLight::SetPenumbraBlockerWidth(const float value)noexcept
	{
		impl->SetPenumbraBlockerWidth(value);
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
	bool JLight::AllowLightCulling()const noexcept
	{
		return false;
	}
	void JLight::DoActivate()noexcept
	{
		JComponent::DoActivate(); 
		impl->Activate(); 
	}
	void JLight::DoDeActivate()noexcept
	{ 
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
	using ChildInterface = JLightPrivate::ChildInterface;
	 
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

	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadLightData(JFileIOTool& tool, JUserPtr<JLight> user)
	{
		if (!tool.CanLoad())
			return 	Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide; 
		JVector3<float> sColor;
		J_SHADOW_RESOLUTION sShadowResolutionType;
		float sPower = 0;
		float sBias = 0;
		float sPenumbraWidth = 0;
		float sPenumbraBlockerScale = 0;
		bool sOnShadow = 0;
		bool sAllowDisplayShadowMap = 0;

		JObjectFileIOHelper::LoadVector3(tool, sColor, "Color:");
		JObjectFileIOHelper::LoadEnumData(tool, sShadowResolutionType, "ShadowResolution:");
		JObjectFileIOHelper::LoadAtomicData(tool, sPower, "Power:");
		JObjectFileIOHelper::LoadAtomicData(tool, sBias, "Bias:");
		JObjectFileIOHelper::LoadAtomicData(tool, sPenumbraWidth, "PenumbraWidth:");
		JObjectFileIOHelper::LoadAtomicData(tool, sPenumbraBlockerScale, "PenumbraBlockerScale:");
		JObjectFileIOHelper::LoadAtomicData(tool, sOnShadow, "OnShadow:");
		JObjectFileIOHelper::LoadAtomicData(tool, sAllowDisplayShadowMap, "AllowDisplayShadowMap:");

		user->SetColor(sColor);
		user->SetShadowResolution(sShadowResolutionType);
		user->SetPower(sPower);
		user->SetBias(sBias);
		user->SetPenumbraWidth(sPenumbraWidth);
		user->SetPenumbraBlockerWidth(sPenumbraBlockerScale);
		user->SetAllowDisplayShadowMap(sAllowDisplayShadowMap);
		user->SetShadow(sOnShadow);
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreLightData(JFileIOTool& tool, const JUserPtr<JLight>& user)
	{
		if (!tool.CanStore())
			return 	Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
		 
		JObjectFileIOHelper::StoreVector3(tool, user->GetColor(), "Color:");
		JObjectFileIOHelper::StoreEnumData(tool, user->GetShadowResolutionType(), "ShadowResolution:");
		JObjectFileIOHelper::StoreAtomicData(tool, user->GetPower(), "Power:");
		JObjectFileIOHelper::StoreAtomicData(tool, user->GetBias(), "Bias:");
		JObjectFileIOHelper::StoreAtomicData(tool, user->GetPenumbraWidth(), "PenumbraWidth:");
		JObjectFileIOHelper::StoreAtomicData(tool, user->GetPenumbraBlockerWidth(), "PenumbraBlockerScale:");
		JObjectFileIOHelper::StoreAtomicData(tool, user->IsShadowActivated(), "OnShadow:");
		JObjectFileIOHelper::StoreAtomicData(tool, user->AllowDisplayShadowMap(), "AllowDisplayShadowMap:");
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	void ChildInterface::RegisterFrameDirtyListener(JLight* lit, Graphic::JFrameDirty* listener, const size_t guid)noexcept
	{ 
		lit->impl->RegisterFrameDirtyListener(listener, guid);
	}
	void ChildInterface::DeRegisterFrameDirtyListener(JLight* lit, const size_t guid)noexcept
	{
		lit->impl->DeRegisterFrameDirtyListener(guid);
	}
	void ChildInterface::UpdateLightShape(const JUserPtr<JLight>& lit)noexcept
	{
		lit->impl->UpdateLightShape();
	}
	 
	Core::JIdentifierPrivate::DestroyInstanceInterface& JLightPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
}