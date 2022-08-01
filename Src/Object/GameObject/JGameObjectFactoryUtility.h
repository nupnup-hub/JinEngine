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
		static JGameObject* CreateShape(JGameObject& parent, const JOBJECT_FLAG flag, const J_DEFAULT_SHAPE shape);
		static JGameObject* CreateSky(JGameObject& parent, const JOBJECT_FLAG flag, const std::string name = "Skymap");
		static JGameObject* CreateCamera(JGameObject& parent, const JOBJECT_FLAG flag, bool isMainCamera, const std::string name = "JCamera");
		static JGameObject* CreateLight(JGameObject& parent, const JOBJECT_FLAG flag, const J_LIGHT_TYPE type, const std::string name = "JLight");
		static JGameObject* CreateDebugGameObject(JGameObject& parent,
			const JOBJECT_FLAG flag,
			const J_DEFAULT_SHAPE meshType,
			const J_DEFAULT_MATERIAL matType);
	};

	using JGFU = JGameObjectFactoryUtility;
}