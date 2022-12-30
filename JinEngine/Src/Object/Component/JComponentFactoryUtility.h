#pragma once
#include"../JObjectFlag.h"
#include"../Component/RenderItem/JRenderLayer.h"
#include"../Component/RenderItem/JRenderItemSpaceSpatialMask.h"
#include"../Component/Light/JLightType.h"
#include"../Component/JComponentType.h"
#include<d3d12.h>
#include<string>
#include <vector>
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
			const D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			const J_RENDER_LAYER renderLayer = J_RENDER_LAYER::OPAQUE_OBJECT,
			const J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask = SPACE_SPATIAL_ALLOW_ALL);
		static JRenderItem* CreateRenderItem(const size_t guid,
			const J_OBJECT_FLAG flag,
			JGameObject& owner,
			JMeshGeometry* mesh,
			std::vector<JMaterial*> mat,
			const D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			const J_RENDER_LAYER renderLayer = J_RENDER_LAYER::OPAQUE_OBJECT,
			const J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask = SPACE_SPATIAL_ALLOW_ALL);
		static JComponent* CreateComponent(const std::string& componentName, JGameObject& owner);
	};

	using JCFU = JComponentFactoryUtility;
}