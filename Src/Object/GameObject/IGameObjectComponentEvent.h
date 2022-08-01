#pragma once  

namespace JinEngine
{
	class JComponent;  
	class JAnimator;
	class JRenderItem;
	class JComponent;
	class JCamera;
	class JLight;
	class JGameObject;
 
	class IGameObjectComponentEvent abstract
	{ 
		friend class JComponent;
		friend class JAnimator;
		friend class JCamera;
		friend class JRenderItem;
		friend class JLight;
		friend class JTransform;
	public:
		virtual ~IGameObjectComponentEvent() = default;
	private:
		virtual bool RegisterAnimator(JAnimator* animator)noexcept = 0;
		virtual bool RegisterCamera(JCamera* camrea)noexcept = 0;
		virtual bool RegisterRenderItem(JRenderItem* renderItem)noexcept = 0;
		virtual bool RegisterLight(JLight* light)noexcept = 0;
		virtual bool RegisterShadowLight(JLight* light)noexcept = 0;

		virtual bool DeRegisterAnimator(JAnimator* animator)noexcept = 0;
		virtual bool DeRegisterCamera(JCamera* camrea)noexcept = 0;
		virtual bool DeRegisterRenderItem(JRenderItem* renderItem)noexcept = 0;
		virtual bool DeRegisterLight(JLight* light)noexcept = 0;
		virtual bool DeRegisterShadowLight(JLight* light)noexcept = 0;

		virtual bool ReRegisterAnimator(JAnimator* animator)noexcept = 0;
		virtual bool ReRegisterRenderItem(JRenderItem* renderItem)noexcept = 0;
		virtual JCamera* SetMainCamera(JCamera* camrea)noexcept = 0;
		virtual void UpdateGameObjectTransform()noexcept = 0;
	};
}