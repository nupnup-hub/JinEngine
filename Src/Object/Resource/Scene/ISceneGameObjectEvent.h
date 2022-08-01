#pragma once  
#include<memory>

namespace JinEngine
{ 
	class JAnimator;
	class JCamera;
	class JRenderItem;
	class JTransform; 
	class JLight;
	class JGameObject;

	class ISceneGameObjectEvent abstract
	{
		friend class GameObjectCreator;
		friend class JGameObject;
	public:
		virtual ~ISceneGameObjectEvent() = default;
	private: 
		virtual void EraseGameObject(JGameObject* gameObj)noexcept = 0; 

		virtual bool RegisterAnimator(JGameObject* gameObject, JAnimator* animator)noexcept = 0; 
		virtual bool RegisterCamera(JGameObject* gameObject, JCamera* camera)noexcept = 0;
		virtual bool RegisterLight(JGameObject* gameObject, JLight* light)noexcept = 0;
		virtual bool RegisterRenderItem(JGameObject* gameObject, JRenderItem* animator)noexcept = 0;
		virtual bool RegisterShadowLight(JGameObject* gameObject, JLight* light)noexcept = 0;

		virtual bool DeRegisterAnimator(JGameObject* gameObject, JAnimator* animator)noexcept = 0;
		virtual bool DeRegisterCamera(JGameObject* gameObject, JCamera* camera)noexcept = 0;
		virtual bool DeRegisterLight(JGameObject* gameObject, JLight* light)noexcept = 0;
		virtual bool DeRegisterRenderItem(JGameObject* gameObject, JRenderItem* animator)noexcept = 0;
		virtual bool DeRegisterShadowLight(JGameObject* gameObject, JLight* light)noexcept = 0;
		
		virtual JCamera* SetMainCamera(JGameObject* gameObject, JCamera* animator)noexcept = 0;  
		virtual void UpdateGameObjectTransform(JGameObject* gameObject)noexcept = 0;
	};
}