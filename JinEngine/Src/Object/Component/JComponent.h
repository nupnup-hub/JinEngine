#pragma once 
#include"JComponentInterface.h" 
#include"JComponentType.h" 
#include"../../Core/File/JFileIOResult.h"
#include"../../Utility/JTypeUtility.h"

namespace JinEngine
{
	class JGameObject; 
	class JComponent : public JComponentInterface
	{
		REGISTER_CLASS(JComponent)
	private:
		JGameObject* owner = nullptr;
	private:
		using JObject::SetName;
	public:
		J_OBJECT_TYPE GetObjectType()const noexcept final;
		JGameObject* GetOwner()const noexcept;
		virtual J_COMPONENT_TYPE GetComponentType()const noexcept = 0;

		virtual bool IsAvailableOverlap()const noexcept = 0;
		/*Defect Inspection When Use JComponent */
		virtual bool PassDefectInspection()const noexcept;
	protected:
		//Register in owner scene
		bool RegisterComponent()noexcept; 
		//DeRegist in owner Scene
		bool DeRegisterComponent()noexcept;
		bool ReRegisterComponent()noexcept;
	protected:
		//step) DoActivate => RegistComponent
		void DoActivate()noexcept override;
		//step) DoDeActivate => DeRegistComponent
		void DoDeActivate()noexcept override;
	protected:
		bool Destroy(const bool isForced) final;
	private:
		bool RegisterCashData()noexcept final;
		bool DeRegisterCashData()noexcept final;
	public:
		template<typename T>
		static auto Convert(JObject* obj) -> typename Core::TypeCondition<T*, std::is_base_of_v<JComponent, T>>::Type
		{
			if (obj->GetObjectType() == J_OBJECT_TYPE::COMPONENT_OBJECT)
			{
				JComponent* jCobj = static_cast<JComponent*>(obj);
				if (jCobj->GetComponentType() == T::GetStaticComponentType())
					return static_cast<T*>(jCobj);
			}
			return nullptr;
		}
	protected:
		JComponent(const std::string& cTypeName, const size_t guid, J_OBJECT_FLAG flag, JGameObject* owner) noexcept;
		~JComponent();
	};
}
