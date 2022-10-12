#pragma once
#include"../JComponent.h" 
#include<DirectXMath.h> 

namespace JinEngine
{ 
	class JGameObject;

	//¹Ì±¸Çö
	class JBehavior final : public JComponent
	{
		REGISTER_CLASS(JBehavior)
	public:	 
		J_COMPONENT_TYPE GetComponentType()const noexcept final;  
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::USER_DEFIENED_BEHAVIOR;
		}
		bool IsAvailableOverlap()const noexcept final; 
		bool PassDefectInspection()const noexcept final; 
	private:
		void DoCopy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final; 
	private:
		Core::J_FILE_IO_RESULT CallStoreComponent(std::wofstream& stream)final;
		static Core::J_FILE_IO_RESULT StoreObject(std::wofstream& stream, JBehavior* bahavior);
		static JBehavior* LoadObject(std::wifstream& stream, JGameObject* owner);
		static void RegisterJFunc();
	private:
		JBehavior(const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
		~JBehavior();
	};
}