#include"JBehavior.h" 
#include"../JComponentFactory.h"
#include"../../GameObject/JGameObject.h" 
#include"../../../Core/Guid/GuidCreator.h"

namespace JinEngine
{
	J_COMPONENT_TYPE JBehavior::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	J_COMPONENT_TYPE JBehavior::GetStaticComponentType()noexcept
	{
		return J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT;
	}

	bool JBehavior::IsAvailableOverlap()const noexcept
	{
		return true;
	}
	bool JBehavior::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection())
			return true;
		else
			return false;
	}
	void JBehavior::DoActivate()noexcept
	{
		JComponent::DoActivate();
		RegisterComponent<JBehavior>(this);
	}
	void JBehavior::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		DeRegisterComponent<JBehavior>(this);
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
	void JBehavior::RegisterFunc()
	{
		auto defaultC = [](JGameObject* owner) -> JComponent*
		{
			return new JBehavior(Core::MakeGuid(), OBJECT_FLAG_NONE, owner);
		};
		auto initC = [](const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner)-> JComponent*
		{
			return new JBehavior(guid, objFlag, owner);
		};
		auto loadC = [](std::wifstream& stream, JGameObject* owner) -> JComponent*
		{
			return LoadObject(stream, owner);
		};
		auto copyC = [](JComponent* oriC, JGameObject* owner) -> JComponent*
		{
			JBehavior* oriB = static_cast<JBehavior*>(oriC);
			JBehavior* newB = new JBehavior(Core::MakeGuid(), oriB->GetFlag(), owner);
			 
			return newB;
		};
		JCFI<JBehavior>::Regist(defaultC, initC, loadC, copyC);
	}
	JBehavior::JBehavior(const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner)
		:JComponent(TypeName(), guid, objFlag, owner)
	{}
	JBehavior::~JBehavior() {}
}