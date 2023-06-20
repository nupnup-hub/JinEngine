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
	class JGameObject;
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JScene>& ownerScene)
	{
		if (ownerScene->GetRootGameObject() != nullptr)
			return nullptr;
		return JICI::Create<JGameObject>(name, guid, flag, nullptr, ownerScene);
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateDebugRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JScene>& ownerScene)
	{
		if (ownerScene->GetDebugRootGameObject() != nullptr)
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
		newGameObj->GetTransform()->SetScale(XMFLOAT3(5000, 5000, 5000));
		JUserPtr<JRenderItem> newRItem = JCCI::CreateRenderItem(newGameObj,
			_JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::SPHERE),
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			J_RENDER_LAYER::SKY);
		 
		newRItem->SetMaterial(0, _JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_SKY));
		return newGameObj;
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateCamera(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const std::wstring name)
	{
		return CreateCamera(parent, flag, false, name);
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateCamera(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const bool allowCulling, const std::wstring name)
	{
		JUserPtr<JGameObject> newGameObj = JICI::Create<JGameObject>(name, Core::MakeGuid(), flag, parent);
		auto cam =  JCCI::CreateCamera(newGameObj);
		if (allowCulling)
		{ 
			cam->SetAllowFrustumCulling(true);
			cam->SetAllowHzbOcclusionCulling(true);
		}
		return newGameObj;
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateDebugCamera(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const std::wstring name )
	{
		JUserPtr<JGameObject> newGameObj = JICI::Create<JGameObject>(name, Core::MakeGuid(), flag, parent); 
		auto cam = JCCI::CreateCamera(newGameObj);
		cam->SetAllowDisplayDebug(true);
		cam->SetAllowFrustumCulling(false);
		cam->SetAllowHzbOcclusionCulling(false);
		return newGameObj;
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateLight(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const J_LIGHT_TYPE type, const std::wstring name)
	{
		JUserPtr<JGameObject> newGameObj = JICI::Create<JGameObject>(name, Core::MakeGuid(), flag, parent);
		JCCI::CreateLight(newGameObj, type);
		return newGameObj;
	}
	JUserPtr<JGameObject> JGameObjectCreatorInterface::CreateDebugLineShape(JUserPtr<JGameObject> parent,
		const J_OBJECT_FLAG flag,
		const J_DEFAULT_SHAPE meshType,
		const J_DEFAULT_MATERIAL matType,
		const bool isDebugUI)
	{
		const std::wstring name = JDefaultShape::ConvertToName(meshType) + L"DebugObject";
		const J_RENDER_LAYER layer = isDebugUI ? J_RENDER_LAYER::DEBUG_UI : J_RENDER_LAYER::DEBUG_OBJECT;
		//has sequence dependency
		JUserPtr<JGameObject> newGameObj = JICI::Create<JGameObject>(name, Core::MakeGuid(), flag, parent);
		JUserPtr<JRenderItem> newRItem = JCCI::CreateRenderItem(newGameObj,
			_JResourceManager::Instance().GetDefaultMeshGeometry(meshType),
			D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
			layer,
			SPACE_SPATIAL_NOT_ALLOW_ALL);
		newRItem->SetMaterial(0, _JResourceManager::Instance().GetDefaultMaterial(matType));
		return newGameObj;
	}
}