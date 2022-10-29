#pragma once   
#include"JSceneInterface.h" 
#include"../Mesh/JMeshType.h"
#include"../../Component/RenderItem/JRenderLayer.h"
#include"../../../Core/SpaceSpatial/Bvh/JBvhType.h"
#include"../../../Core/Storage/JStorage.h"
#include<memory>
#include<vector>  

namespace JinEngine
{
	class JComponent;
	class JGameObject;

	namespace Graphic
	{
		class JGraphicImpl;
	}

	namespace Core
	{
		class JSceneSpatialStructure;
	}
	class JScene : public JSceneInterface
	{
		REGISTER_CLASS(JScene)
	protected:
		struct JSceneMetadata final: public JResourceMetaData
		{
		public:
			bool isOpen = false;
			bool isMainScene = false;
			bool isSpatialStructureActivated = false;
			int spatialStructureType;
		};
	public:
		struct JSceneInitData : public JResourceInitData
		{
		public:
			JSceneInitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				JDirectory* directory,
				const uint8 formatIndex = JResourceObject::GetFormatIndex<JScene>(GetAvailableFormat()[0]));
			JSceneInitData(const std::wstring& name,
				JDirectory* directory,
				const uint8 formatIndex = JResourceObject::GetFormatIndex<JScene>(GetAvailableFormat()[0]));
			JSceneInitData(JDirectory* directory,
				const uint8 formatIndex = JResourceObject::GetFormatIndex<JScene>(GetAvailableFormat()[0]));
		public:
			J_RESOURCE_TYPE GetResourceType() const noexcept;
		};
		using InitData = JSceneInitData;
	private:
		JGameObject* root = nullptr;
		std::unique_ptr<Core::JSceneSpatialStructure> spatialStructure;
		std::vector<JGameObject*> allObjects;
		std::vector<JGameObject*> objectLayer[(int)J_RENDER_LAYER::COUNT][(int)J_MESHGEOMETRY_TYPE::COUNT];
		std::unordered_map<J_COMPONENT_TYPE, std::vector<JComponent*>> componentCash;
		JCamera* mainCamera = nullptr;
		bool isAnimatorActivated = false;
		bool isSpatialStructureActivated = false;  
	public:
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::SCENE;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
	public:
		JGameObject* FindGameObject(const size_t guid)noexcept;
		JGameObject* GetRootGameObject()noexcept;
		JGameObject* GetGameObject(const uint index)noexcept;
		JCamera* GetMainCamera()noexcept;
		uint GetGameObjectCount()const noexcept;
		uint GetComponetCount(const J_COMPONENT_TYPE cType)const noexcept;
		uint GetMeshCount()const noexcept;

		bool IsAnimatorActivated()const noexcept; 
		bool IsMainScene()const noexcept; 
		bool HasComponent(const J_COMPONENT_TYPE cType)const noexcept;
	public:
		JSceneCashInterface* CashInterface() final;
		JSceneGameObjInterface* GameObjInterface() final;
		JSceneCompInterface* CompInterface()final;
		JSceneRegisterInterface* RegisterInterface() final;
		JSceneFrameInterface* AppInterface() final;
		JSceneSpaceSpatialInterface* SpaceSpatialInterface() final;
	private:
		void DoCopy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void StuffResource() final;
		void ClearResource() final;
	private:
		void CreateDefaultGameObject()noexcept;
	private:
		const std::vector<JGameObject*>& GetGameObjectCashVec(const J_RENDER_LAYER rLayer, const J_MESHGEOMETRY_TYPE meshType)const noexcept final;
		const std::vector<JComponent*>& GetComponentCashVec(const J_COMPONENT_TYPE cType)const noexcept final;

		bool AddGameObject(JGameObject& newGameObject)noexcept final;
		bool RemoveGameObject(JGameObject& gameObj)noexcept final;

		void SetAnimation()noexcept final;
		void SetMainCamera(JCamera* mainCam)noexcept final;
		void UpdateTransform(JGameObject* owner)noexcept final;

		bool RegisterComponent(JComponent& component)noexcept final;
		bool DeRegisterComponent(JComponent& component)noexcept final;

		void SetAllComponentDirty()noexcept final;
		void SetComponentDirty(const J_COMPONENT_TYPE cType) noexcept final;
		void SetBackSideComponentDirty(JComponent& jComp) noexcept final;
		void SetBackSideComponentDirty(JComponent& jComp, bool(*condition)(JComponent&))noexcept final;

		//SceneSpatial
		void ViewCulling()noexcept final;
		Core::JSceneSpatialStructureOption GetSpatialStructureOption()const noexcept final;
		void SetSceneSpatialStructure(const bool value)noexcept final;
		void SetSceneSpatialStructureType(const Core::J_SCENE_SPATIAL_STRUCTURE_TYPE type)noexcept final;
		void SetDebugBoundingBox(const bool value)noexcept final;
		void SetDebugOnlyLeaf(const bool value)noexcept final;
	private:
		void BuildSpatialStructure(const Core::J_SCENE_SPATIAL_STRUCTURE_TYPE type)noexcept;
		void ClearSpatialStructure()noexcept;
		void BuildOctree(const uint minSize, const uint octreeSizeSquare, const float looseFactor, const bool isLooseOctree)noexcept;
		void BuildBvh(const Core::J_BVH_BUILD_TYPE bvhBuildType, const Core::J_BVH_SPLIT_TYPE splitType)noexcept;
		void CreateDemoGameObject()noexcept;
		void DestroyDemoGameObject()noexcept;
	private:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JScene* scene); 
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JScene* scene);
		static JScene* LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData);
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, const std::wstring& folderPath, JSceneMetadata& metadata);
		static void RegisterJFunc();
	private:
		JScene(const JSceneInitData& initdata);
		~JScene();
	};
}

/*

		uint StuffObjConstants(_Inout_ JFrameResource* frameResource, uint offset = 0)noexcept;
		uint StuffPassConstants(_Inout_ JFrameResource* frameResource,
			_In_ JGameTimer* JGameTimer,
			const uint width,
			const uint height,
			JCamera* camera = nullptr,
			uint offset = 0)noexcept;
		uint StuffAnimationConstants(_Inout_ JFrameResource* frameResource,
			_Inout_ JAnimationConstants* animationConstants,
			_In_ JGameTimer* JGameTimer,
			uint offset = 0)noexcept;


				uint JScene::StuffObjConstants(_Inout_ JFrameResource* frameResource, uint offset)noexcept
	{
		uint updateCount = 0;
		auto currObjectCB = frameResource->ObjectCB.get();
		const uint allObjCount = (uint)allObjects.size();
		for (uint i = 0; i < allObjCount; ++i)
		{
			JRenderItem* renderItem = allObjects[i]->GetRenderItem();
			if (renderItem != nullptr && renderItem->PassDefectInspection())
			{
				if (allObjects[i]->IsObjectConstantDirtied())
				{
					const XMFLOAT4X4 fTexTransform = renderItem->GetTextransform();
					const XMMATRIX world = allObjects[i]->GetTransform()->GetWorld();
					const XMMATRIX texTransform = XMLoadFloat4x4(&fTexTransform);

					JObjectConstants objConstants;
					XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
					XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
					objConstants.MaterialIndex = renderItem->GetMaterial()->GetMatCBIndex();
					currObjectCB->CopyData(renderItem->GetObjCBIndex() + offset, objConstants);
					allObjects[i]->MinusTrnasformDirty();
					allObjects[i]->MinusRenderItemDirty();
				}
			}
		}
		return updateCount;
	}
	uint JScene::StuffPassConstants(_Inout_ JFrameResource* frameResource,
		_In_ JGameTimer* timer,
		const uint width,
		const uint height,
		JCamera* camera,
		uint offset)noexcept
	{
		JPassConstants passContants;
		if (camera == nullptr)
			camera = GetMainCamera();
		camera->UpdateViewMatrix();
		camera->StuffPassConstant(passContants);
		const uint lightCount = (uint)light.size();
		for (uint i = 0; i < lightCount; ++i)
			light[i]->StuffPassConstant(passContants.lights[i]);

		passContants.renderTargetSize = XMFLOAT2((float)width, (float)height);
		passContants.invRenderTargetSize = XMFLOAT2(1.0f / width, 1.0f / height);
		passContants.totalTime = timer->TotalTime();
		passContants.deltaTime = timer->DeltaTime();
		passContants.ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

		auto currPassCB = frameResource->PassCB.get();
		currPassCB->CopyData(offset, passContants);

		return 1;
	}
	uint JScene::StuffAnimationConstants(_Inout_ JFrameResource* frameResource,
		_Inout_ JAnimationConstants* animationConstants,
		_In_ JGameTimer* timer,
		uint offset)noexcept
	{
		int updateCount = 0;
		auto currSkinnedCB = frameResource->SkinnedCB.get();
		const uint animatorCount = (uint)animators.size();

		for (uint i = 0; i < animatorCount; ++i)
		{
			if (!animators[i]->IsActivated())
				continue;

			animators[i]->Update(timer, animationConstants->boneTransforms);
			currSkinnedCB->CopyData(animators[i]->GetAnimationCBIndex() + offset, *animationConstants);
			++updateCount;
		}
		return updateCount;
	}

*/