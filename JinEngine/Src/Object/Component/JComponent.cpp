#include"JComponent.h"
#include"JComponentFactory.h"
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
	JGameObject* JComponent::GetOwner()const noexcept
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
	bool JComponent::Destroy(const bool isForced)
	{
		if(HasFlag(J_OBJECT_FLAG::OBJECT_FLAG_UNDESTROYABLE) && !isForced)
			return false;

		if (IsActivated())
			DeActivate(); 
		return true;
	}
	bool JComponent::RegisterCashData()noexcept
	{
		//For editor redo undo
		if (owner == nullptr && ownerInfo != nullptr)
			owner = static_cast<JGameObject*>(Core::GetRawPtr(*ownerInfo));
		return static_cast<Core::JTypeCashInterface<JComponent>*>(owner)->AddType(this);
	}
	bool JComponent::DeRegisterCashData()noexcept
	{
		ownerInfo = std::make_unique<Core::JTypeInstanceSearchHint>(Core::GetUserPtr(owner));
		return static_cast<Core::JTypeCashInterface<JComponent>*>(owner)->RemoveType(this);
	}
	JComponent::JComponent(const std::string& cTypeName, const size_t guid, J_OBJECT_FLAG flag, JGameObject* owner)noexcept
		:JComponentInterface(JCUtil::StrToWstr(cTypeName), guid, flag), owner(owner)
	{ }
	JComponent::~JComponent()
	{}
}
