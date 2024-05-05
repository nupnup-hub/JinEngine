#pragma once
#include"../JObjectFlag.h"
#include"../Component/RenderItem/JRenderLayer.h"
#include"../Component/RenderItem/JRenderItemPrimitive.h"
#include"../Component/RenderItem/JRenderItemAcceleratorMask.h"
#include"../Component/Light/JLightType.h"
#include"../Component/JComponentType.h" 

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
			const J_RENDER_PRIMITIVE primitiveType = J_RENDER_PRIMITIVE::TRIANGLE,
			const J_RENDER_LAYER renderLayer = J_RENDER_LAYER::OPAQUE_OBJECT,
			const J_RENDERITEM_ACCELERATOR_MASK acceleratorMask = ACCELERATOR_ALLOW_ALL);
		static JUserPtr<JRenderItem>CreateRenderItem(const JUserPtr<JGameObject>& owner,
			const JUserPtr<JMeshGeometry>& mesh,
			std::vector<JUserPtr<JMaterial>>& mat,
			const J_RENDER_PRIMITIVE primitiveType = J_RENDER_PRIMITIVE::TRIANGLE,
			const J_RENDER_LAYER renderLayer = J_RENDER_LAYER::OPAQUE_OBJECT,
			const J_RENDERITEM_ACCELERATOR_MASK acceleratorMask = ACCELERATOR_ALLOW_ALL); 
		static JUserPtr<JComponent> CreateComponent(const Core::JTypeInfo&  typeInfo, const JUserPtr<JGameObject>& owner);
	};

	using JCCI = JComponentCreatorInterface;
}