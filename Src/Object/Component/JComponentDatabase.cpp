#include"JComponentDatabase.h"
#include"JComponent.h" 
#include"Animator/JAnimator.h"
#include"Camera/JCamera.h"
#include"Light/JLight.h"
#include"RenderItem/JRenderItem.h"
 
namespace JinEngine
{
	std::vector<std::unique_ptr<JComponentData>> JComponentDatabase::componentData;
	std::vector<JComponentData*> JComponentDatabase::componentDataVec;
	std::unordered_map<std::string, JComponentData*> JComponentDatabase::componentDataMap;
 
	JComponentData::JComponentData(JComponent* component)
		:componentName(component->GetName()), type(component->GetComponentType())
	{

	}
	JComponentData::JComponentData(const std::string& componentName, const J_COMPONENT_TYPE type)
		: componentName(componentName), type(type)
	{

	}
	void JComponentDatabase::RegisterComponent(JComponent* component)noexcept
	{
		if (component == nullptr)
			return;

		RegisterComponent(component->GetName(), component->GetComponentType());
	}
	void JComponentDatabase::RegisterComponent(const std::string& componentName, const J_COMPONENT_TYPE type)noexcept
	{
		if (!IsRegisteredComponent(componentName))
		{
			std::unique_ptr<JComponentData> newData = std::make_unique<JComponentData>(componentName, type);
			componentDataMap.emplace(newData->componentName, newData.get());
			componentDataVec.push_back(newData.get());
			componentData.push_back(std::move(newData));
		}
	}
	void JComponentDatabase::DeRegisterComponent(JComponent* component)noexcept
	{	 
		if (component == nullptr)
			return;
		DeRegisterComponent(component->GetName());
	}
	void JComponentDatabase::DeRegisterComponent(const std::string& componentName)noexcept
	{
		if (IsRegisteredComponent(componentName))
		{
			componentDataMap.erase(componentName);			 
			const uint componentVecCount = (uint)componentDataVec.size();
			for (uint i = 0; i < componentVecCount; ++i)
			{
				if (componentDataVec[i]->componentName == componentName)
				{
					componentDataVec.erase(componentDataVec.begin() + i);
					break;
				}
			}

			const uint componentCount = (uint)componentData.size();
			for (uint i = 0; i < componentCount; ++i)
			{
				if (componentData[i]->componentName == componentName)
				{
					componentData.erase(componentData.begin() + i);
					break;
				}
			}
		}
	}
	bool JComponentDatabase::IsRegisteredComponent(JComponent* component)noexcept
	{
		return componentDataMap.find(component->GetName()) != componentDataMap.end();
	}
	bool JComponentDatabase::IsRegisteredComponent(const std::string& componentName)noexcept
	{
		return componentDataMap.find(componentName) != componentDataMap.end();
	}
	std::vector<JComponentData*>::const_iterator JComponentDatabase::GetComponnetDataVec(_Out_ uint& componentCount)noexcept
	{
		componentCount = (uint)componentDataVec.size();
		return componentDataVec.cbegin();
	}
	void JComponentDatabase::Initialize()
	{ 
		static bool init = false;
		if (!init)
		{
			RegisterComponent(JAnimator::TypeName(), J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
			RegisterComponent(JCamera::TypeName(), J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
			RegisterComponent(JLight::TypeName(), J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
			RegisterComponent(JRenderItem::TypeName(), J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
		}
	}
}