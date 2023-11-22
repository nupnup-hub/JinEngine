#include"JComponentCreator.h"  
#include"../../Object/Component/JComponentHint.h"
#include"../../Object/Component/Camera/JCamera.h"
#include"../../Object/Component/Light/JLight.h"
#include"../../Object/Component/Light/JDirectionalLight.h"
#include"../../Object/Component/Light/JPointLight.h"
#include"../../Object/Component/Light/JSpotLight.h"
#include"../../Object/Component/Light/JRectLight.h"
#include"../../Object/Component/RenderItem/JRenderItem.h"
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../Object/Resource/Material/JMaterial.h"
#include"../../Core/Identity/JIdenCreator.h"
#include"../../Core/Guid/JGuidCreator.h"

namespace JinEngine
{
	JUserPtr<JCamera> JComponentCreatorInterface::CreateCamera(const JUserPtr<JGameObject>& owner)
	{    
		return JICI::Create<JCamera>(owner);
	}
	JUserPtr<JLight> JComponentCreatorInterface::CreateLight(const JUserPtr<JGameObject>& owner, J_LIGHT_TYPE type)
	{ 
		if (type == J_LIGHT_TYPE::DIRECTIONAL)
			return JICI::Create<JDirectionalLight>(owner);
		else if (type == J_LIGHT_TYPE::POINT)
			return JICI::Create<JPointLight>(owner);
		else if (type == J_LIGHT_TYPE::SPOT)
			return JICI::Create<JSpotLight>(owner);
		else if (type == J_LIGHT_TYPE::RECT)
			return JICI::Create<JRectLight>(owner);
		else
			return nullptr;
	}
	JUserPtr<JRenderItem> JComponentCreatorInterface::CreateRenderItem(const JUserPtr<JGameObject>& owner,
		const JUserPtr<JMeshGeometry>& mesh,  
		const J_RENDER_PRIMITIVE primitiveType,
		const J_RENDER_LAYER renderLayer,
		const J_RENDERITEM_ACCELERATOR_MASK acceleratorMask)
	{
		if (mesh == nullptr)
			return nullptr;
 
		//Has dependency into order
		JUserPtr<JRenderItem> newRenderItem = JICI::Create<JRenderItem>(Core::MakeGuid(),
			OBJECT_FLAG_NONE, 
			owner, 
			renderLayer,
			acceleratorMask);
		newRenderItem->SetPrimitiveType(primitiveType); 
		newRenderItem->SetMesh(mesh);
		return newRenderItem;
	}
	JUserPtr<JRenderItem> JComponentCreatorInterface::CreateRenderItem(const JUserPtr<JGameObject>& owner,
		const JUserPtr<JMeshGeometry>& mesh,
		std::vector<JUserPtr<JMaterial>>& mat,
		const J_RENDER_PRIMITIVE primitiveType,
		const J_RENDER_LAYER renderLayer,
		const J_RENDERITEM_ACCELERATOR_MASK acceleratorMask)
	{
		if (mesh == nullptr)
			return nullptr;

		//Has dependency into order
		JUserPtr<JRenderItem> newRenderItem = JICI::Create<JRenderItem>(Core::MakeGuid(),
			OBJECT_FLAG_NONE,
			owner,
			renderLayer,
			acceleratorMask);
		 
		newRenderItem->SetPrimitiveType(primitiveType); 
		newRenderItem->SetMesh(mesh);
		 
		const uint matCount = (uint)mat.size();
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