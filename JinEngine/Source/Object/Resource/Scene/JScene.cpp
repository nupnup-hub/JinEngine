/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JScene.h"
#include"JScenePrivate.h"
#include"JSceneManager.h"
#include"JSceneManagerPrivate.h"
#include"Accelerator/JSceneAcceleratorStructure.h" 
#include"../JClearableInterface.h"
#include"../JResourceObjectHint.h"
#include"../Mesh/JMeshGeometry.h" 
#include"../Material/JMaterial.h"    
#include"../../Component/Light/JLight.h" 
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
#include"../../JObjectFileIOHelper.h"
#include"../../GraphicRule/JGraphicModuleInterfaceHolder.h"
#include"../../GraphicRule/JGraphicModuleUtility.h"
#include"../../GraphicRule/JGraphicModuleMacro.h"
#include"../../../Application/Project/JApplicationProject.h"
#include"../../../Core/Identity/JIdenCreator.h"
#include"../../../Core/Guid/JGuidCreator.h" 
#include"../../../Core/Time/JGameTimer.h"
#include"../../../Core/File/JFileConstant.h"  
#include"../../../Core/Utility/JCommonUtility.h"  
#include"../../../Core/Reflection/JTypeImplBase.h"
#include<DirectXColors.h> 
  
namespace JinEngine
{
	namespace
	{ 
		using SetCompCondition = bool(*)(const JUserPtr<JComponent>&);
		using CompSortPtr = JScenePrivate::CompRegisterInterface::CompSortPtr;
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
 
	class JScene::JSceneImpl : public Core::JTypeImplBase, public JClearableInterface 
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JSceneImpl)
	public: 
		JWeakPtr<JScene> thisPointer;
		JFastPtr<JGraphicModuleManagedDataFrame> graphicData;
	public:
		JUserPtr<JGameObject> root;
		JUserPtr<JGameObject> debugRoot;
		std::unique_ptr<JSceneAcceleratorStructure> accelerator;
		std::vector<JUserPtr<JGameObject>> allObjects;
		std::vector<JUserPtr<JGameObject>> objectLayer[(int)J_RENDER_LAYER::COUNT][(int)Core::J_MESHGEOMETRY_TYPE::COUNT];
		std::vector<JUserPtr<JComponent>> componentCash[totalCompVariation];
		//std::unordered_map<J_COMPONENT_TYPE, std::vector<JUserPtr<JComponent>>> componentCash;
	public:
		const size_t debugRootGuid;
		const J_SCENE_USE_CASE_TYPE useCaseType;
	public:
		std::unique_ptr<Core::JGameTimer> sceneTimer;
		bool requestInitialize = false;
	public:
		JSceneImpl(const InitData& initData, JScene* thisSceneRaw)
			:debugRootGuid(Core::MakeGuid()), useCaseType(initData.useCaseType)
		{ 
			if (useCaseType == J_SCENE_USE_CASE_TYPE::MAIN ||
				useCaseType == J_SCENE_USE_CASE_TYPE::THREE_DIMENSIONAL_PREVIEW)
				accelerator = std::make_unique<JSceneAcceleratorStructure>(); 
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
			const std::vector<JUserPtr<JComponent>>& rVec = componentCash[ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_RENDERITEM>()];
			for (const auto& data : rVec)
				sum += static_cast<JRenderItem*>(data.Get())->GetSubmeshCount();
			return sum;
		}
		const std::vector<JUserPtr<JGameObject>>& GetGameObjectCacheVec(const J_RENDER_LAYER rLayer, const Core::J_MESHGEOMETRY_TYPE meshType)const noexcept
		{
			return objectLayer[(int)rLayer][(int)meshType];
		}
		const std::vector<JUserPtr<JComponent>>& GetComponentCacheVec(const UniqueIndex index)const noexcept
		{
			return componentCash[index]; 
		} 
		const std::vector<JUserPtr<JComponent>>& GetComponentCacheVec(const JComponent* comp)const noexcept
		{
			return componentCash[ConvertUniqueIndex(comp->GetComponentType(), comp->GetSubTypeIndex())];
		}
		std::vector<JUserPtr<JComponent>> GetComponentVec(const UniqueIndex index)const noexcept
		{
			return componentCash[index];
		}
		JOctreeOption GetOctreeOption(const J_ACCELERATOR_LAYER layer)const noexcept
		{
			return accelerator != nullptr ? accelerator->GetOctreeOption(layer) : JOctreeOption();
		}
		JBvhOption GetBvhOption(const J_ACCELERATOR_LAYER layer)const noexcept
		{
			return accelerator != nullptr ? accelerator->GetBvhOption(layer) : JBvhOption();
		}
		JKdTreeOption GetKdTreeOption(const J_ACCELERATOR_LAYER layer)const noexcept
		{
			return accelerator != nullptr ? accelerator->GetKdTreeOption(layer) : JKdTreeOption();
		}
		DirectX::BoundingBox GetSceneBBox(const J_ACCELERATOR_LAYER layer)const noexcept
		{
			Core::JBBox result;
			bool isValid = false;
			if (accelerator != nullptr)
				result = accelerator->GetSceneBBox(layer, isValid);

			if (isValid && !result.IsDistanceZero())
				return result.Convert();
			else
			{
				result = Core::JBBox();
				auto& vec = GetComponentCacheVec(ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_RENDERITEM>());
				for (const auto& data : vec)
				{
					JRenderItem* r = static_cast<JRenderItem*>(data.Get());
					if (ConvertAcceleratorLayer(r->GetRenderLayer()) == layer)
						result = Core::JBBox::Union(result, r->GetBoundingBox());
				}
				return result.Convert();
			}
		}
		float GetTotalTime()const noexcept
		{
			return IsActivatedSceneTime() ? sceneTimer->TotalTime() : 0;
		}
		float GetDeltaTime()const noexcept
		{
			return IsActivatedSceneTime() ? sceneTimer->DeltaTime() : 0;
		}
	public:
		void SetOctreeOption(const J_ACCELERATOR_LAYER layer, JOctreeOption newOption)noexcept
		{
			if (accelerator == nullptr)
				return; 

			if (newOption.commonOption.innerRoot == nullptr)
				newOption.commonOption.innerRoot = root;
			if (newOption.commonOption.debugRoot == nullptr)
				newOption.commonOption.debugRoot = debugRoot;

			accelerator->SetOctreeOption(layer, newOption);
			SetComponentFrameDirty(ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_CAMERA>(), 0, SpaceStructureUseCamCond());
		}
		void SetBvhOption(const J_ACCELERATOR_LAYER layer, JBvhOption newOption)noexcept
		{
			if (accelerator == nullptr)
				return;

			if (newOption.commonOption.innerRoot == nullptr)
				newOption.commonOption.innerRoot = root;
			if (newOption.commonOption.debugRoot == nullptr)
				newOption.commonOption.debugRoot = debugRoot;

			accelerator->SetBvhOption(layer, newOption);
			SetComponentFrameDirty(ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_CAMERA>(), 0, SpaceStructureUseCamCond());
		}
		void SetKdTreeOption(const J_ACCELERATOR_LAYER layer, JKdTreeOption newOption)noexcept
		{
			if (accelerator == nullptr)
				return;

			if (newOption.commonOption.innerRoot == nullptr)
				newOption.commonOption.innerRoot = root;
			if (newOption.commonOption.debugRoot == nullptr)
				newOption.commonOption.debugRoot = debugRoot;

			accelerator->SetKdTreeOption(layer, newOption);
			SetComponentFrameDirty(ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_CAMERA>(), 0, SpaceStructureUseCamCond());
		}
		void SetGpuAcceleratorBuildOption(const JGpuAcceleratorOption& option)
		{
			if (accelerator == nullptr || !thisPointer->IsMainScene())
				return;

			accelerator->SetGpuAccelerator(option);
			SetComponentFrameDirty(ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_CAMERA>(), 0, SpaceStructureUseCamCond());
		}
		void SetGpuAcceleratorOnlyOpaque()
		{ 
			if (accelerator == nullptr)
				return;
			  
			JGpuAcceleratorOption buildOption;
			buildOption.root = root;
			buildOption.flag = J_GPU_ACCELERATOR_BUILD_OPTION_OPAQUE;
			//J_GPU_ACCELERATOR_BUILD_OPTION_LIGHT_SHAPE
			SetGpuAcceleratorBuildOption(buildOption);
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
		bool HasComponent(const UniqueIndex index)const noexcept
		{
			return componentCash[index].size() > 0;
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
			auto& vec = GetComponentCacheVec(ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_CAMERA>());
			for (const auto& data : vec)
			{ 
				if (!allowEditorCam && data->GetOwner()->IsEditorObject())
					continue;
				return Core::ConnectChildUserPtr<JCamera>(data);
			}
			return nullptr;
		}
		JUserPtr<JGameObject> IntersectFirst(JAcceleratorIntersectInfo& info)const noexcept
		{
			info.untilFirst = true; 
			accelerator->Intersect(info);
			return info.result.size() != 0 ? info.result[0].obj : nullptr;
		}
		void Intersect(JAcceleratorIntersectInfo& info)const noexcept
		{
			info.untilFirst = false;
			accelerator->Intersect(info); 
		}
		void Contain(JAcceleratorContainInfo& info)
		{
			accelerator->Contain(info);
		}
		std::vector<JUserPtr<JGameObject>> AlignedObject(JAcceleratorAlignInfo& info)const noexcept
		{
			if (accelerator != nullptr)
				return accelerator->AlignedObject(info);
			else
				return std::vector<JUserPtr<JGameObject>>();
		}
		void AlignedObjectF(JAcceleratorAlignInfo& info, _Out_ std::vector<JUserPtr<JGameObject>>& aligned, _Out_ int& count)const noexcept
		{
			if (accelerator != nullptr)
				accelerator->AlignedObjectF(info, aligned, count);
		}
	public:
		void CreateDefaultGameObject()noexcept
		{
			const J_OBJECT_FLAG sceneFlag = thisPointer->GetFlag();
			const J_OBJECT_FLAG rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE);
			root = JGCI::CreateRoot(L"RootGameObject", Core::MakeGuid(), Core::AddSQValueEnum(sceneFlag, rootFlag), thisPointer);

			const bool is3DScene = (useCaseType == J_SCENE_USE_CASE_TYPE::MAIN) ||
				(useCaseType == J_SCENE_USE_CASE_TYPE::THREE_DIMENSIONAL_PREVIEW);
			J_OBJECT_FLAG objFlag = Core::AddSQValueEnum(sceneFlag, OBJECT_FLAG_UNDESTROYABLE);
			if (is3DScene)
				JGCI::CreateSky(root, objFlag, L"DefaultSky");

			JUserPtr<JGameObject> mainCam;
			const bool canCreateCulling = useCaseType == J_SCENE_USE_CASE_TYPE::MAIN || useCaseType == J_SCENE_USE_CASE_TYPE::THREE_DIMENSIONAL_PREVIEW;
			mainCam = JGCI::CreateCamera(root, L"MainCamera", Core::MakeGuid(), objFlag, canCreateCulling);
			
			auto cam = mainCam->GetComponent<JCamera>();
			cam->SetCameraState(J_CAMERA_STATE::RENDER);
			JUserPtr<JGameObject> lit = JGCI::CreateLight(root, L"MainLight", Core::MakeGuid(), objFlag, J_LIGHT_TYPE::DIRECTIONAL);
			if (is3DScene)
				lit->GetComponent<JLight>()->SetShadow(true);
		}
		void CreateDebugRoot()noexcept
		{
			const J_OBJECT_FLAG rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_DO_NOT_SAVE | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE);
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
				const std::vector<JUserPtr<JComponent>>& cashVec = GetComponentCacheVec(ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_ANIMATOR>());
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
			const std::vector<JUserPtr<JComponent>>& cashVec = GetComponentCacheVec(ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_ANIMATOR>());
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
		void UpdateTransform(const JUserPtr<JComponent>& comp)noexcept
		{
			// && owner->GetRenderItem()->GetRenderLayer() != J_RENDER_LAYER::DEBUG
			if (comp == nullptr)
				return;

			if (thisPointer->IsActivated() && accelerator != nullptr)
				accelerator->UpdateGameObject(comp);
		} 
	public:
		bool RegisterComponent(const JUserPtr<JComponent>& component, CompSortPtr comparePtr)noexcept
		{
			if (!component->PassDefectInspection())
				return false;

			const J_COMPONENT_TYPE compType = component->GetComponentType();
			const UniqueIndex uniqueIndex = ConvertUniqueIndex(component->GetComponentType(), component->GetSubTypeIndex());
			auto& cashVec = componentCash[uniqueIndex];

			const uint compCount = (uint)cashVec.size();
			const size_t guid = component->GetGuid();

			//for sorting	 
			int insertIndex = -1;
			for (uint i = 0; i < compCount; ++i)
			{
				if (cashVec[i]->GetGuid() == guid)
					return false;

				if (comparePtr != nullptr && insertIndex == -1 && comparePtr(component, cashVec[i]))
					insertIndex = i;
			}

			if (comparePtr == nullptr || insertIndex == -1)
				cashVec.push_back(component);
			else
				cashVec.insert(cashVec.begin() + insertIndex, component);

			if (compType == J_COMPONENT_TYPE::ENGINE_RENDERITEM)
			{
				JUserPtr<JRenderItem> jRItem = Core::ConnectChildUserPtr<JRenderItem>(component);
				const J_RENDER_LAYER renderLayer = jRItem->GetRenderLayer();
				const Core::J_MESHGEOMETRY_TYPE meshType = jRItem->GetMesh()->GetMeshGeometryType();
				objectLayer[(int)renderLayer][(int)meshType].push_back(jRItem->GetOwner());

				if (accelerator != nullptr)
					accelerator->AddGameObject(jRItem);
			} 
			else if (compType == J_COMPONENT_TYPE::ENGINE_LIGHT && accelerator != nullptr)
				accelerator->AddGameObject(Core::ConnectChildUserPtr<JLight>(component));
			return true;
		}
		bool DeRegisterComponent(const JUserPtr<JComponent>& component)noexcept
		{
			const J_COMPONENT_TYPE compType = component->GetComponentType();
			const UniqueIndex uniqueIndex = ConvertUniqueIndex(component->GetComponentType(), component->GetSubTypeIndex());
			auto& cashVec = componentCash[uniqueIndex];

			int hitIndex = -1; 
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

			if (compType == J_COMPONENT_TYPE::ENGINE_RENDERITEM)
			{
				JUserPtr<JRenderItem> jRItem = Core::ConnectChildUserPtr<JRenderItem>(component);
				JUserPtr<JGameObject> jOwner = jRItem->GetOwner();

				const int rIndex = (int)jRItem->GetRenderLayer();
				const size_t guid = jOwner->GetGuid();

				for (uint j = 0; j < (uint)Core::J_MESHGEOMETRY_TYPE::COUNT; ++j)
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
				if (accelerator != nullptr)
					accelerator->RemoveGameObject(jRItem);
			} 
			else if (compType == J_COMPONENT_TYPE::ENGINE_LIGHT && accelerator != nullptr)
				accelerator->RemoveGameObject(Core::ConnectChildUserPtr<JLight>(component));
			cashVec.erase(cashVec.begin() + hitIndex); 
			return true;
		}
	public:
		void SetAllComponentFrameDirty()noexcept
		{
			for(uint i = 0; i < SIZE_OF_ARRAY(componentCash); ++i)
				SetComponentFrameDirty(i, 0);
		}
		void SetComponentFrameDirty(const J_COMPONENT_TYPE cType, const uint localIndex, JUserPtr<JComponent> stComp = nullptr, SetCompCondition condiiton = nullptr)noexcept
		{
			if (!CTypeCommonCall::GetCTypeHint(cType).hasFrameDirty)
				return;
			 
			const UniqueIndex uniuqeIndex = ConvertUniqueIndex(cType, localIndex);
			if (stComp != nullptr)
				SetComponentFrameDirty(uniuqeIndex, (uint)JCUtil::GetTypeIndex(GetComponentCacheVec(uniuqeIndex), stComp->GetGuid()), condiiton);
			else
				SetComponentFrameDirty(uniuqeIndex, (uint)0, condiiton);
		}
		void SetComponentFrameDirty(const UniqueIndex typeIndex, const uint stIndex, SetCompCondition condiiton = nullptr)noexcept
		{ 
			const std::vector<JUserPtr<JComponent>>& cashVec = GetComponentCacheVec(typeIndex);
			const uint compCount = (uint)cashVec.size();

			if (condiiton != nullptr)
			{
				for (uint i = stIndex; i < compCount; ++i)
				{
					if (condiiton(cashVec[i]))
					{
						auto fUser = cashVec[i]->ModuleManagedData()->GetFrameUpdateUserInterface();
						if (fUser == nullptr)
							continue;

						fUser->SetFrameDirty();
					} 
				}
			}
			else
			{
				for (uint i = stIndex; i < compCount; ++i)
				{
					auto fUser = cashVec[i]->ModuleManagedData()->GetFrameUpdateUserInterface();
					if (fUser == nullptr)
						continue;

					fUser->SetFrameDirty();
				}
			}
		}
	public:
		bool AllowLightCulling()const noexcept
		{
			return useCaseType == J_SCENE_USE_CASE_TYPE::MAIN;
		}
	public:
		void ViewCulling(JCullingUserInterface*cullUser, const DirectX::BoundingFrustum& frustum)
		{
			JAcceleratorCullingInfo info(cullUser, frustum);
			accelerator->Culling(info);
		}
		void ViewCulling(JCullingUserInterface*cullUser, const DirectX::BoundingBox& bbox)
		{
			JAcceleratorCullingInfo info(cullUser, bbox);
			accelerator->Culling(info);
		}
		void ViewCulling(JAcceleratorCullingInfo& cullInfo)
		{ 
			accelerator->Culling(cullInfo);
		}
		void InitializeAccelerator()noexcept
		{
			if (accelerator != nullptr)
			{
				J_ACCELERATOR_LAYER commonLayer = J_ACCELERATOR_LAYER::COMMON_OBJECT;
				J_ACCELERATOR_LAYER debugLayer = J_ACCELERATOR_LAYER::DEBUG_OBJECT;

				JOctreeOption octreeOption = accelerator->GetOctreeOption(commonLayer);
				octreeOption.commonOption.innerRoot = root;
				octreeOption.commonOption.debugRoot = debugRoot;
				accelerator->SetOctreeOption(commonLayer, octreeOption);

				JBvhOption bvhOption = accelerator->GetBvhOption(commonLayer);
				bvhOption.commonOption.innerRoot = root;
				bvhOption.commonOption.debugRoot = debugRoot;
				bvhOption.commonOption.isAcceleratorActivated = true;
				bvhOption.commonOption.isCullingActivated = true;
				accelerator->SetBvhOption(commonLayer, bvhOption);

				JKdTreeOption kdOption = accelerator->GetKdTreeOption(commonLayer);
				//kdOption.isOcclusionCullingActivated = true;
				kdOption.commonOption.innerRoot = root;
				kdOption.commonOption.debugRoot = debugRoot;
				accelerator->SetKdTreeOption(commonLayer, kdOption);

				JBvhOption bvhDebugOption = accelerator->GetBvhOption(debugLayer);
				bvhDebugOption.commonOption.innerRoot = debugRoot;
				bvhDebugOption.commonOption.debugRoot = debugRoot;
				bvhDebugOption.commonOption.isAcceleratorActivated = true;

				//bvhDebugOption.commonOption.isDebugActivated = true;
				//bvhDebugOption.commonOption.isDebugLeafOnly = true;
				accelerator->SetBvhOption(debugLayer, bvhDebugOption); 
				/*
				JKdTreeOption kdDebugOption;
				kdDebugOption.commonOption.innerRoot = debugRoot;
				kdDebugOption.commonOption.debugRoot = debugRoot;
				kdDebugOption.commonOption.isAcceleratorActivated = true;
				accelerator->SetKdTreeOption(debugLayer, kdOption);
				*/
				 
				SetGpuAcceleratorOnlyOpaque();
			}
		}
	public:
		void BuildDebugTree(const J_ACCELERATOR_TYPE type, const J_ACCELERATOR_LAYER layer, JAcceleratorVisualizeInterface* tree)noexcept
		{
			if (accelerator != nullptr)
				accelerator->BuildDebugTree(type, layer, tree);
		}
	public: 
		void Activate()
		{			 
			SceneMangerAccess::RegisterScene(thisPointer);
			JGMUtil::CreateFrame(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SCENE_PASS, thisPointer->GetGuid()); 
			if (accelerator != nullptr)
				accelerator->Activate(graphicData);

			StuffResource();
			JGameObjectPrivate::ActivateInterface::Activate(root);
			JGameObjectPrivate::ActivateInterface::Activate(debugRoot);
			SetAllComponentFrameDirty();
		}
		void DeActivate()
		{  
			JGameObjectPrivate::ActivateInterface::DeActivate(root);
			JGameObjectPrivate::ActivateInterface::DeActivate(debugRoot);
			ClearResource();

			if (accelerator != nullptr)
				accelerator->DeAcitvate();
			GMI()->DestroyFrameUploadData(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SCENE_PASS);
			SceneMangerAccess::DeRegisterScene(thisPointer);  
		}
	public:
		bool ReadAssetData()
		{
			JFileIOTool tool;
			if (!tool.Begin(thisPointer->GetPath(), JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
				return false;

			auto data = JGameObjectPrivate::AssetDataIOInterface::CreateLoadAssetDIData(thisPointer, tool);
			root.ConnnectChild(JGameObjectPrivate::AssetDataIOInterface::LoadAssetData(data.get()));
			tool.Close();
			 
			return true;
		}
		bool WriteAssetData()
		{
			JFileIOTool tool;
			if (!tool.Begin(thisPointer->GetPath(), JFileIOTool::TYPE::JSON))
				return false;

			auto data = JGameObjectPrivate::AssetDataIOInterface::CreateStoreAssetDIData(root, tool);
			Core::J_FILE_IO_RESULT res = JGameObjectPrivate::AssetDataIOInterface::StoreAssetData(data.get());
			tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
			return res == Core::J_FILE_IO_RESULT::SUCCESS;
		}
	public:
		void StuffResource()
		{ 			
			if (requestInitialize)
				Initialize();

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
				BeginForcedDestroy(root.Release()); 

				if (debugRoot != nullptr)
					BeginForcedDestroy(debugRoot.Release()); 

				for (int i = 0; i < (int)J_RENDER_LAYER::COUNT; ++i)
				{
					for (int j = 0; j < (int)Core::J_MESHGEOMETRY_TYPE::COUNT; ++j)
					{
						objectLayer[i][j].clear();
						objectLayer[i][j].shrink_to_fit();
					}
				}
				for(uint i = 0; i < SIZE_OF_ARRAY(componentCash); ++i)
					componentCash[i].clear();

				allObjects.clear();
				allObjects.shrink_to_fit();
				thisPointer->SetValid(false);
			}
		}
	public:
		void NotifyReAlloc()
		{  
		}
	public:
		void Initialize()
		{
			CreateDefaultGameObject();
			CreateDebugRoot();
			if (accelerator != nullptr)
				InitializeAccelerator();
			thisPointer->SetValid(true);
			requestInitialize = false;
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

			RegisterRTypeInfo(JScene::StaticTypeInfo(), rTypeHint, rTypeCFunc, RTypePrivateFunc{});
			Core::JIdentifier::RegisterPrivateInterface(JScene::StaticTypeInfo(), sPrivate);

			IMPL_REALLOC_BIND()
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

	JScene::LoadMetadata::LoadMetadata(const JUserPtr<JDirectory>& directory)
		: JResourceObject::InitData(JScene::StaticTypeInfo(), GetDefaultFormatIndex(), GetStaticResourceType(), directory)
	{}

	Core::JIdentifierPrivate& JScene::PrivateInterface()const noexcept
	{
		return sPrivate;
	}
	JGraphicModuleManagedDataFrame* JScene::ModuleManagedData()const noexcept
	{
		return impl->graphicData.Get();
	}
	uint JScene::GetSubTypeIndex()const noexcept
	{
		return 0;
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
	uint JScene::GetGameObjectCount(const J_RENDER_LAYER layer)const noexcept
	{
		uint sum = 0;
		for (uint i = 0; i < (uint)Core::J_MESHGEOMETRY_TYPE::COUNT; ++i)
			sum += (uint)impl->objectLayer[(uint)layer][i].size();
		return sum;
	}
	uint JScene::GetComponetCount(const UniqueIndex index)const noexcept
	{
		return (uint)impl->GetComponentCacheVec(index).size();
	}
	uint JScene::GetMeshCount()const noexcept
	{
		return (uint)impl->GetMeshCount();
	}
	J_SCENE_USE_CASE_TYPE JScene::GetUseCaseType()const noexcept
	{
		return impl->useCaseType;
	}
	std::vector<JUserPtr<JGameObject>> JScene::GetGameObjectVec()const noexcept
	{
		return impl->allObjects;
	}
	std::vector<JUserPtr<JGameObject>> JScene::GetGameObjectVec(const J_RENDER_LAYER layer, const Core::J_MESHGEOMETRY_TYPE mesh)const noexcept
	{
		return impl->objectLayer[(uint)layer][(uint)mesh];
	}
	const std::vector<JUserPtr<JGameObject>>& JScene::GetGameObjectCacheVec(const J_RENDER_LAYER rLayer, const Core::J_MESHGEOMETRY_TYPE meshType)noexcept
	{
		return impl->GetGameObjectCacheVec(rLayer, meshType);
	}
	std::vector<JUserPtr<JComponent>> JScene::GetComponentVec(const UniqueIndex index)const noexcept
	{
		return impl->GetComponentVec(index);
	}
	const std::vector<JUserPtr<JComponent>>& JScene::GetComponentCacheVec(const UniqueIndex index)noexcept
	{
		return impl->GetComponentCacheVec(index);
	} 
	JUserPtr<JComponent> JScene::GetFirstComponent(const UniqueIndex index)const noexcept
	{
		const std::vector<JUserPtr<JComponent>>& vec = impl->GetComponentCacheVec(index);
		return vec.size() > 0 ? vec[0] : nullptr;
	}
	JOctreeOption JScene::GetOctreeOption(const J_ACCELERATOR_LAYER layer)const noexcept
	{
		return impl->GetOctreeOption(layer);
	}
	JBvhOption JScene::GetBvhOption(const J_ACCELERATOR_LAYER layer)const noexcept
	{
		return impl->GetBvhOption(layer);
	}
	JKdTreeOption JScene::GetKdTreeOption(const J_ACCELERATOR_LAYER layer)const noexcept
	{
		return impl->GetKdTreeOption(layer);
	}
	DirectX::BoundingBox JScene::GetSceneBBox(const J_ACCELERATOR_LAYER layer)const noexcept
	{
		return impl->GetSceneBBox(layer);
	}
	float JScene::GetTotalTime()const noexcept
	{
		return impl->GetTotalTime();
	}
	float JScene::GetDeltaTime()const noexcept
	{
		return impl->GetDeltaTime();
	}
	void JScene::SetOctreeOption(const J_ACCELERATOR_LAYER layer, const JOctreeOption& newOption)noexcept
	{
		impl->SetOctreeOption(layer, newOption);
	}
	void JScene::SetBvhOption(const J_ACCELERATOR_LAYER layer, const JBvhOption& newOption)noexcept
	{
		impl->SetBvhOption(layer, newOption);
	}
	void JScene::SetKdTreeOption(const J_ACCELERATOR_LAYER layer, const JKdTreeOption& newOption)noexcept
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
	bool JScene::IsAcceleratorActivated()const noexcept
	{
		return impl->accelerator != nullptr;
	}
	bool JScene::HasComponent(const UniqueIndex index)const noexcept
	{
		return impl->HasComponent(index);
	}
	bool JScene::HasCanCullingAccelerator(const J_ACCELERATOR_LAYER layer)const noexcept
	{
		return impl->accelerator != nullptr && impl->accelerator->HasCanCullingAccelerator(layer);
	}
	bool JScene::AllowLightCulling()const noexcept
	{
		return impl->AllowLightCulling();
	}
	bool JScene::CanUseAcceleratorUtility(const J_ACCELERATOR_LAYER layer, const J_ACCELERATOR_TYPE type)const noexcept
	{
		return IsAcceleratorActivated() && impl->accelerator->IsActivated(layer, type);
	}
	JUserPtr<JGameObject> JScene::FindGameObject(const size_t guid)noexcept
	{
		return impl->FindGameObject(guid);
	}
	JUserPtr<JCamera> JScene::FindFirstSelectedCamera(const bool allowEditorCam)const noexcept
	{
		return impl->FindFirstSelectedCamera(allowEditorCam);
	} 
	JUserPtr<JGameObject> JScene::IntersectFirst(const Core::JRay& ray, const J_ACCELERATOR_LAYER layer, const bool findOtherAcceleratorIfTypeNull)const noexcept
	{
		JAcceleratorIntersectInfo info(ray, layer, J_ACCELERATOR_SORT_TYPE::NOT_USE, false, true);
		info.findOtherAcceleratorIfTypeNull = findOtherAcceleratorIfTypeNull;

		return IntersectFirst(info);
	}
	JUserPtr<JGameObject> JScene::IntersectFirst(JAcceleratorIntersectInfo& info)const noexcept
	{ 
		return impl->IntersectFirst(info);
	}
	void JScene::Intersect(JAcceleratorIntersectInfo& info)const noexcept
	{
		impl->Intersect(info);
	}
	void JScene::Contain(JAcceleratorContainInfo& info)
	{
		impl->Contain(info);
	}
	std::vector<JUserPtr<JGameObject>> JScene::AlignedObject(JAcceleratorAlignInfo& info)const noexcept
	{
		return impl->AlignedObject(info);
	}
	void JScene::AlignedObjectF(JAcceleratorAlignInfo& info, _Out_ std::vector<JUserPtr<JGameObject>>& aligned, _Out_ int& count)const noexcept
	{
		impl->AlignedObjectF(info, aligned, count);
	}
	void JScene::ViewCulling(const JUserPtr<JComponent>& comp)noexcept
	{
		const J_COMPONENT_TYPE compType = comp->GetComponentType();
		if (compType == J_COMPONENT_TYPE::ENGINE_CAMERA)
		{
			JCamera* cam = static_cast<JCamera*>(comp.Get());
			JAcceleratorCullingInfo info(cam->ModuleManagedData()->GetCullingUserInterface(), cam->GetBoundingFrustum());

			ViewCulling(info);
		}
		else if (compType == J_COMPONENT_TYPE::ENGINE_LIGHT)
		{
			JLight* lit = static_cast<JLight*>(comp.Get());
			JAcceleratorCullingInfo info(lit->ModuleManagedData()->GetCullingUserInterface(), lit->GetBBox());

			ViewCulling(info);
		}
	}
	void JScene::ViewCulling(JAcceleratorCullingInfo& info)noexcept
	{
		impl->ViewCulling(info);
	}
	void JScene::DoActivate() noexcept
	{
		auto reAllocModuleManagedDataPtr = [](const JFastPtr<JGraphicModuleManagedDataFrame>& data, JObject* object)					
		{									
			auto scene = static_cast<ThisType*>(object);
			if (scene->impl->accelerator != nullptr)
				scene->impl->accelerator->SetGraphicData(data);
			scene->impl->graphicData = data;
		};																																
		impl->graphicData = GMI()->Allocate(JGraphicModuleManagedDataCreationDesc(impl->thisPointer, reAllocModuleManagedDataPtr));
 		
		JResourceObject::DoActivate();
		impl->Activate();
	}
	void JScene::DoDeActivate()noexcept
	{
		impl->DeActivate();
		JResourceObject::DoDeActivate();
		GMI()->DeAllocate(impl->graphicData);
	}
	JScene::JScene(const InitData& initData)
		: JResourceObject(initData), impl(std::make_unique<JSceneImpl>(initData, this))
	{ 
	}
	JScene::~JScene()
	{ 
		impl.reset();
	}

	using CreateInstanceInterface = JScenePrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JScenePrivate::AssetDataIOInterface;
	using TimeInterface = JScenePrivate::TimeInterface;
	using OwnTypeInterface = JScenePrivate::OwnTypeInterface;
	using CompSettingInterface = JScenePrivate::CompSettingInterface;
	using CompRegisterInterface = JScenePrivate::CompRegisterInterface;
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
		//새로생성된 scene에 경우 기존에 파일을 가져오는게 불가능하므로 request default object creation
		scene->impl->requestInitialize = !scene->HasFile(); 
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
		JScene::LoadMetadata metadata(loadData->directory);

		if (LoadMetadata(pathData.metaFilePath, &metadata) != Core::J_FILE_IO_RESULT::SUCCESS)
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
			if (metadata.isActivatedAccelerator)
			{
				for (uint i = 0; i < (uint)J_ACCELERATOR_LAYER::COUNT; ++i)
				{
					const uint occIndex = (uint)J_ACCELERATOR_TYPE::OCTREE;
					const uint bvhIndex = (uint)J_ACCELERATOR_TYPE::BVH;
					const uint kdIndex = (uint)J_ACCELERATOR_TYPE::KD_TREE;

					if (metadata.hasInnerRoot[occIndex][i])
					{
						if (i == (int)J_ACCELERATOR_LAYER::DEBUG_OBJECT)
							metadata.octreeOption[i].commonOption.innerRoot = newScene->impl->debugRoot;
						else
							metadata.octreeOption[i].commonOption.innerRoot = Core::GetUserPtr<JGameObject>(metadata.innerRootGuid[occIndex][i]);
					}
					metadata.octreeOption[i].commonOption.debugRoot = newScene->impl->debugRoot;
					newScene->SetOctreeOption((J_ACCELERATOR_LAYER)i, metadata.octreeOption[i]);

					if (metadata.hasInnerRoot[bvhIndex][i])
					{
						if (i == (int)J_ACCELERATOR_LAYER::DEBUG_OBJECT)
							metadata.bvhOption[i].commonOption.innerRoot = newScene->impl->debugRoot;
						else
							metadata.bvhOption[i].commonOption.innerRoot = Core::GetUserPtr<JGameObject>(metadata.innerRootGuid[bvhIndex][i]);
					}
					metadata.bvhOption[i].commonOption.debugRoot = newScene->impl->debugRoot;
					newScene->SetBvhOption((J_ACCELERATOR_LAYER)i, metadata.bvhOption[i]);

					if (metadata.hasInnerRoot[kdIndex][i])
					{
						if (i == (int)J_ACCELERATOR_LAYER::DEBUG_OBJECT)
							metadata.kdTreeOption[i].commonOption.innerRoot = newScene->impl->debugRoot;
						else
							metadata.kdTreeOption[i].commonOption.innerRoot = Core::GetUserPtr<JGameObject>(metadata.innerRootGuid[kdIndex][i]);
					}
					metadata.kdTreeOption[i].commonOption.debugRoot = newScene->impl->debugRoot;
					newScene->SetKdTreeOption((J_ACCELERATOR_LAYER)i, metadata.kdTreeOption[i]);
				}
				newScene->impl->SetGpuAcceleratorOnlyOpaque();
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

		//StoreMetadata(data);
		JUserPtr<JScene> scene;
		scene.ConnnectChild(storeData->obj);
		return scene->impl->WriteAssetData() ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadMetadata(const std::wstring& path, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JScene::LoadMetadata::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JFileIOTool tool;
		if (!tool.Begin(path, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto loadMetadata = static_cast<JScene::LoadMetadata*>(data);
		if (LoadCommonMetadata(tool, loadMetadata) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JObjectFileIOHelper::LoadEnumData(tool, loadMetadata->useCaseType, "UseCaseType:");
		JObjectFileIOHelper::LoadAtomicData(tool, loadMetadata->isOpen, Core::JFileConstant::GetLastOpenSymbol(JScene::StaticTypeInfo()));
		JObjectFileIOHelper::LoadAtomicData(tool, loadMetadata->isMainScene, "IsMainScene:");
		JObjectFileIOHelper::LoadAtomicData(tool, loadMetadata->isActivatedAccelerator, "IsActivatedAccelerator:");

		tool.PushExistStack("AcceleratorOption");
		for (uint i = 0; i < (uint)J_ACCELERATOR_LAYER::COUNT; ++i)
		{
			const uint occIndex = (uint)J_ACCELERATOR_TYPE::OCTREE;
			const uint bvhIndex = (uint)J_ACCELERATOR_TYPE::BVH;
			const uint kdIndex = (uint)J_ACCELERATOR_TYPE::KD_TREE;

			tool.PushExistStack();
			loadMetadata->octreeOption[i].Load(tool, loadMetadata->hasInnerRoot[occIndex][i], loadMetadata->innerRootGuid[occIndex][i]);
			loadMetadata->bvhOption[i].Load(tool, loadMetadata->hasInnerRoot[bvhIndex][i], loadMetadata->innerRootGuid[bvhIndex][i]);
			loadMetadata->kdTreeOption[i].Load(tool, loadMetadata->hasInnerRoot[kdIndex][i], loadMetadata->innerRootGuid[kdIndex][i]);
			tool.PopStack();
		}
		tool.PopStack();
		tool.Close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetadata(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JScene::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JScene::StoreData*>(data);
		JUserPtr<JScene> scene;
		scene.ConnnectChild(storeData->obj);

		JFileIOTool tool;
		if (!tool.Begin(scene->GetMetaFilePath(), JFileIOTool::TYPE::JSON))
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (StoreCommonMetadata(tool, storeData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
 
		JObjectFileIOHelper::StoreEnumData(tool, scene->GetUseCaseType(), "UseCaseType:");
		JObjectFileIOHelper::StoreAtomicData(tool, scene->IsValid(), Core::JFileConstant::GetLastOpenSymbol(JScene::StaticTypeInfo()));
		JObjectFileIOHelper::StoreAtomicData(tool, scene->IsMainScene(), "IsMainScene:");
		JObjectFileIOHelper::StoreAtomicData(tool, scene->IsAcceleratorActivated(), "IsActivatedAccelerator:");

		tool.PushArrayOwner("AcceleratorOption");
		for (uint i = 0; i < (uint)J_ACCELERATOR_LAYER::COUNT; ++i)
		{
			JOctreeOption octreeOption = scene->GetOctreeOption((J_ACCELERATOR_LAYER)i);
			JBvhOption bvhOption = scene->GetBvhOption((J_ACCELERATOR_LAYER)i);
			JKdTreeOption kdTreeOption = scene->GetKdTreeOption((J_ACCELERATOR_LAYER)i);

			tool.PushArrayMember();
			octreeOption.Store(tool);
			bvhOption.Store(tool);
			kdTreeOption.Store(tool);
			tool.PopStack();
		}
		tool.PopStack();
		tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
		return Core::J_FILE_IO_RESULT::SUCCESS;
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

	void CompSettingInterface::UpdateTransform(const JUserPtr<JComponent>& comp)noexcept
	{
		comp->GetOwner()->GetOwnerScene()->impl->UpdateTransform(comp);
	} 

	bool CompRegisterInterface::RegisterComponent(const JUserPtr<JComponent>& comp, CompSortPtr comparePtr)noexcept
	{
		return comp->GetOwner()->GetOwnerScene()->impl->RegisterComponent(comp, comparePtr);
	}
	bool CompRegisterInterface::DeRegisterComponent(const JUserPtr<JComponent>& comp)noexcept
	{
		return comp->GetOwner()->GetOwnerScene()->impl->DeRegisterComponent(comp);
	}
	bool CompRegisterInterface::ReRegisterComponent(const JUserPtr<JComponent>& comp, CompSortPtr comparePtr)noexcept
	{
		DeRegisterComponent(comp);
		return RegisterComponent(comp, comparePtr);
	}
 
	void DebugInterface::BuildDebugTree(const JUserPtr<JScene>& scene, J_ACCELERATOR_TYPE type, const J_ACCELERATOR_LAYER layer, JAcceleratorVisualizeInterface* tree)noexcept
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