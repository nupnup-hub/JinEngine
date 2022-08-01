#include"JComponent.h"
#include"../GameObject/JGameObject.h"
#include"../../Utility/JCommonUtility.h"
#include<fstream>

namespace JinEngine
{
	JComponent::JComponent(const std::string& classTypeName, const size_t guid, JOBJECT_FLAG flag, JGameObject* owner)noexcept
		:JComponentInterface(classTypeName, guid, flag), owner(owner)
	{ 
		gameObjectComponentEvent = owner; 
	}
	JComponent::~JComponent()
	{
		owner = nullptr;
		gameObjectComponentEvent = nullptr;
	}
	std::string JComponent::GetOwnerName()const noexcept
	{
		return owner->GetName();
	}
	size_t JComponent::GetOwnerGuid()const noexcept
	{ 
		return owner->GetGuid();
	}
	J_OBJECT_TYPE JComponent::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::COMPONENT;
	}
	bool JComponent::HasOwner()const noexcept
	{
		return owner != nullptr;
	}
	bool JComponent::PassDefectInspection()const noexcept
	{
		return HasOwner();
	}
	JGameObject* JComponent::GetOwner()noexcept
	{
		return owner;
	}
	IGameObjectComponentEvent* JComponent::GetOwnerInterface()
	{
		return gameObjectComponentEvent;
	}
	void JComponent::DoActivate()noexcept
	{
		JObject::DoActivate();  
	}
	void JComponent::DoDeActivate()noexcept
	{
		JObject::DoDeActivate();  
	}
}
