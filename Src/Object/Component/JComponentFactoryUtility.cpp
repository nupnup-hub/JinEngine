#include"JComponentFactoryUtility.h" 
#include"JComponentFactory.h"
#include"../../Object/Component/Camera/JCamera.h"
#include"../../Object/Component/Light/JLight.h"
#include"../../Object/Component/RenderItem/JRenderItem.h"
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/Mesh/JMeshGeometry.h"

namespace JinEngine
{
	JCamera* JComponentFactoryUtility::CreateCamera(const size_t guid, const J_OBJECT_FLAG flag, JGameObject& owner, bool isMainCam)
	{   
		JCamera* newCamera = JCFI<JCamera>::Create(guid, flag, owner);
		if (isMainCam)
			newCamera->SetMainCamera(true);
		return newCamera;
	}
	JLight* JComponentFactoryUtility::CreateLight(const size_t guid, const J_OBJECT_FLAG flag, JGameObject& owner, J_LIGHT_TYPE type)
	{ 
		JLight* newLight = JCFI<JLight>::Create(guid, flag, owner);
		newLight->SetLightType(type);
		return newLight;
	}
	JRenderItem* JComponentFactoryUtility::CreateRenderItem(const size_t guid,
		const J_OBJECT_FLAG flag,
		JGameObject& owner,
		JMeshGeometry* mesh,  
		D3D12_PRIMITIVE_TOPOLOGY primitiveType,
		J_RENDER_LAYER renderLayer)
	{
		if (mesh == nullptr)
			return nullptr;

		JRenderItem* newRenderItem = JCFI<JRenderItem>::Create(guid, flag, owner);
		newRenderItem->SetMeshGeometry(mesh);
		newRenderItem->SetPrimitiveType(primitiveType);
		newRenderItem->SetRenderLayer(renderLayer); 

		return newRenderItem;
	}
	JRenderItem* JComponentFactoryUtility::CreateRenderItem(const size_t guid,
		const J_OBJECT_FLAG flag,
		JGameObject& owner,
		JMeshGeometry* mesh,
		std::vector<JMaterial*> mat,
		D3D12_PRIMITIVE_TOPOLOGY primitiveType,
		J_RENDER_LAYER renderLayer)
	{
		if (mesh == nullptr)
			return nullptr;

		JRenderItem* newRenderItem = JCFI<JRenderItem>::Create(guid, flag, owner);
		newRenderItem->SetMeshGeometry(mesh);
		newRenderItem->SetPrimitiveType(primitiveType);
		newRenderItem->SetRenderLayer(renderLayer);
		 
		const int matCount = (uint)mat.size();
		for (uint i = 0; i < matCount; ++i)
		{
			if (mat[i] != nullptr)
				newRenderItem->SetMaterial(i, mat[i]);
		}
		return newRenderItem;
	}
	JComponent* JComponentFactoryUtility::CreateComponent(const std::string& componentName, JGameObject& owner)
	{ 
		return JCFI<JComponent>::CreateByName(componentName, owner);
	}
}