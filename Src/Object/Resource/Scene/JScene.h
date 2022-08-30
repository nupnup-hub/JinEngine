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
		struct JSceneMetadata : public ObjectMetadata
		{
		public:
			bool isOpen;
		};
	private:
		JGameObject* root;
		std::unique_ptr<Core::JSceneSpatialStructure> spatialStructure;
		std::vector<JGameObject*> allObjects;
		std::vector<JGameObject*> objectLayer[(int)J_RENDER_LAYER::COUNT][(int)J_MESHGEOMETRY_TYPE::COUNT];
		std::unordered_map<J_COMPONENT_TYPE, std::vector<JComponent*>> componentCash;
		JCamera* mainCamera;
		bool isAnimatorActivated;
		bool isSpatialStructureActivated;
		bool isSpatialStructureDebugActivated;
	public:
		JGameObject* FindGameObject(const size_t guid)noexcept;
		JGameObject* GetRootGameObject()noexcept;
		JGameObject* GetGameObject(const uint index)noexcept;
		JCamera* GetMainCamera()noexcept;
		uint GetGameObjectCount()const noexcept;
		uint GetComponetCount(const J_COMPONENT_TYPE cType)const noexcept;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::SCENE;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;

		bool IsAnimatorActivated()const noexcept; 
		bool IsMainScene()const noexcept;
	public:
		JSceneCashInterface* CashInterface() final;
		JSceneGameObjInterface* GameObjInterface() final;
		JSceneCompInterface* CompInterface()final;
		JSceneRegisterInterface* RegisterInterface() final;
		JSceneFrameInterface* FrameInterface() final;
		JSceneSpaceSpatialInterface* SpaceSpatialInterface() final;
	public:
		bool Copy(JObject* ori) final;
	protected:
		void DoActivate() noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void StuffResource() final;
		void ClearResource() final;
	private:
		std::vector<JGameObject*>& GetGameObjectCashVec(const J_RENDER_LAYER rLayer, const J_MESHGEOMETRY_TYPE meshType)noexcept final;
		std::vector<JComponent*>& GetComponentCashVec(const J_COMPONENT_TYPE cType)noexcept final;

		JGameObject* AddGameObject(JGameObject& newGameObject)noexcept;
		bool RemoveGameObject(JGameObject& gameObj)noexcept final;

		void SetAnimation()noexcept final;
		JCamera* SetMainCamera(JCamera* animator)noexcept final;

		bool RegisterComponent(JComponent& component)noexcept final;
		bool DeRegisterComponent(JComponent& component)noexcept final;

		void SetAllComponentDirty()noexcept final;
		void SetComponentDirty(const J_COMPONENT_TYPE cType) noexcept final;
		void SetBackSideComponentDirty(JComponent& jComp) noexcept final;
		void SetBackSideComponentDirty(JComponent& jComp, bool(*condition)(JComponent&))noexcept final;

		//SceneSpatial
		void ViewCulling()noexcept final;
		void OnSceneSpatialStructure()noexcept final;
		void OffSceneSpatialStructure()noexcept final;
		void OnDebugBoundingBox(bool onlyLeafNode)noexcept final;
		void OffDebugBoundingBox()noexcept final;
		void BuildOctree(const uint octreeSizeSquare, const float looseFactor, const bool isLooseOctree)noexcept;
		void BuildBvh(const Core::J_BVH_BUILD_TYPE bvhBuildType, const Core::J_BVH_SPLIT_TYPE splitType)noexcept;
		void CreateDemoGameObject()noexcept;
		void DestroyDemoGameObject()noexcept;
	private:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JScene* scene);
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JScene* scene);
		static JScene* LoadObject(JDirectory* directory, const JResourcePathData& pathData);
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, const std::wstring& folderPath, JSceneMetadata& metadata);
		static void RegisterJFunc();
	private:
		JScene(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
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