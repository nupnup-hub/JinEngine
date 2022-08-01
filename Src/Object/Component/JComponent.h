#pragma once 
#include"JComponentInterface.h"
#include"../GameObject/IGameObjectComponentEvent.h"
#include"../../Core/File/JFileIOResult.h"
#include"JComponentType.h" 

namespace JinEngine
{
	class JGameObject;
	class JAnimator;
	class JCamera;
	class JLight;
	class JRenderItem;

	class JComponent : public JComponentInterface
	{
		REGISTER_CLASS(JComponent)
		//friend class JGameObject;
		friend class ComponentCreator;
	private:
		JGameObject* owner;
		IGameObjectComponentEvent* gameObjectComponentEvent;
	public:
		JComponent(const std::string& classTypeName, const size_t guid, JOBJECT_FLAG flag, JGameObject* owner) noexcept;
		~JComponent();
		JComponent(JComponent&& rhs) = default;
		JComponent& operator=(JComponent&& rhs) = default;

		std::string GetOwnerName()const noexcept;
		size_t GetOwnerGuid()const noexcept;
		J_OBJECT_TYPE GetObjectType()const noexcept final;
		/*
		* JGameObject erase component and remain component vectors are update guid
		*/ 
		bool HasOwner()const noexcept;

		virtual J_COMPONENT_TYPE GetComponentType()const noexcept = 0;
		virtual bool IsAvailableOverlap()const noexcept = 0;
		/*
		Defect Inspection When Use JComponent 
		*/
		virtual bool PassDefectInspection()const noexcept;   
	protected:
		JGameObject* GetOwner()noexcept;
		IGameObjectComponentEvent* GetOwnerInterface();
		void DoActivate()noexcept override;
		void DoDeActivate()noexcept override;
	protected:
		template<typename T, typename C = std::enable_if_t<std::is_base_of<JComponent, T>::value>>
		bool RegisterComponent(T* component)
		{
			if constexpr (std::is_same<T, JAnimator>::value)
				return gameObjectComponentEvent->RegisterAnimator(component);
			else if constexpr (std::is_same<T, JCamera>::value)
				return gameObjectComponentEvent->RegisterCamera(component);
			else if constexpr (std::is_same<T, JLight>::value)
				return gameObjectComponentEvent->RegisterLight(component);
			else if constexpr (std::is_same<T, JRenderItem>::value)
				return gameObjectComponentEvent->RegisterRenderItem(component);
			else
				return true;
		}
		template<typename T, typename C = std::enable_if_t<std::is_base_of<JComponent, T>::value>>
		bool DeRegisterComponent(T* component)
		{
			if constexpr (std::is_same<T, JAnimator>::value)
				return gameObjectComponentEvent->DeRegisterAnimator(component);
			else if constexpr (std::is_same<T, JCamera>::value)
				return gameObjectComponentEvent->DeRegisterCamera(component);
			else if constexpr (std::is_same<T, JLight>::value)
				return gameObjectComponentEvent->DeRegisterLight(component);
			else if constexpr (std::is_same<T, JRenderItem>::value)
				return gameObjectComponentEvent->DeRegisterRenderItem(component);
			else
				return false;
		}
		template<typename T, typename C = std::enable_if_t<std::is_base_of<JComponent, T>::value>>
		bool ReRegisterComponent(T* component)
		{
			if constexpr (std::is_same<T, JRenderItem>::value)
				return gameObjectComponentEvent->ReRegisterRenderItem(component);
			else if constexpr (std::is_same<T, JAnimator>::value)
				return gameObjectComponentEvent->ReRegisterAnimator(component);
		}
		/*추가필요
		* JComponent Copy 필요
		* JComponent 복사 및 이동생성자 처리
		*/
	};
}
