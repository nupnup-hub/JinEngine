#pragma once
#include"../JObjectFlag.h"
#include"../Component/RenderItem/JRenderLayer.h"
#include"../Component/RenderItem/JRenderItemAcceleratorMask.h"
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
		static JUserPtr<JCamera> CreateCamera(const JUserPtr<JGameObject>& owner);
		static JUserPtr<JLight> CreateLight(const JUserPtr<JGameObject>& owner, J_LIGHT_TYPE type);
		static JUserPtr<JRenderItem> CreateRenderItem(const JUserPtr<JGameObject>& owner,
			const JUserPtr<JMeshGeometry>& mesh, 
			const D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			const J_RENDER_LAYER renderLayer = J_RENDER_LAYER::OPAQUE_OBJECT,
			const J_RENDERITEM_ACCELERATOR_MASK spaceSpatialMask = ACCELERATOR_ALLOW_ALL);
		static JUserPtr<JRenderItem>CreateRenderItem(const JUserPtr<JGameObject>& owner,
			const JUserPtr<JMeshGeometry>& mesh,
			std::vector<JUserPtr<JMaterial>>& mat,
			const D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			const J_RENDER_LAYER renderLayer = J_RENDER_LAYER::OPAQUE_OBJECT,
			const J_RENDERITEM_ACCELERATOR_MASK spaceSpatialMask = ACCELERATOR_ALLOW_ALL); 
		static JUserPtr<JComponent> CreateComponent(const Core::JTypeInfo&  typeInfo, const JUserPtr<JGameObject>& owner);
	};

	using JCCI = JComponentCreatorInterface;
}