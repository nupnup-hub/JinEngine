#pragma once
#include"JGameObjectFactoryUtility.h"
#include"JGameObjectFactory.h"
#include"JGameObject.h"
#include"../Resource/JResourceManager.h"
#include"../Resource/Material/JDefaultMaterialType.h"
#include"../Component/Transform/JTransform.h"
//#include"../Component/RenderItem/JRenderItem.h"
#include"../Component/JComponentFactoryUtility.h"
#include"../../Core/Guid/GuidCreator.h"

using namespace DirectX;

namespace JinEngine
{
	class JGameObject;
	JGameObject* JGameObjectFactoryUtility::CreateShape(JGameObject& parent, const JOBJECT_FLAG flag, const J_DEFAULT_SHAPE shape)
	{
		JGameObject* newGameObj = JGFI::Create(JDefaultShape::ConvertDefaultShapeName(shape), Core::MakeGuid(), flag, parent);
		JCFU::CreateRenderItem(Core::MakeGuid(), OBJECT_FLAG_NONE, *newGameObj,
			JResourceManager::Instance().GetDefaultMeshGeometry(shape),
			JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_STANDARD));

		return newGameObj;
	}
	JGameObject* JGameObjectFactoryUtility::CreateSky(JGameObject& parent, const JOBJECT_FLAG flag, const std::string name)
	{
		JGameObject* newGameObj = JGFI::Create(name, Core::MakeGuid(), flag, parent);
		newGameObj->GetTransform()->SetScale(XMFLOAT3(5000, 5000, 5000));
		JCFU::CreateRenderItem(Core::MakeGuid(),
			OBJECT_FLAG_NONE,
			*newGameObj,
			JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE),
			JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_SKY),
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			J_RENDER_LAYER::SKY);
		return newGameObj;
	}
	JGameObject* JGameObjectFactoryUtility::CreateCamera(JGameObject& parent, const JOBJECT_FLAG flag, bool isMainCamera, const std::string name)
	{
		JGameObject* newGameObj = JGFI::Create(name, Core::MakeGuid(), flag, parent);
		JCFU::CreateCamera(Core::MakeGuid(), OBJECT_FLAG_NONE, *newGameObj, isMainCamera);
		return newGameObj;
	}
	JGameObject* JGameObjectFactoryUtility::CreateLight(JGameObject& parent, const JOBJECT_FLAG flag, const J_LIGHT_TYPE type, const std::string name)
	{
		JGameObject* newGameObj = JGFI::Create(name, Core::MakeGuid(), flag, parent);
		JCFU::CreateLight(Core::MakeGuid(), OBJECT_FLAG_NONE, *newGameObj, type);
		return newGameObj;
	}
	JGameObject* JGameObjectFactoryUtility::CreateDebugGameObject(JGameObject& parent,
		const JOBJECT_FLAG flag,
		const J_DEFAULT_SHAPE meshType,
		const J_DEFAULT_MATERIAL matType)
	{
		const std::string name = JDefaultShape::ConvertDefaultShapeName(meshType) + "DebugObject";
		JGameObject* newGameObj = JGFI::Create(name, Core::MakeGuid(), flag, parent);
		JCFU::CreateRenderItem(Core::MakeGuid(),
			OBJECT_FLAG_NONE,
			*newGameObj,
			JResourceManager::Instance().GetDefaultMeshGeometry(meshType),
			JResourceManager::Instance().GetDefaultMaterial(matType),
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			J_RENDER_LAYER::DEBUG_LAYER);

		return newGameObj;
	}
}