#include"GameObjectDirty.h"
#include"../Component/JComponent.h"

namespace JinEngine
{
	GameObjectDirty::GameObjectDirty(const size_t ownerGuid)
		:ownerGuid(ownerGuid),
		transfromDirty(Graphic::gNumFrameResources),
		renderItemDirty(Graphic::gNumFrameResources)
	{}
	GameObjectDirty::~GameObjectDirty(){}

	int GameObjectDirty::GetTransformDirty() const noexcept
	{
		return transfromDirty;
	}
	int GameObjectDirty::GetRenderItemDirty() const noexcept
	{
		return renderItemDirty;
	}
	int GameObjectDirty::GetCameraDirty() const noexcept
	{
		return cameraDirty;
	}
	int GameObjectDirty::GetLightDirty() const noexcept
	{
		return lightDirty;
	}
	int GameObjectDirty::GetAnimatorDirty() const noexcept
	{
		return animatorDirty;
	}
	void GameObjectDirty::SetTransformDirty() noexcept
	{
		transfromDirty = Graphic::gNumFrameResources * hasTransform;
		cameraDirty = Graphic::gNumFrameResources * hasCamera;
		lightDirty = Graphic::gNumFrameResources * hasLight;
	}
	void GameObjectDirty::SetRenderItemDirty() noexcept
	{
		renderItemDirty = Graphic::gNumFrameResources * hasRenderItem;
	}
	void GameObjectDirty::SetCameraDirty() noexcept
	{
		cameraDirty = Graphic::gNumFrameResources * hasCamera;
	}
	void GameObjectDirty::SetLightDirty() noexcept
	{
		lightDirty = Graphic::gNumFrameResources * hasLight;
	}
	void GameObjectDirty::SetAnimatorDirty() noexcept
	{
		animatorDirty = Graphic::gNumFrameResources * hasAnimator;
		transfromDirty = Graphic::gNumFrameResources * hasTransform;
	}
	void GameObjectDirty::OffTransformDirty() noexcept
	{
		transfromDirty = 0;
	}
	void GameObjectDirty::OffRenderItemDirty() noexcept
	{
		renderItemDirty = 0;
	}
	void GameObjectDirty::OffCameraDirty() noexcept
	{
		cameraDirty = 0;
	}
	void GameObjectDirty::OffLightDirty() noexcept
	{
		lightDirty = 0;
	}
	void GameObjectDirty::OffAnimatorDirty() noexcept
	{
		animatorDirty = 0;
	}
	void GameObjectDirty::TransformUpdate() noexcept
	{
		--transfromDirty;
		if (transfromDirty < 0)
			transfromDirty = 0;
	}
	void GameObjectDirty::RenderItemUpdate() noexcept
	{
		--renderItemDirty;
		if (renderItemDirty < 0)
			renderItemDirty = 0;
	}
	void GameObjectDirty::CameraUpdate() noexcept
	{
		--cameraDirty;
		if (cameraDirty < 0)
			cameraDirty = 0;
	}
	void GameObjectDirty::LightUpdate() noexcept
	{
		--lightDirty;
		if (lightDirty < 0)
			lightDirty = 0;
	}
	void GameObjectDirty::AnimatorUpdate() noexcept
	{
		--animatorDirty;
		if (animatorDirty < 0)
			animatorDirty = 0;
	}
	void GameObjectDirty::RegisterComponent(JComponent* component)noexcept
	{
		if (component == nullptr)
			return;

		J_COMPONENT_TYPE componentType = component->GetComponentType();
		switch (componentType)
		{
		case JinEngine::J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR:
			hasAnimator = 1;
			break;
		case JinEngine::J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA:
			hasCamera = 1;
			break;
		case JinEngine::J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT:
			hasLight = 1;
			break;
		case JinEngine::J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM:
			hasRenderItem = 1;
			break;
		case JinEngine::J_COMPONENT_TYPE::ENGINE_DEFIENED_TRANSFORM:
			hasTransform = 1;
			break;
		default:
			break;
		}
	}
	void GameObjectDirty::DeRegisterComponent(JComponent* component)noexcept
	{
		if (component == nullptr)
			return;

		J_COMPONENT_TYPE componentType = component->GetComponentType();
		switch (componentType)
		{
		case JinEngine::J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR:
			hasAnimator = 0;
			break;
		case JinEngine::J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA:
			hasCamera = 0;
			break;
		case JinEngine::J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT:
			hasLight = 0;
			break;
		case JinEngine::J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM:
			hasRenderItem = 0;
			break;
		case JinEngine::J_COMPONENT_TYPE::ENGINE_DEFIENED_TRANSFORM:
			hasTransform = 0;
			break;
		default:
			break;
		}
	}
}