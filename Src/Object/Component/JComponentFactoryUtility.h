#pragma once
#include"../JObjectFlag.h"
#include"../Component/RenderItem/JRenderLayer.h"
#include"../Component/Light/JLightType.h"
#include"../Component/JComponentType.h"
#include<d3d12.h>
#include<string>

namespace JinEngine
{
	class JComponent;
	class JMeshGeometry;
	class JMaterial;
	class JCamera;
	class JLight;
	class JRenderItem;
	class JGameObject;

	class JComponentFactoryUtility
	{
	public:
		static JCamera* CreateCamera(const size_t guid, const J_OBJECT_FLAG flag, JGameObject& owner, bool isMainCam);
		static JLight* CreateLight(const size_t guid, const J_OBJECT_FLAG flag, JGameObject& owner, J_LIGHT_TYPE type);
		static JRenderItem* CreateRenderItem(const size_t guid,
			const J_OBJECT_FLAG flag,
			JGameObject& owner,
			JMeshGeometry* mesh,
			JMaterial* mat,
			D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			J_RENDER_LAYER renderLayer = J_RENDER_LAYER::OPAQUE_OBJECT);
		static JComponent* CreateComponent(const std::string& componentName, JGameObject& owner);
	};

	using JCFU = JComponentFactoryUtility;
}