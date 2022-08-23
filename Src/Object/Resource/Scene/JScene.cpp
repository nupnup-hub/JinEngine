#include"JScene.h"
#include"JSceneManager.h"
#include"../Mesh/JMeshGeometry.h" 
#include"../Material/JMaterial.h"  
#include"../JResourceObjectFactory.h"
#include"../JResourceManager.h" 

#include"../../JFrameInterface.h"
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
#include"../../../Core/DirectXEx/JCullingFrustum.h"
#include"../../../Core/GameTimer/JGameTimer.h"
#include"../../../Core/SpaceSpatial/JSceneSpatialStructure.h" 

#include"../../../Graphic/JGraphicDrawList.h"  
#include"../../../Test/CullingDemonstration.h"
#include<DirectXColors.h>

namespace JinEngine
{
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
		return (uint)componentCash.find(cType)->second.size();
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
	bool JScene::IsAnimatorActivated()const noexcept
	{
		return isAnimatorActivated;
	}
	bool JScene::IsMainScene()const noexcept
	{
		return !HasFlag((J_OBJECT_FLAG)(OBJECT_FLAG_HIDDEN | OBJECT_FLAG_DO_NOT_SAVE));
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
	JSceneFrameInterface* JScene::FrameInterface()
	{
		return this;
	}
	JSceneSpaceSpatialInterface* JScene::SpaceSpatialInterface()
	{
		return this;
	}
	void JScene::DoActivate() noexcept
	{
		JResourceObject::DoActivate();
		StuffResource();

		root->Activate();
		const uint objCount = (uint)allObjects.size();
		for (uint i = 0; i < objCount; ++i)
			allObjects[i]->Activate();

		OnSceneSpatialStructure();

		Graphic::JGraphicDrawList::AddDrawList(this);
	}
	void JScene::DoDeActivate()noexcept
	{
		JResourceObject::DoDeActivate();

		OffSceneSpatialStructure();

		root->DeActivate();
		const uint objCount = (uint)allObjects.size();
		for (uint i = 0; i < objCount; ++i)
			allObjects[i]->DeActivate();

		ClearResource();
		Graphic::JGraphicDrawList::PopDrawList(this);	
	}
	void JScene::StuffResource()
	{
		if (!IsValid())
		{
			std::wifstream stream;
			stream.open(GetWPath(), std::ios::in | std::ios::binary);
			if(JGFI::Create(stream, root) != nullptr)
				SetValid(true);
			stream.close();		 
		}
	}
	void JScene::ClearResource()
	{
		if (IsValid())
		{
			const bool preIgnore = IsIgnoreUndestroyableFlag();
			if(!preIgnore)
				SetIgnoreUndestroyableFlag(true);
			root->BeginDestroy();
			if (!preIgnore)
				SetIgnoreUndestroyableFlag(false);

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
			 
			Graphic::JGraphicDrawList::PopDrawList(this);
			SetValid(false);
		}
	}
	std::vector<JGameObject*>& JScene::GetGameObjectCashVec(const J_RENDER_LAYER rLayer, const J_MESHGEOMETRY_TYPE meshType)noexcept
	{
		return objectLayer[(int)rLayer][(int)meshType];
	}
	std::vector<JComponent*>& JScene::GetComponentCashVec(const J_COMPONENT_TYPE cType)noexcept
	{
		return componentCash.find(cType)->second;
	}
	JGameObject* JScene::AddGameObject(JGameObject& newGameObject)noexcept
	{
		if (IsActivated())
			newGameObject.Activate();
		allObjects.push_back(&newGameObject);
		return &newGameObject;
	}
	bool JScene::RemoveGameObject(JGameObject& gameObj)noexcept
	{
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
			std::vector<JComponent*>& cashVec = componentCash.find(J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR)->second;
			const uint compCount = (uint)cashVec.size();
			for (uint i = 0; i < compCount; ++i)
				static_cast<JAnimator*>(cashVec[i])->OnAnimation();
		}
	}
	JCamera* JScene::SetMainCamera(JCamera* camera)noexcept
	{
		if (camera == nullptr)
			return nullptr;

		JCamera* preCam = mainCamera;
		mainCamera = camera;
		return preCam;
	}
	bool JScene::RegisterComponent(JComponent& component)noexcept
	{
		if (!component.PassDefectInspection())
			return false;

		const J_COMPONENT_TYPE compType = component.GetComponentType();
		std::vector<JComponent*>& cashVec = componentCash.find(compType)->second;

		const uint compCount = (uint)cashVec.size();
		const size_t guid = component.GetGuid();
		for (uint i = 0; i < compCount; ++i)
		{
			if (cashVec[i]->GetGuid() == guid)
				return false;
		}

		Graphic::JGraphicDrawList::UpdateScene(this, compType);
		cashVec.push_back(&component);

		if (compType == J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM)
		{
			JRenderItem* jRItem = static_cast<JRenderItem*>(&component);
			const J_RENDER_LAYER renderLayer = jRItem->GetRenderLayer();
			const J_MESHGEOMETRY_TYPE meshType = jRItem->GetMesh()->GetMeshGeometryType();
			objectLayer[(int)renderLayer][(int)meshType].push_back(jRItem->GetOwner());
			if (renderLayer == J_RENDER_LAYER::OPAQUE_OBJECT && isSpatialStructureActivated)
				spatialStructure->AddGameObject(jRItem->GetOwner());
		}

		return true;
	}
	bool JScene::DeRegisterComponent(JComponent& component)noexcept
	{
		const J_COMPONENT_TYPE compType = component.GetComponentType();
		std::vector<JComponent*>& cashVec = componentCash.find(compType)->second;

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

					objectLayer[rIndex][mIndex].push_back(jOwner);
					if (jRItem->GetRenderLayer() == J_RENDER_LAYER::OPAQUE_OBJECT && isSpatialStructureActivated)
						spatialStructure->RemoveGameObject(jOwner);
				}
				SetBackSideComponentDirty(*cashVec[i]);
				cashVec.erase(cashVec.begin() + i);
				Graphic::JGraphicDrawList::UpdateScene(this, compType);
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
			std::vector<JComponent*>& cashVec = componentCash.find(cType)->second;
			const uint compCount = (uint)cashVec.size();
			for (uint i = 0; i < compCount; ++i)
				SetFrameDirtyCallable(nullptr, *cashVec[i]);
		}
	}
	void JScene::SetBackSideComponentDirty(JComponent& jComp) noexcept
	{
		const J_COMPONENT_TYPE cType = jComp.GetComponentType();
		std::vector<JComponent*>& cashVec = componentCash.find(cType)->second;
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
		std::vector<JComponent*>& cashVec = componentCash.find(cType)->second;
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
	void JScene::OnSceneSpatialStructure()noexcept
	{
		if (!isSpatialStructureActivated)
		{
			BuildBvh(Core::J_BVH_BUILD_TYPE::TOP_DOWN, Core::J_BVH_SPLIT_TYPE::SAH);
			isSpatialStructureActivated = true;
		}
	}
	void JScene::OffSceneSpatialStructure()noexcept
	{
		if (isSpatialStructureActivated)
		{
			spatialStructure->Clear();
			isSpatialStructureActivated = false;
		}
	}
	void JScene::OnDebugBoundingBox(bool onlyLeafNode)noexcept
	{
		if (!isSpatialStructureActivated)
		{
			isSpatialStructureDebugActivated = true;
			spatialStructure->OnDebugBoundingBox(root, onlyLeafNode);
		}
	}
	void JScene::OffDebugBoundingBox()noexcept
	{
		if (isSpatialStructureActivated)
		{
			isSpatialStructureDebugActivated = false;
			spatialStructure->OffDebugBoundingBox();
		}
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
			stream.open(scene->GetWPath(), std::ios::out | std::ios::binary);
			stream << scene->isSpatialStructureActivated;

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

			stream << scene->IsValid();
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
 
		JScene* newScene = nullptr;
		if (directory->HasFile(pathData.fullName))
			newScene = JResourceManager::Instance().GetResourceByPath<JScene>(pathData.strPath);

		if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
			newScene = new JScene(pathData.name, metadata.guid, metadata.flag, directory, GetFormatIndex<JScene>(pathData.format));
		else
			newScene = new JScene(pathData.name, Core::MakeGuid(), OBJECT_FLAG_NONE, directory, GetFormatIndex<JScene>(pathData.format));

		if (!metadata.isOpen)
			return newScene;

		stream.open(pathData.wstrPath, std::ios::in | std::ios::binary);
		if (stream.is_open())
		{
			if (newScene->IsValid())
			{
				stream.close();
				return newScene;
			}

			bool isSpatialStructureActivated;
			stream >> isSpatialStructureActivated;
			if (isSpatialStructureActivated)
				newScene->OnSceneSpatialStructure();
			else
				newScene->OffSceneSpatialStructure();

			if (JGFI::Create(stream, newScene->root) != nullptr)
			{
				stream.close();
				newScene->SetValid(true);
				JSceneManager::Instance().TryOpenScene(newScene);
				return newScene;
			}
			else
			{
				stream.close();
				delete newScene;
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
	void JScene::RegisterJFunc()
	{
		auto defaultC = [](JDirectory* owner) ->JResourceObject*
		{
			JScene* newScene = new JScene(owner->MakeUniqueFileName(JResourceObject::GetDefaultName<JScene>()),
				Core::MakeGuid(),
				OBJECT_FLAG_NONE,
				owner,
				JResourceObject::GetDefaultFormatIndex());

			const J_OBJECT_FLAG rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED| OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE);
			newScene->root = JGFI::CreateRoot("RootGameObject", Core::MakeGuid(), rootFlag, *newScene);

			J_OBJECT_FLAG flag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE);
			if (newScene->allObjects.size() == 1)
			{
				JGFU::CreateSky(*newScene->root, flag, "DefaultSky");
				JGFU::CreateCamera(*newScene->root, flag, true, "MainCamera");
				JGFU::CreateLight(*newScene->root, flag, J_LIGHT_TYPE::DIRECTIONAL, "MainLight");
			}

			return newScene;
		};
		auto initC = [](const std::string& name, const size_t guid, const J_OBJECT_FLAG objFlag, JDirectory* directory, const uint8 formatIndex)-> JResourceObject*
		{
			JScene* newScene = new JScene(name, guid, objFlag, directory, formatIndex);

			const J_OBJECT_FLAG rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE);
			newScene->root = JGFI::CreateRoot("RootGameObject", Core::MakeGuid(), rootFlag, *newScene);

			J_OBJECT_FLAG flag = OBJECT_FLAG_NONE;
			if (objFlag == OBJECT_FLAG_EDITOR_OBJECT)
				flag = OBJECT_FLAG_EDITOR_OBJECT;
			else
				flag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE);

			//only has root
			if (newScene->allObjects.size() == 1)
			{
				JGFU::CreateSky(*newScene->root, flag, "DefaultSky");
				JGFU::CreateCamera(*newScene->root, flag, true, "MainCamera");
				JGFU::CreateLight(*newScene->root, flag, J_LIGHT_TYPE::DIRECTIONAL, "MainLight");
			}
			return newScene;
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

		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, false, false, false };
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{});

		JGameObjectFactoryImpl::RegisterAddStroage(&JScene::AddGameObject);
	}
	JScene::JScene(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		:JSceneInterface(name, guid, flag, directory, formatIndex), isAnimatorActivated(false)
	{
		spatialStructure = std::make_unique<Core::JSceneSpatialStructure>();
	}
	JScene::~JScene()
	{ }
}