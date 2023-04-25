#include"JGameObjectCreator.h" 
#include"JGameObject.h"
#include"../Resource/JResourceManager.h"
#include"../Resource/Mesh/JMeshGeometry.h"
#include"../Resource/Material/JDefaultMaterialType.h"
#include"../Resource/Scene/JScene.h"
#include"../Component/Transform/JTransform.h"
#include"../Component/RenderItem/JRenderItem.h"
#include"../Component/JComponentCreator.h"
#include"../../Core/Guid/GuidCreator.h"
#include"../../Core/Identity/JIdenCreator.h"

using namespace DirectX;

namespace JinEngine
{
	class JGameObject;
	JGameObject* JGameObjectCreatorInterface::CreateRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JScene* ownerScene)
	{
		if (ownerScene->GetRootGameObject() != nullptr)
			return nullptr;
		return JICI::Create<JGameObject>(name, guid, flag, nullptr, ownerScene);
	}
	JGameObject* JGameObjectCreatorInterface::CreateDebugRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JScene* ownerScene)
	{
		if (ownerScene->GetDebugRootGameObject() != nullptr)
			return nullptr;
		return JICI::Create<JGameObject>(name, guid, flag, nullptr, ownerScene);
	}
	JGameObject* JGameObjectCreatorInterface::CreateShape(JGameObject* parent, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape)
	{
		return CreateShape(parent, Core::MakeGuid(), flag, shape);
	}
	JGameObject* JGameObjectCreatorInterface::CreateShape(JGameObject* parent, const size_t guid, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape)
	{
		return CreateShape(parent, JDefaultShape::ConvertToName(shape), guid, flag, shape);
	}
	JGameObject* JGameObjectCreatorInterface::CreateShape(JGameObject* parent, const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape)
	{
		JGameObject* newGameObj = JICI::Create<JGameObject>(name, guid, flag, parent);
		JCCI::CreateRenderItem(newGameObj, _JResourceManager::Instance().GetDefaultMeshGeometry(shape).Get());
		return newGameObj;
	}
	JGameObject* JGameObjectCreatorInterface::CreateModel(JGameObject* parent, const J_OBJECT_FLAG flag, JMeshGeometry* mesh)
	{
		return CreateModel(parent, Core::MakeGuid(), flag, mesh);
	}
	JGameObject* JGameObjectCreatorInterface::CreateModel(JGameObject* parent, const size_t guid, const J_OBJECT_FLAG flag, JMeshGeometry* mesh)
	{
		JGameObject* newGameObj = JICI::Create<JGameObject>(mesh->GetName(), guid, flag, parent);
		JCCI::CreateRenderItem(newGameObj, mesh);
		return newGameObj;
	}
	JGameObject* JGameObjectCreatorInterface::CreateSky(JGameObject* parent, const J_OBJECT_FLAG flag, const std::wstring name)
	{
		JGameObject* newGameObj = JICI::Create <JGameObject>(name, Core::MakeGuid(), flag, parent);
		newGameObj->GetTransform()->SetScale(XMFLOAT3(5000, 5000, 5000));
		JRenderItem* newRItem = JCCI::CreateRenderItem(newGameObj,
			_JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE).Get(),
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			J_RENDER_LAYER::SKY);
		 
		newRItem->SetMaterial(0, _JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_SKY));
		return newGameObj;
	}
	JGameObject* JGameObjectCreatorInterface::CreateCamera(JGameObject* parent, const J_OBJECT_FLAG flag, bool isMainCamera, const std::wstring name)
	{
		JGameObject* newGameObj = JICI::Create<JGameObject>(name, Core::MakeGuid(), flag, parent);
		JCCI::CreateCamera(newGameObj, isMainCamera);
		return newGameObj;
	}
	JGameObject* JGameObjectCreatorInterface::CreateLight(JGameObject* parent, const J_OBJECT_FLAG flag, const J_LIGHT_TYPE type, const std::wstring name)
	{
		JGameObject* newGameObj = JICI::Create<JGameObject>(name, Core::MakeGuid(), flag, parent);
		JCCI::CreateLight(newGameObj, type);
		return newGameObj;
	}
	JGameObject* JGameObjectCreatorInterface::CreateDebugLineShape(JGameObject* parent,
		const J_OBJECT_FLAG flag,
		const J_DEFAULT_SHAPE meshType,
		const J_DEFAULT_MATERIAL matType,
		const bool isDebugUI)
	{
		const std::wstring name = JDefaultShape::ConvertToName(meshType) + L"DebugObject";
		const J_RENDER_LAYER layer = isDebugUI ? J_RENDER_LAYER::DEBUG_UI : J_RENDER_LAYER::DEBUG_OBJECT;
		//has sequence dependency
		JGameObject* newGameObj = JICI::Create<JGameObject>(name, Core::MakeGuid(), flag, parent);
		JRenderItem* newRItem = JCCI::CreateRenderItem(newGameObj,
			_JResourceManager::Instance().GetDefaultMeshGeometry(meshType).Get(),
			D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
			layer,
			SPACE_SPATIAL_NOT_ALLOW_ALL);
		newRItem->SetMaterial(0, _JResourceManager::Instance().GetDefaultMaterial(matType));
		return newGameObj;
	}
}