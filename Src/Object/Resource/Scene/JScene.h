#pragma once  
#include"IScene.h"
#include"JSceneInterface.h"
#include"../Mesh/JMeshType.h"
#include"../../Component/RenderItem/JRenderLayer.h"
#include<memory>
#include<vector>  

namespace JinEngine
{
	class JAnimator;
	class JRenderItem;
	class JComponent;
	class JCamera;
	class JLight;
	class JGameObject;
	class JResourceIO;

	namespace Graphic
	{
		class JGraphicImpl;
	}

	namespace Core
	{
		class JSceneSpatialStructure;
	}
	class JScene : public JSceneInterface, public IScene
	{
		REGISTER_CLASS(JScene)
	private:
		friend class JResourceIO;
		friend class Graphic::JGraphicImpl;
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
		std::vector<JAnimator*> animators;
		std::vector<JRenderItem*> renderItems;
		std::vector<JCamera*> cameras;
		std::vector<JLight*>light;
		std::vector<JLight*>shadowLight;
		JCamera* mainCamera;
		bool isOpen;
		bool activateAnimator; 
	public:
		bool IsOpen()const noexcept;
		bool IsAnimatorActivated()const noexcept;

		JGameObject* FindGameObject(const size_t guid)noexcept;
		JGameObject* GetRootGameObject()noexcept;
		JGameObject* GetGameObject(const uint index)noexcept;
		JAnimator* GetAnimator(const uint index)noexcept;
		JCamera* GetMainCamera()noexcept;
		uint GetGameObjectCount()const noexcept;
		uint GetAnimatorCount()const noexcept;
		uint GetCameraCount()const noexcept;
		uint GetRenderItemCount()const noexcept; 
		uint GetLightCount()const noexcept;
		uint GetShadowCount()const noexcept; 
		J_RESOURCE_TYPE GetResourceType()const noexcept final; 		
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::SCENE;
		}
		std::string GetFormat()const noexcept final;
		static std::vector<std::string> GetAvailableFormat()noexcept;
	protected:
		void DoActivate() noexcept final;
		void DoDeActivate()noexcept final; 
	private:  		
		void AddGameObject(JGameObject* newGameObject)noexcept;
		void EraseGameObject(JGameObject* gameObj)noexcept; 
		void MakeDefaultObject(bool isEditorScene)noexcept final;
		//ISceneFrameDirty
		void SetAllComponentDirty()noexcept; 
		void SetAllTransformDirty()noexcept;
		void SetAllRenderItemDirty()noexcept;
		void SetAllAnimatorDirty()noexcept;
		void SetAllCameraDirty()noexcept;
		void SetAllLightDirty()noexcept;

		//ISceneGameObjectEventListener
		bool RegisterAnimator(JGameObject* gameObject, JAnimator* animator)noexcept;
		bool RegisterCamera(JGameObject* gameObject, JCamera* camera)noexcept;
		bool RegisterLight(JGameObject* gameObject, JLight* light)noexcept;
		bool RegisterShadowLight(JGameObject* gameObject, JLight* light)noexcept;
		bool RegisterRenderItem(JGameObject* gameObject, JRenderItem* renderItem)noexcept;
		bool DeRegisterAnimator(JGameObject* gameObject, JAnimator* animator)noexcept;
		bool DeRegisterCamera(JGameObject* gameObject, JCamera* camera)noexcept;
		bool DeRegisterLight(JGameObject* gameObject, JLight* light)noexcept;
		bool DeRegisterShadowLight(JGameObject* gameObject, JLight* light)noexcept;
		bool DeRegisterRenderItem(JGameObject* gameObject, JRenderItem* renderItem)noexcept;
		JCamera* SetMainCamera(JGameObject* gameObject, JCamera* camera)noexcept;  
		void UpdateGameObjectTransform(JGameObject* gameObject)noexcept;

		//ISceneSpatialStructure
		void ViewCulling()noexcept;
		void OnSceneSpatialStructure()noexcept;
		void OffSceneSpatialStructure()noexcept;
		void OnDebugBoundingBox(bool onlyLeafNode)noexcept;
		void OffDebugBoundingBox()noexcept;
		void CreateDemoGameObject()noexcept;
		void EraseDemoGameObject()noexcept;
		void BuildOctree(const uint octreeSizeSquare, const float looseFactor, const bool isLooseOctree)noexcept;
		void BuildBvh(const Core::J_BVH_BUILD_TYPE bvhBuildType, const Core::J_BVH_SPLIT_TYPE splitType)noexcept;

		//ISceneAnimator
		void ActivateAnimtor()noexcept;
	private:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JScene* scene);
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JScene* scene);
		static JScene* LoadObject(JDirectory* directory, const JResourcePathData& pathData);
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, const std::string& folderPath, JSceneMetadata& metadata);
		static void RegisterFunc();
	private:
		JScene(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
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