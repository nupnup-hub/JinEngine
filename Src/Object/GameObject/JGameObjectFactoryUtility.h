#pragma once
#include"../Component/RenderItem/JRenderLayer.h"
#include"../Component/Light/JLightType.h"
#include"../Resource/Mesh/JDefaultShapeType.h"
#include"../Resource/Material/JDefaultMaterialType.h"
#include"../JObjectFlag.h"

namespace JinEngine
{
	class JGameObject;
	class JGameObjectFactoryUtility
	{
	public:
		static JGameObject* CreateShape(JGameObject& parent, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape);
		static JGameObject* CreateSky(JGameObject& parent, const J_OBJECT_FLAG flag, const std::wstring name = L"Skymap");
		static JGameObject* CreateCamera(JGameObject& parent, const J_OBJECT_FLAG flag, bool isMainCamera, const std::wstring name = L"JCamera");
		static JGameObject* CreateLight(JGameObject& parent, const J_OBJECT_FLAG flag, const J_LIGHT_TYPE type, const std::wstring name = L"JLight");
		static JGameObject* CreateDebugGameObject(JGameObject& parent,
			const J_OBJECT_FLAG flag,
			const J_DEFAULT_SHAPE meshType,
			const J_DEFAULT_MATERIAL matType);
	};

	using JGFU = JGameObjectFactoryUtility;
}