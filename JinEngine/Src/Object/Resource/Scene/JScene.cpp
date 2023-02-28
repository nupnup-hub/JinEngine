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
#include"../../../Core/Geometry/JCullingFrustum.h"
#include"../../../Core/SpaceSpatial/JSceneSpatialStructure.h"  
#include"../../../Utility/JCommonUtility.h"
#include<DirectXColors.h>

namespace JinEngine
{
	static const std::vector<JComponent*> emptyVec;

	JScene::JSceneInitData::JSceneInitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		JDirectory* directory,
		const J_SCENE_USE_CASE_TYPE useCaseType,
		const uint8 formatIndex)
		:JResourceInitData(name, guid, flag, directory, formatIndex, J_RESOURCE_TYPE::SCENE), useCaseType(useCaseType)
	{}
	JScene::JSceneInitData::JSceneInitData(const std::wstring& name,
		JDirectory* directory,
		const J_SCENE_USE_CASE_TYPE useCaseType,
		const uint8 formatIndex)
		: JResourceInitData(name, directory, formatIndex, J_RESOURCE_TYPE::SCENE), useCaseType(useCaseType)
	{}
	JScene::JSceneInitData::JSceneInitData(JDirectory* directory,
		const J_SCENE_USE_CASE_TYPE useCaseType,
		const uint8 formatIndex)
		: JResourceInitData(JResourceObject::GetDefaultName<JScene>(), directory, formatIndex, J_RESOURCE_TYPE::SCENE),
		useCaseType(useCaseType)
	{}

	J_RESOURCE_TYPE JScene::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	}
	std::wstring JScene::GetFormat()const noexcept
	{
		return GetAvailableFormat()[GetFormatIndex()];
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
	JGameObject* JScene::GetDebugRootGameObject()noexcept
	{
		return debugRoot;
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
		for (const auto& data : rVec)
			sum += static_cast<JRenderItem*>(data)->GetSubmeshCount();
		return sum;
	}
	J_SCENE_USE_CASE_TYPE JScene::GetUseCaseType()const noexcept
	{
		return useCaseType;
	}
	bool JScene::IsActivatedSceneTime()const noexcept
	{
		return sceneTimer != nullptr;
	}
	bool JScene::IsPauseSceneTime()const noexcept
	{
		return sceneTimer != nullptr ? sceneTimer->IsStop() : true;
	}
	bool JScene::IsMainScene()const noexcept
	{
		return JSceneManager::Instance().IsMainScene(this);
	}
	bool JScene::IsSpaceSpatialActivated()const noexcept
	{
		return spatialStructure != nullptr;
	}
	bool JScene::HasComponent(const J_COMPONENT_TYPE cType)const noexcept
	{
		return componentCash.find(cType) != componentCash.end();
	}
	JGameObject* JScene::IntersectFirst(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JRay& ray)const noexcept
	{
		if (spatialStructure != nullptr)
			return spatialStructure->IntersectFirst(layer, ray);
		else
			return nullptr;
	}
	JSceneCashInterface* JScene::CashInterface()
	{
		return this;
	}
	JSceneTimeInterface* JScene::TimeInterface()
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

		SetAllComponentFrameDirty();
		if (spatialStructure != nullptr)
		{
			if (spatialStructure->IsInitTriggerActivated())
				InitializeSpaceSpatial();
			spatialStructure->Activate(root, debugRoot);
		}
	}
	void JScene::DoDeActivate()noexcept
	{
		if (spatialStructure != nullptr)
			spatialStructure->DeAcitvate();
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
			}
			else
				CreateDefaultGameObject();
			CreateDebugRoot();
			SetValid(true);
		}
	}
	void JScene::ClearResource()
	{
		if (IsValid())
		{
			BegineForcedDestroy(root);
			root = nullptr;

			if (debugRoot != nullptr)
				BegineForcedDestroy(debugRoot);
			debugRoot = nullptr;

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
		const J_OBJECT_FLAG sceneFlag = GetFlag();
		const J_OBJECT_FLAG rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE);
		root = JGFI::CreateRoot(L"RootGameObject", Core::MakeGuid(), Core::AddSQValueEnum(sceneFlag, rootFlag), *this);

		const bool is3DScene = (useCaseType == J_SCENE_USE_CASE_TYPE::MAIN ) || 
			(useCaseType == J_SCENE_USE_CASE_TYPE::THREE_DIMENSIONAL_PREVIEW);
		J_OBJECT_FLAG objFlag = Core::AddSQValueEnum(sceneFlag, OBJECT_FLAG_UNDESTROYABLE);
		if (is3DScene)
			JGFU::CreateSky(*root, objFlag, L"DefaultSky");
		JGFU::CreateCamera(*root, objFlag, true, L"MainCamera");
		JGameObject* lit = JGFU::CreateLight(*root, objFlag, J_LIGHT_TYPE::DIRECTIONAL, L"MainLight");
		if(is3DScene)
			lit->GetComponent<JLight>()->SetShadow(true);
	}
	void JScene::CreateDebugRoot()noexcept
	{
		const J_OBJECT_FLAG rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_DO_NOT_SAVE);
		debugRoot = JGFI::CreateRoot(L"DebugRoot", debugRootGuid, rootFlag, *this);
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
	bool JScene::AddType(JGameObject* newGameObject)noexcept
	{
		if (IsActivated())
			newGameObject->Activate();
		allObjects.push_back(newGameObject);
		return true;
	}
	bool JScene::RemoveType(JGameObject* gameObj)noexcept
	{
		if (gameObj->IsActivated())
			gameObj->DeActivate();

		//if (spatialStructure != nullptr)
		//	spatialStructure->RemoveGameObject(&gameObj);

		const uint allObjCount = (uint)allObjects.size();
		const size_t guid = gameObj->GetGuid();
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
	void JScene::ActivateSceneTime()noexcept
	{
		if (sceneTimer == nullptr)
		{
			sceneTimer = std::make_unique<Core::JGameTimer>();
			sceneTimer->Start();
			sceneTimer->Reset();
			const std::vector<JComponent*>& cashVec = GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
			const uint compCount = (uint)cashVec.size();
			for (uint i = 0; i < compCount; ++i)
				static_cast<JAnimator*>(cashVec[i])->UpdateTriggerInterface()->OnAnimationUpdate(sceneTimer.get());
		}
	}
	void JScene::PlaySceneTimer(const bool value)noexcept
	{
		if (sceneTimer == nullptr)
		{
			if (value)
				sceneTimer->Start();
			else
				sceneTimer->Stop();
		}
	}
	void JScene::DeActivateSceneTime()noexcept
	{
		const std::vector<JComponent*>& cashVec = GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
		const uint compCount = (uint)cashVec.size();
		for (uint i = 0; i < compCount; ++i)
			static_cast<JAnimator*>(cashVec[i])->UpdateTriggerInterface()->OffAnimationUpdate();
		sceneTimer.reset();
	}
	void JScene::SetMainCamera(JCamera* mainCam)noexcept
	{
		mainCamera = mainCam;
	}
	void JScene::UpdateTransform(JGameObject* owner)noexcept
	{
		// && owner->GetRenderItem()->GetRenderLayer() != J_RENDER_LAYER::DEBUG
		if (owner == nullptr || !owner->HasRenderItem())
			return;

		if (IsActivated() && spatialStructure != nullptr)
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

			if (spatialStructure != nullptr)
				spatialStructure->AddGameObject(jRItem->GetOwner());
		} 
		JComponent* preComp = compCount > 0 ? cashVec->second[compCount - 1] : nullptr; 
		SetComponentFrameOffset(compType, preComp, compCount, true);
		return true;
	}
	bool JScene::DeRegisterComponent(JComponent& component)noexcept
	{
		const J_COMPONENT_TYPE compType = component.GetComponentType();
		auto cashData = componentCash.find(compType);
		if (cashData == componentCash.end())
			return false;

		int hitIndex = -1;
		std::vector<JComponent*>& cashVec = cashData->second;
		const uint compCount = (uint)cashVec.size();
		const size_t guid = component.GetGuid();
		for (uint i = 0; i < compCount; ++i)
		{
			if (cashVec[i]->GetGuid() == guid)
			{ 
				hitIndex = i;
				break; 
			}
		}

		if (hitIndex == -1)
			return false;

		if (compType == J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM)
		{
			JRenderItem* jRItem = static_cast<JRenderItem*>(&component);
			JGameObject* jOwner = jRItem->GetOwner();

			const int rIndex = (int)jRItem->GetRenderLayer();
			const size_t guid = jOwner->GetGuid();

			for (uint j = 0; j < (uint)J_MESHGEOMETRY_TYPE::COUNT; ++j)
			{
				const uint vecCount = (uint)objectLayer[rIndex][j].size();
				for (uint k = 0; k < vecCount; ++k)
				{
					if (guid == objectLayer[rIndex][j][k]->GetGuid())
					{
						objectLayer[rIndex][j].erase(objectLayer[rIndex][j].begin() + k);
						break;
					}
				}
			}
			//objectLayer[rIndex][mIndex].push_back(jOwner);
			if (spatialStructure != nullptr)
				spatialStructure->RemoveGameObject(jOwner);
		}

		SetComponentFrameDirty(compType, hitIndex+ 1);
		SetComponentFrameOffset(compType, &component, hitIndex + 1, false);

		cashVec.erase(cashVec.begin() + hitIndex);
		if (JCI::GetCTypeHint(compType).hasFrameDirty)
			JSceneManager::Instance().UpdateScene(this, compType);
		return true; 
	}
	void JScene::SetAllComponentFrameDirty()noexcept
	{
		for (auto& data : componentCash)
			SetComponentFrameDirty(data.first);
	}
	void JScene::SetComponentFrameDirty(const J_COMPONENT_TYPE cType, JComponent* stComp, SetCompCondition condiiton)noexcept
	{
		if (JCI::GetCTypeHint(cType).hasFrameDirty)
		{
			if (stComp != nullptr)
				SetComponentFrameDirty(cType, (uint)JCUtil::GetJIdenIndex(GetComponentCashVec(cType), stComp->GetGuid()), condiiton);
			else
				SetComponentFrameDirty(cType, (uint)0, condiiton);
		};
	} 
	void JScene::SetComponentFrameDirty(const J_COMPONENT_TYPE cType, const uint stIndex, SetCompCondition condiiton)noexcept
	{
		if (JCI::GetCTypeHint(cType).hasFrameDirty)
		{
			auto SetFrameDirtyCallable = JCI::GetSetFrameDirtyCallable(cType);
			if (SetFrameDirtyCallable == nullptr)
				return;

			const std::vector<JComponent*>& cashVec = GetComponentCashVec(cType);
			const uint compCount = (uint)cashVec.size();
			 
			if (condiiton != nullptr)
			{
				for (uint i = stIndex; i < compCount; ++i)
				{
					if (condiiton(*cashVec[i]))
						(*SetFrameDirtyCallable)(nullptr, *cashVec[i]);
				}
			}
			else
			{
				for (uint i = stIndex; i < compCount; ++i)
					(*SetFrameDirtyCallable)(nullptr, *cashVec[i]);
			}
		}
	}
	void JScene::SetComponentFrameOffset(const J_COMPONENT_TYPE cType, JComponent* refComp, const uint stIndex, const bool isCreated)noexcept
	{
		if (JCI::GetCTypeHint(cType).hasFrameOffset)
		{
			auto SetFrameOffsetCallable = JCI::GetSetFrameOffsetCallable(cType);
			const std::vector<JComponent*>& cashVec = GetComponentCashVec(cType);
			const uint compCount = (uint)cashVec.size();
		 
			for (uint i = stIndex; i < compCount; ++i)
				(*SetFrameOffsetCallable)(nullptr, *cashVec[i], refComp, isCreated);
		}
	}
	void JScene::ViewCulling()noexcept
	{
		if (spatialStructure != nullptr && mainCamera != nullptr)
		{
			//DirectX::BoundingFrustum camFrustum = mainCamera->GetBoundingFrustum();
			//DirectX::BoundingFrustum worldCamFrustum;
			//camFrustum.Transform(worldCamFrustum, mainCamera->GetTransform()->GetWorldMatrix());
			spatialStructure->Culling(mainCamera->GetBoundingFrustum());

			//Caution: Has Bug
			//Core::JCullingFrustum cFrustum(worldCamFrustum);
			//spatialStructure->Culling(cFrustum);
		}
	}
	std::vector<JGameObject*> JScene::GetAlignedObject(const Core::J_SPACE_SPATIAL_LAYER layer, const DirectX::BoundingFrustum& frustum)const noexcept
	{
		if (spatialStructure != nullptr)
			return spatialStructure->GetAlignedObject(layer, frustum);
		else
			return std::vector<JGameObject*>();
	}
	Core::JOctreeOption JScene::GetOctreeOption(const Core::J_SPACE_SPATIAL_LAYER layer)const noexcept
	{
		return spatialStructure != nullptr ? spatialStructure->GetOctreeOption(layer) : Core::JOctreeOption();
	}
	Core::JBvhOption JScene::GetBvhOption(const Core::J_SPACE_SPATIAL_LAYER layer)const noexcept
	{
		return spatialStructure != nullptr ? spatialStructure->GetBvhOption(layer) : Core::JBvhOption();
	}
	Core::JKdTreeOption JScene::GetKdTreeOption(const Core::J_SPACE_SPATIAL_LAYER layer)const noexcept
	{
		return spatialStructure != nullptr ? spatialStructure->GetKdTreeOption(layer) : Core::JKdTreeOption();
	}
	void JScene::SetOctreeOption(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JOctreeOption& newOption)noexcept
	{
		if (spatialStructure != nullptr)
			spatialStructure->SetOctreeOption(layer, newOption);
	}
	void JScene::SetBvhOption(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JBvhOption& newOption)noexcept
	{
		if (spatialStructure != nullptr)
			spatialStructure->SetBvhOption(layer, newOption);
	}
	void JScene::SetKdTreeOption(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JKdTreeOption& newOption)noexcept
	{
		if (spatialStructure != nullptr)
			spatialStructure->SetKdTreeOption(layer, newOption);
	}
	void JScene::BuildDebugTree(Core::J_SPACE_SPATIAL_TYPE type, const Core::J_SPACE_SPATIAL_LAYER layer, Editor::JEditorBinaryTreeView& tree)noexcept
	{
		if (spatialStructure != nullptr)
			spatialStructure->BuildDebugTree(type, layer, tree);
	}
	void JScene::InitializeSpaceSpatial()noexcept
	{
		if (spatialStructure != nullptr)
		{
			Core::J_SPACE_SPATIAL_LAYER commonLayer = Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT;
			Core::J_SPACE_SPATIAL_LAYER debugLayer = Core::J_SPACE_SPATIAL_LAYER::DEBUG_OBJECT;

			Core::JOctreeOption octreeOption = spatialStructure->GetOctreeOption(commonLayer);
			octreeOption.commonOption.innerRoot = root;
			octreeOption.commonOption.debugRoot = debugRoot;
			spatialStructure->SetOctreeOption(commonLayer, octreeOption);

			Core::JBvhOption bvhOption = spatialStructure->GetBvhOption(commonLayer);
			bvhOption.commonOption.innerRoot = root;
			bvhOption.commonOption.debugRoot = debugRoot;
			bvhOption.commonOption.isSpaceSpatialActivated = true;
			bvhOption.commonOption.isCullingActivated = true;
			spatialStructure->SetBvhOption(commonLayer, bvhOption);

			Core::JKdTreeOption kdOption = spatialStructure->GetKdTreeOption(commonLayer);
			//kdOption.isOcclusionCullingActivated = true;
			kdOption.commonOption.innerRoot = root;
			kdOption.commonOption.debugRoot = debugRoot;
			spatialStructure->SetKdTreeOption(commonLayer, kdOption);

			Core::JBvhOption bvhDebugOption = spatialStructure->GetBvhOption(debugLayer);
			bvhDebugOption.commonOption.innerRoot = debugRoot;
			bvhDebugOption.commonOption.debugRoot = debugRoot;
			bvhDebugOption.commonOption.isSpaceSpatialActivated = true;

			//bvhDebugOption.commonOption.isDebugActivated = true;
			//bvhDebugOption.commonOption.isDebugLeafOnly = true;

			spatialStructure->SetBvhOption(debugLayer, bvhDebugOption);
			spatialStructure->SetInitTrigger(false);
			/*
					Core::JKdTreeOption kdDebugOption;
			kdDebugOption.commonOption.innerRoot = debugRoot;
			kdDebugOption.commonOption.debugRoot = debugRoot;
			kdDebugOption.commonOption.isSpaceSpatialActivated = true;
			spatialStructure->SetKdTreeOption(debugLayer, kdOption);
			*/
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

			JFileIOHelper::StoreAtomicData(stream, L"IsOpen:", scene->IsValid());
			JFileIOHelper::StoreAtomicData(stream, L"IsMainScene:", JSceneManager::Instance().IsMainScene(scene));
			JFileIOHelper::StoreAtomicData(stream, L"IsActivatedSpaceSpatial:", scene->IsSpaceSpatialActivated());

			for (uint i = 0; i < (uint)Core::J_SPACE_SPATIAL_LAYER::COUNT; ++i)
			{
				Core::JOctreeOption octreeOption = scene->GetOctreeOption((Core::J_SPACE_SPATIAL_LAYER)i);
				Core::JBvhOption bvhOption = scene->GetBvhOption((Core::J_SPACE_SPATIAL_LAYER)i);
				Core::JKdTreeOption kdTreeOption = scene->GetKdTreeOption((Core::J_SPACE_SPATIAL_LAYER)i);

				octreeOption.Store(stream);
				bvhOption.Store(stream);
				kdTreeOption.Store(stream);
			}
			JFileIOHelper::StoreEnumData(stream, L"UseCaseType:", scene->GetUseCaseType());
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
			JSceneInitData initdata{ pathData.name, metadata.guid,metadata.flag, directory, metadata.useCaseType, (uint8)metadata.formatIndex };
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
			newScene->spatialStructure->SetInitTrigger(false);
			JSceneManager::Instance().TryOpenScene(newScene, false);
			JSceneManager::Instance().SetMainScene(newScene);
			if (metadata.isActivatedSpaceSpatial)
			{
				for (uint i = 0; i < (uint)Core::J_SPACE_SPATIAL_LAYER::COUNT; ++i)
				{
					const uint occIndex = (uint)Core::J_SPACE_SPATIAL_TYPE::OCTREE;
					const uint bvhIndex = (uint)Core::J_SPACE_SPATIAL_TYPE::BVH;
					const uint kdIndex = (uint)Core::J_SPACE_SPATIAL_TYPE::KD_TREE;

					if (metadata.hasInnerRoot[occIndex][i])
					{
						if (i == (int)Core::J_SPACE_SPATIAL_LAYER::DEBUG_OBJECT)
							metadata.octreeOption[i].commonOption.innerRoot = newScene->debugRoot;
						else
							metadata.octreeOption[i].commonOption.innerRoot = Core::GetUserPtr<JGameObject>(metadata.innerRootGuid[occIndex][i]).Get();
					}
					metadata.octreeOption[i].commonOption.debugRoot = newScene->debugRoot;
					newScene->SetOctreeOption((Core::J_SPACE_SPATIAL_LAYER)i, metadata.octreeOption[i]);

					if (metadata.hasInnerRoot[bvhIndex][i])
					{
						if (i == (int)Core::J_SPACE_SPATIAL_LAYER::DEBUG_OBJECT)
							metadata.bvhOption[i].commonOption.innerRoot = newScene->debugRoot;
						else
							metadata.bvhOption[i].commonOption.innerRoot = Core::GetUserPtr<JGameObject>(metadata.innerRootGuid[bvhIndex][i]).Get();
					}
					metadata.bvhOption[i].commonOption.debugRoot = newScene->debugRoot;
					newScene->SetBvhOption((Core::J_SPACE_SPATIAL_LAYER)i, metadata.bvhOption[i]);

					if (metadata.hasInnerRoot[kdIndex][i])
					{
						if (i == (int)Core::J_SPACE_SPATIAL_LAYER::DEBUG_OBJECT)
							metadata.kdTreeOption[i].commonOption.innerRoot = newScene->debugRoot;
						else
							metadata.kdTreeOption[i].commonOption.innerRoot = Core::GetUserPtr<JGameObject>(metadata.innerRootGuid[kdIndex][i]).Get();
					}
					metadata.kdTreeOption[i].commonOption.debugRoot = newScene->debugRoot;
					newScene->SetKdTreeOption((Core::J_SPACE_SPATIAL_LAYER)i, metadata.kdTreeOption[i]);
				}
			}
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
			JFileIOHelper::LoadAtomicData(stream, metadata.isActivatedSpaceSpatial);

			for (uint i = 0; i < (uint)Core::J_SPACE_SPATIAL_LAYER::COUNT; ++i)
			{
				const uint occIndex = (uint)Core::J_SPACE_SPATIAL_TYPE::OCTREE;
				const uint bvhIndex = (uint)Core::J_SPACE_SPATIAL_TYPE::BVH;
				const uint kdIndex = (uint)Core::J_SPACE_SPATIAL_TYPE::KD_TREE;

				metadata.octreeOption[i].Load(stream, metadata.hasInnerRoot[occIndex][i], metadata.innerRootGuid[occIndex][i]);
				metadata.bvhOption[i].Load(stream, metadata.hasInnerRoot[bvhIndex][i], metadata.innerRootGuid[bvhIndex][i]);
				metadata.kdTreeOption[i].Load(stream, metadata.hasInnerRoot[kdIndex][i], metadata.innerRootGuid[kdIndex][i]);
			}
			JFileIOHelper::LoadEnumData(stream, metadata.useCaseType);
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

				newScene->SetValid(true);
				newScene->CreateDefaultGameObject();
				StoreObject(newScene);
				newScene->ClearResource();
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
				static_cast<JScene*>(ori)->GetUseCaseType(),
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
		:JSceneInterface(initdata), debugRootGuid(Core::MakeGuid()), useCaseType(initdata.useCaseType)
	{
		if (useCaseType == J_SCENE_USE_CASE_TYPE::MAIN ||
			useCaseType == J_SCENE_USE_CASE_TYPE::THREE_DIMENSIONAL_PREVIEW)
		{
			spatialStructure = std::make_unique<Core::JSceneSpatialStructure>();
			spatialStructure->SetInitTrigger(true);
		}
	}
	JScene::~JScene()
	{}
}