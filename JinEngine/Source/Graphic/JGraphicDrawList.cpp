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


#include"JGraphicDrawList.h" 
#include"GraphicResource/JGraphicResourceInfo.h"
#include"../Object/Resource/Scene/JScene.h" 
#include"../Object/Resource/Scene/JScenePrivate.h" 
#include"../Object/Component/JComponent.h" 
#include"../Object/Component/Camera/JCamera.h" 
#include"../Object/Component/Light/JLight.h" 
#include"../Object/GameObject/JGameObject.h"
#include"../Core/Utility/JCommonUtility.h"  

namespace JinEngine
{
	namespace Graphic
	{
		JDrawRequestor::JDrawRequestor(JGraphicDrawTarget* ownerTarget, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency)
			:ownerTarget(ownerTarget), updateFrequency(updateFrequency)
		{}
		JGraphicDrawTarget* JDrawRequestor::GetOwnerTarget()const noexcept
		{
			return ownerTarget;
		} 

		JSceneDrawRequestor::JSceneDrawRequestor(JGraphicDrawTarget* ownerTarget, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency, const JUserPtr<JCamera>& jCamera)
			:JDrawRequestor(ownerTarget, updateFrequency), jCamera(jCamera)
		{}
		JSceneDrawRequestor::~JSceneDrawRequestor()
		{}
		size_t JSceneDrawRequestor::GetReferenceComponentGuid()const noexcept
		{
			return jCamera->GetGuid();
		}

		JShadowMapDrawRequestor::JShadowMapDrawRequestor(JGraphicDrawTarget* ownerTarget, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency, const JUserPtr<JLight>& jLight)
			: JDrawRequestor(ownerTarget, updateFrequency), jLight(jLight)
		{}
		JShadowMapDrawRequestor::~JShadowMapDrawRequestor()
		{}
		size_t JShadowMapDrawRequestor::GetReferenceComponentGuid()const noexcept
		{
			return jLight->GetGuid();
		}

		JSceneFrustumCullingRequestor::JSceneFrustumCullingRequestor(JGraphicDrawTarget* ownerTarget, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency, const JUserPtr<JComponent>& comp)
			:JDrawRequestor(ownerTarget, updateFrequency), comp(comp)
		{}
		JSceneFrustumCullingRequestor::~JSceneFrustumCullingRequestor()
		{}
		size_t JSceneFrustumCullingRequestor::GetReferenceComponentGuid()const noexcept
		{
			return comp->GetGuid();
		}

		JSceneHzbOccCullingRequestor::JSceneHzbOccCullingRequestor(JGraphicDrawTarget* ownerTarget, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency, const JUserPtr<JComponent>& comp)
			:JDrawRequestor(ownerTarget, updateFrequency), comp(comp)
		{}
		JSceneHzbOccCullingRequestor::~JSceneHzbOccCullingRequestor()
		{}
		size_t JSceneHzbOccCullingRequestor::GetReferenceComponentGuid()const noexcept
		{
			return comp->GetGuid();
		}

		JSceneHdOccCullingRequestor::JSceneHdOccCullingRequestor(JGraphicDrawTarget* ownerTarget, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency, const JUserPtr<JComponent>& comp)
			:JDrawRequestor(ownerTarget, updateFrequency), comp(comp)
		{}
		JSceneHdOccCullingRequestor::~JSceneHdOccCullingRequestor()
		{}
		size_t JSceneHdOccCullingRequestor::GetReferenceComponentGuid()const noexcept
		{
			return comp->GetGuid();
		}

		namespace
		{
			using CompareDrawListPtr = bool(*)(JGraphicDrawTarget*, const size_t);
			using CompareRequestorPtr = bool(*)(JDrawRequestor*, const size_t); 

			static CompareDrawListPtr compareDrawListPtr = [](JGraphicDrawTarget* target, const size_t guid) {return target->scene->GetGuid() == guid; };
			static CompareRequestorPtr compareDrawRequestorPtr = [](JDrawRequestor* target, const size_t guid) {return target->GetReferenceComponentGuid() == guid; };
		}

		void JGraphicDrawTarget::UpdateInfo::BeginUpdate()
		{
			memset(updateCount, 0, sizeof(uint) * totalCompVariation);
			memset(hotUpdateCount, 0, sizeof(uint) * totalCompVariation);

			thisFrameObjCount = 0;
			sceneUpdated = false;
			shadowUpdated = false;
			hzbOccCullingUpdated = hdOccCullingUpdated = false;
			 
			hasObjRecopy = false;
		}
		void JGraphicDrawTarget::UpdateInfo::EndUpdate()
		{ 
			static constexpr UniqueIndex ritemIndex = ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_RENDERITEM>();
			static constexpr UniqueIndex aniIndex = ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_ANIMATOR>();
			static constexpr UniqueIndex dLitIndex = ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_LIGHT>(J_LIGHT_TYPE::DIRECTIONAL);
			static constexpr UniqueIndex pLitIndex = ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_LIGHT>(J_LIGHT_TYPE::POINT);
			static constexpr UniqueIndex sLitIndex = ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_LIGHT>(J_LIGHT_TYPE::SPOT);
			static constexpr UniqueIndex rLitIndex = ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_LIGHT>(J_LIGHT_TYPE::RECT);

			//cam이 자기자신의 update이외에 영향을 주는 객체들의 update count
			const uint sceneUpdateFactor = updateCount[ritemIndex] + 
				updateCount[aniIndex] +
				updateCount[dLitIndex] + 
				updateCount[pLitIndex] +
				updateCount[sLitIndex] +
				updateCount[rLitIndex];

			if (sceneUpdateFactor > 0 || nextSceneUpdate)
				sceneUpdated = true;
	 
			//const uint shadowUpdateCount = hotObjUpdateCount + hotAniUpdateCount + hotLitghtUpdateCount + shadowMapUpdateCount;
			//shadow map을 소유한 shadow requestor update이외에 영향을 주는 객체들의 update count
			const uint shadowUpdateFactor = updateCount[ritemIndex] + updateCount[aniIndex] + hasObjRecopy;

			if (shadowUpdateFactor > 0)
				shadowUpdated = true;
			 
			//const uint occUpdateHotFactor = hotObjUpdateCount + hotAniUpdateCount + hasObjRecopy;
			const uint occUpdateFactor = shadowUpdateFactor; 

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
				if (updateInfo->sceneUpdated || data->isUpdated || data->updateFrequency == J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY::ALWAYS)
					data->canDrawThisFrame = true; 
			}
			for (const auto& data : shadowRequestor)
			{
				if (updateInfo->shadowUpdated || data->isUpdated || data->updateFrequency == J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY::ALWAYS)
					data->canDrawThisFrame = true;
				if (data->passNextFrame)
					data->canDrawThisFrame = false;
			}
			for (const auto& data : frustumCullingRequestor)
			{
				if (updateInfo->sceneUpdated || data->isUpdated || data->updateFrequency == J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY::ALWAYS)
					data->canDrawThisFrame = true;
				if (data->keepCanDrawTrigger)
					data->canDrawThisFrame = true;	
			}
			for (const auto& data : hzbOccCullingRequestor)
			{
				if (updateInfo->hzbOccCullingUpdated || data->isUpdated || data->updateFrequency == J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY::ALWAYS)
					data->canDrawThisFrame = true;
				if (data->keepCanDrawTrigger)
					data->canDrawThisFrame = true;
			}
			for (const auto& data : hdOccCullingRequestor)
			{
				if (updateInfo->hdOccCullingUpdated || data->isUpdated || data->updateFrequency == J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY::ALWAYS)
					data->canDrawThisFrame = true;
				if (data->keepCanDrawTrigger)
					data->canDrawThisFrame = true;
			}
		}

		class DrawListPrivateData
		{
		public:
			std::vector<std::unique_ptr<JGraphicDrawTarget>> drawList;
		public:
			std::vector<JSceneDrawRequestor*> sceneDrawRequestorCacheVec;
			std::vector<JShadowMapDrawRequestor*> shadowMapDrawRequestorCacheVec;
			std::vector<JSceneFrustumCullingRequestor*> frustumCullingRequestorCacheVec;
			std::vector<JSceneHzbOccCullingRequestor*> hzbOccCullingRequestorCacheVec;
			std::vector<JSceneHdOccCullingRequestor*> hdOccCullingRequestorCacheVec;
		public:
			bool AddDrawList(const JUserPtr<JScene>& scene)noexcept
			{
				if (scene == nullptr)
					return false;

				if (HasDrawList(scene))
					return false;

				drawList.push_back(std::make_unique<JGraphicDrawTarget>(scene));
				return true;
			}
			bool PopDrawList(const JUserPtr<JScene>& scene)noexcept
			{
				if (scene == nullptr)
					return false;

				int index = GetIndex(scene);
				if (index == invalidIndex)
					return false;
				 
				drawList.erase(drawList.begin() + index);
				return true;
			} 
			bool AddDrawSceneRequest(const JUserPtr<JScene>& scene, const JUserPtr<JCamera>& jCamera, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency)noexcept
			{
				if (scene == nullptr || jCamera == nullptr)
					return false;

				int index = GetIndex(scene);
				if (index == invalidIndex)
					return false;

				int existIndex = GetReqIndex(drawList[index]->sceneRequestor, jCamera);
				if (existIndex != invalidIndex)
					return false;

				auto unqPtr = std::make_unique<JSceneDrawRequestor>(drawList[index].get(), updateFrequency, jCamera);
				auto rawPtr = unqPtr.get();
				rawPtr->isUpdated = true; 

				drawList[index]->sceneRequestor.emplace_back(std::move(unqPtr));
				sceneDrawRequestorCacheVec.push_back(rawPtr);
				return true;
			}
			bool AddDrawShadowRequest(const JUserPtr<JScene>& scene, const JUserPtr<JLight>& jLight)noexcept
			{
				if (scene == nullptr || jLight == nullptr)
					return false;

				int index = GetIndex(scene);
				if (index == invalidIndex)
					return false;

				int existIndex = GetReqIndex(drawList[index]->shadowRequestor, jLight);
				if (existIndex != invalidIndex)
					return false;

				auto unqPtr = std::make_unique<JShadowMapDrawRequestor>(drawList[index].get(), J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY::UPDATED, jLight);
				auto rawPtr = unqPtr.get();
				rawPtr->isUpdated = true; 

				drawList[index]->shadowRequestor.emplace_back(std::move(unqPtr));
				shadowMapDrawRequestorCacheVec.push_back(rawPtr); 
				return true;
			}
			bool AddFrustumCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency)noexcept
			{
				if (scene == nullptr || jComp == nullptr)
					return false;

				const J_COMPONENT_TYPE compType = jComp->GetComponentType();
				int index = GetIndex(scene);
				if (index == invalidIndex)
					return false;

				int existIndex = GetReqIndex(drawList[index]->frustumCullingRequestor, jComp);
				if (existIndex != invalidIndex)
					return false;

				auto unqPtr = std::make_unique<JSceneFrustumCullingRequestor>(drawList[index].get(), updateFrequency, jComp);
				auto rawPtr = unqPtr.get();
				rawPtr->isUpdated = true; 

				drawList[index]->frustumCullingRequestor.emplace_back(std::move(unqPtr));
				frustumCullingRequestorCacheVec.push_back(rawPtr);
				return true;
			}
			bool AddHzbOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency)noexcept
			{
				if (scene == nullptr || jComp == nullptr)
					return false;

				const J_COMPONENT_TYPE compType = jComp->GetComponentType();
				int index = GetIndex(scene);
				if (index == invalidIndex)
					return false;

				int existIndex = GetReqIndex(drawList[index]->hzbOccCullingRequestor, jComp);
				if (existIndex != invalidIndex)
					return false;

				auto unqPtr = std::make_unique<JSceneHzbOccCullingRequestor>(drawList[index].get(), updateFrequency, jComp);
				auto rawPtr = unqPtr.get();
				rawPtr->isUpdated = true; 

				drawList[index]->hzbOccCullingRequestor.emplace_back(std::move(unqPtr));
				hzbOccCullingRequestorCacheVec.push_back(rawPtr);
				return true;
			}
			bool AddHdOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency)noexcept
			{
				if (scene == nullptr || jComp == nullptr)
					return false;

				const J_COMPONENT_TYPE compType = jComp->GetComponentType();
				int index = GetIndex(scene);
				if (index == invalidIndex)
					return false;

				int existIndex = GetReqIndex(drawList[index]->hdOccCullingRequestor, jComp);
				if (existIndex != invalidIndex)
					return false;

				auto unqPtr = std::make_unique<JSceneHdOccCullingRequestor>(drawList[index].get(), updateFrequency, jComp);
				auto rawPtr = unqPtr.get();
				rawPtr->isUpdated = true; 

				drawList[index]->hdOccCullingRequestor.emplace_back(std::move(unqPtr));
				hdOccCullingRequestorCacheVec.push_back(rawPtr);
				return true;
			}
			bool PopDrawSceneRequest(const JUserPtr<JScene>& scene, const JUserPtr<JCamera>& jCamera)noexcept
			{
				if (scene == nullptr || jCamera == nullptr)
					return false;

				int index = GetIndex(scene);
				if (index == invalidIndex)
					return false;

				int existIndex = GetReqIndex(drawList[index]->sceneRequestor, jCamera);
				if (existIndex == invalidIndex)
					return false;

				int cacheIndex = GetReqIndex(sceneDrawRequestorCacheVec, jCamera);
				sceneDrawRequestorCacheVec.erase(sceneDrawRequestorCacheVec.begin() + cacheIndex);

				drawList[index]->sceneRequestor.erase(drawList[index]->sceneRequestor.begin() + existIndex);
				return true;
				//const JUserPtr<JScene>& scene = drawList[index]->scene;
				//scene->DeActivate();
			}
			bool PopDrawShadowRequest(const JUserPtr<JScene>& scene, const JUserPtr<JLight>& jLight)noexcept
			{
				if (scene == nullptr || jLight == nullptr)
					return false;

				int index = GetIndex(scene);
				if (index == invalidIndex)
					return false;

				int existIndex = GetReqIndex(drawList[index]->shadowRequestor, jLight);
				if (existIndex == invalidIndex)
					return false;
				   
				int cacheIndex = GetReqIndex(shadowMapDrawRequestorCacheVec, jLight);
				shadowMapDrawRequestorCacheVec.erase(shadowMapDrawRequestorCacheVec.begin() + cacheIndex);

				drawList[index]->shadowRequestor.erase(drawList[index]->shadowRequestor.begin() + existIndex);
				drawList[index]->updateInfo->nextSceneUpdate = true;
				return true;
			}
			bool PopFrustumCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept
			{
				if (scene == nullptr || jComp == nullptr)
					return false;

				int index = GetIndex(scene);
				if (index == invalidIndex)
					return false;

				int existIndex = GetReqIndex(drawList[index]->frustumCullingRequestor, jComp);
				if (existIndex == invalidIndex)
					return false;
				 
				int cacheIndex = GetReqIndex(frustumCullingRequestorCacheVec, jComp);
				frustumCullingRequestorCacheVec.erase(frustumCullingRequestorCacheVec.begin() + cacheIndex);

				drawList[index]->frustumCullingRequestor.erase(drawList[index]->frustumCullingRequestor.begin() + existIndex);
				return true;
			}
			bool PopHzbOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept
			{
				if (scene == nullptr || jComp == nullptr)
					return false;

				int index = GetIndex(scene);
				if (index == invalidIndex)
					return false;

				int existIndex = GetReqIndex(drawList[index]->hzbOccCullingRequestor, jComp);
				if (existIndex == invalidIndex)
					return false;
				
				int cacheIndex = GetReqIndex(hzbOccCullingRequestorCacheVec, jComp);
				hzbOccCullingRequestorCacheVec.erase(hzbOccCullingRequestorCacheVec.begin() + cacheIndex);

				drawList[index]->hzbOccCullingRequestor.erase(drawList[index]->hzbOccCullingRequestor.begin() + existIndex);
				return true;
			}
			bool PopHdOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept
			{
				if (scene == nullptr || jComp == nullptr)
					return false;

				int index = GetIndex(scene);
				if (index == invalidIndex)
					return false;

				int existIndex = GetReqIndex(drawList[index]->hdOccCullingRequestor, jComp);
				if (existIndex == invalidIndex)
					return false;
				 
				int cacheIndex = GetReqIndex(hdOccCullingRequestorCacheVec, jComp);
				hdOccCullingRequestorCacheVec.erase(hdOccCullingRequestorCacheVec.begin() + cacheIndex);

				drawList[index]->hdOccCullingRequestor.erase(drawList[index]->hdOccCullingRequestor.begin() + existIndex);
				return true;
			}
		public:
			uint GetListCount()const noexcept
			{
				return (uint)drawList.size();
			}
			JGraphicDrawTarget* GetDrawScene(const uint index)const noexcept
			{
				return drawList[index].get();
			} 
			int GetIndex(const JUserPtr<JScene>& scene)const noexcept
			{
				return JCUtil::GetIndex(drawList, compareDrawListPtr, scene->GetGuid());
			}
			template<typename T>
			int GetReqIndex(std::vector<std::unique_ptr<T>>& requestor, const JUserPtr<JComponent>& targetComp)const noexcept
			{
				return  JCUtil::GetIndex(requestor, compareDrawRequestorPtr, targetComp->GetGuid());
			}
			template<typename T>
			int GetReqIndex(std::vector<T*>& requestor, const JUserPtr<JComponent>& targetComp)const noexcept
			{
				return JCUtil::GetIndex(requestor, compareDrawRequestorPtr, targetComp->GetGuid());
			}
		public:
			bool HasDrawList(const JUserPtr<JScene>& scene)const noexcept
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
			bool HasRequestor(const JUserPtr<JScene>& scene)const noexcept
			{
				if (scene == nullptr)
					return false;

				int index = GetIndex(scene);
				if (index == invalidIndex)
					return false;

				return drawList[index]->sceneRequestor.size() + drawList[index]->shadowRequestor.size() > 0;
			} 
		private:
			void RemoveSceneCache(const size_t guid)
			{

			}
		};
		using DrawListDataHolder = Core::JSingletonHolder<DrawListPrivateData>;
		static DrawListPrivateData& GetPrivate()
		{
			return DrawListDataHolder::Instance();
		}

		bool JGraphicDrawList::AddDrawList(const JUserPtr<JScene>& scene)noexcept
		{
			return GetPrivate().AddDrawList(scene);
		}
		bool JGraphicDrawList::PopDrawList(const JUserPtr<JScene>& scene)noexcept
		{
			return GetPrivate().PopDrawList(scene);
		} 
		bool JGraphicDrawList::AddDrawSceneRequest(const JUserPtr<JScene>& scene, const JUserPtr<JCamera>& jCamera, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency)noexcept
		{
			return GetPrivate().AddDrawSceneRequest(scene, jCamera, updateFrequency);
		}
		bool JGraphicDrawList::AddDrawShadowRequest(const JUserPtr<JScene>& scene, const JUserPtr<JLight>& jLight)noexcept
		{
			return GetPrivate().AddDrawShadowRequest(scene, jLight);
		}
		bool JGraphicDrawList::AddFrustumCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency)noexcept
		{
			return GetPrivate().AddFrustumCullingRequest(scene, jComp, updateFrequency);
		}
		bool JGraphicDrawList::AddHzbOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency)noexcept
		{
			return GetPrivate().AddHzbOccCullingRequest(scene, jComp, updateFrequency);
		}
		bool JGraphicDrawList::AddHdOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency)noexcept
		{
			return GetPrivate().AddHdOccCullingRequest(scene, jComp, updateFrequency);
		}
		bool JGraphicDrawList::PopDrawSceneRequest(const JUserPtr<JScene>& scene, const JUserPtr<JCamera>& jCamera)noexcept
		{
			return GetPrivate().PopDrawSceneRequest(scene, jCamera);
		}
		bool JGraphicDrawList::PopDrawShadowRequest(const JUserPtr<JScene>& scene, const JUserPtr<JLight>& jLight)noexcept
		{
			return	GetPrivate().PopDrawShadowRequest(scene, jLight);
		} 
		bool JGraphicDrawList::PopFrustumCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept
		{
			return GetPrivate().PopFrustumCullingRequest(scene, jComp);
		}
		bool JGraphicDrawList::PopHzbOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept
		{
			return GetPrivate().PopHzbOccCullingRequest(scene, jComp);
		}
		bool JGraphicDrawList::PopHdOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept
		{
			return GetPrivate().PopHdOccCullingRequest(scene, jComp);
		}
		uint JGraphicDrawList::GetListCount()noexcept
		{
			return GetPrivate().GetListCount();
		}
		JGraphicDrawTarget* JGraphicDrawList::GetDrawScene(const uint index)noexcept
		{
			return GetPrivate().GetDrawScene(index);
		} 
		std::vector<JSceneDrawRequestor*>& JGraphicDrawList::GetRegisteredSceneDrawRequestor()noexcept
		{
			return GetPrivate().sceneDrawRequestorCacheVec;
		}
		std::vector<JShadowMapDrawRequestor*>& JGraphicDrawList::GetRegisteredShadowMapDrawRequestor()noexcept
		{
			return GetPrivate().shadowMapDrawRequestorCacheVec;
		}
		std::vector<JSceneFrustumCullingRequestor*>& JGraphicDrawList::GetRegisteredFrustumCullingRequestor()noexcept
		{
			return GetPrivate().frustumCullingRequestorCacheVec;
		}
		std::vector<JSceneHzbOccCullingRequestor*>& JGraphicDrawList::GetRegisteredHzbOccCullingRequestor()noexcept
		{
			return GetPrivate().hzbOccCullingRequestorCacheVec;
		}
		std::vector<JSceneHdOccCullingRequestor*>& JGraphicDrawList::GetRegisteredHdOccCullingRequestor()noexcept
		{
			return GetPrivate().hdOccCullingRequestorCacheVec;
		}
		bool JGraphicDrawList::HasDrawList(const JUserPtr<JScene>& scene)noexcept
		{
			return GetPrivate().HasDrawList(scene);
		}
		bool JGraphicDrawList::HasRequestor(const JUserPtr<JScene>& scene)noexcept
		{
			return GetPrivate().HasRequestor(scene);
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