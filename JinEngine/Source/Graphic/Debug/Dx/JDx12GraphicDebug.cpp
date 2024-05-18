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


#include"JDx12GraphicDebug.h" 
#include"../../JGraphicInfo.h"
#include"../../JGraphicUpdateHelper.h"
#include"../../Command/Dx/JDx12CommandContext.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../Device/Dx/JDx12GraphicDevice.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h" 
#include"../../Utility/Dx/JDx12Utility.h" 
#include"../../Utility/Dx/JDx12ObjectCreation.h" 
#include"../../../Application/Engine/JApplicationEngine.h" 
#include"../../../Core/Exception/JExceptionMacro.h"  
#include"../../../Core/Platform/JHardwareInfo.h"  
#include"../../../Object/Component/Camera/JCamera.h"
#include"../../../Object/Component/Light/JLight.h"
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h" 
#include<DirectXColors.h>
 

using namespace DirectX;
using namespace Microsoft::WRL;

namespace JinEngine::Graphic
{
	namespace Private
	{
		static constexpr uint srcTextureHandleIndex = 0; 
		static constexpr uint destTextureHandleIndex = srcTextureHandleIndex + 1;
		static constexpr uint settingCbIndex = destTextureHandleIndex + 1;
		static constexpr uint slotCount = settingCbIndex + 1;
		
		static JVector3<uint> GetThreadDim()
		{
			return JVector3<uint>(16, 16, 1);
		}
	}
	JDx12GraphicDebug::DebugDataSet::DebugDataSet(JDx12CommandContext* context,
		const JGraphicDebugRsComputeSet* debugSet,
		const JDrawHelper& helper,
		JDx12GraphicResourceComputeSetBufferBase& srcBase,
		JDx12GraphicResourceComputeSetBufferBase& destBase)
	{
		if (helper.cam != nullptr)
		{
			auto gRInterface = helper.cam->GraphicResourceUserInterface();
			auto rtSet = context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
			
			//DEBUG_TYPE_DEPTH	-depth
			//DEBUG_TYPE_ALBEDO
			//DEBUG_TYPE_SPECULAR
			//DEBUG_TYPE_NORMAL	-normal 
			//DEBUG_TYPE_TANGENT	-normal 
			//DEBUG_TYPE_VELOCITY	-restir이외에 velocity 사용시 다시 활성화
			//DEBUG_TYPE_AO	-ao
			srcBase.Push(context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW));
			if (rtSet.info->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP))
				srcBase.Push(context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP));
			else
				srcBase.Push(JDx12GraphicResourceComputeSet());
			if (rtSet.info->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP))
				srcBase.Push(context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP));
			else
				srcBase.Push(JDx12GraphicResourceComputeSet());

			if (rtSet.info->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP))
				srcBase.Push(context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP));
			else
				srcBase.Push(JDx12GraphicResourceComputeSet());	 

			if (rtSet.info->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP))
				srcBase.Push(context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP));
			else
				srcBase.Push(JDx12GraphicResourceComputeSet());

			//if (rtSet.info->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY))
			//	srcBase.Push(context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY));
			//else
			//	srcBase.Push(JDx12GraphicResourceComputeSet());
			srcBase.Push(context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, J_GRAPHIC_TASK_TYPE::APPLY_SSAO));

			for (uint i = 0; i < DEBUG_TYPE_COUNT; ++i)
				allowTrigger[i] = srcBase(i).IsValid();

			J_GRAPHIC_TASK_TYPE taskType[DEBUG_TYPE_COUNT]
			{
				J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE,
				J_GRAPHIC_TASK_TYPE::ALBEDO_MAP_VISUALIZE,
				J_GRAPHIC_TASK_TYPE::SPECULAR_MAP_VISUALIZE,
				J_GRAPHIC_TASK_TYPE::NORMAL_MAP_VISUALIZE,
				J_GRAPHIC_TASK_TYPE::TANGENT_MAP_VISUALIZE,
				//J_GRAPHIC_TASK_TYPE::VELOCITY_MAP_VISUALIZE,
				J_GRAPHIC_TASK_TYPE::SSAO_VISUALIZE
			};
			 
			for (uint i = 0; i < DEBUG_TYPE_COUNT; ++i)
			{
				if (allowTrigger[i])
				{
					destBase.Push(context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, taskType[i]));
					allowTrigger[i] &= destBase(i).IsValid(); 
				}
			}
			size = rtSet.info->GetResourceSize();		 
			nearFar.x = helper.cam->GetNear();
			nearFar.y = helper.cam->GetFar();
			isNonLinearDepthMap = !helper.cam->IsOrthoCamera();
			allowHzb = helper.cam->AllowHzbOcclusionCulling();
		}
		else if (helper.lit != nullptr)
		{
			bool isNonLinearDepthMap = false;
			/*
			* light는 type에 따라서 projection type이 정해진다.
			* directional => orhto ... Linear
			* point, spot => perspective ... NonLinear
			*/
			const J_LIGHT_TYPE litType = helper.lit->GetLightType();
			if (litType == J_LIGHT_TYPE::SPOT || litType == J_LIGHT_TYPE::POINT)
				isNonLinearDepthMap = true;	//spot is perspective

			nearFar.x = helper.lit->GetFrustumNear();
			nearFar.y = helper.lit->GetFrustumFar();
			allowTrigger[DEBUG_TYPE_DEPTH] = true;
			allowHzb = helper.lit->AllowHzbOcclusionCulling();

			auto gRInterface = helper.lit->GraphicResourceUserInterface();
			const J_GRAPHIC_RESOURCE_TYPE grType = JLightType::SmToGraphicR(helper.lit->GetShadowMapType());

			const uint shadowDataIndex = gRInterface.GetResourceDataIndex(grType, J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW);
			const uint debugDataIndex = gRInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE);
			const uint debugCount = gRInterface.GetDataCount(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP);
			const uint macCount = srcBase.GetMaxCount();

			for (uint i = 0; i < debugCount && i < macCount; ++i)
			{
				srcBase.Push(context->ComputeSet(gRInterface, grType, shadowDataIndex + i));
				destBase.Push(context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, debugDataIndex + i));
			}
		}
		allowOccDepth = helper.allowDrawOccDepthMap;
	}
	void JDx12GraphicDebug::DebugDataSet::SetOcclusionBuffer(JDx12CommandContext* context,
		JGraphicResourceUserInterface* gRInterface,
		const JDrawHelper& helper,
		const J_GRAPHIC_RESOURCE_TYPE srcType,
		const J_GRAPHIC_RESOURCE_TYPE destType,
		JDx12GraphicResourceComputeSetBufferBase& srcBase,
		JDx12GraphicResourceComputeSetBufferBase& destBase)const
	{
		const uint dataCount = gRInterface->GetDataCount(srcType);
		const uint macCount = srcBase.GetMaxCount();

		for (uint i = 0; i < dataCount && i < macCount; ++i)
		{
			srcBase.Push(context->ComputeSet(*gRInterface, srcType, i));
			destBase.Push(context->ComputeSet(*gRInterface, destType, i));
		}
	}

	JDx12GraphicDebug::~JDx12GraphicDebug()
	{
		ClearResource();
	}
	void JDx12GraphicDebug::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		BuildResource(device, gM, GetGraphicInfo());
	}
	void JDx12GraphicDebug::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicDebug::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	void JDx12GraphicDebug::ComputeCamDebug(const JGraphicDebugRsComputeSet* debugSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(debugSet) || !helper.allowDrawDebugMap)
			return;

		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(debugSet->context);
		JDx12GraphicResourceComputeSetBuffer<DEBUG_TYPE_COUNT> srcBuff;
		JDx12GraphicResourceComputeSetBuffer<DEBUG_TYPE_COUNT> destBuff;

		DebugDataSet set(context, debugSet, helper, srcBuff, destBuff);
		JDx12ComputeShaderDataHolder* shader[DEBUG_TYPE_COUNT]
		{
			(set.isNonLinearDepthMap ? nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE].get() : linearDepthMapShaderData.get()),
			albedoMapShaderData.get(),
			specularMapShaderData.get(),
			normalMapShaderData.get(),
			tangentMapShaderData.get(),
			//velocityMapShaderData.get(),
			aoMapShaderData.get()
		};

		context->Transition(&srcBuff, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(&destBuff, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

		for (uint i = 0; i < DEBUG_TYPE_COUNT; ++i)
		{
			if (set.allowTrigger[i])
			{ 
				set.srcHandle = srcBuff(i).GetGpuSrvHandle();
				set.destHandle = destBuff(i).GetGpuUavHandle();
				Execute(context, set, shader[i]);
			}
		}
		context->Transition(&destBuff, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		if (set.allowOccDepth)
			ComputeOccDebug(context, set, helper);
		else
			context->FlushResourceBarriers();
	}
	void JDx12GraphicDebug::ComputeLitDebug(const JGraphicDebugRsComputeSet* debugSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(debugSet) || !helper.allowDrawDebugMap)
			return;
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(debugSet->context);

		constexpr uint maxDebugPerLight = 2;
		JDx12GraphicResourceComputeSetBuffer<maxDebugPerLight> srcBuff;
		JDx12GraphicResourceComputeSetBuffer<maxDebugPerLight> destBuff;
		DebugDataSet set(context, debugSet, helper, srcBuff, destBuff);

		context->Transition(&srcBuff, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(&destBuff, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

		for (uint i = 0; i < srcBuff.validCount; ++i)
		{
			set.srcHandle = srcBuff(i).GetGpuSrvHandle();
			set.destHandle = destBuff(i).GetGpuUavHandle();
			set.size = srcBuff(i).info->GetResourceSize();
			if (set.isNonLinearDepthMap)
				Execute(context, set, nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE].get());
			else
				Execute(context, set, linearDepthMapShaderData.get());
		}
		context->Transition(&destBuff, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		if (set.allowOccDepth)
			ComputeOccDebug(context, set, helper);
		else
			context->FlushResourceBarriers();
	}
	void JDx12GraphicDebug::ComputeOccDebug(JDx12CommandContext* context, DebugDataSet& set, const JDrawHelper& helper)
	{
		JVector2<uint> occlusionSize = JVector2<uint>(helper.info.resource.occlusionWidth, helper.info.resource.occlusionHeight);
		J_GRAPHIC_RESOURCE_TYPE srcType;
		J_GRAPHIC_RESOURCE_TYPE destType;
		if (!helper.option.culling.isOcclusionQueryActivated)
			return;

		if (set.allowHzb)
		{
			//Debug and mipmap viwe count is same 
			srcType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP;
			destType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG;
			set.isNonLinearDepthMap = false;
		}
		else
		{
			//hd은 srcType == SCENE_LAYER_DEPTH_STENCIL이므로 ComputeCamDebug()에서 Compute
			return;
		}

		Graphic::JGraphicResourceUserInterface gRInterface;
		if (helper.cam != nullptr)
			gRInterface = helper.cam->GraphicResourceUserInterface();
		else if (helper.lit != nullptr)
			gRInterface = helper.lit->GraphicResourceUserInterface();
		else
			return;

		JDx12GraphicResourceComputeSetBuffer<1> srcBuff;
		JDx12GraphicResourceComputeSetBuffer<1> destBuff;
		set.SetOcclusionBuffer(context, &gRInterface, helper, srcType, destType, srcBuff, destBuff);
		//context->Transition(&srcBuff, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		//context->Transition(&destBuff, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

		const uint dataCount = gRInterface.GetDataCount(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG);
		for (uint i = 0; i < dataCount; ++i)
		{
			JDx12GraphicResourceComputeSet& srcSet = srcBuff(i);
			JDx12GraphicResourceComputeSet& destSet = destBuff(i);

			const uint viewCount = destSet.info->GetViewCount(J_GRAPHIC_BIND_TYPE::SRV);
			for (uint j = 0; j < viewCount; ++j)
			{
				srcSet.viewOffset = destSet.viewOffset = j;
				set.srcHandle = srcSet.GetGpuSrvHandle();
				set.destHandle = destSet.GetGpuUavHandle();
				set.size = occlusionSize;

				if (set.isNonLinearDepthMap)
					Execute(context, set, nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE].get());
				else
					Execute(context, set, linearDepthMapShaderData.get());
				//mipmap
				if (helper.option.culling.isOcclusionQueryActivated)
					occlusionSize /= 2.0f;
			}
			srcSet.viewOffset = destSet.viewOffset = 0;
		}

		//context->Transition(&destBuff, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//context->FlushResourceBarriers();
	}
	void JDx12GraphicDebug::Execute(JDx12CommandContext* context, const DebugDataSet& set, JDx12ComputeShaderDataHolder* holder)
	{
		context->SetComputeRootSignature(cRootSignature.Get());
		context->SetPipelineState(holder);
		context->SetComputeRootDescriptorTable(Private::srcTextureHandleIndex, set.srcHandle); 
		context->SetComputeRootDescriptorTable(Private::destTextureHandleIndex, set.destHandle);
		context->SetComputeRoot32BitConstants(Private::settingCbIndex, 0, set.size); 
		context->SetComputeRoot32BitConstants(Private::settingCbIndex, 2, set.nearFar); 
		context->Dispatch2D(set.size, holder->dispatchInfo.threadDim.XY());
	}
	void JDx12GraphicDebug::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)
	{
		ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
		BuildRootSignature(d3d12Device);
		BuildPso(d3d12Device); 
	}
	void JDx12GraphicDebug::BuildRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder2<Private::slotCount, 1> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//srcTextureHandleIndex 
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);		//destTextureHandleIndex
		builder.PushConstants(4, 0);	//settingCbIndex
		builder.PushSampler(D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressUVW
			0.0f,                               // mipLODBias
			16.0f,                                 // maxAnisotropy
			D3D12_COMPARISON_FUNC_LESS_EQUAL,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);
		builder.Create(device, L"Debug RootSignature", cRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
	}
	void JDx12GraphicDebug::BuildPso(ID3D12Device* device)
	{
 
		linearDepthMapShaderData = std::make_unique<JDx12ComputeShaderDataHolder>();
		nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE] = std::make_unique<JDx12ComputeShaderDataHolder>();
		nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::ORTHOLOGIC] = std::make_unique<JDx12ComputeShaderDataHolder>();
		
		albedoMapShaderData = std::make_unique<JDx12ComputeShaderDataHolder>();
		specularMapShaderData = std::make_unique<JDx12ComputeShaderDataHolder>();
		normalMapShaderData = std::make_unique<JDx12ComputeShaderDataHolder>();
		tangentMapShaderData = std::make_unique<JDx12ComputeShaderDataHolder>();
		velocityMapShaderData = std::make_unique<JDx12ComputeShaderDataHolder>();
		aoMapShaderData = std::make_unique<JDx12ComputeShaderDataHolder>();

		constexpr uint shaderCount = SIZE_OF_ARRAY(nonLinearDepthMapShaderData) + 7;

		JDx12ComputeShaderDataHolder* holderSet[shaderCount]
		{
			linearDepthMapShaderData.get(),
			nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE].get(),
			nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::ORTHOLOGIC].get(),
			albedoMapShaderData.get(),
			specularMapShaderData.get(),
			normalMapShaderData.get(),
			tangentMapShaderData.get(),
			velocityMapShaderData.get(),
			aoMapShaderData.get(),
		};
		JCompileInfo compileInfoSet[shaderCount]
		{ 
			JCompileInfo(ShaderRelativePath::Image(L"DebugVisualize.hlsl"), L"VisualizeLinearMap"),
			JCompileInfo(ShaderRelativePath::Image(L"DebugVisualize.hlsl"), L"VisualizeNonLinearMap"),
			JCompileInfo(ShaderRelativePath::Image(L"DebugVisualize.hlsl"), L"VisualizeNonLinearMap"),
			JCompileInfo(ShaderRelativePath::Image(L"DebugVisualize.hlsl"), L"VisualizeAlbedoMap"),
			JCompileInfo(ShaderRelativePath::Image(L"DebugVisualize.hlsl"), L"VisualizeSpecularMap"),
			JCompileInfo(ShaderRelativePath::Image(L"DebugVisualize.hlsl"), L"VisualizeNormalMap"),
			JCompileInfo(ShaderRelativePath::Image(L"DebugVisualize.hlsl"), L"VisualizeTangentMap"),
			JCompileInfo(ShaderRelativePath::Image(L"DebugVisualize.hlsl"), L"VisualizeVelocityMap"),
			JCompileInfo(ShaderRelativePath::Image(L"DebugVisualize.hlsl"), L"VisualizeAoMap")
		};
		std::vector<JMacroSet> macroSet[shaderCount]
		{ 
			std::vector<JMacroSet>{ { L"DEPTH_LINEAR_MAP", std::to_wstring(1) }, {L"USE_PERSPECTIVE", std::to_wstring(1)}},
			std::vector<JMacroSet>{ { L"DEPTH_NON_LINEAR_MAP", std::to_wstring(1) },{ L"USE_PERSPECTIVE", std::to_wstring(1)}},
			std::vector<JMacroSet>{ { L"DEPTH_NON_LINEAR_MAP", std::to_wstring(1) }},
			std::vector<JMacroSet>{ { L"ALBEDO_MAP", std::to_wstring(1) }},
			std::vector<JMacroSet>{ { L"SPECULAR_MAP", std::to_wstring(1) }},
			std::vector<JMacroSet>{ { L"NORMAL_MAP", std::to_wstring(1) }},
			std::vector<JMacroSet>{ { L"TANGENT_MAP", std::to_wstring(1) }},
			std::vector<JMacroSet>{ { L"VELOCITY_MAP", std::to_wstring(1) }},
			std::vector<JMacroSet>{ { L"SSAO_MAP", std::to_wstring(1) }},
		};
		JVector3<uint> threadDim[shaderCount]
		{
			Private::GetThreadDim(),
			Private::GetThreadDim(),
			Private::GetThreadDim(),
			Private::GetThreadDim(),
			Private::GetThreadDim(),
			Private::GetThreadDim(),
			Private::GetThreadDim(),
			Private::GetThreadDim(),
			Private::GetThreadDim()
		};
		ID3D12RootSignature* rootSignature[shaderCount]
		{
			cRootSignature.Get(),
			cRootSignature.Get(),
			cRootSignature.Get(),
			cRootSignature.Get(),
			cRootSignature.Get(),
			cRootSignature.Get(),
			cRootSignature.Get(),
			cRootSignature.Get(),
			cRootSignature.Get()
		};

		for (uint i = 0; i < shaderCount; ++i)
		{
			JDx12ComputeShaderDataHolder* holder = holderSet[i];
			JComputeShaderInitData initData;
			initData.dispatchInfo.threadDim = threadDim[i];
			initData.macro = macroSet[i];
			initData.PushThreadDimensionMacro();

			holder->cs = JDxShaderDataUtil::CompileShader(compileInfoSet[i].filePath, initData.macro, compileInfoSet[i].functionName, L"cs_6_6");
			holder->dispatchInfo = initData.dispatchInfo;

			JDx12ComputePso::Create(device, holder, rootSignature[i], D3D12_PIPELINE_STATE_FLAG_NONE);
		}
	} 
	void JDx12GraphicDebug::ClearResource()
	{
		linearDepthMapShaderData = nullptr;
		for (uint i = 0; i < (uint)J_GRAPHIC_PROJECTION_TYPE::COUNT; ++i)
			nonLinearDepthMapShaderData[i] = nullptr;
		specularMapShaderData = nullptr;
		normalMapShaderData = nullptr;
		tangentMapShaderData = nullptr;
		velocityMapShaderData = nullptr;
		aoMapShaderData = nullptr;
		cRootSignature = nullptr;
	}
}
