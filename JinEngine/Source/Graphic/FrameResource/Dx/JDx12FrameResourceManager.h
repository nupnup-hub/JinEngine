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
#include"JDx12FrameResource.h"
#include"JDx12FrameUpdateInfo.h"
#include"../JFrameResourceManager.h"
#include"../../../Core/Threading/JThreadInfo.h" 

namespace JinEngine
{ 
	class JTexture;
	namespace Graphic
	{
		class JDx12FrameResourceManager final : public JFrameResourceManager
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JDx12FrameResourceManager)
		private:
			using WorkerF = Core::JMFunctorType<JDx12FrameResourceManager, void, uint>;
		public:
			struct CacheData
			{
			public:
				//used by pass
				//initialize first update constants buffer after initialize graphic class 
				//always exist until enigne end
				JUserPtr<JTexture> missing;
				JUserPtr<JTexture> bluseNoise;
				JUserPtr<JTexture> ltcAmp;
				JUserPtr<JTexture> ltcMat;
			public:
				void Initialize();
				void Clear();
			public:
				void Update();
			};
		public: 
			static constexpr uint maxNumOfUpdateThread = 8;
		private: 
			using InfoVec = std::vector<JOwnerPtr<JDx12FrameUpdateInfo>>;
			using AreaInfoVec = std::vector<std::unique_ptr<JFrameUpdateAreaInfo>>;
		public:
			InfoVec updateInfoVec[(uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT];
			AreaInfoVec areaInfoVec[(uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT];
		private:
			JDx12FrameResource resource[Constants::gNumFrameResources];
			int currResourceIndex = 0;  
		private:
			std::unique_ptr<WorkerF::Functor> workerFunctor;
			Core::JThreadUserHandle threadHandle[maxNumOfUpdateThread];
			JFrameUpdateDataSet cacheSet[maxNumOfUpdateThread];
		private:
			CacheData cacheData; 
		private:
			bool hasRequestThreadSync = false;
		public:
			~JDx12FrameResourceManager();
		public: 
			void Initialize(JGraphicDevice* device) final;
			void Clear() final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			JFrameResource* GetCurrentFrameResource() noexcept final;
			JDx12FrameResource* GetCurrentDxFrameResource() noexcept;
			JFrameResource* GetFrameResource(const uint index) noexcept final;
			uint GetCurrentFrameIndex() const noexcept final;
			uint GetNextFrameIndex()const noexcept  final;
			uint GetTotalRegistedCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept final;
			uint GetTotalFrameCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept final;
			uint GetAreaRegistedCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const size_t areaGuid)const noexcept final;
			//count 0 ~ areaStart
			uint GetAreaRegistedOffset(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const size_t areaGuid)const noexcept final;
			uint GetFrameResourceCapacity(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept final;
		private:
			const JFrameUpdateAreaInfo* GetAreaInfo(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const size_t areaGuid)const noexcept;
			int GetAreaVecIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const size_t areaGuid)const noexcept;
			int GetAreaStIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const size_t areaGuid)const noexcept; 
			int GetArrayIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type, JFrameUpdateInfo* ptr)const noexcept;
		public:
			void SetNextFrameResource();
		public:
			//areaGuid는 scene별 혹은 다른 object별 frameData를 구분해 정렬시키기 위해 사용된다
			JUserPtr<JFrameUpdateInfo> Register(const JFrameUploadDataCreationDesc& desc) final;
			bool DeRegister(JFrameUpdateInfo* info) final;
		private:
			//area count == 0 일시
			JUserPtr<JFrameUpdateInfo> PushBack(const JFrameUploadDataCreationDesc& desc);
			JUserPtr<JFrameUpdateInfo> Insert(const JFrameUploadDataCreationDesc& desc, const int areaIndex);
			bool Pop(JFrameUpdateInfo* info);
		private:
			JOwnerPtr<JDx12FrameUpdateInfo> CreateInfo(const JFrameUploadDataCreationDesc& desc, JFrameUpdateAreaInfo* areaInfo);
		private:
			void ReBuild(JGraphicDevice* device, const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint newCount)final;
		public:
			void BeginUpdate()final;
			void Update(JFrameUpdateDataSet& set)final;
			void EndUpdate()final; 
		private: 
			void WorkerThread(uint threadIndex);
			void WaitAllThreadTaskDone(); 
		private:
			void BuildResource(JGraphicDevice* device);
			void ClearResource();
		public:
			static void RegisterTypeData();
		};
	}
}
