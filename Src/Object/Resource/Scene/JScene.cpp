#include"JScene.h" 
#include"../Mesh/JMeshGeometry.h" 
#include"../Material/JMaterial.h"  
#include"../JResourceObjectFactory.h"
#include"../../../Core/Guid/GuidCreator.h" 
#include"../../Component/Animator/JAnimator.h"
#include"../../Component/RenderItem/JRenderItem.h"
#include"../../Component/Transform/JTransform.h"
#include"../../Component/Light/JLight.h" 
#include"../../Component/Camera/JCamera.h"  
#include"../../GameObject/JGameObjectFactory.h"
#include"../../GameObject/JGameObjectFactoryUtility.h" 
#include"../../GameObject/JGameObject.h"
#include"../JResourceManager.h" 

#include"../../../Application/JApplicationVariable.h"
#include"../../../Core/DirectXEx/JCullingFrustum.h"
#include"../../../Core/GameTimer/JGameTimer.h"
#include"../../../Core/SpaceSpatial/JSceneSpatialStructure.h" 

#include"../../../Graphic/JGraphicDrawList.h"  
#include"../../../Test/CullingDemonstration.h"
#include<DirectXColors.h>

namespace JinEngine
{  
	bool JScene::IsOpen()const noexcept
	{
		return isOpen;
	}
	bool JScene::IsAnimatorActivated()const noexcept
	{
		return activateAnimator;
	}
	JGameObject* JScene::FindGameObject(const size_t guid)noexcept
	{
		const uint allObjectCount = (uint)allObjects.size();
		for (uint i = 0; i < allObjectCount; ++i)
			if (allObjects[i]->GetGuid() == guid)
				return allObjects[i];
		return nullptr;
	}
	JGameObject* JScene::GetRootGameObject()noexcept
	{
		return root;
	}
	JGameObject* JScene::GetGameObject(const uint index)noexcept
	{
		if (index < allObjects.size())
			return allObjects[index];
		else
			return nullptr;
	}
	JAnimator* JScene::GetAnimator(const uint index)noexcept
	{
		return animators[index];
	}
	JCamera* JScene::GetMainCamera()noexcept
	{
		return mainCamera;
	}
	uint JScene::GetGameObjectCount()const noexcept
	{
		return (uint)allObjects.size();
	}
	uint JScene::GetAnimatorCount()const noexcept
	{
		return (uint)animators.size();
	}
	uint  JScene::GetCameraCount()const noexcept
	{
		return (uint)cameras.size();
	}
	uint JScene::GetRenderItemCount()const noexcept
	{
		return (uint)renderItems.size();
	}
	uint JScene::GetLightCount()const noexcept
	{
		return (uint)light.size();
	}
	uint JScene::GetShadowCount()const noexcept
	{
		return (uint)shadowLight.size();
	}
	J_RESOURCE_TYPE JScene::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	}
	std::string JScene::GetFormat()const noexcept
	{
		return GetAvailableFormat()[0];
	}
	std::vector<std::string> JScene::GetAvailableFormat()noexcept
	{
		static std::vector<std::string> format{ ".scene" };
		return format;
	}
	void JScene::DoActivate() noexcept
	{
		isOpen = true;
		JResourceObject::DoActivate();
		root->Activate();
		const uint objCount = (uint)allObjects.size();
		for (uint i = 0; i < objCount; ++i)
			allObjects[i]->Activate();
	}
	void JScene::DoDeActivate()noexcept
	{
		isOpen = false;
		JResourceObject::DoDeActivate();
		if (spatialStructure != nullptr)
			spatialStructure->Clear();

		root->DeActivate();
		const uint objCount = (uint)allObjects.size();
		for (uint i = 0; i < objCount; ++i)
			allObjects[i]->DeActivate();

		for (int i = 0; i < (int)J_RENDER_LAYER::COUNT; ++i)
		{
			for (int j = 0; j < (int)J_MESHGEOMETRY_TYPE::COUNT; ++j)
			{
				objectLayer[i][j].clear();
				objectLayer[i][j].shrink_to_fit();
			}
		}
		allObjects.clear();
		renderItems.clear();
		animators.clear();
		cameras.clear();
		light.clear();
		shadowLight.clear();

		allObjects.shrink_to_fit();
		renderItems.shrink_to_fit();
		animators.shrink_to_fit();
		cameras.shrink_to_fit();
		light.shrink_to_fit();
		shadowLight.shrink_to_fit();
		 
		const JOBJECT_FLAG rootFlag = (JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_INERASABLE);
		root = JGFI::CreateRoot("RootGameObject", Core::MakeGuid(), rootFlag, *this);
	}
	void JScene::AddGameObject(JGameObject* newGameObject)noexcept
	{
		if (IsActivated())
			newGameObject->Activate();
		allObjects.push_back(newGameObject);
	}
	void JScene::EraseGameObject(JGameObject* gameObj)noexcept
	{
		if (IsActivated())
			gameObj->DeActivate();
		if (spatialStructure != nullptr)
			spatialStructure->EraseGameObject(gameObj->GetGuid());

		const uint allObjCount = (uint)allObjects.size();
		const size_t guid = gameObj->GetGuid();
		for (uint i = 0; i < allObjCount; ++i)
		{
			if (allObjects[i]->GetGuid() == guid)
			{
				allObjects.erase(allObjects.begin() + i);
				break;
			}
		}
	}
	void JScene::MakeDefaultObject(bool isEditorScene)noexcept
	{
		JOBJECT_FLAG flag;
		if (isEditorScene)
			flag = OBJECT_FLAG_EDITOR_OBJECT;
		else
			flag = (JOBJECT_FLAG)(OBJECT_FLAG_HIDDEN | OBJECT_FLAG_INERASABLE | OBJECT_FLAG_UNEDITABLE);

		//only has root
		if (allObjects.size() == 1)
		{
			JGFU::CreateSky(*root, flag, "DefaultSky");
			JGFU::CreateCamera(*root, flag, true, "MainCamera");
			JGFU::CreateLight(*root, OBJECT_FLAG_AUTO_GENERATED, J_LIGHT_TYPE::DIRECTIONAL, "MainLight");
		}
	}
	void JScene::SetAllComponentDirty()noexcept
	{
		SetAllTransformDirty();
		SetAllRenderItemDirty();
		SetAllAnimatorDirty();
		SetAllCameraDirty();
		SetAllLightDirty();
	}
	void JScene::SetAllTransformDirty()noexcept
	{
		const uint gameObjCount = (uint)allObjects.size();
		for (uint i = 0; i < gameObjCount; ++i)
			allObjects[i]->GetTransform()->SetDirty();
	}
	void JScene::SetAllRenderItemDirty()noexcept
	{
		const uint renderItemCount = (uint)renderItems.size();
		for (uint i = 0; i < renderItemCount; ++i)
			renderItems[i]->SetDirty();
	}
	void JScene::SetAllAnimatorDirty()noexcept
	{
		const uint animatorCount = (uint)animators.size();
		for (uint i = 0; i < animatorCount; ++i)
			animators[i]->SetDirty();
	}
	void JScene::SetAllCameraDirty()noexcept
	{
		const uint cameraCount = (uint)cameras.size();
		for (uint i = 0; i < cameraCount; ++i)
			cameras[i]->SetDirty();
	}
	void JScene::SetAllLightDirty()noexcept
	{
		const uint lightCount = (uint)light.size();
		for (uint i = 0; i < lightCount; ++i)
			light[i]->SetDirty();
	}
	bool JScene::RegisterAnimator(JGameObject* gameObject, JAnimator* animator)noexcept
	{
		if (!animator->PassDefectInspection())
			return false;

		const size_t guid = animator->GetGuid();
		const uint animatorCount = (uint)animators.size();

		bool overlap = false;
		for (uint i = 0; i < animatorCount; ++i)
		{
			if (animators[i]->GetGuid() == guid)
			{
				overlap = true;
				break;
			}
		}

		if (!overlap)
		{
			if (activateAnimator)
				animator->OnAnimation();

			animators.push_back(animator);
			animators[animatorCount]->SetAnimationCBIndex(animatorCount);
			Graphic::JGraphicDrawList::UpdateScene(this);
			return true;
		}
		else
			return false;
	}
	bool JScene::RegisterCamera(JGameObject* gameObject, JCamera* camera)noexcept
	{
		if (!camera->PassDefectInspection())
			return false;

		const size_t guid = camera->GetGuid();
		const uint cameraCount = (uint)cameras.size();

		bool overlap = false;
		for (uint i = 0; i < cameraCount; ++i)
		{
			if (cameras[i]->GetGuid() == guid)
			{
				overlap = true;
				break;
			}
		}

		if (!overlap)
		{
			cameras.push_back(camera);
			cameras[cameraCount]->SetCameraCBIndex(cameraCount);
			Graphic::JGraphicDrawList::UpdateScene(this);
			return true;
		}
		else
			return false;
	}
	bool JScene::RegisterLight(JGameObject* gameObject, JLight* newLight)noexcept
	{
		if (!newLight->PassDefectInspection())
			return false;

		const size_t guid = newLight->GetGuid();
		const uint lightCount = (uint)light.size();

		bool overlap = false;
		for (uint i = 0; i < lightCount; ++i)
		{
			if (light[i]->GetGuid() == guid)
			{
				overlap = true;
				break;
			}
		}

		if (!overlap)
		{
			light.push_back(newLight);
			light[lightCount]->SetLightCBIndex(lightCount);
			RegisterShadowLight(gameObject, newLight);
			Graphic::JGraphicDrawList::UpdateScene(this);
			return true;
		}
		else
			return false;
	}
	bool JScene::RegisterShadowLight(JGameObject* gameObject, JLight* light)noexcept
	{
		if (!light->PassDefectInspection() || !light->IsShadowActivated())
			return false;

		const size_t guid = light->GetGuid();
		const uint shadowLightCount = (uint)shadowLight.size();

		bool overlap = false;
		for (uint i = 0; i < shadowLightCount; ++i)
		{
			if (shadowLight[i]->GetGuid() == guid)
			{
				overlap = true;
				break;
			}
		}

		if (!overlap)
		{
			shadowLight.push_back(light);
			shadowLight[shadowLightCount]->SetShadowCBIndex(shadowLightCount);
			Graphic::JGraphicDrawList::UpdateScene(this);
			return true;
		}
		else
			return false;
	}
	bool JScene::RegisterRenderItem(JGameObject* gameObject, JRenderItem* renderItem)noexcept
	{
		if (!renderItem->PassDefectInspection())
			return false;

		const size_t guid = renderItem->GetGuid();
		const uint renderItemCount = (uint)renderItems.size();

		bool overlap = false;
		for (uint i = 0; i < renderItemCount; ++i)
		{
			if (renderItems[i]->GetGuid() == guid)
			{
				overlap = true;
				break;
			}
		}

		if (!overlap)
		{
			const J_MESHGEOMETRY_TYPE meshType = renderItem->GetMesh()->GetMeshGeometryType();
			const J_RENDER_LAYER renderLayer = renderItem->GetRenderLayer();
			renderItems.push_back(renderItem);
			renderItems[renderItemCount]->SetObjCBIndex(renderItemCount);
			objectLayer[(int)renderLayer][(int)meshType].push_back(gameObject);
			Graphic::JGraphicDrawList::UpdateScene(this);
			if (renderLayer == J_RENDER_LAYER::OPAQUE_OBJECT && spatialStructure != nullptr)
				spatialStructure->AddGameObject(gameObject);
			return true;
		}
		else
			return false;
	}
	bool JScene::DeRegisterAnimator(JGameObject* gameObject, JAnimator* animator)noexcept
	{
		bool result = false;
		const size_t ownerGuid = animator->GetOwnerGuid();
		const uint animatorCount = (uint)animators.size();
		uint index;
		for (index = 0; index < animatorCount; ++index)
		{
			if (animators[index]->GetOwnerGuid() == ownerGuid)
			{
				animators.erase(animators.begin() + index);
				result = true;
				break;
			}
		}
		if (result)
		{
			for (; index < animatorCount - 1; ++index)
				animators[index]->SetAnimationCBIndex(index);
			Graphic::JGraphicDrawList::UpdateScene(this);
		}
		return result;
	}
	bool JScene::DeRegisterCamera(JGameObject* gameObject, JCamera* camera)noexcept
	{
		//if (cameras.size() == 1)
		//	return false;
		bool result = false;
		const size_t ownerGuid = camera->GetOwnerGuid();
		const uint cameraCount = (uint)cameras.size();
		uint index;
		for (index = 0; index < cameraCount; ++index)
		{
			if (cameras[index]->GetOwnerGuid() == ownerGuid)
			{
				cameras.erase(cameras.begin() + index);
				result = true;
				break;
			}
		}

		if (result)
		{
			for (; index < cameraCount - 1; ++index)
				cameras[index]->SetCameraCBIndex(index);
			Graphic::JGraphicDrawList::UpdateScene(this);
		}
		return result;
	}
	bool JScene::DeRegisterLight(JGameObject* gameObject, JLight* newLight)noexcept
	{
		//수정필요
		if (light.size() == 1)
			return false;

		bool result = false;
		const size_t ownerGuid = newLight->GetOwnerGuid();
		const uint lightCount = (uint)light.size();
		uint index;
		for (index = 0; index < lightCount; ++index)
		{
			if (light[index]->GetOwnerGuid() == ownerGuid)
			{
				light.erase(light.begin() + index);
				result = true;
				break;
			}
		}

		if (result)
		{
			for (; index < lightCount - 1; ++index)
				light[index]->SetLightCBIndex(index);
			DeRegisterShadowLight(gameObject, newLight);
			Graphic::JGraphicDrawList::UpdateScene(this);
		}
		return false;
	}
	bool JScene::DeRegisterShadowLight(JGameObject* gameObject, JLight* light)noexcept
	{
		if (!light->IsShadowActivated())
			return false;

		bool result = false;
		const size_t ownerGuid = light->GetOwnerGuid();
		const uint shadowLightCount = (uint)shadowLight.size();
		uint index;
		for (index = 0; index < shadowLightCount; ++index)
		{
			if (shadowLight[index]->GetOwnerGuid() == ownerGuid)
			{
				shadowLight.erase(shadowLight.begin() + index);
				result = true;
				break;
			}
		}

		if (result)
		{
			for (; index < shadowLightCount - 1; ++index)
				shadowLight[index]->SetShadowCBIndex(index);
			Graphic::JGraphicDrawList::UpdateScene(this);
		}
		return false;
	}
	bool JScene::DeRegisterRenderItem(JGameObject* gameObject, JRenderItem* renderItem)noexcept
	{
		bool result = false;
		const size_t ownerGuid = renderItem->GetOwnerGuid();
		const uint renderItemCount = (uint)renderItems.size();

		uint i;
		for (i = 0; i < renderItemCount; ++i)
		{
			if (renderItems[i]->GetOwnerGuid() == ownerGuid)
			{
				renderItems.erase(renderItems.begin() + i);
				result = true;
				break;
			}
		}
		if (result)
		{
			for (; i < renderItemCount - 1; ++i)
				renderItems[i]->SetObjCBIndex(i);

			for (uint i = 0; i < (int)J_RENDER_LAYER::COUNT; ++i)
			{
				for (uint j = 0; j < (int)J_MESHGEOMETRY_TYPE::COUNT; ++j)
				{
					const uint layerCount = (uint)objectLayer[i][j].size();
					for (uint k = 0; k < layerCount; ++k)
					{
						if (ownerGuid == objectLayer[i][j][k]->GetGuid())
						{
							objectLayer[i][j].erase(objectLayer[i][j].begin() + k);
							break;
						}
					}
				}
			}
			Graphic::JGraphicDrawList::UpdateScene(this);
			if (renderItem->GetRenderLayer() == J_RENDER_LAYER::OPAQUE_OBJECT && spatialStructure != nullptr)
				spatialStructure->EraseGameObject(gameObject->GetGuid());
		}
		return result;
	}
	JCamera* JScene::SetMainCamera(JGameObject* gameObject, JCamera* camera)noexcept
	{
		if (camera == nullptr)
			return nullptr;
		JCamera* preCam = mainCamera;
		mainCamera = camera;
		return preCam;
	}
	void JScene::UpdateGameObjectTransform(JGameObject* gameObject)noexcept
	{
		if (gameObject != nullptr && gameObject->HasRenderItem() && gameObject->GetRenderItem()->GetRenderLayer() == J_RENDER_LAYER::OPAQUE_OBJECT)
			spatialStructure->UpdateGameObject(gameObject->GetGuid());
	}
	void JScene::ViewCulling()noexcept
	{
		if (spatialStructure != nullptr && mainCamera != nullptr)
		{
			DirectX::BoundingFrustum camFrustum = mainCamera->GetBoundingFrustum();
			DirectX::BoundingFrustum worldCamFrustum;
			camFrustum.Transform(worldCamFrustum, mainCamera->GetTransform()->GetWorld());
			//spatialStructure->Culling(worldCamFrustum);
			Core::JCullingFrustum cFrustum(worldCamFrustum);
			spatialStructure->Culling(cFrustum);
		}
	}
	void JScene::OnSceneSpatialStructure()noexcept
	{
		if (spatialStructure == nullptr)
			spatialStructure = std::make_unique<Core::JSceneSpatialStructure>();
	}
	void JScene::OffSceneSpatialStructure()noexcept
	{
		if (spatialStructure != nullptr)
		{
			spatialStructure->Clear();
			spatialStructure.reset();
		}
	}
	void JScene::OnDebugBoundingBox(bool onlyLeafNode)noexcept
	{
		if (spatialStructure != nullptr)
			spatialStructure->OnDebugBoundingBox(root, onlyLeafNode);
	}
	void JScene::OffDebugBoundingBox()noexcept
	{
		if (spatialStructure != nullptr)
			spatialStructure->OffDebugBoundingBox();
	}
	void JScene::CreateDemoGameObject()noexcept
	{
		CullingDemonstration::OnCullingDemonstration(root, 6, 6, 6, J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE);
	}
	void JScene::EraseDemoGameObject()noexcept
	{
		CullingDemonstration::OffCullingDemonstration();
	}
	void JScene::BuildOctree(const uint octreeSizeSquare, const float looseFactor, const bool isLooseOctree)noexcept
	{
		if (spatialStructure != nullptr)
		{
			const uint staticObjectCount = (uint)objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][0].size();
			const uint skinnedObjectCount = (uint)objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][1].size();
			const uint objectCount = staticObjectCount + skinnedObjectCount;
			std::vector<JGameObject*> gameObject(objectCount);
			std::copy(objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][0].begin(), objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][0].end(), gameObject.begin());
			std::copy(objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][1].begin(), objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][1].end(), gameObject.begin() + staticObjectCount);

			spatialStructure->BuildOctree(gameObject, octreeSizeSquare, looseFactor, isLooseOctree);
		}
	}
	void JScene::BuildBvh(const Core::J_BVH_BUILD_TYPE bvhBuildType, const Core::J_BVH_SPLIT_TYPE splitType)noexcept
	{
		if (spatialStructure != nullptr)
		{
			const uint staticObjectCount = (uint)objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][0].size();
			const uint skinnedObjectCount = (uint)objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][1].size();
			std::vector<JGameObject*> gameObject(staticObjectCount + skinnedObjectCount);
			std::copy(objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][0].begin(), objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][0].end(), gameObject.begin());
			std::copy(objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][1].begin(), objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][1].end(), gameObject.begin() + staticObjectCount);

			spatialStructure->BuildBvh(gameObject, bvhBuildType, splitType);
		}
	}
	void JScene::ActivateAnimtor()noexcept
	{
		activateAnimator = !activateAnimator;
		if (activateAnimator)
		{
			for (auto& animator : animators)
				animator->OnAnimation();
		}
	}
	Core::J_FILE_IO_RESULT JScene::CallStoreResource()
	{
		return StoreObject(this);
	}
	Core::J_FILE_IO_RESULT JScene::StoreObject(JScene* scene)
	{
		if (scene == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (((int)scene->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		if (scene->isOpen)
		{
			std::wofstream stream;
			stream.open(scene->GetMetafilePath(), std::ios::out | std::ios::binary);
			Core::J_FILE_IO_RESULT res = StoreMetadata(stream, scene);
			stream.close();

			if (res != Core::J_FILE_IO_RESULT::SUCCESS)
				return res;
			stream.open(scene->GetWPath(), std::ios::out | std::ios::binary);
			JGameObjectInterface* gI = scene->root;
			res = gI->CallStoreGameObject(stream);
			stream.close();
			return res;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	Core::J_FILE_IO_RESULT JScene::StoreMetadata(std::wofstream& stream, JScene* scene)
	{
		if (stream.is_open())
		{
			Core::J_FILE_IO_RESULT res = JResourceObject::StoreMetadata(stream, scene);
			if (res != Core::J_FILE_IO_RESULT::SUCCESS)
				return res;

			stream << scene->isOpen;
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JScene* JScene::LoadObject(JDirectory* directory, const JResourcePathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;

		if (!JResourceObject::IsResourceFormat<JScene>(pathData.format))
			return nullptr;

		std::wifstream stream;
		stream.open(ConvertMetafilePath(pathData.wstrPath), std::ios::in | std::ios::binary);
		JSceneMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, pathData.folderPath, metadata);
		stream.close();

		JScene* newScene;
		if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
			newScene = new JScene(pathData.name, metadata.guid, metadata.flag, directory, GetFormatIndex<JScene>(pathData.format));
		else
			newScene = new JScene(pathData.name, Core::MakeGuid(), OBJECT_FLAG_NONE, directory, GetFormatIndex<JScene>(pathData.format));

		stream.open(pathData.wstrPath, std::ios::in | std::ios::binary);
		if (stream.is_open())
		{
			if (!metadata.isOpen)
				return newScene;
			else
			{
				if (JGFI::Create(stream, newScene->root) != nullptr)
				{
					newScene->Activate();
					return newScene;
				}
				else
					return nullptr;
			}
		}
		else
			return nullptr;
	}
	Core::J_FILE_IO_RESULT JScene::LoadMetadata(std::wifstream& stream, const std::string& folderPath, JSceneMetadata& metadata)
	{
		if (stream.is_open())
		{
			Core::J_FILE_IO_RESULT res = JResourceObject::LoadMetadata(stream, metadata);
			stream >> metadata.isOpen;
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	void JScene::RegisterFunc()
	{
		auto defaultC = [](JDirectory* owner) ->JResourceObject*
		{
			return new JScene(owner->MakeUniqueFileName(GetDefaultName<JScene>()),
				Core::MakeGuid(),
				OBJECT_FLAG_NONE,
				owner,
				JResourceObject::GetDefaultFormatIndex());
		};
		auto initC = [](const std::string& name, const size_t guid, const JOBJECT_FLAG objFlag, JDirectory* directory, const uint8 formatIndex)-> JResourceObject*
		{
			return  new JScene(name, guid, objFlag, directory, formatIndex);
		};
		auto loadC = [](JDirectory* directory, const JResourcePathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori)->JResourceObject*
		{
			return static_cast<JScene*>(ori)->CopyResource();
		};

		JRFI<JScene>::Register(defaultC, initC, loadC, copyC);

		auto getFormatIndexLam = [](const std::string& format) {return JResourceObject::GetFormatIndex<JScene>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JScene::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JScene::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };
		 
		RegisterTypeInfo(RTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, false },
			RTypeUtil{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable });
	}
	JScene::JScene(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		:JSceneInterface(name, guid, flag, directory, formatIndex), isOpen(false), activateAnimator(false)
	{
		const JOBJECT_FLAG rootFlag = (JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_INERASABLE);
		root = JGFI::CreateRoot("RootGameObject", Core::MakeGuid(), rootFlag, *this);
		spatialStructure = std::make_unique<Core::JSceneSpatialStructure>();
	}
	JScene::~JScene()
	{
	}
}