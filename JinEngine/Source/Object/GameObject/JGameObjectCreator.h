#pragma once
#include"../Component/RenderItem/JRenderLayer.h"
#include"../Component/RenderItem/JRenderItemAcceleratorMask.h"
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
		static JUserPtr<JGameObject> CreateRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JScene>& ownerScene);
		static JUserPtr<JGameObject> CreateShape(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape);
		static JUserPtr<JGameObject> CreateShape(JUserPtr<JGameObject> parent, const size_t guid, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape);
		static JUserPtr<JGameObject> CreateShape(JUserPtr<JGameObject> parent, const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape);
		static JUserPtr<JGameObject> CreateModel(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const JUserPtr<JMeshGeometry>& mesh);
		static JUserPtr<JGameObject> CreateModel(JUserPtr<JGameObject> parent, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JMeshGeometry>& mesh);
		static JUserPtr<JGameObject> CreateSky(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const std::wstring name = L"Skymap"); 
		//if allowCulling(true) set hdOcc
		static JUserPtr<JGameObject> CreateCamera(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const bool allowCulling, const std::wstring name = L"Camera");
		static JUserPtr<JGameObject> CreateLight(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const J_LIGHT_TYPE type, const std::wstring name = L"JLight");
	public:
		static JUserPtr<JGameObject> CreateDebugRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JScene>& ownerScene);
		static JUserPtr<JGameObject> CreateDebugCamera(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const std::wstring name = L"DebugCamera");
		static JUserPtr<JGameObject> CreateDebugShape(JUserPtr<JGameObject> parent,
			const std::wstring& name,
			const J_OBJECT_FLAG flag,
			const J_DEFAULT_SHAPE meshType,
			const J_DEFAULT_MATERIAL matType, 
			const bool isDebugUI,
			const bool isLine,
			const J_RENDERITEM_ACCELERATOR_MASK spaceSpatialMask); 
		static JUserPtr<JGameObject> CreateDebugLineShape(JUserPtr<JGameObject> parent,
			const J_OBJECT_FLAG flag,
			const J_DEFAULT_SHAPE meshType,
			const J_DEFAULT_MATERIAL matType,
			const bool isDebugUI);
	};

	using JGCI = JGameObjectCreatorInterface;
}