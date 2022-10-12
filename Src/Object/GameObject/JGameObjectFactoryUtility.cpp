#pragma once
#include"JGameObjectFactoryUtility.h"
#include"JGameObjectFactory.h"
#include"JGameObject.h"
#include"../Resource/JResourceManager.h"
#include"../Resource/Material/JDefaultMaterialType.h"
#include"../Component/Transform/JTransform.h"
#include"../Component/RenderItem/JRenderItem.h"
#include"../Component/JComponentFactoryUtility.h"
#include"../../Core/Guid/GuidCreator.h"

using namespace DirectX;

namespace JinEngine
{
	class JGameObject;
	JGameObject* JGameObjectFactoryUtility::CreateShape(JGameObject& parent, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape)
	{
		return CreateShape(parent, Core::MakeGuid(), flag, shape);
	}
	JGameObject* JGameObjectFactoryUtility::CreateShape(JGameObject& parent, const size_t guid, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape)
	{
		JGameObject* newGameObj = JGFI::Create(JDefaultShape::ConvertDefaultShapeName(shape), guid, flag, parent);
		JCFU::CreateRenderItem(Core::MakeGuid(), OBJECT_FLAG_NONE, *newGameObj,
			JResourceManager::Instance().GetDefaultMeshGeometry(shape));
		return newGameObj;
	}
	JGameObject* JGameObjectFactoryUtility::CreateModel(JGameObject& parent, const J_OBJECT_FLAG flag, JMeshGeometry* mesh)
	{
		return CreateModel(parent, Core::MakeGuid(), flag, mesh);
	}
	JGameObject* JGameObjectFactoryUtility::CreateModel(JGameObject& parent, const size_t guid, const J_OBJECT_FLAG flag, JMeshGeometry* mesh)
	{
		JGameObject* newGameObj = JGFI::Create(JObject::GetDefaultName<JGameObject>(), guid, flag, parent);
		JCFU::CreateRenderItem(Core::MakeGuid(), OBJECT_FLAG_NONE, *newGameObj, mesh);
		return newGameObj;
	}
	JGameObject* JGameObjectFactoryUtility::CreateSky(JGameObject& parent, const J_OBJECT_FLAG flag, const std::wstring name)
	{
		JGameObject* newGameObj = JGFI::Create(name, Core::MakeGuid(), flag, parent);
		newGameObj->GetTransform()->SetScale(XMFLOAT3(5000, 5000, 5000));
		JRenderItem* newRItem = JCFU::CreateRenderItem(Core::MakeGuid(),
			OBJECT_FLAG_NONE,
			*newGameObj,
			JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE),
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			J_RENDER_LAYER::SKY);

		newRItem->SetMaterial(0, JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_SKY));
		return newGameObj;
	}
	JGameObject* JGameObjectFactoryUtility::CreateCamera(JGameObject& parent, const J_OBJECT_FLAG flag, bool isMainCamera, const std::wstring name)
	{
		JGameObject* newGameObj = JGFI::Create(name, Core::MakeGuid(), flag, parent);
		JCFU::CreateCamera(Core::MakeGuid(), OBJECT_FLAG_NONE, *newGameObj, isMainCamera);
		return newGameObj;
	}
	JGameObject* JGameObjectFactoryUtility::CreateLight(JGameObject& parent, const J_OBJECT_FLAG flag, const J_LIGHT_TYPE type, const std::wstring name)
	{
		JGameObject* newGameObj = JGFI::Create(name, Core::MakeGuid(), flag, parent);
		JCFU::CreateLight(Core::MakeGuid(), OBJECT_FLAG_NONE, *newGameObj, type);
		return newGameObj;
	}
	JGameObject* JGameObjectFactoryUtility::CreateDebugGameObject(JGameObject& parent,
		const J_OBJECT_FLAG flag,
		const J_DEFAULT_SHAPE meshType,
		const J_DEFAULT_MATERIAL matType)
	{
		const std::wstring name = JDefaultShape::ConvertDefaultShapeName(meshType) + L"DebugObject";
		JGameObject* newGameObj = JGFI::Create(name, Core::MakeGuid(), flag, parent);
		JRenderItem* newRItem = JCFU::CreateRenderItem(Core::MakeGuid(),
			OBJECT_FLAG_NONE,
			*newGameObj,
			JResourceManager::Instance().GetDefaultMeshGeometry(meshType),
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			J_RENDER_LAYER::DEBUG_LAYER);

		newRItem->SetMaterial(0, JResourceManager::Instance().GetDefaultMaterial(matType));
		return newGameObj;
	}
}