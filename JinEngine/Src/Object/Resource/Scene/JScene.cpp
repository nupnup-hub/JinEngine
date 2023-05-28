#include"JScene.h"
#include"JScenePrivate.h"
#include"JSceneManager.h"
#include"JSceneManagerPrivate.h"
#include"../JClearableInterface.h"
#include"../JResourceObjectHint.h"
#include"../Mesh/JMeshGeometry.h" 
#include"../Material/JMaterial.h"   
#include"../Mesh/JMeshType.h" 
#include"../../../Core/Reflection/JTypeImplBase.h"
//#include"../JResourceManager.h" 
 
#include"../../Component/JComponentHint.h"
#include"../../Component/JComponentType.h"
#include"../../Component/Animator/JAnimator.h"
#include"../../Component/Animator/JAnimatorPrivate.h"
#include"../../Component/RenderItem/JRenderItem.h"
#include"../../Component/Transform/JTransform.h"
#include"../../Component/Light/JLight.h" 
#include"../../Component/Camera/JCamera.h"    
#include"../../GameObject/JGameObjectCreator.h" 
#include"../../GameObject/JGameObject.h"
#include"../../GameObject/JGameObjectPrivate.h"
#include"../../Directory/JDirectory.h"

#include"../../../Application/JApplicationProject.h"
#include"../../../Core/Identity/JIdenCreator.h"
#include"../../../Core/Guid/GuidCreator.h" 
#include"../../../Core/Time/JGameTimer.h"
#include"../../../Core/File/JFileConstant.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Geometry/JCullingFrustum.h"
#include"../../../Core/SpaceSpatial/JSceneSpatialStructure.h"   
#include"../../../Graphic/Upload/Frameresource/JFrameUpdate.h" 
#include"../../../Utility/JCommonUtility.h"
#include<DirectXColors.h>

namespace JinEngine
{
	namespace
	{ 
		using SetCompCondition = JScenePrivate::CompFrameInterface::SetCompCondition;
		using SceneMangerAccess = JSceneManagerPrivate::SceneAccess;

		static SetCompCondition SpaceStructureUseCamCond()
		{
			return [](const JUserPtr<JComponent>& comp)
			{
				JCamera* cam = static_cast<JCamera*>(comp.Get());
				return cam->AllowHzbOcclusionCulling() || cam->AllowFrustumCulling();
			}; 
		}
 
	}
	namespace
	{
		static const std::vector<JUserPtr<JComponent>> emptyVec;
		static JScenePrivate sPrivate;
	}
 
	class JScene::JSceneImpl : public Core::JTypeImplBase,
		public JClearableInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JSceneImpl)
	public:
		JWeakPtr<JScene> thisPointer = nullptr;
	public:
		JUserPtr<JGameObject> root = nullptr;
		JUserPtr<JGameObject> debugRoot = nullptr;
		std::unique_ptr<Core::JSceneSpatialStructure> spatialStructure;
		std::vector<JUserPtr<JGameObject>> allObjects;
		std::vector<JUserPtr<JGameObject>> objectLayer[(int)J_RENDER_LAYER::COUNT][(int)J_MESHGEOMETRY_TYPE::COUNT];
		std::unordered_map<J_COMPONENT_TYPE, std::vector<JUserPtr<JComponent>>> componentCash;
	public:
		const size_t debugRootGuid;
		const J_SCENE_USE_CASE_TYPE useCaseType;
	public:
		std::unique_ptr<Core::JGameTimer> sceneTimer;
	public:
		JSceneImpl(const InitData& initData, JScene* thisSceneRaw)
			:debugRootGuid(Core::MakeGuid()), useCaseType(initData.useCaseType)
		{
			if (useCaseType == J_SCENE_USE_CASE_TYPE::MAIN ||
				useCaseType == J_SCENE_USE_CASE_TYPE::THREE_DIMENSIONAL_PREVIEW)
				spatialStructure = std::make_unique<Core::JSceneSpatialStructure>(); 
		}
		~JSceneImpl(){}
	public:
		JUserPtr<JGameObject> GetGameObject(const uint index)noexcept
		{
			if (index < allObjects.size())
				return allObjects[index];
			else
				return nullptr;
		}
		JUserPtr<JGameObject> GetGameObject(const std::wstring& name)noexcept
		{
			for (const auto& data : allObjects)
			{
				if (data->GetName() == name)
					return data;
			}
			return nullptr;
		}
		uint GetMeshCount()const noexcept
		{
			uint sum = 0;
			const std::vector<JUserPtr<JComponent>>& rVec = GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
			for (const auto& data : rVec)
				sum += static_cast<JRenderItem*>(data.Get())->GetSubmeshCount();
			return sum;
		}
		std::vector<JUserPtr<JGameObject>> GetAlignedObject(const Core::J_SPACE_SPATIAL_LAYER layer, const DirectX::BoundingFrustum& frustum)const noexcept
		{
			if (spatialStructure != nullptr)
				return spatialStructure->GetAlignedObject(layer, frustum);
			else
				return std::vector<JUserPtr<JGameObject>>();
		}
		const std::vector<JUserPtr<JGameObject>>& GetGameObjectCashVec(const J_RENDER_LAYER rLayer, const J_MESHGEOMETRY_TYPE meshType)const noexcept
		{
			return objectLayer[(int)rLayer][(int)meshType];
		}
		const std::vector<JUserPtr<JComponent>>& GetComponentCashVec(const J_COMPONENT_TYPE cType)const noexcept
		{
			auto vec = componentCash.find(cType);
			if (vec == componentCash.end())
				return emptyVec;
			else
				return vec->second;
		}
		std::vector<JUserPtr<JComponent>> GetComponentVec(const J_COMPONENT_TYPE cType)const noexcept
		{
			auto vec = componentCash.find(cType);
			if (vec == componentCash.end())
				return std::vector<JUserPtr<JComponent>>{};
			else
				return vec->second;
		}
		Core::JOctreeOption GetOctreeOption(const Core::J_SPACE_SPATIAL_LAYER layer)const noexcept
		{
			return spatialStructure != nullptr ? spatialStructure->GetOctreeOption(layer) : Core::JOctreeOption();
		}
		Core::JBvhOption GetBvhOption(const Core::J_SPACE_SPATIAL_LAYER layer)const noexcept
		{
			return spatialStructure != nullptr ? spatialStructure->GetBvhOption(layer) : Core::JBvhOption();
		}
		Core::JKdTreeOption GetKdTreeOption(const Core::J_SPACE_SPATIAL_LAYER layer)const noexcept
		{
			return spatialStructure != nullptr ? spatialStructure->GetKdTreeOption(layer) : Core::JKdTreeOption();
		}
	public:
		void SetOctreeOption(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JOctreeOption& newOption)noexcept
		{
			if (spatialStructure == nullptr)
				return; 

			spatialStructure->SetOctreeOption(layer, newOption);
			SetComponentFrameDirty(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA, 0, SpaceStructureUseCamCond());
		}
		void SetBvhOption(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JBvhOption& newOption)noexcept
		{
			if (spatialStructure == nullptr)
				return;

			spatialStructure->SetBvhOption(layer, newOption);
			SetComponentFrameDirty(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA, 0, SpaceStructureUseCamCond());
		}
		void SetKdTreeOption(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JKdTreeOption& newOption)noexcept
		{
			if (spatialStructure == nullptr)
				return;

			spatialStructure->SetKdTreeOption(layer, newOption);
			SetComponentFrameDirty(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA, 0, SpaceStructureUseCamCond());
		}
	public:
		bool IsActivatedSceneTime()const noexcept
		{
			return sceneTimer != nullptr;
		}
		bool IsPauseSceneTime()const noexcept
		{
			return sceneTimer != nullptr ? sceneTimer->IsStop() : true;
		}
		bool HasComponent(const J_COMPONENT_TYPE cType)const noexcept
		{
			return componentCash.find(cType) != componentCash.end();
		}
	public:
		JUserPtr<JGameObject> FindGameObject(const size_t guid)noexcept
		{
			const uint allObjectCount = (uint)allObjects.size();
			for (uint i = 0; i < allObjectCount; ++i)
				if (allObjects[i]->GetGuid() == guid)
					return allObjects[i];
			return nullptr;
		}
		JUserPtr<JCamera> FindFirstSelectedCamera(const bool allowEditorCam) const noexcept
		{
			auto& vec = componentCash.find(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)->second;
			for (const auto& data : vec)
			{ 
				if (!allowEditorCam && data->GetOwner()->IsEditorObject())
					continue;
				return Core::ConnectChildUserPtr<JCamera>(data);
			}
			return nullptr;
		}
		JUserPtr<JGameObject> IntersectFirst(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JRay& ray)const noexcept
		{
			if (spatialStructure != nullptr)
				return spatialStructure->IntersectFirst(layer, ray);
			else
				return nullptr;
		}
	public:
		void CreateDefaultGameObject()noexcept
		{
			const J_OBJECT_FLAG sceneFlag = thisPointer->GetFlag();
			const J_OBJECT_FLAG rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE);
			root = JGCI::CreateRoot(L"RootGameObject", Core::MakeGuid(), Core::AddSQValueEnum(sceneFlag, rootFlag), thisPointer);

			const bool is3DScene = (useCaseType == J_SCENE_USE_CASE_TYPE::MAIN) ||
				(useCaseType == J_SCENE_USE_CASE_TYPE::THREE_DIMENSIONAL_PREVIEW);
			J_OBJECT_FLAG objFlag = Core::AddSQValueEnum(sceneFlag, OBJECT_FLAG_UNDESTROYABLE);
			if (is3DScene)
				JGCI::CreateSky(root, objFlag, L"DefaultSky");

			JUserPtr<JGameObject> mainCam;
			if (useCaseType == J_SCENE_USE_CASE_TYPE::MAIN || useCaseType == J_SCENE_USE_CASE_TYPE::THREE_DIMENSIONAL_PREVIEW)
				mainCam = JGCI::CreateCamera(root, objFlag, true, L"MainCamera");		
			else if (useCaseType == J_SCENE_USE_CASE_TYPE::TWO_DIMENSIONAL_PREVIEW)
				mainCam = JGCI::CreateCamera(root, objFlag, L"MainCamera");
			
			mainCam->GetComponent<JCamera>()->SetCameraState(J_CAMERA_STATE::RENDER);
			JUserPtr<JGameObject> lit = JGCI::CreateLight(root, objFlag, J_LIGHT_TYPE::DIRECTIONAL, L"MainLight");
			if (is3DScene)
				lit->GetComponent<JLight>()->SetShadow(true);
		}
		void CreateDebugRoot()noexcept
		{
			const J_OBJECT_FLAG rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_DO_NOT_SAVE);
			debugRoot = JGCI::CreateDebugRoot(L"DebugRoot", debugRootGuid, rootFlag, thisPointer);
		}
	public:
		void ActivateSceneTime()noexcept
		{ 
			if (sceneTimer == nullptr)
			{
				sceneTimer = std::make_unique<Core::JGameTimer>();
				sceneTimer->Start();
				sceneTimer->Reset();
				const std::vector<JUserPtr<JComponent>>& cashVec = GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
				const uint compCount = (uint)cashVec.size();
				for (uint i = 0; i < compCount; ++i)
					JAnimatorPrivate::AnimationUpdateInterface::OnAnimationUpdate(Core::ConnectChildUserPtr<JAnimator>(cashVec[i]), sceneTimer.get());
			}
		}
		void PlaySceneTimer(const bool value)noexcept
		{
			if (sceneTimer == nullptr)
			{
				if (value)
					sceneTimer->Start();
				else
					sceneTimer->Stop();
			}
		}
		void DeActivateSceneTime()noexcept
		{
			const std::vector<JUserPtr<JComponent>>& cashVec = GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
			const uint compCount = (uint)cashVec.size();
			for (uint i = 0; i < compCount; ++i)
				JAnimatorPrivate::AnimationUpdateInterface::OffAnimationUpdate(Core::ConnectChildUserPtr<JAnimator>(cashVec[i]));
			sceneTimer.reset();
		}
	public:
		bool AddGameObject(const JUserPtr<JGameObject>& newGameObject)noexcept
		{ 
			allObjects.push_back(newGameObject);
			return true;
		}
		bool RemoveGameObject(const JUserPtr<JGameObject>& gameObj)noexcept
		{ 
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
	public:
		void UpdateTransform(const JUserPtr<JGameObject>& owner)noexcept
		{
			// && owner->GetRenderItem()->GetRenderLayer() != J_RENDER_LAYER::DEBUG
			if (owner == nullptr || !owner->HasRenderItem())
				return;

			if (thisPointer->IsActivated() && spatialStructure != nullptr)
				spatialStructure->UpdateGameObject(owner);
		}
	public:
		bool RegisterComponent(const JUserPtr<JComponent>& component)noexcept
		{
			if (!component->PassDefectInspection())
				return false;

			const J_COMPONENT_TYPE compType = component->GetComponentType();
			auto cashVec = componentCash.find(compType);
			if (cashVec == componentCash.end())
			{
				componentCash.emplace(compType, std::vector<JUserPtr<JComponent>>());
				cashVec = componentCash.find(compType);
			}

			const uint compCount = (uint)cashVec->second.size();
			const size_t guid = component->GetGuid();
			for (uint i = 0; i < compCount; ++i)
			{
				if (cashVec->second[i]->GetGuid() == guid)
					return false;
			}
			 
			cashVec->second.push_back(component);
			if (compType == J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM)
			{
				JRenderItem* jRItem = static_cast<JRenderItem*>(component.Get());
				const J_RENDER_LAYER renderLayer = jRItem->GetRenderLayer();
				const J_MESHGEOMETRY_TYPE meshType = jRItem->GetMesh()->GetMeshGeometryType();
				objectLayer[(int)renderLayer][(int)meshType].push_back(jRItem->GetOwner());

				if (spatialStructure != nullptr)
					spatialStructure->AddGameObject(jRItem->GetOwner());
			}
			return true;
		}
		bool DeRegisterComponent(const JUserPtr<JComponent>& component)noexcept
		{
			const J_COMPONENT_TYPE compType = component->GetComponentType();
			auto cashData = componentCash.find(compType);
			if (cashData == componentCash.end())
				return false;

			int hitIndex = -1;
			std::vector<JUserPtr<JComponent>>& cashVec = cashData->second;
			const uint compCount = (uint)cashVec.size();
			const size_t guid = component->GetGuid();
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
				JRenderItem* jRItem = static_cast<JRenderItem*>(component.Get());
				JUserPtr<JGameObject> jOwner = jRItem->GetOwner();

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

			cashVec.erase(cashVec.begin() + hitIndex); 
			return true;
		}
	public:
		void SetAllComponentFrameDirty()noexcept
		{
			for (auto& data : componentCash)
				SetComponentFrameDirty(data.first);
		}
		void SetComponentFrameDirty(const J_COMPONENT_TYPE cType, JUserPtr< JComponent> stComp = nullptr, SetCompCondition condiiton = nullptr)noexcept
		{
			if (!CTypeCommonCall::GetCTypeHint(cType).hasFrameDirty)
				return;

			if (stComp != nullptr)
				SetComponentFrameDirty(cType, (uint)JCUtil::GetTypeIndex(GetComponentCashVec(cType), stComp->GetGuid()), condiiton);
			else
				SetComponentFrameDirty(cType, (uint)0, condiiton);
		}
		void SetComponentFrameDirty(const J_COMPONENT_TYPE cType, const uint stIndex, SetCompCondition condiiton = nullptr)noexcept
		{
			if (!CTypeCommonCall::GetCTypeHint(cType).hasFrameDirty)
				return;
			auto setFrameDirtyCallable = CTypePrivateCall::GetSetFrameDirtyCallable(cType);
			if (setFrameDirtyCallable == nullptr)
				return;

			const std::vector<JUserPtr<JComponent>>& cashVec = GetComponentCashVec(cType);
			const uint compCount = (uint)cashVec.size();

			if (condiiton != nullptr)
			{
				for (uint i = stIndex; i < compCount; ++i)
				{
					if (condiiton(cashVec[i]))
						(*setFrameDirtyCallable)(nullptr, cashVec[i].Get());
				}
			}
			else
			{
				for (uint i = stIndex; i < compCount; ++i)
					(*setFrameDirtyCallable)(nullptr, cashVec[i].Get());
			}
		}
	public:
		void ViewCulling(const Graphic::JCullingUserInterface& cullUser, const DirectX::BoundingFrustum& frustum)
		{
			spatialStructure->Culling(cullUser, frustum);
			
			/*	Old Log
				//DirectX::BoundingFrustum camFrustum = mainCamera->GetBoundingFrustum();
				//DirectX::BoundingFrustum worldCamFrustum;
				//camFrustum.Transform(worldCamFrustum, mainCamera->GetTransform()->GetWorldMatrix());
				spatialStructure->Culling(cam->GetBoundingFrustum());

				//Caution: Has Bug
				//Core::JCullingFrustum cFrustum(worldCamFrustum);
				//spatialStructure->Culling(cFrustum);
			*/
		}
		void InitializeSpaceSpatial()noexcept
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
				/*
						Core::JKdTreeOption kdDebugOption;
				kdDebugOption.commonOption.innerRoot = debugRoot;
				kdDebugOption.commonOption.debugRoot = debugRoot;
				kdDebugOption.commonOption.isSpaceSpatialActivated = true;
				spatialStructure->SetKdTreeOption(debugLayer, kdOption);
				*/
			}
		}
	public:
		void BuildDebugTree(Core::J_SPACE_SPATIAL_TYPE type, const Core::J_SPACE_SPATIAL_LAYER layer, Editor::JEditorBinaryTreeView& tree)noexcept
		{
			if (spatialStructure != nullptr)
				spatialStructure->BuildDebugTree(type, layer, tree);
		}
	public:
		void Activate()
		{
			SceneMangerAccess::RegisterScene(thisPointer);
			StuffResource();
			if (root != nullptr)
				JGameObjectPrivate::ActivateInterface::Activate(root);
			for (auto& data : allObjects)
				JGameObjectPrivate::ActivateInterface::Activate(data);

			SetAllComponentFrameDirty();
			if (spatialStructure != nullptr)
				spatialStructure->Activate();
		}
		void DeActivate()
		{
			if (spatialStructure != nullptr)
				spatialStructure->DeAcitvate();
			JGameObjectPrivate::ActivateInterface::DeActivate(root);
			for (auto& data : allObjects)
				JGameObjectPrivate::ActivateInterface::DeActivate(data); 
			ClearResource();
			SceneMangerAccess::DeRegisterScene(thisPointer); 
		}
	public:
		bool ReadAssetData()
		{
			std::wifstream stream;
			stream.open(thisPointer->GetPath(), std::ios::in | std::ios::binary);
			if (!stream.is_open())
				return false;

			auto data = JGameObjectPrivate::AssetDataIOInterface::CreateLoadAssetDIData(thisPointer, stream);
			root.ConnnectChild(JGameObjectPrivate::AssetDataIOInterface::LoadAssetData(data.get()));
			stream.close();
			return true;
		}
		bool WriteAssetData()
		{
			std::wofstream stream;
			stream.open(thisPointer->GetPath(), std::ios::out | std::ios::binary);
			if (!stream.is_open())
				return false;

			auto data = JGameObjectPrivate::AssetDataIOInterface::CreateStoreAssetDIData(root, stream);
			Core::J_FILE_IO_RESULT res = JGameObjectPrivate::AssetDataIOInterface::StoreAssetData(data.get());
			stream.close();
			return res == Core::J_FILE_IO_RESULT::SUCCESS;
		}
	public:
		void StuffResource()
		{
			if (!thisPointer->IsValid())
			{
				if (!thisPointer->HasFlag(OBJECT_FLAG_DO_NOT_SAVE))
					ReadAssetData();
				if (root == nullptr)
					CreateDefaultGameObject();
				CreateDebugRoot();
				thisPointer->SetValid(true);
			}
		}
		void ClearResource()
		{
			if (thisPointer->IsValid())
			{
				BeginForcedDestroy(root.Get());
				root = nullptr;

				if (debugRoot != nullptr)
					BeginForcedDestroy(debugRoot.Get());
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
				thisPointer->SetValid(false);
			}
		}
	public:
		void Initialize(InitData* initData)
		{
			if (!thisPointer->HasFile())
			{
				CreateDefaultGameObject();
				CreateDebugRoot();
				if (spatialStructure != nullptr)
					InitializeSpaceSpatial();
			}
			else
				ReadAssetData();

			if (debugRoot == nullptr)
				CreateDebugRoot();
			thisPointer->SetValid(true);
		}
		void RegisterThisPointer(JScene* scene)
		{
			thisPointer = Core::GetWeakPtr(scene);
		}
		static void RegisterTypeData()
		{ 
			auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex(GetStaticResourceType(), format); };

			static GetRTypeInfoCallable getTypeInfoCallable{ &JScene::StaticTypeInfo };
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

			static RTypeHint rTypeHint{ GetStaticResourceType(), allRType, false, false, true, false };
			static RTypeCommonFunc rTypeCFunc{ getTypeInfoCallable, getAvailableFormatCallable, getFormatIndexCallable };

			RegisterRTypeInfo(rTypeHint, rTypeCFunc, RTypePrivateFunc{});
			Core::JIdentifier::RegisterPrivateInterface(JScene::StaticTypeInfo(), sPrivate);

			IMPL_REALLOC_BIND(JScene::JSceneImpl, thisPointer)
		}
	};

	JScene::InitData::InitData(const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		const J_SCENE_USE_CASE_TYPE useCaseType)
		:JResourceObject::InitData(JScene::StaticTypeInfo(), formatIndex, GetStaticResourceType(), directory), useCaseType(useCaseType)
	{}
	JScene::InitData::InitData(const size_t guid,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		const J_SCENE_USE_CASE_TYPE useCaseType)
		: JResourceObject::InitData(JScene::StaticTypeInfo(), guid, formatIndex, GetStaticResourceType(), directory), useCaseType(useCaseType)
	{}
	JScene::InitData::InitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		const J_SCENE_USE_CASE_TYPE useCaseType)
		: JResourceObject::InitData(JScene::StaticTypeInfo(), name, guid, flag, formatIndex, GetStaticResourceType(), directory),
		useCaseType(useCaseType)
	{}

	JScene::LoadMetaData::LoadMetaData(const JUserPtr<JDirectory>& directory)
		: JResourceObject::InitData(JScene::StaticTypeInfo(), GetDefaultFormatIndex(), GetStaticResourceType(), directory)
	{}

	Core::JIdentifierPrivate& JScene::PrivateInterface()const noexcept
	{
		return sPrivate;
	}
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
	JUserPtr<JGameObject> JScene::GetRootGameObject()noexcept
	{
		return impl->root;
	}
	JUserPtr<JGameObject> JScene::GetDebugRootGameObject()noexcept
	{
		return impl->debugRoot;
	}
	JUserPtr<JGameObject> JScene::GetGameObject(const uint index)noexcept
	{
		return impl->GetGameObject(index);
	}
	JUserPtr<JGameObject> JScene::GetGameObject(const std::wstring& name)noexcept
	{
		return impl->GetGameObject(name);
	}
	uint JScene::GetGameObjectCount()const noexcept
	{
		return (uint)impl->allObjects.size();
	}
	uint JScene::GetComponetCount(const J_COMPONENT_TYPE cType)const noexcept
	{
		return (uint)impl->GetComponentCashVec(cType).size();
	}
	uint JScene::GetMeshCount()const noexcept
	{
		return (uint)impl->GetMeshCount();
	}
	J_SCENE_USE_CASE_TYPE JScene::GetUseCaseType()const noexcept
	{
		return impl->useCaseType;
	}
	std::vector<JUserPtr<JGameObject>> JScene::GetAlignedObject(const Core::J_SPACE_SPATIAL_LAYER layer, const DirectX::BoundingFrustum& frustum)const noexcept
	{
		return impl->GetAlignedObject(layer, frustum);
	}
	std::vector<JUserPtr<JGameObject>> JScene::GetGameObjectVec()const noexcept
	{
		return impl->allObjects;
	}
	std::vector<JUserPtr<JComponent>> JScene::GetComponentVec(const J_COMPONENT_TYPE cType)const noexcept
	{
		return impl->GetComponentVec(cType);
	}
	Core::JOctreeOption JScene::GetOctreeOption(const Core::J_SPACE_SPATIAL_LAYER layer)const noexcept
	{
		return impl->GetOctreeOption(layer);
	}
	Core::JBvhOption JScene::GetBvhOption(const Core::J_SPACE_SPATIAL_LAYER layer)const noexcept
	{
		return impl->GetBvhOption(layer);
	}
	Core::JKdTreeOption JScene::GetKdTreeOption(const Core::J_SPACE_SPATIAL_LAYER layer)const noexcept
	{
		return impl->GetKdTreeOption(layer);
	}
	void JScene::SetOctreeOption(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JOctreeOption& newOption)noexcept
	{
		impl->SetOctreeOption(layer, newOption);
	}
	void JScene::SetBvhOption(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JBvhOption& newOption)noexcept
	{
		impl->SetBvhOption(layer, newOption);
	}
	void JScene::SetKdTreeOption(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JKdTreeOption& newOption)noexcept
	{
		impl->SetKdTreeOption(layer, newOption);
	}
	bool JScene::IsActivatedSceneTime()const noexcept
	{
		return impl->IsActivatedSceneTime();
	}
	bool JScene::IsPauseSceneTime()const noexcept
	{
		return impl->IsPauseSceneTime();
	}
	bool JScene::IsMainScene()const noexcept
	{
		return impl->useCaseType == J_SCENE_USE_CASE_TYPE::MAIN;
	}
	bool JScene::IsSpaceSpatialActivated()const noexcept
	{
		return impl->spatialStructure != nullptr;
	}
	bool JScene::HasComponent(const J_COMPONENT_TYPE cType)const noexcept
	{
		return impl->HasComponent(cType);
	}
	JUserPtr<JGameObject> JScene::FindGameObject(const size_t guid)noexcept
	{
		return impl->FindGameObject(guid);
	}
	JUserPtr<JCamera> JScene::FindFirstSelectedCamera(const bool allowEditorCam)const noexcept
	{
		return impl->FindFirstSelectedCamera(allowEditorCam);
	}
	JUserPtr<JGameObject> JScene::IntersectFirst(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JRay& ray)const noexcept
	{
		return impl->IntersectFirst(layer, ray);
	}
	void JScene::DoActivate() noexcept
	{
		JResourceObject::DoActivate();
		impl->Activate();
	}
	void JScene::DoDeActivate()noexcept
	{
		impl->DeActivate();
		JResourceObject::DoDeActivate();
	}
	JScene::JScene(const InitData& initData)
		: JResourceObject(initData), impl(std::make_unique<JSceneImpl>(initData, this))
	{ }
	JScene::~JScene()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JScenePrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JScenePrivate::AssetDataIOInterface;
	using CashInterface = JScenePrivate::CashInterface;
	using TimeInterface = JScenePrivate::TimeInterface;
	using OwnTypeInterface = JScenePrivate::OwnTypeInterface;
	using CompSettingInterface = JScenePrivate::CompSettingInterface;
	using CompRegisterInterface = JScenePrivate::CompRegisterInterface;
	using CompFrameInterface = JScenePrivate::CompFrameInterface;
	using CullingInterface = JScenePrivate::CullingInterface;
	using DebugInterface = JScenePrivate::DebugInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JScene>(*static_cast<JScene::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JResourceObjectPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JScene* scene = static_cast<JScene*>(createdPtr);
		scene->impl->RegisterThisPointer(scene);
		scene->impl->Initialize(static_cast<JScene::InitData*>(initData));
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JScene::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JScene::LoadData::StaticTypeInfo()))
			return nullptr;

		auto loadData = static_cast<JScene::LoadData*>(data);
		auto pathData = loadData->pathData;
		JUserPtr<JDirectory> directory = loadData->directory;
		JScene::LoadMetaData metadata(loadData->directory);

		if (LoadMetaData(pathData.engineMetaFileWPath, &metadata) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;

		JUserPtr<JScene> newScene = nullptr;
		if (directory->HasFile(metadata.guid))
			newScene = Core::GetUserPtr<JScene>(JScene::StaticTypeInfo().TypeGuid(), metadata.guid);

		if (newScene == nullptr)
		{
			std::unique_ptr<JScene::InitData> initData = std::make_unique<JScene::InitData>(pathData.name,
				metadata.guid,
				metadata.flag,
				(uint8)metadata.formatIndex,
				directory,
				metadata.useCaseType);

			auto idenUser = sPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &sPrivate);
			newScene.ConnnectChild(idenUser);
		}
		if (newScene != nullptr  && metadata.isMainScene)
		{ 
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
							metadata.octreeOption[i].commonOption.innerRoot = newScene->impl->debugRoot;
						else
							metadata.octreeOption[i].commonOption.innerRoot = Core::GetUserPtr<JGameObject>(metadata.innerRootGuid[occIndex][i]);
					}
					metadata.octreeOption[i].commonOption.debugRoot = newScene->impl->debugRoot;
					newScene->SetOctreeOption((Core::J_SPACE_SPATIAL_LAYER)i, metadata.octreeOption[i]);

					if (metadata.hasInnerRoot[bvhIndex][i])
					{
						if (i == (int)Core::J_SPACE_SPATIAL_LAYER::DEBUG_OBJECT)
							metadata.bvhOption[i].commonOption.innerRoot = newScene->impl->debugRoot;
						else
							metadata.bvhOption[i].commonOption.innerRoot = Core::GetUserPtr<JGameObject>(metadata.innerRootGuid[bvhIndex][i]);
					}
					metadata.bvhOption[i].commonOption.debugRoot = newScene->impl->debugRoot;
					newScene->SetBvhOption((Core::J_SPACE_SPATIAL_LAYER)i, metadata.bvhOption[i]);

					if (metadata.hasInnerRoot[kdIndex][i])
					{
						if (i == (int)Core::J_SPACE_SPATIAL_LAYER::DEBUG_OBJECT)
							metadata.kdTreeOption[i].commonOption.innerRoot = newScene->impl->debugRoot;
						else
							metadata.kdTreeOption[i].commonOption.innerRoot = Core::GetUserPtr<JGameObject>(metadata.innerRootGuid[kdIndex][i]);
					}
					metadata.kdTreeOption[i].commonOption.debugRoot = newScene->impl->debugRoot;
					newScene->SetKdTreeOption((Core::J_SPACE_SPATIAL_LAYER)i, metadata.kdTreeOption[i]);
				}
			} 
		}
		return newScene;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JScene::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JScene::StoreData*>(data);
		if (!storeData->HasCorrectType(JScene::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		StoreMetaData(data);

		JUserPtr<JScene> scene;
		scene.ConnnectChild(storeData->obj);
		return scene->impl->WriteAssetData() ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JScene::LoadMetaData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		std::wifstream stream;
		stream.open(path, std::ios::in | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto loadMetaData = static_cast<JScene::LoadMetaData*>(data);
		if (LoadCommonMetaData(stream, loadMetaData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JFileIOHelper::LoadEnumData(stream, loadMetaData->useCaseType);
		JFileIOHelper::LoadAtomicData(stream, loadMetaData->isOpen);
		JFileIOHelper::LoadAtomicData(stream, loadMetaData->isMainScene);
		JFileIOHelper::LoadAtomicData(stream, loadMetaData->isActivatedSpaceSpatial);

		for (uint i = 0; i < (uint)Core::J_SPACE_SPATIAL_LAYER::COUNT; ++i)
		{
			const uint occIndex = (uint)Core::J_SPACE_SPATIAL_TYPE::OCTREE;
			const uint bvhIndex = (uint)Core::J_SPACE_SPATIAL_TYPE::BVH;
			const uint kdIndex = (uint)Core::J_SPACE_SPATIAL_TYPE::KD_TREE;

			loadMetaData->octreeOption[i].Load(stream, loadMetaData->hasInnerRoot[occIndex][i], loadMetaData->innerRootGuid[occIndex][i]);
			loadMetaData->bvhOption[i].Load(stream, loadMetaData->hasInnerRoot[bvhIndex][i], loadMetaData->innerRootGuid[bvhIndex][i]);
			loadMetaData->kdTreeOption[i].Load(stream, loadMetaData->hasInnerRoot[kdIndex][i], loadMetaData->innerRootGuid[kdIndex][i]);
		}
		stream.close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JScene::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JScene::StoreData*>(data);
		JUserPtr<JScene> scene;
		scene.ConnnectChild(storeData->obj);

		std::wofstream stream;
		stream.open(scene->GetMetaFilePath(), std::ios::out | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (StoreCommonMetaData(stream, storeData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JFileIOHelper::StoreEnumData(stream, L"UseCaseType:", scene->GetUseCaseType());
		JFileIOHelper::StoreAtomicData(stream, Core::JFileConstant::StreamLastOpenSymbol(JScene::StaticTypeInfo()), scene->IsValid());
		JFileIOHelper::StoreAtomicData(stream, L"IsMainScene:", scene->IsMainScene());
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
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	const std::vector<JUserPtr<JGameObject>>& CashInterface::GetGameObjectCashVec(const JUserPtr<JScene>& scene, const J_RENDER_LAYER rLayer, const J_MESHGEOMETRY_TYPE meshType)noexcept
	{
		return scene->impl->GetGameObjectCashVec(rLayer, meshType);
	}
	const std::vector<JUserPtr<JComponent>>& CashInterface::GetComponentCashVec(const JUserPtr<JScene>& scene, const J_COMPONENT_TYPE cType)noexcept
	{
		return scene->impl->GetComponentCashVec(cType);
	}

	void TimeInterface::ActivateSceneTime(const JUserPtr<JScene>& scene)noexcept
	{
		scene->impl->ActivateSceneTime();
	}
	void TimeInterface::PlaySceneTimer(const JUserPtr<JScene>& scene, const bool value)noexcept
	{
		scene->impl->PlaySceneTimer(value);
	}
	void TimeInterface::DeActivateSceneTime(const JUserPtr<JScene>& scene)noexcept
	{
		scene->impl->DeActivateSceneTime();
	}

	bool OwnTypeInterface::AddGameObject(const JUserPtr<JGameObject>& gObject)noexcept
	{
		return gObject->GetOwnerScene()->impl->AddGameObject(gObject);
	}
	bool OwnTypeInterface::RemoveGameObject(const JUserPtr<JGameObject>& gObject)noexcept
	{
		return gObject->GetOwnerScene()->impl->RemoveGameObject(gObject);
	}

	void CompSettingInterface::UpdateTransform(const JUserPtr<JGameObject>& gObject)noexcept
	{
		gObject->GetOwnerScene()->impl->UpdateTransform(gObject);
	}

	bool CompRegisterInterface::RegisterComponent(const JUserPtr<JComponent>& comp)noexcept
	{
		return comp->GetOwner()->GetOwnerScene()->impl->RegisterComponent(comp);
	}
	bool CompRegisterInterface::DeRegisterComponent(const JUserPtr<JComponent>& comp)noexcept
	{
		return comp->GetOwner()->GetOwnerScene()->impl->DeRegisterComponent(comp);
	}
	bool CompRegisterInterface::ReRegisterComponent(const JUserPtr<JComponent>& comp)noexcept
	{
		DeRegisterComponent(comp);
		return RegisterComponent(comp);
	}

	void CompFrameInterface::SetAllComponentFrameDirty(const JUserPtr<JScene>& scene)noexcept
	{
		scene->impl->SetAllComponentFrameDirty();
	}
	void CompFrameInterface::SetComponentFrameDirty(const JUserPtr<JScene>& scene, const J_COMPONENT_TYPE cType, JUserPtr<JComponent> stComp, SetCompCondition condiiton)noexcept
	{
		scene->impl->SetComponentFrameDirty(cType, stComp, condiiton);
	}
 
	void CullingInterface::ViewCulling(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& comp)noexcept
	{
		const J_COMPONENT_TYPE compType = comp->GetComponentType();
		if (compType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
		{
			JCamera* cam = static_cast<JCamera*>(comp.Get());
			scene->impl->ViewCulling(cam->CullingUserInterface(), cam->GetBoundingFrustum());
		}
	}
 
	void DebugInterface::BuildDebugTree(const JUserPtr<JScene>& scene, Core::J_SPACE_SPATIAL_TYPE type, const Core::J_SPACE_SPATIAL_LAYER layer, _Out_ Editor::JEditorBinaryTreeView& tree)noexcept
	{
		scene->impl->BuildDebugTree(type, layer, tree);
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JScenePrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	JResourceObjectPrivate::AssetDataIOInterface& JScenePrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}

}