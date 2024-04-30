#include"JGraphicDrawList.h" 
#include"GraphicResource/JGraphicResourceInfo.h"
#include"../Object/Resource/Scene/JScene.h" 
#include"../Object/Resource/Scene/JScenePrivate.h" 
#include"../Object/Component/JComponent.h" 
#include"../Object/Component/Camera/JCamera.h" 
#include"../Object/Component/Light/JLight.h" 
#include"../Object/GameObject/JGameObject.h"
#include"../Core/Utility/JCommonUtility.h" 
#include"FrameResource/JFrameUpdate.h" 
#include"FrameResource/JFrameUpdateUserAccess.h"

namespace JinEngine
{
	namespace Graphic
	{
		JShadowMapDrawRequestor::JShadowMapDrawRequestor(const JUserPtr<JLight>& jLight, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)
			:jLight(jLight), updateFrequency(updateFrequency)
		{}
		JShadowMapDrawRequestor::~JShadowMapDrawRequestor()
		{}

		JSceneDrawRequestor::JSceneDrawRequestor(const JUserPtr<JCamera>& jCamera, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)
			:jCamera(jCamera), updateFrequency(updateFrequency)
		{}
		JSceneDrawRequestor::~JSceneDrawRequestor()
		{}

		JSceneFrustumCullingRequestor::JSceneFrustumCullingRequestor(const JUserPtr<JComponent>& comp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)
			:comp(comp), updateFrequency(updateFrequency)
		{}
		JSceneFrustumCullingRequestor::~JSceneFrustumCullingRequestor()
		{}

		JSceneHzbOccCullingRequestor::JSceneHzbOccCullingRequestor(const JUserPtr<JComponent>& comp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)
			: comp(comp), updateFrequency(updateFrequency)
		{}
		JSceneHzbOccCullingRequestor::~JSceneHzbOccCullingRequestor()
		{}

		JSceneHdOccCullingRequestor::JSceneHdOccCullingRequestor(const JUserPtr<JComponent>& comp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)
			:comp(comp), updateFrequency(updateFrequency)
		{}
		JSceneHdOccCullingRequestor::~JSceneHdOccCullingRequestor()
		{}

		namespace
		{
			using JSceneDrawRequestorMap = std::unordered_map<size_t, JSceneDrawRequestor*>;
			using JShadowMapDrawRequestorMap = std::unordered_map<size_t, JShadowMapDrawRequestor*>;
			using JSceneFrustumCullingRequestorMap = std::unordered_map<size_t, JSceneFrustumCullingRequestor*>;
			using JSceneHzbOccRequestorMap = std::unordered_map<size_t, JSceneHzbOccCullingRequestor*>;
			using JSceneHdOccRequestorMap = std::unordered_map<size_t, JSceneHdOccCullingRequestor*>;
			struct RequestorMapData
			{
			public:
				JSceneDrawRequestorMap scene;
				JShadowMapDrawRequestorMap shadow;
				JSceneFrustumCullingRequestorMap frustum;
				JSceneHzbOccRequestorMap hzbOcc;
				JSceneHdOccRequestorMap hdOcc;
			};
			static RequestorMapData mapData;
		}
		namespace
		{
			using CompareDrawListPtr = bool(*)(JGraphicDrawTarget*, const size_t);
			using CompareSceneDrawRequestorPtr = bool(*)(JSceneDrawRequestor*, const size_t);
			using CompareShadowDrawRequestorPtr = bool(*)(JShadowMapDrawRequestor*, const size_t);
			using CompareFrustumRequestorPtr = bool(*)(JSceneFrustumCullingRequestor*, const size_t);
			using CompareHzbOcclusionRequestorPtr = bool(*)(JSceneHzbOccCullingRequestor*, const size_t);
			using CompareHdOcclusionRequestorPtr = bool(*)(JSceneHdOccCullingRequestor*, const size_t);

			static std::vector<std::unique_ptr<JGraphicDrawTarget>> drawList;
			static CompareDrawListPtr compareDrawListPtr = [](JGraphicDrawTarget* target, const size_t guid) {return target->scene->GetGuid() == guid; };
			static CompareSceneDrawRequestorPtr compareSceneDrawRequestorPtr = [](JSceneDrawRequestor* target, const size_t guid) {return target->jCamera->GetGuid() == guid; };
			static CompareShadowDrawRequestorPtr compareShadowDrawRequestorPtr = [](JShadowMapDrawRequestor* target, const size_t guid) {return target->jLight->GetGuid() == guid; };
			static CompareFrustumRequestorPtr compareFrustumRequestorPtr = [](JSceneFrustumCullingRequestor* target, const size_t guid) {return target->comp->GetGuid() == guid; };
			static CompareHzbOcclusionRequestorPtr compareHzbOcclusionRequestorPtr = [](JSceneHzbOccCullingRequestor* target, const size_t guid) {return target->comp->GetGuid() == guid; };
			static CompareHdOcclusionRequestorPtr compareHdOcclusionRequestorPtr = [](JSceneHdOccCullingRequestor* target, const size_t guid) {return target->comp->GetGuid() == guid; };

			int GetIndex(const JUserPtr<JScene>& scene)noexcept
			{
				return JCUtil::GetIndex(drawList, compareDrawListPtr, scene->GetGuid());
			}
			int GetSceneReqIndex(const int drawListIndex, const JUserPtr<JComponent>& comp)noexcept
			{
				if (drawListIndex == -1)
					return false;

				return JCUtil::GetIndex(drawList[drawListIndex]->sceneRequestor, compareSceneDrawRequestorPtr, comp->GetGuid());
			}
			int GetShadowReqIndex(const int drawListIndex, const JUserPtr<JComponent>& comp)noexcept
			{
				if (drawListIndex == -1)
					return false;
				return JCUtil::GetIndex(drawList[drawListIndex]->shadowRequestor, compareShadowDrawRequestorPtr, comp->GetGuid());
			}
			int GetFrustumReqIndex(const int drawListIndex, const JUserPtr<JComponent>& comp)noexcept
			{
				if (drawListIndex == -1)
					return false;
				return JCUtil::GetIndex(drawList[drawListIndex]->frustumCullingRequestor, compareFrustumRequestorPtr, comp->GetGuid());
			}
			int GetHzbOccReqIndex(const int drawListIndex, const JUserPtr<JComponent>& comp)noexcept
			{
				if (drawListIndex == -1)
					return false;
				return JCUtil::GetIndex(drawList[drawListIndex]->hzbOccCullingRequestor, compareHzbOcclusionRequestorPtr, comp->GetGuid());
			}
			int GetHdOccReqIndex(const int drawListIndex, const JUserPtr<JComponent>& comp)noexcept
			{
				if (drawListIndex == -1)
					return false;
				return JCUtil::GetIndex(drawList[drawListIndex]->hdOccCullingRequestor, compareHdOcclusionRequestorPtr, comp->GetGuid());
			}
		}

		void JGraphicDrawTarget::UpdateInfo::BeginUpdate()
		{
			objUpdateCount = aniUpdateCount = camUpdateCount = lightUpdateCount = shadowMapUpdateCount = hzbOccUpdateCount = hdOccUpdateCount =
				hotObjUpdateCount = hotAniUpdateCount = hotCamUpdateCount = hotLitghtUpdateCount =
				thisFrameObjCount = 0;

			sceneUpdated = false;
			shadowUpdated = false;
			hzbOccCullingUpdated = hdOccCullingUpdated = false;

			if (observationFrame != nullptr && observationFrame->IsFrameDirted())
				sceneUpdated = true;
			hasObjRecopy = false;
		}
		void JGraphicDrawTarget::UpdateInfo::EndUpdate()
		{
			//cam이 자기자신의 update이외에 영향을 주는 객체들의 update count
			const uint sceneUpdateFactor = objUpdateCount + aniUpdateCount + lightUpdateCount;
			if (sceneUpdateFactor > 0 || nextSceneUpdate)
				sceneUpdated = true;
	 
			//const uint shadowUpdateCount = hotObjUpdateCount + hotAniUpdateCount + hotLitghtUpdateCount + shadowMapUpdateCount;
			//shadow map을 소유한 shadow requestor update이외에 영향을 주는 객체들의 update count
			const uint shadowUpdateFactor = objUpdateCount + aniUpdateCount + hasObjRecopy;
			if (shadowUpdateFactor > 0)
				shadowUpdated = true;
			 
			//const uint occUpdateHotFactor = hotObjUpdateCount + hotAniUpdateCount + hasObjRecopy;
			const uint occUpdateFactor = objUpdateCount + aniUpdateCount + hasObjRecopy;
			const uint occUpdateHotFactor = hotObjUpdateCount + hotAniUpdateCount + hasObjRecopy;

			hdOccCullingUpdated = occUpdateFactor;
			hzbOccCullingUpdated = occUpdateFactor;
			if (thisFrameObjCount < lastFrameObjCount)
				shadowUpdated = hdOccCullingUpdated = hzbOccCullingUpdated = true;

			lastFrameObjCount = thisFrameObjCount;
			nextSceneUpdate = false;
		}

		JGraphicDrawTarget::JGraphicDrawTarget(const JUserPtr<JScene>& scene)
			: scene(scene)
		{
			updateInfo = std::make_unique<UpdateInfo>();
		}
		JGraphicDrawTarget::~JGraphicDrawTarget() {}
		void JGraphicDrawTarget::BeginUpdate()
		{
			updateInfo->BeginUpdate();
			for (const auto& data : sceneRequestor)
			{
				data->isUpdated = false;
				data->canDrawThisFrame = false;
			}
			for (const auto& data : shadowRequestor)
			{
				data->isUpdated = false;
				data->canDrawThisFrame = false;
				if (data->passNextFrame)
					data->canDrawThisFrame = true;
				data->passNextFrame = false;
			}
			for (const auto& data : frustumCullingRequestor)
			{
				data->isUpdated = false;
				data->canDrawThisFrame = false;
			}
			for (const auto& data : hzbOccCullingRequestor)
			{
				data->isUpdated = false;
				data->canDrawThisFrame = false;
			}
			for (const auto& data : hdOccCullingRequestor)
			{
				data->isUpdated = false;  
				data->canDrawThisFrame = false;
			}
		}
		void JGraphicDrawTarget::EndUpdate()
		{ 
			//manage drawing trigger
			updateInfo->EndUpdate();
			for (const auto& data : sceneRequestor)
			{
				if (updateInfo->sceneUpdated || data->isUpdated || data->updateFrequency == J_GRAPHIC_DRAW_FREQUENCY::ALWAYS)
					data->canDrawThisFrame = true; 
			}
			for (const auto& data : shadowRequestor)
			{
				if (updateInfo->shadowUpdated || data->isUpdated || data->updateFrequency == J_GRAPHIC_DRAW_FREQUENCY::ALWAYS)
					data->canDrawThisFrame = true;
				if (data->passNextFrame)
					data->canDrawThisFrame = false;
			}
			for (const auto& data : frustumCullingRequestor)
			{
				if (updateInfo->sceneUpdated || data->isUpdated || data->updateFrequency == J_GRAPHIC_DRAW_FREQUENCY::ALWAYS)
					data->canDrawThisFrame = true;
				if (data->keepCanDrawTrigger)
					data->canDrawThisFrame = true;	
			}
			for (const auto& data : hzbOccCullingRequestor)
			{
				if (updateInfo->hzbOccCullingUpdated || data->isUpdated || data->updateFrequency == J_GRAPHIC_DRAW_FREQUENCY::ALWAYS)
					data->canDrawThisFrame = true;
				if (data->keepCanDrawTrigger)
					data->canDrawThisFrame = true;
			}
			for (const auto& data : hdOccCullingRequestor)
			{
				if (updateInfo->hdOccCullingUpdated || data->isUpdated || data->updateFrequency == J_GRAPHIC_DRAW_FREQUENCY::ALWAYS)
					data->canDrawThisFrame = true;
				if (data->keepCanDrawTrigger)
					data->canDrawThisFrame = true;
			}
		}

		bool JGraphicDrawList::AddDrawList(const JUserPtr<JScene>& scene)noexcept
		{
			if (scene == nullptr)
				return false;

			if (HasDrawList(scene))
				return false;

			std::unique_ptr<JGraphicDrawTarget> newTarget = std::make_unique<JGraphicDrawTarget>(scene);
			drawList.push_back(std::move(newTarget));
			return true;
		}
		bool JGraphicDrawList::PopDrawList(const JUserPtr<JScene>& scene)noexcept
		{
			if (scene == nullptr)
				return false;

			int index = GetIndex(scene);
			if (index == -1)
				return false;

			drawList.erase(drawList.begin() + index);
			const uint drawListCount = (uint)drawList.size();
			for (uint i = index; i < drawListCount; ++i)
				JScenePrivate::CompFrameInterface::SetAllComponentFrameDirty(drawList[i]->scene);

			return true;
		}
		bool JGraphicDrawList::AddObservationFrame(const JUserPtr<JScene>& scene, const JUserPtr<JFrameUpdateUserAccess>& observationFrame)noexcept
		{
			int index = GetIndex(scene);
			if (index == -1)
				return false;
			drawList[index]->updateInfo->observationFrame = observationFrame;
			return true;
		}
		bool JGraphicDrawList::HasDrawList(const JUserPtr<JScene>& scene)noexcept
		{
			const size_t sceneGuid = scene->GetGuid();
			const uint drawListCount = (uint)drawList.size();
			for (uint i = 0; i < drawListCount; ++i)
			{
				if (drawList[i]->scene->GetGuid() == sceneGuid)
					return true;
			}
			return false;
		}
		void JGraphicDrawList::UpdateScene(const JUserPtr<JScene>& scene, const J_COMPONENT_TYPE cType)noexcept
		{
			if (scene == nullptr)
				return;

			int index = GetIndex(scene);

			if (index == -1)
				return;

			const uint drawListCount = (uint)drawList.size();
			for (uint i = index + 1; i < drawListCount; ++i)
				JScenePrivate::CompFrameInterface::SetComponentFrameDirty(drawList[i]->scene, cType);
		}
		void JGraphicDrawList::AddDrawShadowRequest(const JUserPtr<JScene>& scene, const JUserPtr<JLight>& jLight)noexcept
		{
			if (scene == nullptr || jLight == nullptr)
				return;

			int index = GetIndex(scene); 
			if (index == -1)
				return;

			if (mapData.shadow.find(jLight->GetGuid()) != mapData.shadow.end())
				return;

			auto unqPtr = std::make_unique<JShadowMapDrawRequestor>(jLight, J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
			auto rawPtr = unqPtr.get();
			rawPtr->isUpdated = true;
			mapData.shadow.emplace(jLight->GetGuid(), rawPtr);

			drawList[index]->shadowRequestor.emplace_back(std::move(unqPtr)); 
		}
		void JGraphicDrawList::AddDrawSceneRequest(const JUserPtr<JScene>& scene, const JUserPtr<JCamera>& jCamera, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)noexcept
		{
			if (scene == nullptr || jCamera == nullptr)
				return;

			int index = GetIndex(scene);
			if (index == -1)
				return;

			if (mapData.scene.find(jCamera->GetGuid()) != mapData.scene.end())
				return;

			auto unqPtr = std::make_unique<JSceneDrawRequestor>(jCamera, updateFrequency);
			auto rawPtr = unqPtr.get();
			rawPtr->isUpdated = true;
			mapData.scene.emplace(jCamera->GetGuid(), rawPtr);

			drawList[index]->sceneRequestor.emplace_back(std::move(unqPtr)); 
		}
		void JGraphicDrawList::AddFrustumCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)noexcept
		{
			if (scene == nullptr || jComp == nullptr)
				return;

			const J_COMPONENT_TYPE compType = jComp->GetComponentType();
			int index = GetIndex(scene);
			if (index == -1)
				return;

			auto unqPtr = std::make_unique<JSceneFrustumCullingRequestor>(jComp, updateFrequency);
			auto rawPtr = unqPtr.get();
			rawPtr->isUpdated = true;
			mapData.frustum.emplace(jComp->GetGuid(), rawPtr);

			drawList[index]->frustumCullingRequestor.emplace_back(std::move(unqPtr)); 
		}
		void JGraphicDrawList::AddHzbOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)noexcept
		{
			if (scene == nullptr || jComp == nullptr)
				return;

			const J_COMPONENT_TYPE compType = jComp->GetComponentType();
			int index = GetIndex(scene);
			if (index == -1)
				return;

			auto unqPtr = std::make_unique<JSceneHzbOccCullingRequestor>(jComp, updateFrequency);
			auto rawPtr = unqPtr.get();
			rawPtr->isUpdated = true;
			mapData.hzbOcc.emplace(jComp->GetGuid(), rawPtr);

			drawList[index]->hzbOccCullingRequestor.emplace_back(std::move(unqPtr));
		}
		void JGraphicDrawList::AddHdOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)noexcept
		{
			if (scene == nullptr || jComp == nullptr)
				return;

			const J_COMPONENT_TYPE compType = jComp->GetComponentType();
			int index = GetIndex(scene);
			if (index == -1)
				return;

			auto unqPtr = std::make_unique<JSceneHdOccCullingRequestor>(jComp, updateFrequency);
			auto rawPtr = unqPtr.get();
			rawPtr->isUpdated = true;
			mapData.hdOcc.emplace(jComp->GetGuid(), rawPtr);

			drawList[index]->hdOccCullingRequestor.emplace_back(std::move(unqPtr));
		}
		void JGraphicDrawList::PopDrawRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept
		{
			if (scene == nullptr || jComp == nullptr)
				return;

			int index = GetIndex(scene);
			if (index == -1)
				return;

			const J_COMPONENT_TYPE compType = jComp->GetComponentType();
			if (compType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
			{
				int reqIndex = GetSceneReqIndex(index, jComp);
				if (reqIndex == -1)
					return;

				mapData.scene.erase(drawList[index]->sceneRequestor[reqIndex]->jCamera->GetGuid());
				drawList[index]->sceneRequestor.erase(drawList[index]->sceneRequestor.begin() + reqIndex);
			}
			else if (compType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
			{
				int reqIndex = GetShadowReqIndex(index, jComp);
				if (reqIndex == -1)
					return;
				 
				mapData.shadow.erase(drawList[index]->shadowRequestor[reqIndex]->jLight->GetGuid());
				drawList[index]->shadowRequestor.erase(drawList[index]->shadowRequestor.begin() + reqIndex);
				drawList[index]->updateInfo->nextSceneUpdate = true; 
			}

			if (drawList[index]->sceneRequestor.size() == 0)
			{
				//const JUserPtr<JScene>& scene = drawList[index]->scene;
				//scene->DeActivate();
			}
		}
		void JGraphicDrawList::PopFrustumCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept
		{
			if (scene == nullptr || jComp == nullptr)
				return;

			int index = GetIndex(scene);
			if (index == -1)
				return;

			int reqIndex = GetFrustumReqIndex(index, jComp);
			if (reqIndex == -1)
				return;

			mapData.frustum.erase(drawList[index]->frustumCullingRequestor[reqIndex]->comp->GetGuid());
			drawList[index]->frustumCullingRequestor.erase(drawList[index]->frustumCullingRequestor.begin() + reqIndex);
		}
		void JGraphicDrawList::PopHzbOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept
		{
			if (scene == nullptr || jComp == nullptr)
				return;

			int index = GetIndex(scene);
			if (index == -1)
				return;

			int reqIndex = GetHzbOccReqIndex(index, jComp);
			if (reqIndex == -1)
				return;

			mapData.hzbOcc.erase(drawList[index]->hzbOccCullingRequestor[reqIndex]->comp->GetGuid());
			drawList[index]->hzbOccCullingRequestor.erase(drawList[index]->hzbOccCullingRequestor.begin() + reqIndex);
		}
		void JGraphicDrawList::PopHdOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept
		{
			if (scene == nullptr || jComp == nullptr)
				return;

			int index = GetIndex(scene);
			if (index == -1)
				return;

			int reqIndex = GetHdOccReqIndex(index, jComp);
			if (reqIndex == -1)
				return;

			mapData.hdOcc.erase(drawList[index]->hdOccCullingRequestor[reqIndex]->comp->GetGuid());
			drawList[index]->hdOccCullingRequestor.erase(drawList[index]->hdOccCullingRequestor.begin() + reqIndex);
		}
		bool JGraphicDrawList::HasRequestor(const JUserPtr<JScene>& scene)noexcept
		{
			if (scene == nullptr)
				return false;

			int index = GetIndex(scene);

			if (index == -1)
				return false;

			return drawList[index]->sceneRequestor.size() + drawList[index]->shadowRequestor.size() > 0;
		}
		uint JGraphicDrawList::GetListCount()noexcept
		{
			return (uint)drawList.size();
		}
		JGraphicDrawTarget* JGraphicDrawList::GetDrawScene(const uint index)noexcept
		{
			return drawList[index].get();
		}
		JSceneDrawRequestor* JGraphicDrawList::GetSceneDrawRequestor(const size_t guid)noexcept
		{
			return mapData.scene.find(guid)->second;
		}
		/*
			std::vector<JGraphicDrawTarget*> JGraphicDrawList::GetAllDrawTarget()noexcept
		{

		}
		std::vector<JGraphicDrawTarget*> JGraphicDrawList::GetDrawableTarget()noexcept
		{

		}
		*/
	}
}