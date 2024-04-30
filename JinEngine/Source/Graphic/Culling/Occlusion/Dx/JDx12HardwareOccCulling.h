#pragma once
#include"../JHardwareOccCulling.h"
#include"../../../Buffer/Dx/JDx12GraphicBuffer.h" 

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12HardwareOccCulling final : public JHardwareOccCulling
		{
		private:
			struct UpdateData
			{ 
			private:
				//static constexpr float deadlineFrame = 128.0f; 
				//static constexpr float minResolveRate = 1.0f / deadlineFrame;
				static constexpr uint allUpdateCount = 32;
				static constexpr float minResolveRate = 1.0f / (float)(Constants::gNumFrameResources * 3.0f);
				static constexpr float maxResolveRate = 1.0f / (float)(Constants::gNumFrameResources);
			public:
				size_t bufferCapacity = 0;		//cashed data when CalculateRate()
				size_t offset = 0;
				size_t count = 0;
				uint updateCycle = 0;
				float resolveRate = 0.0f;
			public: 
				bool isUpdateEnd = false; 
			public:
				UpdateData(const size_t capacity, const size_t gpuMemoryBusWidth);
			public:
				bool CanPassThisFrame(const uint frameResourceIndex)const noexcept;
			public:
				//split two exponent (1, 1/2, 1/4.... 1/128)
				void CalculateRate(const size_t capacity);
				//split gpuMemoryBusWidth(g) (g, 1/2g, 1/3g, 1/4g.... 1/128.0f)
				void CalculateRate(const size_t capacity, const size_t gpuMemoryBusWidth);
			private:
				uint CalculateCount()const noexcept;
				void CalcuateUpdateCycle() noexcept;
			public:
				//Update countOffset and out(offset, count)
				void Update();
				void Reset();
			};
		private:
			std::vector<UpdateData> updateData;
			size_t gpuMemoryBusWidth = 0; 
		public:
			~JDx12HardwareOccCulling();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			bool HasPreprocessing()const noexcept final;
			bool HasPostprocessing()const noexcept final;
		private:
			bool HasDependency(const JGraphicInfo::TYPE type)const noexcept final;
			bool HasDependency(const JGraphicOption::TYPE type)const noexcept final;
		private:
			void NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)final;
			void NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)final;
		private:
			void NotifyBuildNewHdOccBuffer(JGraphicDevice* device, const size_t initCapacity, const JUserPtr<JCullingInfo>& cullingInfo) final;
			void NotifyReBuildHdOccBuffer(JGraphicDevice* device, const size_t capacity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo) final;
			void NotifyDestroyHdOccBuffer(JCullingInfo* cullingInfo) final;
		public:
			void BeginDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)final;
			void EndDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)final;
		public:
			void DrawOcclusionDepthMap(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper) final;
			void DrawOcclusionDepthMapMultiThread(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper) final;
		public:
			void ExtractHDOcclusionCullingData(const JGraphicHdOccExtractSet* extractSet, const JDrawHelper& helper) final;
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info);
		private:
			void ClearResource();
		};
	}
} 