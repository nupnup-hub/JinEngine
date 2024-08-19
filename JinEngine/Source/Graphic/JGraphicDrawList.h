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


#pragma once
#include"JGraphicDrawListType.h"
#include"../Core/JCoreEssential.h" 
#include"../Object/GraphicRule/JGraphicModuleType.h"
#include"../Object/JObjectTypeStatistics.h"
#include<deque>  

struct ID3D12Device;
namespace JinEngine
{
	class JComponent;
	class JScene; 
	class JSceneManager;
	class JLight;
	class JCamera;  

	namespace Graphic
	{
		class JGraphicResourceInfo;  
		class JGraphicDrawList;
		class JGraphicSingleResourceInterface;
		class JGraphicMultoResourceInterface;
		class JGraphic;
		class JFrameUpdateUserAccess;

		class JGraphicDrawTarget;
		class JDrawRequestor
		{
		private:
			JGraphicDrawTarget* ownerTarget = nullptr;
		public:
			const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency;
		public:
			JDrawRequestor(JGraphicDrawTarget* ownerTarget, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency);
		public:
			virtual size_t GetReferenceComponentGuid()const noexcept = 0;
			JGraphicDrawTarget* GetOwnerTarget()const noexcept; 
		};
		class JSceneDrawRequestor : public JDrawRequestor
		{
		public:
			JUserPtr<JCamera> jCamera;
		public:
			bool isUpdated = false;
			bool canDrawThisFrame = false;
		public:
			JSceneDrawRequestor(JGraphicDrawTarget* ownerTarget, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency, const JUserPtr<JCamera>& jCamera);
			~JSceneDrawRequestor();
		public:
			size_t GetReferenceComponentGuid()const noexcept final;
		};
		class JShadowMapDrawRequestor : public JDrawRequestor
		{
		public:
			JUserPtr<JLight> jLight; 
		public:
			bool isUpdated = false;
			bool canDrawThisFrame = false; 
			bool passNextFrame = false;
		public:
			JShadowMapDrawRequestor(JGraphicDrawTarget* ownerTarget, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency, const JUserPtr<JLight>& jLight);
			~JShadowMapDrawRequestor();
		public:
			size_t GetReferenceComponentGuid()const noexcept final;
		};
		class JSceneFrustumCullingRequestor : public JDrawRequestor
		{
		public:
			JUserPtr<JComponent> comp; 
		public:
			bool isUpdated = false;
			bool canDrawThisFrame = false;
			bool keepCanDrawTrigger = false; 
		public:
			JSceneFrustumCullingRequestor(JGraphicDrawTarget* ownerTarget, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency, const JUserPtr<JComponent>& comp);
			~JSceneFrustumCullingRequestor();
		public:
			size_t GetReferenceComponentGuid()const noexcept final;
		};
		class JSceneHzbOccCullingRequestor : public JDrawRequestor
		{
		public:
			JUserPtr<JComponent> comp; 
		public:
			bool isUpdated = false;
			bool canDrawThisFrame = false;
			bool keepCanDrawTrigger = false; 
		public:
			JSceneHzbOccCullingRequestor(JGraphicDrawTarget* ownerTarget, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency, const JUserPtr<JComponent>& comp);
			~JSceneHzbOccCullingRequestor();
		public:
			size_t GetReferenceComponentGuid()const noexcept final;
		};
		class JSceneHdOccCullingRequestor : public JDrawRequestor
		{
		public:
			JUserPtr<JComponent> comp; 
		public:
			bool isUpdated = false;  
			bool canDrawThisFrame = false;
			bool keepCanDrawTrigger = false; 
		public:
			JSceneHdOccCullingRequestor(JGraphicDrawTarget* ownerTarget, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency, const JUserPtr<JComponent>& comp);
			~JSceneHdOccCullingRequestor();
		public:
			size_t GetReferenceComponentGuid()const noexcept final;
		};
		 
		class JGraphicDrawTarget
		{
		public:
			struct UpdateInfo
			{
			private: 
				friend class JGraphicDrawTarget; 
			public:
				uint updateCount[totalCompVariation];
				uint hotUpdateCount[totalCompVariation];
				uint moveCount[totalCompVariation];
				uint thisFrameCount[totalCompVariation];
				uint lastFrameCount[totalCompVariation];
			public:
				bool sceneUpdated = false;
				bool shadowUpdated = false;  
				bool hzbOccCullingUpdated = false;
				bool hdOccCullingUpdated = false;
			public:
				bool nextSceneUpdate = false;
			private:
				void BeginUpdate();
				void EndUpdate();
			}; 
		public:
			JUserPtr<JScene> scene;
			std::unique_ptr<UpdateInfo> updateInfo;
			std::vector<std::unique_ptr<JShadowMapDrawRequestor>> shadowRequestor;
			std::vector<std::unique_ptr<JSceneDrawRequestor>> sceneRequestor;
			std::vector<std::unique_ptr<JSceneFrustumCullingRequestor>> frustumCullingRequestor;
			std::vector<std::unique_ptr<JSceneHzbOccCullingRequestor>> hzbOccCullingRequestor;
			std::vector<std::unique_ptr<JSceneHdOccCullingRequestor>> hdOccCullingRequestor;
		public:
			JGraphicDrawTarget(const JUserPtr<JScene>& scene);
			~JGraphicDrawTarget();
		public:
			void BeginUpdate();
			void EndUpdate();
		};
		 
		class JGraphicDrawList 
		{ 
		private:
			friend class JGraphic;
			friend class JGraphicDrawListSceneInterface;
			friend class JGraphicDrawListCompInterface; 
		private:
			//drawList
			static bool AddDrawList(const JUserPtr<JScene>& scene)noexcept;
			static bool PopDrawList(const JUserPtr<JScene>& scene)noexcept;
		private: 
			//request
			static bool AddDrawSceneRequest(const JUserPtr<JScene>& scene, const JUserPtr<JCamera>& jCamera, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency)noexcept;
			static bool AddDrawShadowRequest(const JUserPtr<JScene>& scene, const JUserPtr<JLight>& jLight)noexcept;
			static bool AddFrustumCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency)noexcept;
			static bool AddHzbOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency)noexcept;
			static bool AddHdOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY updateFrequency)noexcept;
			static bool PopDrawSceneRequest(const JUserPtr<JScene>& scene, const JUserPtr<JCamera>& jCamera)noexcept;
			static bool PopDrawShadowRequest(const JUserPtr<JScene>& scene, const JUserPtr<JLight>& jLight)noexcept;
			static bool PopFrustumCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept;
			static bool PopHzbOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept;
			static bool PopHdOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept;
		private:
			static uint GetListCount()noexcept;
			static JGraphicDrawTarget* GetDrawScene(const uint index)noexcept;  
			static std::vector<JSceneDrawRequestor*>& GetRegisteredSceneDrawRequestor()noexcept;
			static std::vector<JShadowMapDrawRequestor*>& GetRegisteredShadowMapDrawRequestor()noexcept;
			static std::vector<JSceneFrustumCullingRequestor*>& GetRegisteredFrustumCullingRequestor()noexcept;
			static std::vector<JSceneHzbOccCullingRequestor*>& GetRegisteredHzbOccCullingRequestor()noexcept;
			static std::vector<JSceneHdOccCullingRequestor*>& GetRegisteredHdOccCullingRequestor()noexcept;
		private:
			static bool HasDrawList(const JUserPtr<JScene>& scene)noexcept;
			static bool HasRequestor(const JUserPtr<JScene>& scene)noexcept;
		};	 
	}
}