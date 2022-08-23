#include"JComponent.h"
#include"../GameObject/JGameObject.h"
#include"../Resource/Scene/JScene.h"
#include"../../Utility/JCommonUtility.h"
#include<fstream>

namespace JinEngine
{
	J_OBJECT_TYPE JComponent::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::COMPONENT_OBJECT;
	}
	JGameObject* JComponent::GetOwner()noexcept
	{
		return owner;
	}
	bool JComponent::PassDefectInspection()const noexcept
	{
		return owner != nullptr;
	}
	bool JComponent::RegisterComponent()noexcept
	{
		return owner->GetOwnerScene()->RegisterInterface()->RegisterComponent(*this);
	}
	bool JComponent::DeRegisterComponent()noexcept
	{
		return owner->GetOwnerScene()->RegisterInterface()->DeRegisterComponent(*this);
	}
	bool JComponent::ReRegisterComponent()noexcept
	{
		owner->GetOwnerScene()->RegisterInterface()->DeRegisterComponent(*this);
		return owner->GetOwnerScene()->RegisterInterface()->RegisterComponent(*this);
	}
	void JComponent::DoActivate()noexcept
	{
		JObject::DoActivate();
	}
	void JComponent::DoDeActivate()noexcept
	{
		JObject::DoDeActivate();
	}
	void JComponent::Destroy()
	{
		if(HasFlag(J_OBJECT_FLAG::OBJECT_FLAG_UNDESTROYABLE) && !IsIgnoreUndestroyableFlag())
			return;

		DeActivate();
		owner->CompInterface()->RemoveComponent(*this);
		delete this;
	}
	JComponent::JComponent(const std::string& cTypeName, const size_t guid, J_OBJECT_FLAG flag, JGameObject* owner)noexcept
		:JComponentInterface(cTypeName, guid, flag), owner(owner)
	{
	}
	JComponent::~JComponent()
	{
		owner = nullptr;
	}
}
