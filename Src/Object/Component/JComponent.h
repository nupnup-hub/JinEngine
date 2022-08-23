#pragma once 
#include"JComponentInterface.h" 
#include"../../Core/File/JFileIOResult.h"
#include"JComponentType.h" 

namespace JinEngine
{
	class JGameObject; 
	class JComponent : public JComponentInterface
	{
		REGISTER_CLASS(JComponent)
	private:
		JGameObject* owner; 
	public:
		J_OBJECT_TYPE GetObjectType()const noexcept final;
		JGameObject* GetOwner()noexcept;
		virtual J_COMPONENT_TYPE GetComponentType()const noexcept = 0;
		virtual bool IsAvailableOverlap()const noexcept = 0;

		/*Defect Inspection When Use JComponent */
		virtual bool PassDefectInspection()const noexcept;
	protected:
		//Regist Scene
		bool RegisterComponent()noexcept; 
		//DeRegist Scene
		bool DeRegisterComponent()noexcept;
		bool ReRegisterComponent()noexcept;
	protected:
		//DoActivate => RegistComponent
		void DoActivate()noexcept override;
		//DoDeActivate => DeRegistComponent
		void DoDeActivate()noexcept override;
	protected:
		void Destroy() final; 
	protected:
		JComponent(const std::string& cTypeName, const size_t guid, J_OBJECT_FLAG flag, JGameObject* owner) noexcept;
		~JComponent();
	};
}
