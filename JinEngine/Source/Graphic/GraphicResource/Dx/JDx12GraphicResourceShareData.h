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
#include"../JGraphicResourceShareData.h" 
#include"JDx12GraphicResourceConstants.h" 

namespace JinEngine
{
	namespace Graphic
	{
		/**
		* Task에서 요구되지만 Object간에 공유가능한 resource들을 관리한다.
		* 주로 중간계산에 사용되는 Buffer들이 이에 해당된다.
		*/
		/**
		* dx12 graphic subclass들은 task 별로 설계되었으며 주로 사용하는 resource type이 존재한다.
		* Resource type도 일부를 제외하면 task type에 종속적이므로 Resource 생성시 사용되는 task를 예측해서
		* 사용될 share data를 특정할수있다.
		* 위와 같은 가정은 task별 선택사항이며 subclass에서 event queue에 share data 생성에 대한 요청을 push할수도있다.
		*/
		class JGraphicResourceInfo;	 
		class JDx12GraphicResourceShareData : public JGraphicResourceShareData
		{
		public:  
			//size dependency
			class UserCounting
			{
			public:
				friend class JDx12GraphicResourceShareData;
			public:
				int userCount = 0;
			public:
				UserCounting();
			};
			//size dependency
			class SsaoData : public JShareDataHolderInterface, public UserCounting
			{
			public:
				JUserPtr<JGraphicResourceInfo> intermediate00;
				JUserPtr<JGraphicResourceInfo> intermediate01;
				JUserPtr<JGraphicResourceInfo> interleave;
				JUserPtr<JGraphicResourceInfo> depth;
				JUserPtr<JGraphicResourceInfo> depthInterleave;
			public:
				SsaoData(JGraphicDevice* device, JGraphicResourceManager* gM, const uint width, const uint height);
				~SsaoData();
			public:
				J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			public:
				bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept;
				static bool _IsSupported(const J_GRAPHIC_TASK_TYPE taskType)noexcept;
			public:
				void UpdateBegin() final;
				void UpdateEnd() final;
			};
			class ImageProcessingData : public JShareDataHolderInterface, public UserCounting
			{
			public:
				JUserPtr<JGraphicResourceInfo> intermediate00;
				JUserPtr<JGraphicResourceInfo> intermediate01; 
				JUserPtr<JGraphicResourceInfo> histogram; 
				JUserPtr<JGraphicResourceInfo> defaultExposure;
				JUserPtr<JGraphicResourceInfo> lumaUnorm; 
				JUserPtr<JGraphicResourceInfo> lumaLowResolutionUint;
			public:
				JUserPtr<JGraphicResourceInfo> bloom[Constants::bloomSampleCount][2];	//640x384 (1/3) ~  40x24(1/48) 
			public:
				JUserPtr<JGraphicResourceInfo> fxaaWorkCounter;
				JUserPtr<JGraphicResourceInfo> fxaaIndirectParameters;
				JUserPtr<JGraphicResourceInfo> fxaaWorkerQueue;
				JUserPtr<JGraphicResourceInfo> fxaaColorQueue;
			private:
				int lastUpdatedIndex = invalidIndex;
			public:
				bool applyHdr = false;
			public:
				ImageProcessingData(JGraphicDevice* device, JGraphicResourceManager* gM, const uint width, const uint height);
				~ImageProcessingData();
			public:
				J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
				//ordered by lastUpdatedIndex
				JUserPtr<JGraphicResourceInfo> GetUpdateWaitIntermediate()const noexcept;
				JUserPtr<JGraphicResourceInfo> GetUpdatedIntermediate()const noexcept;
			public:
				void AddUpdatedIndexCount()noexcept;
			public:
				bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept;
				static bool _IsSupported(const J_GRAPHIC_TASK_TYPE taskType)noexcept;
			public:
				void UpdateBegin() final;
				void UpdateEnd() final;
			};
			class RestirTemporalAccumulationData : public JShareDataHolderInterface, public UserCounting
			{
			public: 
				JUserPtr<JGraphicResourceInfo> restirColorHistoryIntermediate00;
				JUserPtr<JGraphicResourceInfo> restirColorHistoryIntermediate01; 
				JUserPtr<JGraphicResourceInfo> restirDepthDerivative;
				JUserPtr<JGraphicResourceInfo> restirDenoiseMipmap[Constants::restirDenoiseMipmapCount];
			public:
				JUserPtr<JGraphicResourceInfo> viewZ;			//sample 연산중 중복되는 계산을 피하기 위해
				JUserPtr<JGraphicResourceInfo> preViewZ;		//sample 연산중 중복되는 계산을 피하기 위해
			public:
				RestirTemporalAccumulationData(JGraphicDevice* device, JGraphicResourceManager* gM, const uint width, const uint height);
				~RestirTemporalAccumulationData();
			public:
				J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			public:
				bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept;
				static bool _IsSupported(const J_GRAPHIC_TASK_TYPE taskType)noexcept;
			public:
				void UpdateBegin() final;
				void UpdateEnd() final;
			};
		private:
			std::unordered_map<size_t, std::unique_ptr<SsaoData>> ssaoDataMap;
			std::unordered_map<size_t, std::unique_ptr<ImageProcessingData>> imageProcessingDataMap;
			std::unordered_map<size_t, std::unique_ptr<RestirTemporalAccumulationData>> restirTemporalAccDataMap;
		public:
			~JDx12GraphicResourceShareData();
		public: 
			void Clear() final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			SsaoData* GetSsaoData(const uint width, const uint height);
			ImageProcessingData* GetImageProcessingData(const uint width, const uint height);
			RestirTemporalAccumulationData* GetRestirTemporalAccumulationData(const uint width, const uint height);
			JShareDataHolderInterface* GetResourceDependencyData(const J_GRAPHIC_TASK_TYPE taskType, JGraphicResourceInfo* info)final;
		public:
			void NotifyGraphicResourceCreation(JGraphicDevice* device, JGraphicResourceManager* gM, JGraphicResourceInfo* newInfo)final;
			void NotifyGraphicResourceDestruction(JGraphicDevice* device, JGraphicResourceManager* gM, JGraphicResourceInfo* info)final;
		private:
			void ClearResource();
		};

		using SsaoShareData = JDx12GraphicResourceShareData::SsaoData;
		using ImageProcessingShareData = JDx12GraphicResourceShareData::ImageProcessingData;
	}
}