#include"JDx12HardwareOccCulling.h"
#include"../../Dx/JDx12CullingManager.h" 
#include"../../Dx/JDx12CullingResourceHolder.h"
#include"../../../Command/Dx/JDx12CommandContext.h"
#include"../../../DepthMap/Dx/JDx12DepthTest.h"
#include"../../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../../Device/Dx/JDx12GraphicDevice.h"
#include"../../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"
#include"../../../Utility/Dx/JDx12Utility.h" 
#include"../../../JGraphicUpdateHelper.h"
#include"../../../../Core/Time/JStopWatch.h"
#include"../../../../Core/Platform/JHardwareInfo.h"
#include"../../../../Core/Log/JLogMacro.h"
#include"../../../../Object/Component/Camera/JCamera.h" 
#include"../../../../Object/Resource/Scene/JScene.h" 
 
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
		static constexpr uint appMaxResolveRateCapacity = allUpdateCount;
		static constexpr uint appMinResolveRateCapacity = 1000;
		if (capacity < appMaxResolveRateCapacity)
			resolveRate = 1.0f;
		else if (capacity < appMinResolveRateCapacity)
		{
			static constexpr uint rateCount = 4;	//1/2, 1/4, 1/8, 1/16, 1/32, 1/64 
			static constexpr float rateWidth = 1.0f / (float)rateCount;

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
		isUpdateEnd = true;
		CalcuateUpdateCycle();
	}
	void JDx12HardwareOccCulling::UpdateData::CalculateRate(const size_t capacity, const size_t gpuMemoryBusWidth)
	{  
		const uint executeCount = capacity / gpuMemoryBusWidth;
		if (capacity < allUpdateCount)
			resolveRate = 1.0f;
		else
		{
			float executeRate = 1.0f / executeCount;
			resolveRate = std::clamp(executeRate, minResolveRate, maxResolveRate); 
		}
 
		offset = count = 0;
		bufferCapacity = capacity; 
		isUpdateEnd = true;
		CalcuateUpdateCycle();
	}
	uint JDx12HardwareOccCulling::UpdateData::CalculateCount()const noexcept
	{
		return (uint)(bufferCapacity * resolveRate);
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
		offset = count = 0;
	}

	JDx12HardwareOccCulling::~JDx12HardwareOccCulling()
	{
		ClearResource();
	}
	void JDx12HardwareOccCulling::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		BuildResource(device, gM, GetGraphicInfo());
	}
	void JDx12HardwareOccCulling::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12HardwareOccCulling::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	bool JDx12HardwareOccCulling::HasPreprocessing()const noexcept
	{
		return true;
	}
	bool JDx12HardwareOccCulling::HasPostprocessing()const noexcept
	{
		return false;
	}
	bool JDx12HardwareOccCulling::HasDependency(const JGraphicInfo::TYPE type)const noexcept
	{
		if (type == JGraphicInfo::TYPE::FRAME)
			return true;
		else
			return false;
	}
	bool JDx12HardwareOccCulling::HasDependency(const JGraphicOption::TYPE type)const noexcept
	{ 
		return false;
	}
	void JDx12HardwareOccCulling::NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)
	{
		auto dx12Set = static_cast<const JDx12GraphicInfoChangedSet&>(set);  
		if (dx12Set.preInfo.frame.upBoundingObjCapacity != dx12Set.newInfo.frame.upBoundingObjCapacity)
			NotifyReBuildHdOccBuffer(dx12Set.device, dx12Set.newInfo.frame.upBoundingObjCapacity, dx12Set.cm->GetCullingInfoVec(J_CULLING_TYPE::HD_OCCLUSION));
	}
	void JDx12HardwareOccCulling::NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)
	{ 
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
		//앞으로 read back buffer로 cpu로 occlusion query 데이터를 가져와서
		//SetPredict가 아닌 pointer에 접근해서 데이터를 읽는다.
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12BindSet->context);

		auto gRInterface = helper.GetOccGResourceInterface(); 
		auto dsSet = context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		
		context->Transition(dsSet.holder, D3D12_RESOURCE_STATE_DEPTH_READ);		//깊이값 write(x)
		context->FlushResourceBarriers();

		auto cInterface = helper.GetCullInterface();
		UpdateData& upData = updateData[cInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM)];
		upData.Update();
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
		JDx12Utility::ResourceTransition(cmdList, resource, D3D12_RESOURCE_STATE_PREDICATION, D3D12_RESOURCE_STATE_COMMON);
		*/
	}
	void JDx12HardwareOccCulling::DrawOcclusionDepthMap(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(occDrawSet) || !helper.allowHdOcclusionCulling)
			return;

		const JDx12GraphicOccDrawSet* dx12DrawSet = static_cast<const JDx12GraphicOccDrawSet*>(occDrawSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12DrawSet->context);

		auto gRInterface = helper.GetOccGResourceInterface(); 
		auto cInterface = helper.GetCullInterface();
		UpdateData& upData = updateData[cInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM)];
		if (upData.CanPassThisFrame(helper.info.frame.currIndex))
			return;

		const bool hasFrustumCulling = cInterface.HasCullingData(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM);
		const bool hasAlignedData = hasFrustumCulling && helper.cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA;
		const uint camFrustumIndex = helper.GetCullInterface().GetArrayIndex(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM);
	 
		upData.Update();

		//use SCENE_LAYER_DEPTH_STENCIL after scene draw
		auto dsSet = context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		context->SetViewportAndRect(dsSet.info->GetResourceSize());
		context->Transition(dsSet.holder, D3D12_RESOURCE_STATE_DEPTH_READ, true);	//깊이값 write(x)
		context->SetDepthStencilView(dsSet);

		JDx12GraphicDepthMapDrawSet depthMapSet(dx12DrawSet);
		JDrawCondition drawCondition(helper, false, true, false, helper.option.culling.allowHDOcclusionUseOccluder);
		drawCondition.SetRestrictRange(upData.offset, upData.count);

		if (hasAlignedData && helper.scene->HasCanCullingAccelerator(J_ACCELERATOR_LAYER::COMMON_OBJECT))
			dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet, helper.objVec.aligned[camFrustumIndex], helper, drawCondition);
		else
		{
			const std::vector<JUserPtr<JGameObject>>& objVec00 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
			const std::vector<JUserPtr<JGameObject>>& objVec01 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::SKINNED);
			
			dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet, objVec00, helper, drawCondition);
			dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet, objVec01, helper, drawCondition);
		}
	}
	void JDx12HardwareOccCulling::DrawOcclusionDepthMapMultiThread(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(occDrawSet) || !helper.allowHdOcclusionCulling)
			return;

		const JDx12GraphicOccDrawSet* dx12DrawSet = static_cast<const JDx12GraphicOccDrawSet*>(occDrawSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12DrawSet->context);

		auto gRInterface = helper.GetOccGResourceInterface(); 
		auto cInterface = helper.GetCullInterface();
		UpdateData& upData = updateData[cInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM)];
		if (upData.CanPassThisFrame(helper.info.frame.currIndex))
			return;

		const bool hasFrustumCulling = cInterface.HasCullingData(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM);
		const bool hasAlignedData = hasFrustumCulling && helper.cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA;
		const uint camFrustumIndex = helper.GetCullInterface().GetArrayIndex(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM);
 
		auto dsSet = context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		context->SetViewportAndRect(dsSet.info->GetResourceSize());
		context->SetDepthStencilView(dsSet);

		JDx12GraphicDepthMapDrawSet depthMapSet(dx12DrawSet);
		JDrawCondition drawCondition(helper, false, true, false, helper.option.culling.allowHDOcclusionUseOccluder);
		drawCondition.SetRestrictRange(upData.offset, upData.count);

		if (hasAlignedData && helper.scene->HasCanCullingAccelerator(J_ACCELERATOR_LAYER::COMMON_OBJECT))
			dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet, helper.objVec.aligned[camFrustumIndex], helper, drawCondition);
		else
		{
			const std::vector<JUserPtr<JGameObject>>& objVec00 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
			const std::vector<JUserPtr<JGameObject>>& objVec01 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::SKINNED);

			dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet, objVec00, helper, drawCondition);
			dx12DrawSet->depthTest->DrawHdOcclusionQueryObject(&depthMapSet, objVec01, helper, drawCondition);
		}
	}
	void JDx12HardwareOccCulling::ExtractHDOcclusionCullingData(const JGraphicHdOccExtractSet* extractSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(extractSet))
			return;

		const JDx12GraphicHdOccExtractSet* dx12ExtractSet = static_cast<const JDx12GraphicHdOccExtractSet*>(extractSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12ExtractSet->context);

		auto cInterface = helper.GetCullInterface();
		auto cSet = context->ComputeSet(cInterface, J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM);

		UpdateData& upData = updateData[cSet.info->GetArrayIndex()];
		if (upData.CanPassThisFrame(helper.info.frame.currIndex))
		{
			JCullingUpdatedInfo updatedInfo;
			updatedInfo.updatedStartIndex = 0;
			updatedInfo.updatedCount = 0;
			cSet.info->SetUpdatedInfo(updatedInfo, helper.info.frame.currIndex);
			return;
		}

		//upData.Update();
		context->ResolveOcclusionQueryData(cSet, upData.offset, upData.count);
		//context->Transition(cSet.gHolder, D3D12_RESOURCE_STATE_COMMON);
 
		JCullingUpdatedInfo updatedInfo;
		updatedInfo.updatedStartIndex = upData.offset;
		updatedInfo.updatedCount = upData.count;

		cSet.info->SetUpdateEnd(upData.isUpdateEnd);
		cSet.info->SetUpdatedInfo(updatedInfo, helper.info.frame.currIndex);
		/*
		*/
		/*
		Develop::JDevelopDebug::PushLog("FrameIndex: " + std::to_string(helper.info.frame.currIndex) +
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
	void JDx12HardwareOccCulling::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)
	{
		auto gpuInfo = Core::JHardwareInfo::GetGpuInfo();
		gpuMemoryBusWidth = gpuInfo[0].memoryBusWidth;
	}
	void JDx12HardwareOccCulling::ClearResource()
	{

	}
}