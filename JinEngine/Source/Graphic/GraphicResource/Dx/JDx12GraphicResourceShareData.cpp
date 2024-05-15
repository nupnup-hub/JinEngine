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


#include"JDx12GraphicResourceShareData.h"
#include"JDx12GraphicResourceManager.h" 
#include"JDx12GraphicResourceInfo.h" 
#include"../../Image/Dx/JDx12ImageConstants.h"
#include"../../Device/JGraphicDevice.h"
#include"../../Utility/Dx/JDx12Utility.h" 

namespace JinEngine::Graphic
{
	namespace Private
	{
		template<typename T, typename = void>
		struct HasUserCount : std::false_type {};
		template<typename T>
		struct HasUserCount<T, std::void_t<decltype(&T::userCount)>> : std::true_type {};

		static bool HasShareData(const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			switch (rType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP: 
				return true;
			default:
				return false;
			}
		}
		static size_t CalRtDependencyMapKey(const uint width, const uint height)noexcept
		{
			return std::hash<size_t>{}(width + ((size_t)height << 4));
		}
		template<typename Key, typename Data>
		static Data* TryGetShareData(std::unordered_map<Key, std::unique_ptr<Data>>& map, const Key key)
		{
			auto data = map.find(key);
			return data != map.end() ? data->second.get() : nullptr; 
		}
		//std::unordered_map<size_t, RTSizeDependency>
		template<typename Key, typename Data>
		static void TryEraseMap(std::unordered_map<Key, std::unique_ptr<Data>>& map, const Key key)
		{
			auto data = map.find(key);
			if (data != map.end())
			{
				if constexpr (HasUserCount<Data>::value)
				{
					--data->second->userCount;
					if (data->second->userCount <= 0)
						map.erase(key);
				}
			}
		}
	}

	JDx12GraphicResourceShareData::UserCounting::UserCounting()
		:userCount(1)
	{ }

	JDx12GraphicResourceShareData::SsaoData::SsaoData(JGraphicDevice* device, JGraphicResourceManager* gM, const uint width, const uint height)
	{
		JGraphicResourceCreationDesc desc;
		desc.width = width;
		desc.height = height;

		intermediate00 = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERMEDIATE_MAP);
		intermediate01 = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERMEDIATE_MAP);
		depth = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_MAP);

		const uint sliceWidth = (width + Constants::ssaoSplitCount - 1) / Constants::ssaoSplitCount;
		const uint sliceHeight = (height + Constants::ssaoSplitCount - 1) / Constants::ssaoSplitCount;

		desc.width = sliceWidth;
		desc.height = sliceHeight;
		desc.arraySize = Constants::ssaoSliceCount;

		interleave = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERLEAVE_MAP);
		depthInterleave = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_INTERLEAVE_MAP);
	}
	JDx12GraphicResourceShareData::SsaoData::~SsaoData()
	{
		JGraphicResourceInfo::Destroy(intermediate00.Release());
		JGraphicResourceInfo::Destroy(intermediate01.Release());
		JGraphicResourceInfo::Destroy(interleave.Release());
		JGraphicResourceInfo::Destroy(depth.Release());
		JGraphicResourceInfo::Destroy(depthInterleave.Release());
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicResourceShareData::SsaoData::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	bool JDx12GraphicResourceShareData::SsaoData::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return _IsSupported(taskType);
	}
	bool JDx12GraphicResourceShareData::SsaoData::_IsSupported(const J_GRAPHIC_TASK_TYPE taskType)noexcept
	{
		return JCUtil::IsAnySame(taskType, J_GRAPHIC_TASK_TYPE::APPLY_SSAO);
	}
	void JDx12GraphicResourceShareData::SsaoData::UpdateBegin()
	{}
	void JDx12GraphicResourceShareData::SsaoData::UpdateEnd()
	{}

	JDx12GraphicResourceShareData::ImageProcessingData::ImageProcessingData(JGraphicDevice* device, JGraphicResourceManager* gM, const uint width, const uint height)
	{
		JGraphicResourceCreationDesc desc;
		desc.width = width;
		desc.height = height;

		intermediate00 = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::IMAGE_PROCESSING);
		intermediate01 = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::IMAGE_PROCESSING);

		uint histogramInitData[Constants::histogramBufferCount];
		memset(histogramInitData, 0, Constants::histogramBufferCount * sizeof(uint));
		 
		desc.width = Constants::histogramBufferCount;
		desc.height = 1;
		desc.uploadBufferDesc = std::make_unique<JUploadBufferCreationDesc>(histogramInitData, Constants::histogramBufferCount * sizeof(uint));
		desc.formatHint = std::make_unique<Graphic::JGraphicFormatHint>();
		desc.formatHint->elementSize = sizeof(uint);
		desc.formatHint->isUnsigned = true;
		histogram = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::BYTE_BUFFER_COMMON);

		desc.width = Constants::exposureBufferCount;
		desc.height = 1;
		desc.uploadBufferDesc = std::make_unique<JUploadBufferCreationDesc>();
		desc.uploadBufferDesc->useEngineDefine = true;
		desc.formatHint = nullptr;
		defaultExposure = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE);

		desc.width = width;
		desc.height = height;
		desc.uploadBufferDesc = nullptr;
		lumaUnorm = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_LUMA);

		desc.width = Graphic::Constants::CalBloomWidth(width);
		desc.height = Graphic::Constants::CalBloomHeight(height);
		desc.formatHint = std::make_unique<Graphic::JGraphicFormatHint>();
		desc.formatHint->isUnsigned = true;
		desc.formatHint->isNormalized = false;
		desc.uploadBufferDesc = nullptr;
		lumaLowResolutionUint = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_LUMA);

		desc.width = Graphic::Constants::CalBloomWidth(width);
		desc.height = Graphic::Constants::CalBloomHeight(height);
		desc.formatHint = nullptr;
		for (uint i = 0; i < Constants::bloomSampleCount; ++i)
		{
			bloom[i][0] = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::IMAGE_PROCESSING);
			bloom[i][1] = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::IMAGE_PROCESSING);

			desc.width /= 2;
			desc.height /= 2;
		}

		constexpr uint counterElementCount = 2;
		constexpr uint indirectElementCount = 2;
		constexpr uint indirectElementSize = sizeof(D3D12_DISPATCH_ARGUMENTS);
		constexpr uint indirectInitArrayCount = indirectElementSize * indirectElementCount / sizeof(uint);
		uint32 initCounter[counterElementCount]{ 0, 0 };
		__declspec(align(16)) const uint32 initIndirect[indirectInitArrayCount]{ 0, 1, 1, 0, 1, 1 };

		desc.width = counterElementCount;
		desc.height = 1;
		desc.formatHint = std::make_unique<JGraphicFormatHint>();
		desc.formatHint->elementSize = sizeof(uint);
		desc.uploadBufferDesc = std::make_unique<JUploadBufferCreationDesc>(initCounter, counterElementCount * sizeof(uint32));
		fxaaWorkCounter = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::BYTE_BUFFER_COMMON);

		desc.width = indirectElementCount;
		desc.height = 1;
		desc.formatHint->elementSize = sizeof(D3D12_DISPATCH_ARGUMENTS);
		desc.uploadBufferDesc = std::make_unique<JUploadBufferCreationDesc>(initIndirect, indirectElementCount * indirectElementSize);
		fxaaIndirectParameters = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::BYTE_BUFFER_COMMON);

		const uint fXAAWorkSize = width * height / 4 + 128;
		desc.width = fXAAWorkSize;
		desc.height = 1;
		desc.formatHint = nullptr;
		desc.uploadBufferDesc = nullptr;
		fxaaColorQueue = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_FXAA_COLOR_QUEUE);

		desc.formatHint = std::make_unique<Graphic::JGraphicFormatHint>();
		desc.formatHint->elementSize = sizeof(uint);
		desc.formatHint->isUnsigned = true;
		fxaaWorkerQueue = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::BYTE_BUFFER_COMMON);
	}
	JDx12GraphicResourceShareData::ImageProcessingData::~ImageProcessingData()
	{
		JGraphicResourceInfo::Destroy(intermediate00.Release());
		JGraphicResourceInfo::Destroy(intermediate01.Release());
		JGraphicResourceInfo::Destroy(histogram.Release());
		JGraphicResourceInfo::Destroy(defaultExposure.Release());
		JGraphicResourceInfo::Destroy(lumaUnorm.Release());
		JGraphicResourceInfo::Destroy(lumaLowResolutionUint.Release());
		for (uint i = 0; i < Constants::bloomSampleCount; ++i)
		{
			JGraphicResourceInfo::Destroy(bloom[i][0].Release());
			JGraphicResourceInfo::Destroy(bloom[i][1].Release());
		}
		JGraphicResourceInfo::Destroy(fxaaWorkCounter.Release());
		JGraphicResourceInfo::Destroy(fxaaIndirectParameters.Release());
		JGraphicResourceInfo::Destroy(fxaaWorkerQueue.Release());
		JGraphicResourceInfo::Destroy(fxaaColorQueue.Release());
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicResourceShareData::ImageProcessingData::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceShareData::ImageProcessingData::GetUpdateWaitIntermediate()const noexcept
	{
		return lastUpdatedIndex == 0 ? intermediate01 : intermediate00;
	}
	JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceShareData::ImageProcessingData::GetUpdatedIntermediate()const noexcept
	{
		return lastUpdatedIndex == invalidIndex ? nullptr : (lastUpdatedIndex == 0 ? intermediate00 : intermediate01);
	}
	void JDx12GraphicResourceShareData::ImageProcessingData::AddUpdatedIndexCount()noexcept
	{
		++lastUpdatedIndex;
		if (lastUpdatedIndex >= 2)
			lastUpdatedIndex = 0;
	}
	bool JDx12GraphicResourceShareData::ImageProcessingData::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return _IsSupported(taskType);
	}
	bool JDx12GraphicResourceShareData::ImageProcessingData::_IsSupported(const J_GRAPHIC_TASK_TYPE taskType)noexcept
	{
		return JCUtil::IsAnySame(taskType, J_GRAPHIC_TASK_TYPE::APPLY_BLOOM,
			J_GRAPHIC_TASK_TYPE::APPLY_TONE_MAPPING,
			J_GRAPHIC_TASK_TYPE::APPLY_POST_PROCESS_RESULT,
			J_GRAPHIC_TASK_TYPE::CONTROLL_POST_PROCESS_PIPELINE);
	}
	void JDx12GraphicResourceShareData::ImageProcessingData::UpdateBegin()
	{
		lastUpdatedIndex = invalidIndex;
		applyHdr = false;
	}
	void JDx12GraphicResourceShareData::ImageProcessingData::UpdateEnd()
	{

	}

	JDx12GraphicResourceShareData::RestirTemporalAccumulationData::RestirTemporalAccumulationData(JGraphicDevice* device, JGraphicResourceManager* gM, const uint width, const uint height)
	{ 
		JGraphicResourceCreationDesc desc;
		desc.width = width;
		desc.height = height;
		desc.bindDesc.requestAdditionalBind[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
		desc.bindDesc.useEngineDefinedBindType = false;
		desc.textureDesc = std::make_unique< JTextureCreationDesc>();
		desc.textureDesc->mipMapDesc.type = J_GRAPHIC_MIP_MAP_TYPE::NONE;

		desc.formatHint = std::make_unique<JGraphicFormatHint>();
		desc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R16G16B16A16_UNORM;
		restirColorHistoryIntermediate00 = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON);
		restirColorHistoryIntermediate01 = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON); 

		desc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R32_FLOAT;
		viewZ = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON);

		desc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R32_FLOAT;
		preViewZ = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON);

		desc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R16G16_UNORM;
		restirDepthDerivative = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON);

		desc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R16G16B16A16_UNORM;
		for (uint i = 0; i < SIZE_OF_ARRAY(restirDenoiseMipmap); ++i)
		{
			restirDenoiseMipmap[i] = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON);
			desc.width *= 0.5f;
			desc.height *= 0.5f;
		}
	}
	JDx12GraphicResourceShareData::RestirTemporalAccumulationData::~RestirTemporalAccumulationData()
	{
		JGraphicResourceInfo::Destroy(restirColorHistoryIntermediate00.Release());
		JGraphicResourceInfo::Destroy(restirColorHistoryIntermediate01.Release()); 
		JGraphicResourceInfo::Destroy(viewZ.Release());
		JGraphicResourceInfo::Destroy(preViewZ.Release());
		JGraphicResourceInfo::Destroy(restirDepthDerivative.Release()); 
		for (uint i = 0; i < SIZE_OF_ARRAY(restirDenoiseMipmap); ++i)
			JGraphicResourceInfo::Destroy(restirDenoiseMipmap[i].Release());
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicResourceShareData::RestirTemporalAccumulationData::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	bool JDx12GraphicResourceShareData::RestirTemporalAccumulationData::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return _IsSupported(taskType);
	}
	bool JDx12GraphicResourceShareData::RestirTemporalAccumulationData::_IsSupported(const J_GRAPHIC_TASK_TYPE taskType)noexcept
	{
		return JCUtil::IsAnySame(taskType, J_GRAPHIC_TASK_TYPE::RAYTRACING_AMBIENT_OCCLUSION,
			J_GRAPHIC_TASK_TYPE::RAYTRACING_GI,
			J_GRAPHIC_TASK_TYPE::RAYTRACING_DENOISE);
	}
	void JDx12GraphicResourceShareData::RestirTemporalAccumulationData::UpdateBegin()
	{

	}
	void JDx12GraphicResourceShareData::RestirTemporalAccumulationData::UpdateEnd()
	{

	}

	JDx12GraphicResourceShareData::~JDx12GraphicResourceShareData()
	{
		ClearResource();
	}
	void JDx12GraphicResourceShareData::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicResourceShareData::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	JDx12GraphicResourceShareData::SsaoData* JDx12GraphicResourceShareData::GetSsaoData(const uint width, const uint height)
	{
		return Private::TryGetShareData(ssaoDataMap, Private::CalRtDependencyMapKey(width, height)); 
	}
	JDx12GraphicResourceShareData::ImageProcessingData* JDx12GraphicResourceShareData::GetImageProcessingData(const uint width, const uint height)
	{
		return Private::TryGetShareData(imageProcessingDataMap, Private::CalRtDependencyMapKey(width, height)); 
	}
	JDx12GraphicResourceShareData::RestirTemporalAccumulationData* JDx12GraphicResourceShareData::GetRestirTemporalAccumulationData(const uint width, const uint height)
	{
		return Private::TryGetShareData(restirTemporalAccDataMap, Private::CalRtDependencyMapKey(width, height));
	}
	JShareDataHolderInterface* JDx12GraphicResourceShareData::GetResourceDependencyData(const J_GRAPHIC_TASK_TYPE taskType, JGraphicResourceInfo* info)
	{
		if (info == nullptr)
			return nullptr;

		if (Private::HasShareData(info->GetGraphicResourceType()))
		{
			if (SsaoData::_IsSupported(taskType))
				return GetSsaoData(info->GetWidth(), info->GetHeight());
			else if (ImageProcessingData::_IsSupported(taskType))
				return GetImageProcessingData(info->GetWidth(), info->GetHeight());
			else if (RestirTemporalAccumulationData::_IsSupported(taskType))
				return GetRestirTemporalAccumulationData(info->GetWidth(), info->GetHeight());
		}
		return nullptr;
	}
	void JDx12GraphicResourceShareData::NotifyGraphicResourceCreation(JGraphicDevice* device, JGraphicResourceManager* gM, JGraphicResourceInfo* newInfo)
	{
		if (!IsAllSameDevice(device, gM, newInfo))
			return;

		const J_GRAPHIC_RESOURCE_TYPE rType = newInfo->GetGraphicResourceType();
		if (Private::HasShareData(rType))
		{
			const uint widht = newInfo->GetWidth();
			const uint height = newInfo->GetHeight();
			const size_t mapKey = Private::CalRtDependencyMapKey(widht, height);
			if (rType == J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP)
			{
				auto exist = ssaoDataMap.find(mapKey);
				if (exist == ssaoDataMap.end())
					ssaoDataMap.emplace(mapKey, std::make_unique<SsaoData>(device, gM, widht, height));
				else
					++exist->second->userCount;
			}
			else if (rType == J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON && newInfo->HasView(J_GRAPHIC_BIND_TYPE::UAV))
			{
				auto imageExist = imageProcessingDataMap.find(mapKey);
				if (imageExist == imageProcessingDataMap.end())
					imageProcessingDataMap.emplace(mapKey, std::make_unique<ImageProcessingData>(device, gM, widht, height));
				else
					++imageExist->second->userCount;

				auto restirExist = restirTemporalAccDataMap.find(mapKey);
				if (restirExist == restirTemporalAccDataMap.end())
					restirTemporalAccDataMap.emplace(mapKey, std::make_unique<RestirTemporalAccumulationData>(device, gM, widht, height));
				else
					++restirExist->second->userCount;
			} 
		}
	}
	void JDx12GraphicResourceShareData::NotifyGraphicResourceDestruction(JGraphicDevice* device, JGraphicResourceManager* gM, JGraphicResourceInfo* info)
	{
		if (!IsAllSameDevice(device, gM, info))
			return;

		const J_GRAPHIC_RESOURCE_TYPE rType = info->GetGraphicResourceType();
		if (Private::HasShareData(info->GetGraphicResourceType()))
		{
			const size_t mapKey = Private::CalRtDependencyMapKey(info->GetWidth(), info->GetHeight());
			if (rType == J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP)
				Private::TryEraseMap(ssaoDataMap, mapKey);
			else if (rType == J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON && info->HasView(J_GRAPHIC_BIND_TYPE::UAV))
			{
				Private::TryEraseMap(imageProcessingDataMap, mapKey);
				Private::TryEraseMap(restirTemporalAccDataMap, mapKey);
			}
		}
	}
	void JDx12GraphicResourceShareData::ClearResource()
	{
		ssaoDataMap.clear();
		imageProcessingDataMap.clear();
		restirTemporalAccDataMap.clear();
	}
}