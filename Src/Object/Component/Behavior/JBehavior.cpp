#include"JBehavior.h" 
#include"../JComponentFactory.h"
#include"../../GameObject/JGameObject.h" 
#include"../../../Core/Guid/GuidCreator.h"

namespace JinEngine
{
	static auto isAvailableoverlapLam = [](){return true;};
	static auto componentTypeLam = [](){return J_COMPONENT_TYPE::USER_DEFIENED_BEHAVIOR; };

	J_COMPONENT_TYPE JBehavior::GetComponentType()const noexcept
	{
		return componentTypeLam();
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
	bool JBehavior::Copy(JObject* ori)
	{
		if (ori->HasFlag(OBJECT_FLAG_UNCOPYABLE) || ori->GetGuid() == GetGuid())
			return false;
		return true;
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
		//추가필요
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JBehavior* JBehavior::LoadObject(std::wifstream& stream, JGameObject* owner)
	{
		//추가필요
		return nullptr;
	}
	void JBehavior::RegisterJFunc()
	{
		auto defaultC = [](JGameObject* owner) -> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JBehavior>(Core::MakeGuid(), OBJECT_FLAG_NONE, owner);
			JComponent* ret = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			return ret;
		};
		auto initC = [](const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)-> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JBehavior>(guid, objFlag, owner);
			JComponent* ret = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			return ret;
		};
		auto loadC = [](std::wifstream& stream, JGameObject* owner) -> JComponent*
		{
			return LoadObject(stream, owner);
		};
		auto copyC = [](JComponent* ori, JGameObject* owner) -> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JBehavior>(Core::MakeGuid(), ori->GetFlag(), owner);
			JBehavior* newB = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			newB->Copy(ori); 		 
			return newB;
		};
		JCFI<JBehavior>::Regist(defaultC, initC, loadC, copyC);

		static GetTypeNameCallable getTypeNameCallable{ &JBehavior::TypeName };
		static GetTypeInfoCallable getTypeInfoCallable{ &JBehavior::StaticTypeInfo };
		bool(*ptr)() = isAvailableoverlapLam;
		static IsAvailableOverlapCallable isAvailableOverlapCallable{ ptr };

		static JCI::CTypeHint cTypeHint{ componentTypeLam(), false };
		static JCI::CTypeCommonFunc cTypeCommonFunc{getTypeNameCallable, getTypeInfoCallable, isAvailableOverlapCallable };

		JCI::RegisterTypeInfo(cTypeHint, cTypeCommonFunc, JCI::CTypeInterfaceFunc{});
	}
	JBehavior::JBehavior(const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)
		:JComponent(TypeName(), guid, objFlag, owner)
	{}
	JBehavior::~JBehavior() {}
}