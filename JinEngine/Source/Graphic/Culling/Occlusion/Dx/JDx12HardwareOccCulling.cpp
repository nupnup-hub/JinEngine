#include"JDx12HardwareOccCulling.h"
#include"../../Dx/JDx12CullingManager.h" 
#include"../../Dx/JDx12CullingResourceHolder.h" 
#include"../../../DepthMap/Dx/JDx12DepthMapDebug.h"
#include"../../../DepthMap/Dx/JDx12DepthTest.h"
#include"../../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../../Device/Dx/JDx12GraphicDevice.h"
#include"../../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../../Utility/Dx/JD3DUtility.h"
#include"../../../JGraphicUpdateHelper.h"
#include"../../../../Core/Time/JStopWatch.h"
#include"../../../../Core/Platform/JHardwareInfo.h"
#include"../../../../Object/Component/Camera/JCamera.h"  
//#include"../../../../Develop/Debug/JDevelopDebug.h"

namespace JinEngine::Graphic
{
	namespace Private
	{
		static constexpr uint querySize = sizeof(JHdDx12CullingResultHolder::ResultType);
		static float CalRate(const float v, const float min, const float max)noexcept
		{
			return 1 - ((v - min) * (1 / (max - min)));
		}
		static float MinusPow(uint e)
		{
			float result = 1.0f;
			while (e > 0)
			{
				result /= 2.0f;
				--e;
			}
			return result;
		}
	}
	JDx12HardwareOccCulling::UpdateData::UpdateData(const size_t capacity, const size_t gpuMemoryBusWidth)
	{
		//CalculateRate(capacity);
		CalculateRate(capacity, gpuMemoryBusWidth);
	}
	void JDx12HardwareOccCulling::UpdateData::CalculateRate(const size_t capacity)
	{
		static constexpr uint appMaxResolveRateCapacity = 16;
		static constexpr uint appMinResolveRateCapacity = 1000;
		if (capacity < appMaxResolveRateCapacity)
			resolveRate = 1.0f;
		else if (capacity < appMinResolveRateCapacity)
		{
			static constexpr uint rateCount = 6;	//1/2, 1/4, 1/8, 1/16, 1/32, 1/64 
			static constexpr float rateWidth = 1.0f / rateCount;

			//minResolveRate ~1.0f
			float rawRate = Private::CalRate(capacity, appMaxResolveRateCapacity, appMinResolveRateCapacity);
			int minusTwoExponent = rateCount - (int)(rawRate / rateWidth);

			//(1, 1/2, 1/4, 1/8, 1/16.... 1 / 64)  
			resolveRate = Private::MinusPow(minusTwoExponent);
		}
		else
			resolveRate = minResolveRate;

		resolveRate = std::clamp(resolveRate, minResolveRate, 1.0f);
		countOffset = 0;
		bufferCapacity = capacity;
		updateAllObject = true;
		isUpdateEnd = true;
	}
	void JDx12HardwareOccCulling::UpdateData::CalculateRate(const size_t capacity, const size_t gpuMemoryBusWidth)
	{
		//2400
		const uint executeCount = capacity / gpuMemoryBusWidth;
		const float executeRate = 1.0f / executeCount;

		resolveRate = std::clamp(executeRate, minResolveRate, 1.0f);
		countOffset = 0;
		bufferCapacity = capacity;
		updateAllObject = true;
		isUpdateEnd = true;
	}
	void JDx12HardwareOccCulling::UpdateData::Update(_Out_ size_t& offset, _Out_ size_t& count)
	{
		if (isUpdateEnd)
			isUpdateEnd = false;

		if (updateAllObject)
		{
			offset = 0;
			count = bufferCapacity;
		}
		else
		{
			const uint delta = (uint)(bufferCapacity * resolveRate);
			offset = countOffset;
			//resolve rest object 
			if (delta + offset >= bufferCapacity)
			{
				count = bufferCapacity - offset;
				countOffset = 0;
				isUpdateEnd = true;
			}
			else
			{
				count = delta;
				countOffset += delta;
			}
		} 
	}
	void JDx12HardwareOccCulling::UpdateData::Reset()
	{
		updateAllObject = false;
		countOffset = 0;
	}

	void JDx12HardwareOccCulling::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)
	{
		auto gpuInfo = Core::JHardwareInfo::GetGpuInfo();
		gpuMemoryBusWidth = gpuInfo[0].memoryBusWidth;
	}
	void JDx12HardwareOccCulling::Clear()
	{
	}
	J_GRAPHIC_DEVICE_TYPE JDx12HardwareOccCulling::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	bool JDx12HardwareOccCulling::HasPreprocessing()const noexcept
	{
		return false;
	}
	bool JDx12HardwareOccCulling::HasPostprocessing()const noexcept
	{
		return false;
	}
	void JDx12HardwareOccCulling::NotifyBuildNewHdOccBuffer(JGraphicDevice* device, const size_t initCapacity, const JUserPtr<JCullingInfo>& cullingInfo)
	{
		if (cullingInfo == nullptr)
			return;

		updateData.push_back(UpdateData(initCapacity, gpuMemoryBusWidth));

		UpdateData& upData = updateData[updateData.size() - 1];
		cullingInfo->SetUpdateFrequency(1.0f);
		cullingInfo->SetUpdatePerObjectRate(upData.resolveRate);
		cullingInfo->SetUpdateEnd(true);
	}
	void JDx12HardwareOccCulling::NotifyReBuildHdOccBuffer(JGraphicDevice* device, const size_t capacity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo)
	{
		for (const auto& data : cullingInfo)
		{
			UpdateData& upData = updateData[data->GetArrayIndex()];
			upData.CalculateRate(capacity, gpuMemoryBusWidth);
			data->SetUpdateFrequency(1.0f);
			data->SetUpdatePerObjectRate(upData.resolveRate);
			data->SetUpdateEnd(true);
		}
	}
	void JDx12HardwareOccCulling::NotifyDestroyHdOccBuffer(JCullingInfo* cullingInfo)
	{
		if (cullingInfo == nullptr)
			return;

		updateData.erase(updateData.begin() + cullingInfo->GetArrayIndex());
	}
	void JDx12HardwareOccCulling::BeginDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		/*	
			//앞으로 read back buffer로 cpu로 occlusion query 데이터를 가져와서 
			//SetPredict가 아닌 pointer에 접근해서 데이터를 읽는다.
			if (!IsSameDevice(bindSet))
			return;

			const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
			JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(dx12BindSet->cullingManager);
			ID3D12GraphicsCommandList* cmdList = dx12BindSet->cmdList;

			auto cInterface = helper.GetCullInterface();
			if (!cInterface.HasCullingData(J_CULLING_TYPE::HD_OCCLUSION))
				return;
					//auto resource = dx12Cm->GetResource(J_CULLING_TYPE::HD_OCCLUSION, cInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION));
		//JD3DUtility::ResourceTransition(cmdList, resource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PREDICATION);
		*/
	}
	void JDx12HardwareOccCulling::EndDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		/*
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(dx12BindSet->cullingManager);
		ID3D12GraphicsCommandList* cmdList = dx12BindSet->cmdList;

		auto cInterface = helper.GetCullInterface();
		if (!cInterface.HasCullingData(J_CULLING_TYPE::HD_OCCLUSION))
			return;

		auto resource = dx12Cm->GetResource(J_CULLING_TYPE::HD_OCCLUSION, cInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION));
		JD3DUtility::ResourceTransition(cmdList, resource, D3D12_RESOURCE_STATE_PREDICATION, D3D12_RESOURCE_STATE_COMMON);
		*/
	}
	void JDx12HardwareOccCulling::DrawOcclusionDepthMap(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(occDrawSet))
			return;

		if (!helper.CanOccCulling() || !helper.GetCullingUserAccess()->AllowHdOcclusionCulling())
			return;

		const JDx12GraphicOccDrawSet* dx12DrawSet = static_cast<const JDx12GraphicOccDrawSet*>(occDrawSet);
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dx12DrawSet->device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12DrawSet->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12DrawSet->cmdList;

		auto gRInterface = helper.GetOccGResourceInterface();
		const uint dataCount = gRInterface.GetDataCount(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL);

		auto cInterface = helper.GetCullInterface();
		UpdateData& upData = updateData[cInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION)];
		const bool canCulling = !upData.updateAllObject;	 
		for (uint i = 0; i < dataCount; ++i)
		{
			const int dsvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, 0);
			const int dsvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV, 0);
		
			JGraphicResourceInfo* dsInfo = dx12Gm->GetInfo(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsvVecIndex);
			ID3D12Resource* dsResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsvVecIndex);
			D3D12_RESOURCE_DESC desc = dsResource->GetDesc();

			D3D12_VIEWPORT viewPort;
			D3D12_RECT rect;
			dx12Device->CalViewportAndRect(JVector2F(desc.Width, desc.Height), viewPort, rect);
			cmdList->RSSetViewports(1, &viewPort);
			cmdList->RSSetScissorRects(1, &rect);

			CD3DX12_CPU_DESCRIPTOR_HANDLE dsv = dx12Gm->GetCpuDsvDescriptorHandle(dsvHeapIndex);
			JD3DUtility::ResourceTransition(cmdList, dsResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			cmdList->OMSetRenderTargets(0, nullptr, false, &dsv);
			//draw specific count 

			JDx12GraphicDepthMapDrawSet depthMapSet(dx12DrawSet);
			const std::vector<JUserPtr<JGameObject>>& objVec00 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
			const std::vector<JUserPtr<JGameObject>>& objVec01 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::SKINNED);

			dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet,
				objVec00,
				helper,
				JDrawCondition(helper, false, canCulling, false));

			dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet,
				objVec01,
				helper,
				JDrawCondition(helper, false, canCulling, false));
			JD3DUtility::ResourceTransition(cmdList, dsResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
		}
	}
	void JDx12HardwareOccCulling::DrawOcclusionDepthMapMultiThread(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(occDrawSet))
			return;

		if (!helper.CanOccCulling() || !helper.GetCullingUserAccess()->AllowHdOcclusionCulling())
			return;

		const JDx12GraphicOccDrawSet* dx12DrawSet = static_cast<const JDx12GraphicOccDrawSet*>(occDrawSet);
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dx12DrawSet->device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12DrawSet->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12DrawSet->cmdList;

		auto gRInterface = helper.GetOccGResourceInterface();
		const uint dataCount = gRInterface.GetDataCount(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL);

		auto cInterface = helper.GetCullInterface();
		UpdateData& upData = updateData[cInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION)];
		const bool canCulling = !upData.updateAllObject;		 
		for (uint i = 0; i < dataCount; ++i)
		{
			const int dsvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, 0);
			const int dsvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV, 0);
			
			ID3D12Resource* dsResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsvVecIndex);
			D3D12_RESOURCE_DESC desc = dsResource->GetDesc();

			D3D12_VIEWPORT viewPort;
			D3D12_RECT rect;
			dx12Device->CalViewportAndRect(JVector2F(desc.Width, desc.Height), viewPort, rect);
			cmdList->RSSetViewports(1, &viewPort);
			cmdList->RSSetScissorRects(1, &rect);

			CD3DX12_CPU_DESCRIPTOR_HANDLE dsv = dx12Gm->GetCpuDsvDescriptorHandle(dsvHeapIndex);
			cmdList->OMSetRenderTargets(0, nullptr, false, &dsv);

			const std::vector<JUserPtr<JGameObject>>& objVec00 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
			const std::vector<JUserPtr<JGameObject>>& objVec01 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::SKINNED);

			JDx12GraphicDepthMapDrawSet depthMapSet(dx12DrawSet);
			dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet,
				objVec00,
				helper,
				JDrawCondition(helper, false, canCulling, false));

			dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet,
				objVec01,
				helper,
				JDrawCondition(helper, false, canCulling, false));
		}
	}
	void JDx12HardwareOccCulling::ExtractHDOcclusionCullingData(const JGraphicHdOccExtractSet* extractSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(extractSet))
			return;

		const JDx12GraphicHdOccExtractSet* dx12ExtractSet = static_cast<const JDx12GraphicHdOccExtractSet*>(extractSet);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(dx12ExtractSet->cullingM);
		ID3D12GraphicsCommandList* cmdList = dx12ExtractSet->cmdList;

		auto cInterface = helper.GetCullInterface();
		const int occVecIndex = cInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION);
		const uint buffSize = cInterface.GetResultBufferSize(J_CULLING_TYPE::HD_OCCLUSION);
		ID3D12QueryHeap* occQueryHeap = dx12Cm->GetQueryHeap(occVecIndex);
		ID3D12Resource* occQueryResource = dx12Cm->GetResource(J_CULLING_TYPE::HD_OCCLUSION, occVecIndex);

		size_t queryOffset = 0;
		size_t queryCount = 0; 
		updateData[occVecIndex].Update(queryOffset, queryCount);
		 
		JD3DUtility::ResourceTransition(cmdList, occQueryResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		cmdList->ResolveQueryData(occQueryHeap, D3D12_QUERY_TYPE_BINARY_OCCLUSION, queryOffset, queryCount, occQueryResource, queryOffset * Private::querySize);
		JD3DUtility::ResourceTransition(cmdList, occQueryResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
		
		//ExtractHDOcclusionCullingData은 항상 single thread에서 수행되며 hdOcc에 마지막 시퀸스 이므로
		//여기서 updateData[occVecIndex].updateAllObject trigger를 해제한다.
		if (updateData[occVecIndex].updateAllObject)
			updateData[occVecIndex].updateAllObject = false;
		  
		dx12Cm->GetCullingInfo(J_CULLING_TYPE::HD_OCCLUSION, occVecIndex)->SetUpdateEnd(updateData[occVecIndex].isUpdateEnd);
		/*
		if (occQueryResource != nullptr)
		{
			std::string result;
			auto dxHolder = dx12Cm->GetDxHolder(J_CULLING_TYPE::HD_OCCLUSION, occVecIndex);
			for (uint i = 0; i < queryCount; ++i)
				result += "(" + std::to_string(queryOffset + i) + "): " + std::to_string(dxHolder->IsCulled(i)) + "	 ";
			Develop::JDevelopDebug::PushLog(result);
			Develop::JDevelopDebug::Write();
		}
		*/
	}
}