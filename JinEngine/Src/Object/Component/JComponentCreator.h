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

	class JComponentCreatorInterface
	{
	public:
		static JCamera* CreateCamera(JGameObject* owner, bool isMainCam);
		static JLight* CreateLight(JGameObject* owner, J_LIGHT_TYPE type);
		static JRenderItem* CreateRenderItem(JGameObject* owner,
			JMeshGeometry* mesh, 
			const D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			const J_RENDER_LAYER renderLayer = J_RENDER_LAYER::OPAQUE_OBJECT,
			const J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask = SPACE_SPATIAL_ALLOW_ALL);
		static JRenderItem* CreateRenderItem(JGameObject* owner,
			JMeshGeometry* mesh,
			std::vector<JMaterial*> mat,
			const D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			const J_RENDER_LAYER renderLayer = J_RENDER_LAYER::OPAQUE_OBJECT,
			const J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask = SPACE_SPATIAL_ALLOW_ALL);
		static JComponent* CreateComponent(const J_COMPONENT_TYPE cType, JGameObject* owner); 
		static JComponent* CreateComponent(const Core::JTypeInfo&  typeInfo, JGameObject* owner);
	};

	using JCCI = JComponentCreatorInterface;
}