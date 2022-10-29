#include"JScene.h"
#include"JSceneManager.h"
#include"../Mesh/JMeshGeometry.h" 
#include"../Material/JMaterial.h"  
#include"../JResourceObjectFactory.h"
#include"../JResourceManager.h" 

#include"../../JFrameUpdate.h"
#include"../../Component/Animator/JAnimator.h"
#include"../../Component/RenderItem/JRenderItem.h"
#include"../../Component/Transform/JTransform.h"
#include"../../Component/Light/JLight.h" 
#include"../../Component/Camera/JCamera.h"  
#include"../../GameObject/JGameObjectFactory.h"
#include"../../GameObject/JGameObjectFactoryUtility.h" 
#include"../../GameObject/JGameObject.h"
#include"../../Directory/JDirectory.h"

#include"../../../Application/JApplicationVariable.h"
#include"../../../Core/Guid/GuidCreator.h" 
#include"../../../Core/Time/JGameTimer.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/DirectXEx/JCullingFrustum.h"
#include"../../../Core/SpaceSpatial/JSceneSpatialStructure.h"  
#include"../../../Utility/JCommonUtility.h"

#include"../../../Test/CullingDemonstration.h"
#include<DirectXColors.h>

namespace JinEngine
{
	static const std::vector<JComponent*> emptyVec;

	JScene::JSceneInitData::JSceneInitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		JDirectory* directory,
		const uint8 formatIndex)
		:JResourceInitData(name, guid, flag, directory, formatIndex)
	{}
	JScene::JSceneInitData::JSceneInitData(const std::wstring& name, JDirectory* directory, const uint8 formatIndex)
		: JResourceInitData(name, directory, formatIndex)
	{}
	JScene::JSceneInitData::JSceneInitData(JDirectory* directory, const uint8 formatIndex)
		: JResourceInitData(JResourceObject::GetDefaultName<JScene>(), directory, formatIndex)
	{}
	J_RESOURCE_TYPE JScene::JSceneInitData::GetResourceType() const noexcept
	{
		return J_RESOURCE_TYPE::SCENE;
	}

	J_RESOURCE_TYPE JScene::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	}
	std::wstring JScene::GetFormat()const noexcept
	{
		return GetAvailableFormat()[0];
	}
	std::vector<std::wstring> JScene::GetAvailableFormat()noexcept
	{
		static std::vector<std::wstring> format{ L".scene" };
		return format;
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
	JCamera* JScene::GetMainCamera()noexcept
	{
		return mainCamera;
	}
	uint JScene::GetGameObjectCount()const noexcept
	{
		return (uint)allObjects.size();
	}
	uint JScene::GetComponetCount(const J_COMPONENT_TYPE cType)const noexcept
	{
		return (uint)GetComponentCashVec(cType).size();
	}
	uint JScene::GetMeshCount()const noexcept
	{
		uint sum = 0;
		const std::vector<JComponent*>& rVec = GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
		for (auto& data : rVec)
			sum += static_cast<JRenderItem*>(data)->GetSubmeshCount();
		return sum;
	}
	bool JScene::IsAnimatorActivated()const noexcept
	{
		return isAnimatorActivated;
	}
	bool JScene::IsMainScene()const noexcept
	{
		return JSceneManager::Instance().IsMainScene(this);
	}
	bool JScene::HasComponent(const J_COMPONENT_TYPE cType)const noexcept
	{
		return componentCash.find(cType) != componentCash.end();
	}
	JSceneCashInterface* JScene::CashInterface()
	{
		return this;
	}
	JSceneGameObjInterface* JScene::GameObjInterface()
	{
		return this;
	}
	JSceneCompInterface* JScene::CompInterface()
	{
		return this;
	}
	JSceneRegisterInterface* JScene::RegisterInterface()
	{
		return this;
	}
	JSceneFrameInterface* JScene::AppInterface()
	{
		return this;
	}
	JSceneSpaceSpatialInterface* JScene::SpaceSpatialInterface()
	{
		return this;
	}
	void JScene::DoCopy(JObject* ori)
	{
		JScene* oriS = static_cast<JScene*>(ori);
		root->Copy(oriS->GetRootGameObject());
		StoreObject(this);
		ClearResource();
		StuffResource();
	}
	void JScene::DoActivate() noexcept
	{
		JResourceObject::DoActivate();
		StuffResource();
		if (root != nullptr)
			root->Activate();
		const uint objCount = (uint)allObjects.size();
		for (uint i = 0; i < objCount; ++i)
			allObjects[i]->Activate();

		SetAllComponentDirty();
		if (isSpatialStructureActivated)
			BuildSpatialStructure(spatialStructure->GetSpatialStructureType());
	}
	void JScene::DoDeActivate()noexcept
	{
		if (isSpatialStructureActivated)
			ClearSpatialStructure();

		StoreObject(this);
		root->DeActivate();
		const uint objCount = (uint)allObjects.size();
		for (uint i = 0; i < objCount; ++i)
			allObjects[i]->DeActivate();
		ClearResource();
		JResourceObject::DoDeActivate();
	}
	void JScene::StuffResource()
	{
		if (!IsValid())
		{
			if (!HasFlag(OBJECT_FLAG_DO_NOT_SAVE))
			{
				std::wifstream stream;
				stream.open(GetPath(), std::ios::in | std::ios::binary);
				if (stream.is_open())
				{
					root = JGFI::Create(stream, nullptr, this);
					stream.close();
				}
				else
					CreateDefaultGameObject();
			}
			else
				CreateDefaultGameObject();
			SetValid(true);
		}
	}
	void JScene::ClearResource()
	{
		if (IsValid())
		{
			root->BegineForcedDestroy();
			root = nullptr;
			for (int i = 0; i < (int)J_RENDER_LAYER::COUNT; ++i)
			{
				for (int j = 0; j < (int)J_MESHGEOMETRY_TYPE::COUNT; ++j)
				{
					objectLayer[i][j].clear();
					objectLayer[i][j].shrink_to_fit();
				}
			}
			componentCash.clear();
			allObjects.clear();
			allObjects.shrink_to_fit();
			SetValid(false);
		}
	}
	void JScene::CreateDefaultGameObject()noexcept
	{
		const J_OBJECT_FLAG rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE);
		root = JGFI::CreateRoot(L"RootGameObject", Core::MakeGuid(), rootFlag, *this);

		J_OBJECT_FLAG flag = OBJECT_FLAG_NONE;
		if (GetFlag() == OBJECT_FLAG_EDITOR_OBJECT)
			flag = OBJECT_FLAG_EDITOR_OBJECT;
		else if (GetFlag() == OBJECT_FLAG_UNIQUE_EDITOR_OBJECT)
			flag = OBJECT_FLAG_UNIQUE_EDITOR_OBJECT;
		else
			flag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE);

		if (!HasFlag(OBJECT_FLAG_EDITOR_OBJECT))
			JGFU::CreateSky(*root, flag, L"DefaultSky");
		JGFU::CreateCamera(*root, flag, true, L"MainCamera");
		JGFU::CreateLight(*root, flag, J_LIGHT_TYPE::DIRECTIONAL, L"MainLight");
	}
	const std::vector<JGameObject*>& JScene::GetGameObjectCashVec(const J_RENDER_LAYER rLayer, const J_MESHGEOMETRY_TYPE meshType)const noexcept
	{
		return objectLayer[(int)rLayer][(int)meshType];
	}
	const std::vector<JComponent*>& JScene::GetComponentCashVec(const J_COMPONENT_TYPE cType)const noexcept
	{
		auto vec = componentCash.find(cType);
		if (vec == componentCash.end())
			return emptyVec;
		else
			return vec->second;
	}
	bool JScene::AddGameObject(JGameObject& newGameObject)noexcept
	{
		if (IsActivated())
			newGameObject.Activate();
		allObjects.push_back(&newGameObject);
		return true;
	}
	bool JScene::RemoveGameObject(JGameObject& gameObj)noexcept
	{
		if (gameObj.IsActivated())
			gameObj.DeActivate();

		if (spatialStructure != nullptr)
			spatialStructure->RemoveGameObject(&gameObj);

		const uint allObjCount = (uint)allObjects.size();
		const size_t guid = gameObj.GetGuid();
		for (uint i = 0; i < allObjCount; ++i)
		{
			if (allObjects[i]->GetGuid() == guid)
			{
				allObjects.erase(allObjects.begin() + i);
				return true;
			}
		}
		return false;
	}
	void JScene::SetAnimation()noexcept
	{
		isAnimatorActivated = !isAnimatorActivated;
		if (isAnimatorActivated)
		{
			const std::vector<JComponent*>& cashVec = GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
			const uint compCount = (uint)cashVec.size();
			for (uint i = 0; i < compCount; ++i)
				static_cast<JAnimator*>(cashVec[i])->OnAnimation();
		}
	}
	void JScene::SetMainCamera(JCamera* mainCam)noexcept
	{
		mainCamera = mainCam;
	}
	void JScene::UpdateTransform(JGameObject* owner)noexcept
	{
		// && owner->GetRenderItem()->GetRenderLayer() != J_RENDER_LAYER::DEBUG_LAYER
		if (owner == nullptr)
			return;
		if (owner->HasRenderItem() && IsActivated())
			spatialStructure->UpdateGameObject(owner);
	}
	bool JScene::RegisterComponent(JComponent& component)noexcept
	{
		if (!component.PassDefectInspection())
			return false;

		const J_COMPONENT_TYPE compType = component.GetComponentType();
		auto cashVec = componentCash.find(compType);
		if (cashVec == componentCash.end())
		{
			componentCash.emplace(compType, std::vector<JComponent*>());
			cashVec = componentCash.find(compType);
		}

		const uint compCount = (uint)cashVec->second.size();
		const size_t guid = component.GetGuid();
		for (uint i = 0; i < compCount; ++i)
		{
			if (cashVec->second[i]->GetGuid() == guid)
				return false;
		}

		JSceneManager::Instance().UpdateScene(this, compType);
		cashVec->second.push_back(&component);

		if (compType == J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM)
		{
			JRenderItem* jRItem = static_cast<JRenderItem*>(&component);
			const J_RENDER_LAYER renderLayer = jRItem->GetRenderLayer();
			const J_MESHGEOMETRY_TYPE meshType = jRItem->GetMesh()->GetMeshGeometryType();
			objectLayer[(int)renderLayer][(int)meshType].push_back(jRItem->GetOwner());

			CallSetFrameBuffOffset(*jRItem, cashVec->second.size() - 1);
			if (renderLayer == J_RENDER_LAYER::OPAQUE_OBJECT && spatialStructure->IsSpatialStructureActivated())
				spatialStructure->AddGameObject(jRItem->GetOwner());
		}

		return true;
	}
	bool JScene::DeRegisterComponent(JComponent& component)noexcept
	{
		const J_COMPONENT_TYPE compType = component.GetComponentType();
		auto cashData = componentCash.find(compType);
		if (cashData == componentCash.end())
			return false;

		std::vector<JComponent*>& cashVec = cashData->second;
		const uint compCount = (uint)cashVec.size();
		const size_t guid = component.GetGuid();
		for (uint i = 0; i < compCount; ++i)
		{
			if (cashVec[i]->GetGuid() == guid)
			{
				if (compType == J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM)
				{
					JRenderItem* jRItem = static_cast<JRenderItem*>(&component);
					JGameObject* jOwner = jRItem->GetOwner();

					for (uint j = i + 1; j < compCount; ++j)
					{
						JRenderItem* backRItem = static_cast<JRenderItem*>(cashVec[j]);
						CallSetFrameBuffOffset(*backRItem, CallGetFrameBuffOffset(*backRItem) - 1);
					}

					const int rIndex = (int)jRItem->GetRenderLayer();
					const int mIndex = (int)jRItem->GetMesh()->GetMeshGeometryType();

					const size_t guid = jOwner->GetGuid();
					const uint vecCount = (uint)objectLayer[rIndex][mIndex].size();

					for (uint j = 0; j < vecCount; ++j)
					{
						if (guid == objectLayer[rIndex][mIndex][j]->GetGuid())
						{
							objectLayer[rIndex][mIndex].erase(objectLayer[rIndex][mIndex].begin() + j);
							break;
						}
					}
					//objectLayer[rIndex][mIndex].push_back(jOwner);
					if (jRItem->GetRenderLayer() == J_RENDER_LAYER::OPAQUE_OBJECT && spatialStructure->IsSpatialStructureActivated())
						spatialStructure->RemoveGameObject(jOwner);
				}
				SetBackSideComponentDirty(*cashVec[i]);
				cashVec.erase(cashVec.begin() + i);
				JSceneManager::Instance().UpdateScene(this, compType);
				return true;
			}
		}
		return false;
	}
	void JScene::SetAllComponentDirty()noexcept
	{
		for (auto& data : componentCash)
			SetComponentDirty(data.first);
	}
	void JScene::SetComponentDirty(const J_COMPONENT_TYPE cType)noexcept
	{
		if (JCI::GetCTypeHint(cType).isFrameInterface)
		{
			auto SetFrameDirtyCallable = JCI::GetSetFrameDirtyCallable(cType);
			const std::vector<JComponent*>& cashVec = GetComponentCashVec(cType);
			const uint compCount = (uint)cashVec.size();
			for (uint i = 0; i < compCount; ++i)
				SetFrameDirtyCallable(nullptr, *cashVec[i]);
		}
	}
	void JScene::SetBackSideComponentDirty(JComponent& jComp) noexcept
	{
		const J_COMPONENT_TYPE cType = jComp.GetComponentType();
		const std::vector<JComponent*>& cashVec = GetComponentCashVec(cType);
		const size_t guid = jComp.GetGuid();
		const uint compCount = (uint)cashVec.size();
		uint i = 0;
		for (; i < compCount; ++i)
		{
			if (guid == cashVec[i]->GetGuid())
			{
				++i;
				break;
			}
		}
		for (; i < compCount; ++i)
		{
			auto SetFrameDirtyCallable = JCI::GetSetFrameDirtyCallable(cType);
			SetFrameDirtyCallable(nullptr, *cashVec[i]);
		}
	}
	void JScene::SetBackSideComponentDirty(JComponent& jComp, bool(*condition)(JComponent&))noexcept
	{
		const J_COMPONENT_TYPE cType = jComp.GetComponentType();
		const std::vector<JComponent*>& cashVec = GetComponentCashVec(cType);
		const size_t guid = jComp.GetGuid();
		const uint compCount = (uint)cashVec.size();
		uint i = 0;
		for (; i < compCount; ++i)
		{
			if (guid == cashVec[i]->GetGuid())
			{
				++i;
				break;
			}
		}
		for (; i < compCount; ++i)
		{
			auto SetFrameDirtyCallable = JCI::GetSetFrameDirtyCallable(cType);
			if (condition(*cashVec[i]))
				SetFrameDirtyCallable(nullptr, *cashVec[i]);
		}
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
	Core::JSceneSpatialStructureOption JScene::GetSpatialStructureOption()const noexcept
	{
		return spatialStructure->GetOption();
	}
	void JScene::SetSceneSpatialStructure(const bool value)noexcept
	{ 
		isSpatialStructureActivated = value;
		if (value)
		{
			if (IsActivated())
				BuildSpatialStructure(spatialStructure->GetSpatialStructureType());
		}
		else
		{
			if (IsActivated())
				ClearSpatialStructure();
		}
	}
	void JScene::SetSceneSpatialStructureType(const Core::J_SCENE_SPATIAL_STRUCTURE_TYPE type)noexcept
	{
		if (spatialStructure->IsSpatialStructureActivated())
		{
			if (spatialStructure->GetSpatialStructureType() != type)
			{
				ClearSpatialStructure();
				BuildSpatialStructure(type);
			}
		}
		else
			spatialStructure->SetSpatialStructureType(type);
	}
	void JScene::SetDebugBoundingBox(const bool value)noexcept
	{
		if (spatialStructure->IsSpatialStructureActivated())
		{
			if (value)
				spatialStructure->OnDebugBoundingBox(root);
			else
				spatialStructure->OffDebugBoundingBox();
		} 
	}
	void JScene::SetDebugOnlyLeaf(const bool value)noexcept
	{
		spatialStructure->SetDebugOnlyLeaf(value);
	}
	void JScene::BuildSpatialStructure(const Core::J_SCENE_SPATIAL_STRUCTURE_TYPE type)noexcept
	{
		if (!spatialStructure->IsSpatialStructureActivated())
		{ 
			if (type == Core::J_SCENE_SPATIAL_STRUCTURE_TYPE::BVH)
				BuildBvh(Core::J_BVH_BUILD_TYPE::TOP_DOWN, Core::J_BVH_SPLIT_TYPE::SAH);
			else
				BuildOctree(32, 9, 2, true);
		}
	}
	void JScene::ClearSpatialStructure()noexcept
	{
		spatialStructure->Clear();
	}
	void JScene::BuildOctree(const uint minSize, const uint octreeSizeSquare, const float looseFactor, const bool isLooseOctree)noexcept
	{
		if (spatialStructure != nullptr)
		{
			const uint staticObjectCount = (uint)objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][0].size();
			const uint skinnedObjectCount = (uint)objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][1].size();
			const uint objectCount = staticObjectCount + skinnedObjectCount;
			std::vector<JGameObject*> gameObject(objectCount);
			std::copy(objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][0].begin(), objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][0].end(), gameObject.begin());
			std::copy(objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][1].begin(), objectLayer[(int)J_RENDER_LAYER::OPAQUE_OBJECT][1].end(), gameObject.begin() + staticObjectCount);

			spatialStructure->BuildOctree(gameObject, minSize, octreeSizeSquare, looseFactor, isLooseOctree);
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
	void JScene::CreateDemoGameObject()noexcept
	{
		CullingDemonstration::OnCullingDemonstration(root, 6, 6, 6, J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE);
	}
	void JScene::DestroyDemoGameObject()noexcept
	{
		CullingDemonstration::OffCullingDemonstration();
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

		std::wofstream stream;
		stream.open(scene->GetMetafilePath(), std::ios::out | std::ios::binary);
		Core::J_FILE_IO_RESULT res = StoreMetadata(stream, scene);
		stream.close();

		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		if (scene->IsValid())
		{
			stream.open(scene->GetPath(), std::ios::out | std::ios::binary);
			JGameObjectInterface* gI = scene->root;
			res = gI->CallStoreGameObject(stream);
			stream.close();
			return res;
		}
		else
			return res;
	}
	Core::J_FILE_IO_RESULT JScene::StoreMetadata(std::wofstream& stream, JScene* scene)
	{
		if (stream.is_open())
		{
			Core::J_FILE_IO_RESULT res = JResourceObject::StoreMetadata(stream, scene);
			if (res != Core::J_FILE_IO_RESULT::SUCCESS)
				return res;

			Core::JSceneSpatialStructureOption option = scene->GetSpatialStructureOption();
			JFileIOHelper::StoreAtomicData(stream, L"IsOpen:", scene->IsValid());
			JFileIOHelper::StoreAtomicData(stream, L"IsMainScene:", JSceneManager::Instance().IsMainScene(scene));
			JFileIOHelper::StoreAtomicData(stream, L"IsSpatialStructureActivated:", scene->isSpatialStructureActivated);  
			JFileIOHelper::StoreAtomicData(stream, L"SpatialStructureType:", (int)option.type);
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JScene* JScene::LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;

		std::wifstream stream;
		stream.open(pathData.engineMetaFileWPath, std::ios::in | std::ios::binary);
		JSceneMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, pathData.folderPath, metadata);
		stream.close();

		JScene* newScene = nullptr;
		if (directory->HasFile(pathData.name))
			newScene = JResourceManager::Instance().GetResourceByPath<JScene>(pathData.engineFileWPath);

		if (newScene == nullptr && loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
		{
			JSceneInitData initdata{ pathData.name, metadata.guid,metadata.flag, directory,(uint8)metadata.formatIndex };
			if (initdata.IsValidLoadData())
			{
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JScene>(initdata);
				newScene = ownerPtr.Get();
				if (!AddInstance(std::move(ownerPtr)))
					return nullptr;
			}
		}

		if (newScene == nullptr)
			return nullptr;

		if (!metadata.isOpen)
			return newScene;

		if (newScene->IsValid())
			return newScene;

		if (metadata.isMainScene)
		{
			JSceneManager::Instance().TryOpenScene(newScene, false);
			JSceneManager::Instance().SetMainScene(newScene);
			newScene->SetSceneSpatialStructureType((Core::J_SCENE_SPATIAL_STRUCTURE_TYPE)metadata.spatialStructureType);
			newScene->SetSceneSpatialStructure(metadata.isSpatialStructureActivated);
		}
		return newScene;
	}
	Core::J_FILE_IO_RESULT JScene::LoadMetadata(std::wifstream& stream, const std::wstring& folderPath, JSceneMetadata& metadata)
	{
		if (stream.is_open())
		{
			JResourceObject::LoadMetadata(stream, metadata);
			JFileIOHelper::LoadAtomicData(stream, metadata.isOpen);
			JFileIOHelper::LoadAtomicData(stream, metadata.isMainScene);
			JFileIOHelper::LoadAtomicData(stream, metadata.isSpatialStructureActivated);
			JFileIOHelper::LoadAtomicData(stream, metadata.spatialStructureType);
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	void JScene::RegisterJFunc()
	{
		auto defaultC = [](const Core::JOwnerPtr<JResourceInitData> initdata)-> JResourceObject*
		{
			if (initdata.IsValid() && initdata->GetResourceType() == J_RESOURCE_TYPE::SCENE && initdata->IsValidCreateData())
			{
				JSceneInitData* sInitdata = static_cast<JSceneInitData*>(initdata.Get());
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JScene>(*sInitdata);
				JScene* newScene = ownerPtr.Get();
				if (!AddInstance(std::move(ownerPtr)))
					return nullptr;

				return newScene;
			}
			else
				return nullptr;
		};
		auto loadC = [](JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori, JDirectory* directory)->JResourceObject*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JScene>(InitData(ori->GetName(),
				Core::MakeGuid(),
				ori->GetFlag(),
				directory,
				GetFormatIndex<JScene>(ori->GetFormat())));

			JScene* newScene = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				newScene->Copy(ori);
				return newScene;
			}
			else
				return nullptr;
		};

		JRFI<JScene>::Register(defaultC, loadC, copyC);

		auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex<JScene>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JScene::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JScene::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

		std::vector<J_RESOURCE_TYPE> allRType = Core::GetEnumElementVec<J_RESOURCE_TYPE>();
		for (uint i = 0; i < allRType.size(); ++i)
		{
			if (allRType[i] == J_RESOURCE_TYPE::SCENE)
			{
				allRType.erase(allRType.begin() + i);
				break;
			}
		}

		static RTypeHint rTypeHint{ GetStaticResourceType(), allRType, false, false };
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{});
	}
	JScene::JScene(const JSceneInitData& initdata)
		:JSceneInterface(initdata)
	{
		spatialStructure = std::make_unique<Core::JSceneSpatialStructure>();
	}
	JScene::~JScene()
	{ }
}