#pragma once
#include"../JComponent.h" 
#include<DirectXMath.h>

namespace JinEngine
{ 
	class JGameObject;

	class JBehavior : public JComponent
	{
		REGISTER_CLASS(JBehavior)
	public:	 
		J_COMPONENT_TYPE GetComponentType()const noexcept final; 
		static J_COMPONENT_TYPE GetStaticComponentType()noexcept;
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final; 
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