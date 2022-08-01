#include"JComponentFactoryUtility.h" 
#include"JComponentFactory.h"
#include"../../Object/Component/Camera/JCamera.h"
#include"../../Object/Component/Light/JLight.h"
#include"../../Object/Component/RenderItem/JRenderItem.h"
#include"../../Object/Resource/JResourceManager.h"

namespace JinEngine
{
	JCamera* JComponentFactoryUtility::CreateCamera(const size_t guid, const JOBJECT_FLAG flag, JGameObject& owner, bool isMainCam)
	{   
		JCamera* newCamera = JCFI<JCamera>::Create(guid, flag, owner);
		if (isMainCam)
			newCamera->SetMainCamera();
		return newCamera;
	}
	JLight* JComponentFactoryUtility::CreateLight(const size_t guid, const JOBJECT_FLAG flag, JGameObject& owner, J_LIGHT_TYPE type)
	{ 
		JLight* newLight = JCFI<JLight>::Create(guid, flag, owner);
		newLight->SetLightType(type);
		return newLight;
	}
	JRenderItem* JComponentFactoryUtility::CreateRenderItem(const size_t guid,
		const JOBJECT_FLAG flag,
		JGameObject& owner,
		JMeshGeometry* mesh,
		JMaterial* mat,
		D3D12_PRIMITIVE_TOPOLOGY primitiveType,
		J_RENDER_LAYER renderLayer)
	{
		if (mesh == nullptr)
			return nullptr;

		JRenderItem* newRenderItem = JCFI<JRenderItem>::Create(guid, flag, owner);
		if (mesh != nullptr)
			newRenderItem->SetMeshGeometry(mesh);
		if (mat != nullptr)
			newRenderItem->SetMaterial(mat);
		newRenderItem->SetPrimitiveType(primitiveType);
		newRenderItem->SetRenderLayer(renderLayer);

		return newRenderItem;
	}
	JComponent* JComponentFactoryUtility::CreateComponent(const std::string& componentName, JGameObject& owner)
	{ 
		return JCFI<JComponent>::CreateByName(componentName, owner);
	}
}