#pragma once 
#include"../../Graphic/FrameResource/JFrameResourceCash.h"

//수정필요
namespace JinEngine
{
	class JComponent;
	class GameObjectDirty
	{	 
	private:
		const size_t ownerGuid;
		int transfromDirty = 0;
		int renderItemDirty = 0;
		int cameraDirty = 0;
		int lightDirty = 0;
		int animatorDirty = 0;

		int hasTransform = 0;
		int hasRenderItem = 0;
		int hasCamera = 0;
		int hasLight = 0;
		int hasAnimator = 0;
	public:
		GameObjectDirty(const size_t guid);
		~GameObjectDirty();
		int GetTransformDirty() const noexcept;
		int GetRenderItemDirty() const noexcept;
		int GetCameraDirty() const noexcept;
		int GetLightDirty() const noexcept;
		int GetAnimatorDirty() const noexcept;

		void SetTransformDirty() noexcept;
		void SetRenderItemDirty() noexcept;
		void SetCameraDirty() noexcept;
		void SetLightDirty() noexcept;
		void SetAnimatorDirty() noexcept;

		void OffTransformDirty() noexcept;
		void OffRenderItemDirty() noexcept;
		void OffCameraDirty() noexcept;
		void OffLightDirty() noexcept;
		void OffAnimatorDirty() noexcept;

		void TransformUpdate() noexcept;
		void RenderItemUpdate() noexcept;
		void CameraUpdate() noexcept;
		void LightUpdate() noexcept;
		void AnimatorUpdate() noexcept;

		void RegisterComponent(JComponent* component)noexcept;
		void DeRegisterComponent(JComponent* component)noexcept;
	};
}