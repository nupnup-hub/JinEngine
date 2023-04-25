#pragma once
#include"../Component/RenderItem/JRenderLayer.h"
#include"../Component/Light/JLightType.h"
#include"../Resource/Mesh/JDefaultShapeType.h"
#include"../Resource/Material/JDefaultMaterialType.h"
#include"../JObjectFlag.h"
#include<string>

namespace JinEngine
{
	class JGameObject;
	class JMeshGeometry;
	class JScene;
	class JGameObjectCreatorInterface
	{
	public:
		static JGameObject* CreateRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JScene* ownerScene);
		static JGameObject* CreateDebugRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JScene* ownerScene);
		static JGameObject* CreateShape(JGameObject* parent, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape);
		static JGameObject* CreateShape(JGameObject* parent, const size_t guid, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape);
		static JGameObject* CreateShape(JGameObject* parent, const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape);
		static JGameObject* CreateModel(JGameObject* parent, const J_OBJECT_FLAG flag, JMeshGeometry* mesh);
		static JGameObject* CreateModel(JGameObject* parent, const size_t guid, const J_OBJECT_FLAG flag, JMeshGeometry* mesh);
		static JGameObject* CreateSky(JGameObject* parent, const J_OBJECT_FLAG flag, const std::wstring name = L"Skymap");
		static JGameObject* CreateCamera(JGameObject* parent, const J_OBJECT_FLAG flag, bool isMainCamera, const std::wstring name = L"JCamera");
		static JGameObject* CreateLight(JGameObject* parent, const J_OBJECT_FLAG flag, const J_LIGHT_TYPE type, const std::wstring name = L"JLight");
		static JGameObject* CreateDebugLineShape(JGameObject* parent,
			const J_OBJECT_FLAG flag,
			const J_DEFAULT_SHAPE meshType,
			const J_DEFAULT_MATERIAL matType,
			const bool isDebugUI);
	};

	using JGCI = JGameObjectCreatorInterface;
}