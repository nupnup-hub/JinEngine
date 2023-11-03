#include"JGameObjectCreator.h" 
#include"JGameObject.h"
#include"../Resource/JResourceManager.h"
#include"../Resource/Mesh/JMeshGeometry.h"
#include"../Resource/Material/JDefaultMaterialType.h"
#include"../Resource/Scene/JScene.h"
#include"../Component/Transform/JTransform.h"
#include"../Component/RenderItem/JRenderItem.h"
#include"../Component/Camera/JCamera.h"
#include"../Component/JComponentCreator.h"
#include"../../Core/Guid/JGuidCreator.h"
#include"../../Core/Identity/JIdenCreator.h"
 
using namespace DirectX;

namespace JinEngine
{
	namespace Private
	{
		static std::wstring GetDefualtLightName(const J_LIGHT_TYPE litType)
		{
			switch (litType)
			{
			case JinEngine::J_LIGHT_TYPE::DIRECTIONAL:
				return L"DirectionalLight";
			case JinEngine::J_LIGHT_TYPE::POINT:
				return L"PointLight";
			case JinEngine::J_LIGHT_TYPE::SPOT:
				return L"SpotLight";
			case JinEngine::J_LIGHT_TYPE::COUNT:
				break;
			default:
				break;
			}
			return L"ShiroWalWal!";
		}
	}

	class JGameObject;
	std::wstring JGameObjectCreatorInterface::GetDefaultLitName(const J_LIGHT_TYPE type)noexcept
	{
		return Private::GetDefualtLightName(type);
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JScene>& ownerScene)
	{
		if (ownerScene->GetRootGameObject() != nullptr)
			return nullptr;
		return JICI::Create<JGameObject>(name, guid, flag, nullptr, ownerScene);
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateShape(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape)
	{
		return CreateShape(parent, Core::MakeGuid(), flag, shape);
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateShape(JUserPtr<JGameObject> parent, const size_t guid, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape)
	{
		return CreateShape(parent, JDefaultShape::ConvertToName(shape), guid, flag, shape);
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateShape(JUserPtr<JGameObject> parent, const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape)
	{
		JUserPtr<JGameObject> newGameObj = JICI::Create<JGameObject>(name, guid, flag, parent);
		JCCI::CreateRenderItem(newGameObj, _JResourceManager::Instance().GetDefaultMeshGeometry(shape));
		return newGameObj;
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateModel(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const JUserPtr<JMeshGeometry>& mesh)
	{
		return CreateModel(parent, Core::MakeGuid(), flag, mesh);
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateModel(JUserPtr<JGameObject> parent, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JMeshGeometry>& mesh)
	{
		JUserPtr<JGameObject> newGameObj = JICI::Create<JGameObject>(mesh->GetName(), guid, flag, parent);
		JCCI::CreateRenderItem(newGameObj, mesh);
		return newGameObj;
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateSky(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const std::wstring name)
	{
		JUserPtr<JGameObject> newGameObj = JICI::Create <JGameObject>(name, Core::MakeGuid(), flag, parent);
		newGameObj->GetTransform()->SetScale({ 5000, 5000, 5000 });
		JUserPtr<JRenderItem> newRItem = JCCI::CreateRenderItem(newGameObj,
			_JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::SPHERE),
			J_RENDER_PRIMITIVE::TRIANGLE,
			J_RENDER_LAYER::SKY);
		 
		newRItem->SetMaterial(0, _JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_SKY));
		return newGameObj;
	} 
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateCamera(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const bool allowCulling)
	{
		return CreateCamera(parent, L"Camera", Core::MakeGuid(), flag, allowCulling);
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateCamera(JUserPtr<JGameObject> parent, const size_t guid, const J_OBJECT_FLAG flag, const bool allowCulling)
	{
		return CreateCamera(parent, L"Camera", guid, flag, allowCulling);
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateCamera(JUserPtr<JGameObject> parent, const std::wstring name, const size_t guid, const J_OBJECT_FLAG flag, const bool allowCulling)
	{
		JUserPtr<JGameObject> newGameObj = JICI::Create<JGameObject>(name, guid, flag, parent);
		auto cam = JCCI::CreateCamera(newGameObj);
		if (allowCulling)
		{
			cam->SetAllowFrustumCulling(true);
			cam->SetAllowHdOcclusionCulling(true);
			//cam->SetAllowHzbOcclusionCulling(true);
		}
		return newGameObj;
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateLight(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const J_LIGHT_TYPE type)
	{
		return CreateLight(parent, Private::GetDefualtLightName(type), Core::MakeGuid(), flag, type);
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateLight(JUserPtr<JGameObject> parent, const size_t guid, const J_OBJECT_FLAG flag, const J_LIGHT_TYPE type)
	{
		return CreateLight(parent, Private::GetDefualtLightName(type), guid, flag, type);
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateLight(JUserPtr<JGameObject> parent, const std::wstring name, const size_t guid, const J_OBJECT_FLAG flag, const J_LIGHT_TYPE type)
	{
		JUserPtr<JGameObject> newGameObj = JICI::Create<JGameObject>(name, guid, flag, parent);
		JCCI::CreateLight(newGameObj, type);
		return newGameObj;
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateDebugRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JScene>& ownerScene)
	{
		if (ownerScene->GetDebugRootGameObject() != nullptr)
			return nullptr;
		return JICI::Create<JGameObject>(name, guid, flag, nullptr, ownerScene);
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateDebugCamera(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const std::wstring name)
	{
		JUserPtr<JGameObject> newGameObj = JICI::Create<JGameObject>(name, Core::MakeGuid(), flag, parent);
		auto cam = JCCI::CreateCamera(newGameObj);
		cam->SetAllowDisplayDebug(true);
		cam->SetAllowFrustumCulling(false);
		cam->SetAllowHzbOcclusionCulling(false);
		return newGameObj;
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateDebugShape(JUserPtr<JGameObject> parent,
		const std::wstring& name,
		const J_OBJECT_FLAG flag,
		const J_DEFAULT_SHAPE meshType,
		const J_DEFAULT_MATERIAL matType,
		const bool isDebugUI,
		const bool isLine,
		const J_RENDERITEM_ACCELERATOR_MASK acceleratorMask)
	{
		const J_RENDER_LAYER layer = isDebugUI ? J_RENDER_LAYER::DEBUG_UI : J_RENDER_LAYER::DEBUG_OBJECT;
		const J_RENDER_PRIMITIVE primitive = isLine ? J_RENDER_PRIMITIVE::LINE : J_RENDER_PRIMITIVE::TRIANGLE;
		//has sequence dependency
		JUserPtr<JGameObject> newGameObj = JICI::Create<JGameObject>(name, Core::MakeGuid(), flag, parent);
		JUserPtr<JRenderItem> newRItem = JCCI::CreateRenderItem(newGameObj,
			_JResourceManager::Instance().GetDefaultMeshGeometry(meshType),
			primitive,
			layer,
			acceleratorMask);
		newRItem->SetMaterial(0, _JResourceManager::Instance().GetDefaultMaterial(matType));
		return newGameObj;
	} 
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateDebugLineShape(JUserPtr<JGameObject> parent,
		const J_OBJECT_FLAG flag,
		const J_DEFAULT_SHAPE meshType,
		const J_DEFAULT_MATERIAL matType,
		const bool isDebugUI)
	{ 
		return CreateDebugShape(parent, JDefaultShape::ConvertToName(meshType) + L"DebugObject",
			flag, meshType, matType, isDebugUI, true, ACCELERATOR_NOT_ALLOW_ALL);
	}
}