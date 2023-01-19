#include"JBehavior.h" 
#include"../JComponentFactory.h"
#include"../../GameObject/JGameObject.h" 
#include"../../../Core/Guid/GuidCreator.h"

namespace JinEngine
{
	static auto isAvailableoverlapLam = [](){return true;}; 

	J_COMPONENT_TYPE JBehavior::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	bool JBehavior::IsAvailableOverlap()const noexcept
	{
		return isAvailableoverlapLam();
	}
	bool JBehavior::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection())
			return true;
		else
			return false;
	}
	void JBehavior::DoCopy(JObject* ori)
	{
		//�̱���
	}
	void JBehavior::DoActivate()noexcept
	{
		JComponent::DoActivate();
		RegisterComponent();
	}
	void JBehavior::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		DeRegisterComponent();
	}
	Core::J_FILE_IO_RESULT JBehavior::CallStoreComponent(std::wofstream& stream)
	{
		return StoreObject(stream, this);
	}
	Core::J_FILE_IO_RESULT JBehavior::StoreObject(std::wofstream& stream, JBehavior* behavior)
	{
		//�߰��ʿ�
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JBehavior* JBehavior::LoadObject(std::wifstream& stream, JGameObject* owner)
	{
		//�߰��ʿ�
		return nullptr;
	}
	void JBehavior::RegisterJFunc()
	{
		auto defaultC = [](JGameObject* owner) -> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JBehavior>(Core::MakeGuid(), owner->GetFlag(), owner);
			JBehavior* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
				return newComp;
			else
				return nullptr;
		};
		auto initC = [](const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)-> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JBehavior>(guid, Core::AddSQValueEnum(owner->GetFlag(), objFlag), owner);
			JBehavior* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
				return newComp;
			else
				return nullptr;
		};
		auto loadC = [](std::wifstream& stream, JGameObject* owner) -> JComponent*
		{
			return LoadObject(stream, owner);
		};
		auto copyC = [](JComponent* ori, JGameObject* owner) -> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JBehavior>(Core::MakeGuid(), ori->GetFlag(), owner);
			JBehavior* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				if (newComp->Copy(ori))
					return newComp;
				else
				{
					BegineForcedDestroy(newComp);
					return nullptr;
				}
			}
			else
				return nullptr;
		};
		JCFI<JBehavior>::Register(defaultC, initC, loadC, copyC);

		static GetTypeNameCallable getTypeNameCallable{ &JBehavior::TypeName };
		static GetTypeInfoCallable getTypeInfoCallable{ &JBehavior::StaticTypeInfo };
		bool(*ptr)() = isAvailableoverlapLam;
		static IsAvailableOverlapCallable isAvailableOverlapCallable{ ptr };

		static JCI::CTypeHint cTypeHint{ GetStaticComponentType(), false };
		static JCI::CTypeCommonFunc cTypeCommonFunc{getTypeNameCallable, getTypeInfoCallable, isAvailableOverlapCallable };

		JCI::RegisterTypeInfo(cTypeHint, cTypeCommonFunc, JCI::CTypeInterfaceFunc{});
	}
	JBehavior::JBehavior(const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)
		:JComponent(TypeName(), guid, objFlag, owner)
	{}
	JBehavior::~JBehavior() {}
}