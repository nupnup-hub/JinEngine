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
	JCamera* JComponentCreatorInterface::CreateCamera(JGameObject* owner, bool isMainCam)
	{   
		JCamera* newCamera = JICI::Create<JCamera>(owner);
		if (isMainCam)
			newCamera->SetMainCamera(true);
		return newCamera;
	}
	JLight* JComponentCreatorInterface::CreateLight(JGameObject* owner, J_LIGHT_TYPE type)
	{ 
		JLight* newLight = JICI::Create<JLight>(owner);
		newLight->SetLightType(type);
		return newLight;
	}
	JRenderItem* JComponentCreatorInterface::CreateRenderItem(JGameObject* owner,
		JMeshGeometry* mesh,  
		const D3D12_PRIMITIVE_TOPOLOGY primitiveType,
		const J_RENDER_LAYER renderLayer,
		const J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask)
	{
		if (mesh == nullptr)
			return nullptr;

		//Has dependency into order
		JRenderItem* newRenderItem = JICI::Create<JRenderItem>(owner);
		newRenderItem->SetPrimitiveType(primitiveType);
		newRenderItem->SetRenderLayer(renderLayer);
		newRenderItem->SetSpaceSpatialMask(spaceSpatialMask);
		newRenderItem->SetMesh(Core::GetUserPtr(mesh));

		return newRenderItem;
	}
	JRenderItem* JComponentCreatorInterface::CreateRenderItem(JGameObject* owner,
		JMeshGeometry* mesh,
		std::vector<JMaterial*> mat,
		const D3D12_PRIMITIVE_TOPOLOGY primitiveType,
		const J_RENDER_LAYER renderLayer,
		const J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask)
	{
		if (mesh == nullptr)
			return nullptr;

		//Has dependency into order
		JRenderItem* newRenderItem = JICI::Create<JRenderItem>(owner);
		newRenderItem->SetPrimitiveType(primitiveType);
		newRenderItem->SetRenderLayer(renderLayer);
		newRenderItem->SetSpaceSpatialMask(spaceSpatialMask);
		newRenderItem->SetMesh(Core::GetUserPtr(mesh));
		 
		const int matCount = (uint)mat.size();
		for (uint i = 0; i < matCount; ++i)
		{
			if (mat[i] != nullptr)
				newRenderItem->SetMaterial(i, Core::GetUserPtr(mat[i]));
		}
		return newRenderItem;
	}
	JComponent* JComponentCreatorInterface::CreateComponent(const J_COMPONENT_TYPE cType, JGameObject* owner)
	{
		auto& typeInfo = CTypeCommonCall::CallGetTypeInfo(cType);
		auto rawPtr = JICI::Create(JComponent::CreateInitDIData(cType, owner), Core::JIdentifier::GetPrivateInterface(typeInfo.TypeGuid()));
		return static_cast<JComponent*>(rawPtr.Get());
	}
	JComponent* JComponentCreatorInterface::CreateComponent(const Core::JTypeInfo& typeInfo, JGameObject* owner)
	{
		if (!typeInfo.IsChildOf<JComponent>())
			return nullptr;

		auto rawPtr = JICI::Create(JComponent::CreateInitDIData(CTypeCommonCall::ConvertCompType(typeInfo), owner),
			Core::JIdentifier::GetPrivateInterface(typeInfo.TypeGuid()));
		return static_cast<JComponent*>(rawPtr.Get());
	}
}