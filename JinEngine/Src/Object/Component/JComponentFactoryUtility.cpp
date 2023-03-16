#include"JComponentFactoryUtility.h" 
#include"JComponentFactory.h"
#include"../../Object/Component/Camera/JCamera.h"
#include"../../Object/Component/Light/JLight.h"
#include"../../Object/Component/RenderItem/JRenderItem.h"
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/Mesh/JMeshGeometry.h"

namespace JinEngine
{
	JCamera* JComponentFactoryUtility::CreateCamera(const size_t guid, JGameObject& owner, bool isMainCam)
	{   
		JCamera* newCamera = JCFI<JCamera>::Create(guid, owner);
		if (isMainCam)
			newCamera->SetMainCamera(true);
		return newCamera;
	}
	JLight* JComponentFactoryUtility::CreateLight(const size_t guid, JGameObject& owner, J_LIGHT_TYPE type)
	{ 
		JLight* newLight = JCFI<JLight>::Create(guid, owner);
		newLight->SetLightType(type);
		return newLight;
	}
	JRenderItem* JComponentFactoryUtility::CreateRenderItem(const size_t guid,
		JGameObject& owner,
		JMeshGeometry* mesh,  
		const D3D12_PRIMITIVE_TOPOLOGY primitiveType,
		const J_RENDER_LAYER renderLayer,
		const J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask)
	{
		if (mesh == nullptr)
			return nullptr;

		//Has dependency into order
		JRenderItem* newRenderItem = JCFI<JRenderItem>::Create(guid, owner);
		newRenderItem->SetPrimitiveType(primitiveType);
		newRenderItem->SetRenderLayer(renderLayer);
		newRenderItem->SetSpaceSpatialMask(spaceSpatialMask);
		newRenderItem->SetMesh(Core::GetUserPtr(mesh));

		return newRenderItem;
	}
	JRenderItem* JComponentFactoryUtility::CreateRenderItem(const size_t guid,
		JGameObject& owner,
		JMeshGeometry* mesh,
		std::vector<JMaterial*> mat,
		const D3D12_PRIMITIVE_TOPOLOGY primitiveType,
		const J_RENDER_LAYER renderLayer,
		const J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask)
	{
		if (mesh == nullptr)
			return nullptr;

		//Has dependency into order
		JRenderItem* newRenderItem = JCFI<JRenderItem>::Create(guid, owner);
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
	JComponent* JComponentFactoryUtility::CreateComponent(const std::string& componentName, JGameObject& owner)
	{ 
		return JCFI<JComponent>::CreateByName(componentName, owner);
	}
}