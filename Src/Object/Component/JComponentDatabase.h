#pragma once
#include<unordered_map>
#include<vector>
#include<memory>
#include"JComponentType.h"
#include"../../Core/JDataType.h"

namespace JinEngine
{
	class JComponent;
	struct JComponentData
	{
	public:
		std::string componentName;
		J_COMPONENT_TYPE type;
	public:
		JComponentData(JComponent* component);
		JComponentData(const std::string& componentName, const J_COMPONENT_TYPE type);
	};
	 
	namespace Application
	{
		class JApplication;
	}
	class JComponentDatabase
	{
	private:
		friend class Application::JApplication;
	private:
		static std::vector<std::unique_ptr<JComponentData>> componentData;
		static std::vector<JComponentData*> componentDataVec;
		static std::unordered_map<std::string, JComponentData*> componentDataMap;
	public: 
		static void RegisterComponent(JComponent* component)noexcept;
		static void RegisterComponent(const std::string& componentName, const J_COMPONENT_TYPE type)noexcept;
		static void DeRegisterComponent(JComponent* component)noexcept;
		static void DeRegisterComponent(const std::string& componentName)noexcept;
		static bool IsRegisteredComponent(JComponent* component)noexcept;
		static bool IsRegisteredComponent(const std::string& componentName)noexcept;
		static std::vector<JComponentData*>::const_iterator GetComponnetDataVec(_Out_ uint& componentCount)noexcept;
	private:
		static void Initialize();
	};
}