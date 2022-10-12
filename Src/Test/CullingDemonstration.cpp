#include"CullingDemonstration.h" 
#include"../Object/Component/Transform/JTransform.h"
#include"../Object/Component/RenderItem/JRenderItem.h"
#include"../Object/GameObject/JGameObject.h" 
#include"../Object/GameObject/JGameObjectFactoryUtility.h"

namespace JinEngine
{
	bool CullingDemonstration::isActivated;
	std::vector<JGameObject*> CullingDemonstration::gameObjectVec;

	void CullingDemonstration::OnCullingDemonstration(JGameObject* parent, const uint x, const uint y, const uint z,  const J_DEFAULT_SHAPE DefaultShapeType)
	{
		if (isActivated)
			OffCullingDemonstration();

		isActivated = true;
		uint objectCount = x * y * z; 

		gameObjectVec.resize(objectCount);
		for (uint i = 0; i < objectCount; ++i)
		{
			gameObjectVec[i] = JGFU::CreateDebugGameObject(*parent, OBJECT_FLAG_EDITOR_OBJECT, DefaultShapeType, J_DEFAULT_MATERIAL::DEFAULT_STANDARD);
			gameObjectVec[i]->SetName(std::to_wstring(i) + L"CullingDemoObj");
		}
		DirectX::BoundingBox boundingBox = gameObjectVec[0]->GetRenderItem()->GetBoundingBox();
		float xPad = boundingBox.Extents.x + 2;
		float yPad = boundingBox.Extents.y + 2;
		float zPad = boundingBox.Extents.z + 2;

		uint count = 0;
		for (uint i = 0; i < x; ++i)
		{
			for (uint j = 0; j < y; ++j)
			{
				for (uint k = 0; k < z; ++k)
				{
					JTransform* transform = gameObjectVec[count]->GetTransform();
					transform->SetPosition(DirectX::XMFLOAT3((i + 1) * xPad, (j + 1) * yPad, (k + 1) * zPad));
					++count;
				}
			}
		}
	}
	void CullingDemonstration::OffCullingDemonstration()
	{
		const uint objCount = (uint)gameObjectVec.size();
		for (uint i = 0; i < objCount; ++i)
		{
			gameObjectVec[i]->BeginDestroy();
			gameObjectVec[i] = nullptr;
		}
		gameObjectVec.clear();
		isActivated = false;
	}

}