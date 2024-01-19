#include"JDx12HardwareOccCulling.h"
#include"../../Dx/JDx12CullingManager.h" 
#include"../../Dx/JDx12CullingResourceHolder.h"  
#include"../../../DepthMap/Dx/JDx12DepthTest.h"
#include"../../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../../Device/Dx/JDx12GraphicDevice.h"
#include"../../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../../Utility/Dx/JD3DUtility.h"
#include"../../../JGraphicUpdateHelper.h"
#include"../../../../Core/Time/JStopWatch.h"
#include"../../../../Core/Platform/JHardwareInfo.h"
#include"../../../../Object/Component/Camera/JCamera.h" 

#include"../../../../Develop/Debug/JDevelopDebug.h"
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
	bool JDx12HardwareOccCulling::UpdateData::CanPassThisFrame(const uint frameResourceIndex)const noexcept
	{
		return updateCycle <= frameResourceIndex;
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
		offset = count = 0;
		bufferCapacity = capacity;
		updateAllObject = false;
		isUpdateEnd = true;
		CalcuateUpdateCycle();
	}
	void JDx12HardwareOccCulling::UpdateData::CalculateRate(const size_t capacity, const size_t gpuMemoryBusWidth)
	{
		//2400
		const uint executeCount = capacity / gpuMemoryBusWidth;
		const float executeRate = 1.0f / executeCount;

		resolveRate = std::clamp(executeRate, minResolveRate, 1.0f);
		offset = count = 0;
		bufferCapacity = capacity;
		updateAllObject = false;
		isUpdateEnd = true;
		CalcuateUpdateCycle();
	}
	uint JDx12HardwareOccCulling::UpdateData::CalculateCount()const noexcept
	{
		return updateAllObject ? bufferCapacity : (uint)(bufferCapacity * resolveRate);
	}
	void JDx12HardwareOccCulling::UpdateData::CalcuateUpdateCycle() noexcept
	{
		const uint updateCount = CalculateCount();
		updateCycle = bufferCapacity % updateCount ? (bufferCapacity / updateCount + 1) : (bufferCapacity / updateCount);
	}
	void JDx12HardwareOccCulling::UpdateData::Update()
	{ 
		if (isUpdateEnd)
			isUpdateEnd = false;

		//update pre count
		offset += count;
		count = CalculateCount();

		if (offset >= bufferCapacity)
		{
			offset = 0;
			isUpdateEnd = true;
		}
		if (offset + count > bufferCapacity)
			count = bufferCapacity - offset;
	}
	void JDx12HardwareOccCulling::UpdateData::Reset()
	{
		updateAllObject = false;
		offset = count = 0;
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
		cullingInfo->SetUpdateFrequency(upData.resolveRate);
		cullingInfo->SetUpdateEnd(true);
	}
	void JDx12HardwareOccCulling::NotifyReBuildHdOccBuffer(JGraphicDevice* device, const size_t capacity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo)
	{
		for (const auto& data : cullingInfo)
		{
			UpdateData& upData = updateData[data->GetArrayIndex()];
			upData.CalculateRate(capacity, gpuMemoryBusWidth);
			data->SetUpdateFrequency(upData.resolveRate);  
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
		if (!IsSameDevice(occDrawSet) || !helper.allowHdOcclusionCulling)
			return; 

		const JDx12GraphicOccDrawSet* dx12DrawSet = static_cast<const JDx12GraphicOccDrawSet*>(occDrawSet);
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dx12DrawSet->device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12DrawSet->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12DrawSet->cmdList;

		auto gRInterface = helper.GetOccGResourceInterface();
		const uint dataCount = gRInterface.GetDataCount(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL);

		auto cInterface = helper.GetCullInterface();
		UpdateData& upData = updateData[cInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM)];
		if (upData.CanPassThisFrame(helper.info.currFrameResourceIndex))
			return;

		const bool hasFrustumCulling = cInterface.HasCullingData(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM);
		const bool hasAlignedData = hasFrustumCulling && helper.cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA;
		const uint camFrustumIndex = helper.GetCullInterface().GetArrayIndex(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM);
		
		const bool canCulling = !upData.updateAllObject;
		const std::vector<JUserPtr<JGameObject>>& objVec00 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
		const std::vector<JUserPtr<JGameObject>>& objVec01 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::SKINNED);

		for (uint i = 0; i < dataCount; ++i)
		{
			const int dsvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, 0);
			const int dsvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV, 0);
		
			JGraphicResourceInfo* dsInfo = dx12Gm->GetInfo(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsvVecIndex);
			ID3D12Resource* dsResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsvVecIndex);
			D3D12_RESOURCE_DESC desc = dsResource->GetDesc();

			D3D12_VIEWPORT viewPort;
			D3D12_RECT rect;
			dx12Device->CalViewportAndRect(JVector2F(desc.Width, desc.Height), true, viewPort, rect);
			cmdList->RSSetViewports(1, &viewPort);
			cmdList->RSSetScissorRects(1, &rect);

			CD3DX12_CPU_DESCRIPTOR_HANDLE dsv = dx12Gm->GetCpuDsvDescriptorHandle(dsvHeapIndex);
			JD3DUtility::ResourceTransition(cmdList, dsResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			cmdList->OMSetRenderTargets(0, nullptr, false, &dsv);
			//draw specific count 

			JDx12GraphicDepthMapDrawSet depthMapSet(dx12DrawSet);
			if (hasAlignedData)
			{
				dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet,
					helper.objVec.aligned[camFrustumIndex],
					helper,
					JDrawCondition(helper, false, canCulling, false, helper.option.allowHDOcclusionUseOccluder));
			}
			else
			{
				dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet,
					objVec00,
					helper,
					JDrawCondition(helper, false, canCulling, false, helper.option.allowHDOcclusionUseOccluder));
				dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet,
					objVec01,
					helper,
					JDrawCondition(helper, false, canCulling, false, helper.option.allowHDOcclusionUseOccluder));
			}
			JD3DUtility::ResourceTransition(cmdList, dsResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
		}
	}
	void JDx12HardwareOccCulling::DrawOcclusionDepthMapMultiThread(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(occDrawSet) || !helper.allowHdOcclusionCulling)
			return;

		const JDx12GraphicOccDrawSet* dx12DrawSet = static_cast<const JDx12GraphicOccDrawSet*>(occDrawSet);
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dx12DrawSet->device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12DrawSet->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12DrawSet->cmdList;

		auto gRInterface = helper.GetOccGResourceInterface();
		const uint dataCount = gRInterface.GetDataCount(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL);

		auto cInterface = helper.GetCullInterface();
		UpdateData& upData = updateData[cInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM)];
		if (upData.CanPassThisFrame(helper.info.currFrameResourceIndex))
			return;

		const bool hasFrustumCulling = cInterface.HasCullingData(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM);
		const bool hasAlignedData = hasFrustumCulling && helper.cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA;
		const uint camFrustumIndex = helper.GetCullInterface().GetArrayIndex(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM);

		const bool canCulling = !upData.updateAllObject;
		const std::vector<JUserPtr<JGameObject>>& objVec00 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
		const std::vector<JUserPtr<JGameObject>>& objVec01 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::SKINNED);

		for (uint i = 0; i < dataCount; ++i)
		{
			const int dsvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, 0);
			const int dsvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV, 0);
			
			ID3D12Resource* dsResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsvVecIndex);
			D3D12_RESOURCE_DESC desc = dsResource->GetDesc();

			D3D12_VIEWPORT viewPort;
			D3D12_RECT rect;
			dx12Device->CalViewportAndRect(JVector2F(desc.Width, desc.Height), true, viewPort, rect);
			cmdList->RSSetViewports(1, &viewPort);
			cmdList->RSSetScissorRects(1, &rect);

			CD3DX12_CPU_DESCRIPTOR_HANDLE dsv = dx12Gm->GetCpuDsvDescriptorHandle(dsvHeapIndex);
			cmdList->OMSetRenderTargets(0, nullptr, false, &dsv);
			
			JDx12GraphicDepthMapDrawSet depthMapSet(dx12DrawSet);
			if (hasAlignedData)
			{
				dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet,
					helper.objVec.aligned[camFrustumIndex],
					helper,
					JDrawCondition(helper, false, canCulling, false, helper.option.allowHDOcclusionUseOccluder));
			}
			else
			{
				dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet,
					objVec00,
					helper,
					JDrawCondition(helper, false, canCulling, false, helper.option.allowHDOcclusionUseOccluder));
				dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet,
					objVec01,
					helper,
					JDrawCondition(helper, false, canCulling, false, helper.option.allowHDOcclusionUseOccluder));
			} 
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
		const int occVecIndex = cInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM);
		auto cullingInfo = dx12Cm->GetCullingInfo(J_CULLING_TYPE::HD_OCCLUSION, occVecIndex);
		UpdateData& upData = updateData[occVecIndex];
		if (upData.CanPassThisFrame(helper.info.currFrameResourceIndex))
		{
			JCullingUpdatedInfo updatedInfo;
			updatedInfo.updatedStartIndex = 0;
			updatedInfo.updatedCount = 0;		 
			cullingInfo->SetUpdatedInfo(updatedInfo, helper.info.currFrameResourceIndex);
			return;
		}

		//const uint buffSize = cInterface.GetResultBufferSize(J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM);
		ID3D12QueryHeap* occQueryHeap = dx12Cm->GetQueryHeap(occVecIndex);
		ID3D12Resource* occQueryResource = dx12Cm->GetResource(J_CULLING_TYPE::HD_OCCLUSION, occVecIndex);

		upData.Update();
		const size_t queryOffset = upData.offset;
		const size_t queryCount = upData.count;

		JD3DUtility::ResourceTransition(cmdList, occQueryResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		cmdList->ResolveQueryData(occQueryHeap, D3D12_QUERY_TYPE_BINARY_OCCLUSION, queryOffset, queryCount, occQueryResource, queryOffset * Private::querySize);
		JD3DUtility::ResourceTransition(cmdList, occQueryResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
		
		//ExtractHDOcclusionCullingData은 항상 single thread에서 수행되며 hdOcc에 마지막 시퀸스 이므로
		//여기서 upData.updateAllObject trigger를 해제한다.
		if (upData.updateAllObject)
			upData.updateAllObject = false;
		 
		JCullingUpdatedInfo updatedInfo;
		updatedInfo.updatedStartIndex = queryOffset;
		updatedInfo.updatedCount = queryCount;
		 
		cullingInfo->SetUpdateEnd(upData.isUpdateEnd);
		cullingInfo->SetUpdatedInfo(updatedInfo, helper.info.currFrameResourceIndex);
		 /*
		 */
		/*
		Develop::JDevelopDebug::PushLog("FrameIndex: " + std::to_string(helper.info.currFrameResourceIndex) +
			" Offset: " + std::to_string(queryOffset) +
			" Count: " + std::to_string(queryCount) +
			" Cyclate: " + std::to_string(upData.updateCycle));
		Develop::JDevelopDebug::Write();
		if (occQueryResource != nullptr)
		{
			std::string result; occVecIndex);
			for (uint i = 0; i < queryCount; ++i)
				result += "(" + std::to_string(queryOffset + i) +
			auto dxHolder = dx12Cm->GetDxHolder(J_CULLING_TYPE::HD_OCCLUSION, "): " + std::to_string(dxHolder->IsCulled(i)) + "	 ";
			Develop::JDevelopDebug::PushLog(result);
			Develop::JDevelopDebug::Write();
		}
		*/
	}
}