#include"JComponentCreator.h"  
#include"../../Object/Component/JComponentHint.h"
#include"../../Object/Component/Camera/JCamera.h"
#include"../../Object/Component/Light/JLight.h"
#include"../../Object/Component/RenderItem/JRenderItem.h"
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../Object/Resource/Material/JMaterial.h"
#include"../../Core/Identity/JIdenCreator.h"

namespace JinEngine
{
	JUserPtr<JCamera> JComponentCreatorInterface::CreateCamera(const JUserPtr<JGameObject>& owner)
	{    
		return JICI::Create<JCamera>(owner);
	}
	JUserPtr<JLight> JComponentCreatorInterface::CreateLight(const JUserPtr<JGameObject>& owner, J_LIGHT_TYPE type)
	{ 
		JUserPtr<JLight> newLight = JICI::Create<JLight>(owner);
		newLight->SetLightType(type);
		return newLight;
	}
	JUserPtr<JRenderItem> JComponentCreatorInterface::CreateRenderItem(const JUserPtr<JGameObject>& owner,
		const JUserPtr<JMeshGeometry>& mesh,  
		const D3D12_PRIMITIVE_TOPOLOGY primitiveType,
		const J_RENDER_LAYER renderLayer,
		const J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask)
	{
		if (mesh == nullptr)
			return nullptr;

		//Has dependency into order
		JUserPtr<JRenderItem> newRenderItem = JICI::Create<JRenderItem>(owner);
		newRenderItem->SetPrimitiveType(primitiveType);
		newRenderItem->SetRenderLayer(renderLayer);
		newRenderItem->SetSpaceSpatialMask(spaceSpatialMask);
		newRenderItem->SetMesh(mesh);

		return newRenderItem;
	}
	JUserPtr<JRenderItem> JComponentCreatorInterface::CreateRenderItem(const JUserPtr<JGameObject>& owner,
		const JUserPtr<JMeshGeometry>& mesh,
		std::vector<JUserPtr<JMaterial>>& mat,
		const D3D12_PRIMITIVE_TOPOLOGY primitiveType,
		const J_RENDER_LAYER renderLayer,
		const J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask)
	{
		if (mesh == nullptr)
			return nullptr;

		//Has dependency into order
		JUserPtr<JRenderItem> newRenderItem = JICI::Create<JRenderItem>(owner);
		newRenderItem->SetPrimitiveType(primitiveType);
		newRenderItem->SetRenderLayer(renderLayer);
		newRenderItem->SetSpaceSpatialMask(spaceSpatialMask);
		newRenderItem->SetMesh(mesh);
		 
		const int matCount = (uint)mat.size();
		for (uint i = 0; i < matCount; ++i)
		{
			if (mat[i] != nullptr)
				newRenderItem->SetMaterial(i, mat[i]);
		}
		return newRenderItem;
	}
	JUserPtr<JComponent> JComponentCreatorInterface::CreateComponent(const Core::JTypeInfo& typeInfo, const JUserPtr<JGameObject>& owner)
	{
		if (!typeInfo.IsChildOf<JComponent>())
			return nullptr;

		auto idenUser = JICI::Create(JComponent::CreateInitDIData(CTypeCommonCall::ConvertCompType(typeInfo), typeInfo, owner),
			Core::JIdentifier::PrivateInterface(typeInfo.TypeGuid()));
		return JUserPtr<JComponent>::ConvertChild(std::move(idenUser));
	}
}