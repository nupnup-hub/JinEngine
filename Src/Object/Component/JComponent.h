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
		bool RegisterComponent()noexcept; 
		bool DeRegisterComponent()noexcept;
		bool ReRegisterComponent()noexcept;
	protected:
		void DoActivate()noexcept override;
		void DoDeActivate()noexcept override;
	protected:
		JComponent(const std::string& cTypeName, const size_t guid, JOBJECT_FLAG flag, JGameObject* owner) noexcept;
		~JComponent();
	};
}
