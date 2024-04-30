#include"JDx12CommandContext.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"
#include"../../GraphicResource/JGraphicResourceInterface.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceHolder.h"
#include"../../Culling/Dx/JDx12CullingManager.h"
#include"../../Culling/Dx/JDx12CullingResourceHolder.h"
#include"../../Shader/Dx/JDxShaderDataHolderInterface.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include"../../Accelerator/Dx/JDx12GpuAcceleratorHolder.h"
#include"../../Buffer/Dx/JHlslDebug.h"
#include"../../FrameResource/Dx/JDx12FrameResource.h"
#include"../../JGraphicInfo.h"
#include"../../JGraphicOption.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../../Core/Log/JLogMacro.h"

namespace JinEngine::Graphic
{ 
	namespace Private
	{
		static constexpr short convertValue[(uint)J_RENDER_PRIMITIVE::COUNT]
		{
			D3D_PRIMITIVE_TOPOLOGY_UNDEFINED, //J_RENDER_PRIMITIVE::INVALID,
			D3D_PRIMITIVE_TOPOLOGY_POINTLIST, //J_RENDER_PRIMITIVE::POINT,
			D3D_PRIMITIVE_TOPOLOGY_LINELIST, //J_RENDER_PRIMITIVE::LINE,
			D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, //J_RENDER_PRIMITIVE::LINE_STRIP,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, //J_RENDER_PRIMITIVE::TRIANGLE,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, //J_RENDER_PRIMITIVE::TRIANGLE_STRIP,
			D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ, //J_RENDER_PRIMITIVE::LINE_ADJACENCY,
			D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ, //J_RENDER_PRIMITIVE::LINE_STRIP_ADJACENCY,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ, //J_RENDER_PRIMITIVE::TRIANGLE_ADJACENCY,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ, //J_RENDER_PRIMITIVE::TRIANGLE_STRIP_ADJACENCY,
			D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_1,
			D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_2,
			D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_3,
			D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_4,
			D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_5,
			D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_6,
			D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_7,
			D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_8,
			D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_9,
			D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_10,
			D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_11,
			D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_12,
			D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_13,
			D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_14,
			D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_15,
			D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_16,
			D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_17,
			D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_18,
			D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_19,
			D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_20,
			D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_21,
			D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_22,
			D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_23,
			D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_24,
			D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_25,
			D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_26,
			D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_27,
			D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_28,
			D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_29,
			D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_30,
			D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_31,
			D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST, //J_RENDER_PRIMITIVE::CONTROL_POINT_PATCHLIST_32,
		};
	}

	JDx12CommandContext::JDx12CommandContext(const std::string& name, const bool isAlwausActivated)
		:JCommandContext(name, isAlwausActivated)
	{}
	JDx12CommandContext::~JDx12CommandContext()
	{ 
	}
	bool JDx12CommandContext::Begin()
	{
		if (JCommandContext::Begin())
		{
			barrierBufferCount = 0;
			return true;
		}
		else
			return false;
	}
	void JDx12CommandContext::End()
	{
		JCommandContext::End(); 
		cmd = nullptr;
		gm = nullptr;
		cm = nullptr;
		am = nullptr;
		frameResource = nullptr;
		info = nullptr;
		option = nullptr;
		raytracingCmd = nullptr;
	}
	void JDx12CommandContext::Initialize(ID3D12GraphicsCommandList* cmd,
		JDx12GraphicDevice* device,
		JDx12GraphicResourceManager* gm,
		JDx12CullingManager* cm,
		JDx12GpuAcceleratorManager* am,
		JDx12FrameResource* frameResource,
		const JGraphicInfo* info,
		const JGraphicOption* option,
		const bool settingRaycastDevice)
	{
		JDx12CommandContext::cmd = cmd;
		JDx12CommandContext::device = device;
		JDx12CommandContext::gm = gm;
		JDx12CommandContext::cm = cm;
		JDx12CommandContext::am = am;
		JDx12CommandContext::frameResource = frameResource;
		JDx12CommandContext::info = info;
		JDx12CommandContext::option = option;
		if (settingRaycastDevice && device->IsRaytracingSupported())
			cmd->QueryInterface(IID_PPV_ARGS(&raytracingCmd));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12CommandContext::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	} 
	uint JDx12CommandContext::GetDescriptorSize(const J_GRAPHIC_BIND_TYPE bType)const noexcept
	{
		return gm->GetDescriptorSize(bType);
	}
	ID3D12QueryHeap* JDx12CommandContext::GetQueryHeap(const uint index)const noexcept
	{
		return cm->GetQueryHeap(index);
	} 
	void JDx12CommandContext::SetPipelineState(JDx12RasterizeShaderDataHolderInterface* holder, const uint index)
	{
		cmd->SetPipelineState(holder->GetPso(index));
	} 
	JDx12GraphicResourceComputeSet JDx12CommandContext::ComputeSet(JGraphicResourceInfo* info)
	{
		return JDx12GraphicResourceComputeSet(gm, info);
	}
	JDx12GraphicResourceComputeSet JDx12CommandContext::ComputeSet(const JUserPtr<JGraphicResourceInfo>& info)
	{
		return JDx12GraphicResourceComputeSet(gm, info);
	}
	JDx12GraphicResourceComputeSet JDx12CommandContext::ComputeSet(const JGraphicResourceUserInterface& gInterface, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)
	{
		return JDx12GraphicResourceComputeSet(gm, gInterface, rType, taskType);
	}
	JDx12GraphicResourceComputeSet JDx12CommandContext::ComputeSet(const JGraphicResourceUserInterface& gInterface, const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)
	{
		return JDx12GraphicResourceComputeSet(gm, gInterface, rType, dataIndex);
	} 
	JDx12GraphicResourceComputeSet JDx12CommandContext::ComputeSet(JGraphicResourceInfo* info, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
	{
		return JDx12GraphicResourceComputeSet(gm, info, opType);
	}
	JDx12GraphicResourceComputeSet JDx12CommandContext::ComputeSet(const JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
	{
		return JDx12GraphicResourceComputeSet(gm, info, opType);
	}
	JDx12CullingResourceComputeSet JDx12CommandContext::ComputeSet(const JUserPtr<JCullingInfo>& info)
	{
		return JDx12CullingResourceComputeSet(cm, info);
	}
	JDx12CullingResourceComputeSet JDx12CommandContext::ComputeSet(const JCullingUserInterface& cInterface, const J_CULLING_TYPE cType, const J_CULLING_TARGET cTarget)
	{
		return JDx12CullingResourceComputeSet(cm, cInterface, cType, cTarget);
	}
	JDx12AcceleratorResourceComputeSet JDx12CommandContext::ComputeSet(const JUserPtr<JGpuAcceleratorInfo>& info)
	{
		return JDx12AcceleratorResourceComputeSet(am, info);
	}
	JDx12AcceleratorResourceComputeSet JDx12CommandContext::ComputeSet(const JGpuAcceleratorUserInterface& user)
	{
		return JDx12AcceleratorResourceComputeSet(am, user);
	}
	void JDx12CommandContext::Transition(JDx12GraphicResourceHolder* holder, const D3D12_RESOURCE_STATES newState, const bool flushImmediate)
	{ 
		if (holder == nullptr)
			return;

		const D3D12_RESOURCE_STATES oldState = holder->GetState();
		if (oldState != newState)
		{
			D3D12_RESOURCE_BARRIER& barrierDesc = resourceBarrierBuffer[barrierBufferCount++];
			barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrierDesc.Transition.pResource = holder->GetResource();
			barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrierDesc.Transition.StateBefore = oldState;
			barrierDesc.Transition.StateAfter = newState;

			holder->SetState(newState);
		}
		else if (newState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			InsertUAVBarrier(holder);

		if (flushImmediate || barrierBufferCount == 16)
			FlushResourceBarriers();
	}
	void JDx12CommandContext::Transition(JDx12GraphicResourceComputeSetBufferBase* base, const D3D12_RESOURCE_STATES newState, const bool flushImmediate)
	{
		if (base == nullptr)
			return;

		const uint validCount = base->GetValidCount();
		for (uint i = 0; i < validCount; ++i)
			Transition((*base)(i).holder, newState);

		if (flushImmediate || barrierBufferCount == 16)
			FlushResourceBarriers();
	}
	void JDx12CommandContext::InsertUAVBarrier(JDx12GraphicResourceHolder* holder, const bool flushImmediate)
	{
		D3D12_RESOURCE_BARRIER& barrierDesc = resourceBarrierBuffer[barrierBufferCount++];
		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrierDesc.UAV.pResource = holder->GetResource();

		if (flushImmediate)
			FlushResourceBarriers();
	}
	void JDx12CommandContext::InsertAliasBarrier(JDx12GraphicResourceHolder* before, JDx12GraphicResourceHolder* after, const bool flushImmediate)
	{
		D3D12_RESOURCE_BARRIER& barrierDesc = resourceBarrierBuffer[barrierBufferCount++];
		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
		barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrierDesc.Aliasing.pResourceBefore = before->GetResource();
		barrierDesc.Aliasing.pResourceAfter = after->GetResource();

		if (flushImmediate)
			FlushResourceBarriers();
	}
	void JDx12CommandContext::FlushResourceBarriers()
	{
		if (barrierBufferCount > 0)
		{
			cmd->ResourceBarrier(barrierBufferCount, resourceBarrierBuffer);
			barrierBufferCount = 0;
		}
	}
	void JDx12CommandContext::CopyResource(JDx12GraphicResourceHolder* from, JDx12GraphicResourceHolder* to)
	{
		Transition(from, D3D12_RESOURCE_STATE_COPY_SOURCE);
		Transition(to, D3D12_RESOURCE_STATE_COPY_DEST, true); 
		cmd->CopyResource(to->GetResource(), from->GetResource());
	}
	void JDx12CommandContext::CopyResource(JHlslDebugBase* base)
	{
		CopyResource(base->GetUABufferInterface()->GetHolder(), base->GetRBBufferInterface()->GetHolder());
	}
	void JDx12CommandContext::ResolveQueryData(ID3D12QueryHeap* heap, JDx12GraphicResourceHolder* holder, const D3D12_QUERY_TYPE type, const uint offset, const uint count, const uint elementSize)
	{
		Transition(holder, D3D12_RESOURCE_STATE_COPY_DEST);
		cmd->ResolveQueryData(heap, type, offset, count, holder->GetResource(), (size_t)offset * elementSize);
	}
	void JDx12CommandContext::ResolveOcclusionQueryData(const JDx12CullingResourceComputeSet& set, const uint offset, const uint count)
	{ 
		ID3D12QueryHeap* queryHeap = cm->GetQueryHeap(set.info->GetArrayIndex());
		if (queryHeap == nullptr)
			return;

		Transition(set.gHolder, D3D12_RESOURCE_STATE_COPY_DEST, true);
		cmd->ResolveQueryData(queryHeap, D3D12_QUERY_TYPE_BINARY_OCCLUSION, offset, count, set.resource, offset * sizeof(HardwareOcclusionResultType));
	} 
	void JDx12CommandContext::BeginQuery(ID3D12QueryHeap* heap, const D3D12_QUERY_TYPE type, const uint index)
	{
		cmd->BeginQuery(heap, type, index);
	}
	void JDx12CommandContext::BeginOcclusionQuery(const JDx12CullingResourceComputeSet& set, const uint index)
	{
		cmd->BeginQuery(cm->GetQueryHeap(set.info->GetArrayIndex()), D3D12_QUERY_TYPE_BINARY_OCCLUSION, index);
	}
	void JDx12CommandContext::EndQuery(ID3D12QueryHeap* heap, const D3D12_QUERY_TYPE type, const uint index)
	{
		cmd->EndQuery(heap, type, index);
	}
	void JDx12CommandContext::EndOcclusionQuery(const JDx12CullingResourceComputeSet& set, const uint index)
	{
		cmd->EndQuery(cm->GetQueryHeap(set.info->GetArrayIndex()), D3D12_QUERY_TYPE_BINARY_OCCLUSION, index);
	}
	void JDx12CommandContext::SetGraphicsRootSignature(ID3D12RootSignature* root)noexcept
	{
		cmd->SetGraphicsRootSignature(root);
	}
	void JDx12CommandContext::SetGraphicsRootDescriptorTable(const uint rootIndex, const J_GRAPHIC_RESOURCE_TYPE rType)
	{
		cmd->SetGraphicsRootDescriptorTable(rootIndex, gm->GetFirstGpuSrvDescriptorHandle(rType));
	}
	void JDx12CommandContext::SetGraphicsRootDescriptorTable(const uint rootIndex, const uint index)
	{
		cmd->SetGraphicsRootDescriptorTable(rootIndex, gm->GetGpuDsvDescriptorHandle(index));
	}
	void JDx12CommandContext::SetGraphicsRootDescriptorTable(const uint rootIndex, const CD3DX12_GPU_DESCRIPTOR_HANDLE handle)
	{
		cmd->SetGraphicsRootDescriptorTable(rootIndex, handle);
	}
	void JDx12CommandContext::SetGraphicsRootShaderResourceView(const uint rootIndex, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint addressOffset)
	{
		frameResource->GetDx12Buffer(type)->SetGraphicsRootShaderResourceView(cmd, rootIndex, addressOffset);
	}
	void JDx12CommandContext::SetGraphicsRootShaderResourceView(const uint rootIndex, JDx12GraphicBufferInterface* bufferInterface, const uint addressOffset)
	{
		bufferInterface->SetGraphicsRootShaderResourceView(cmd, rootIndex, addressOffset);
	}
	void JDx12CommandContext::SetGraphicsRootShaderResourceView(const uint rootIndex, const D3D12_GPU_VIRTUAL_ADDRESS address)
	{
		cmd->SetGraphicsRootShaderResourceView(rootIndex, address);
	}
	void JDx12CommandContext::SetGraphicsRootUnorderedAccessView(const uint rootIndex, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint addressOffset)
	{
		frameResource->GetDx12Buffer(type)->SetGraphicsRootUnorderedAccessView(cmd, rootIndex, addressOffset);
	}
	void JDx12CommandContext::SetGraphicsRootConstantBufferView(const uint rootIndex, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint addressOffset)
	{
		frameResource->GetDx12Buffer(type)->SetGraphicCBBufferView(cmd, rootIndex, addressOffset);
	}
	void JDx12CommandContext::SetGraphicsRootConstantBufferView(const uint rootIndex, JDx12GraphicBufferInterface* bufferInterface, const uint addressOffset)
	{
		bufferInterface->SetGraphicCBBufferView(cmd, rootIndex, addressOffset);
	}
	void JDx12CommandContext::SetGraphicsRootConstantBufferView(const uint rootIndex, const D3D12_GPU_VIRTUAL_ADDRESS address)
	{
		cmd->SetGraphicsRootConstantBufferView(rootIndex, address);
	}
	void JDx12CommandContext::SetDepthStencilView(const JDx12GraphicResourceComputeSet& dsSet)
	{
		auto handle = dsSet.GetCpuDsvHandle();
		cmd->OMSetRenderTargets(0, nullptr, false, &handle);
	}
	void JDx12CommandContext::SetRenderTargetView(const JDx12GraphicResourceComputeSet& rtSet, const uint rtCount, const bool isContinuous)
	{
		auto handle = rtSet.GetCpuRtvHandle();
		cmd->OMSetRenderTargets(rtCount, &handle, isContinuous, nullptr);
	}
	void JDx12CommandContext::SetRenderTargetView(const JDx12GraphicResourceComputeSet& rtSet, const JDx12GraphicResourceComputeSet& dsSet, const uint rtCount, const bool isContinuous)
	{
		auto rtHandle = rtSet.GetCpuRtvHandle();
		auto dsHandle = dsSet.GetCpuDsvHandle();
		cmd->OMSetRenderTargets(rtCount, &rtHandle, isContinuous, &dsHandle);
	}
	void JDx12CommandContext::SetViewportAndRect(const JVector2F& rtSize)
	{
		D3D12_VIEWPORT viewPort;
		D3D12_RECT rect;
		viewPort.TopLeftX = 0;
		viewPort.TopLeftY = 0;
		viewPort.Width = static_cast<float>(rtSize.x);
		viewPort.Height = static_cast<float>(rtSize.y);
		viewPort.MinDepth = 0.0f;
		viewPort.MaxDepth = 1.0f;
		rect = { 0, 0, (int)rtSize.x, (int)rtSize.y };

		cmd->RSSetViewports(1, &viewPort);
		cmd->RSSetScissorRects(1, &rect);
	}
	void JDx12CommandContext::SetViewport(const D3D12_VIEWPORT& viewPort)
	{
		cmd->RSSetViewports(1, &viewPort);
	}
	void JDx12CommandContext::SetScissorRect(const D3D12_RECT& rect)
	{
		cmd->RSSetScissorRects(1, &rect);
	}
	void JDx12CommandContext::SetMeshGeometryData(const JUserPtr<JRenderItem>& rItem, const uint slotCount)
	{
		JUserPtr<JMeshGeometry> mesh = rItem->GetMesh();
		const D3D12_VERTEX_BUFFER_VIEW vertexPtr = gm->VertexBufferView(mesh);
		const D3D12_INDEX_BUFFER_VIEW indexPtr = gm->IndexBufferView(mesh);

		cmd->IASetVertexBuffers(slotCount, 1, &vertexPtr);
		cmd->IASetIndexBuffer(&indexPtr);
		cmd->IASetPrimitiveTopology((D3D12_PRIMITIVE_TOPOLOGY)Private::convertValue[(uint)rItem->GetPrimitiveType()]);
	}
	void JDx12CommandContext::SetMeshGeometryData(const JUserPtr<JMeshGeometry>& mesh, const uint slotCount)
	{
		const D3D12_VERTEX_BUFFER_VIEW vertexPtr = gm->VertexBufferView(mesh);
		const D3D12_INDEX_BUFFER_VIEW indexPtr = gm->IndexBufferView(mesh);
		cmd->IASetVertexBuffers(slotCount, 1, &vertexPtr);
		cmd->IASetIndexBuffer(&indexPtr);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	void JDx12CommandContext::SetStencilRef(const uint value)
	{
		cmd->OMSetStencilRef(value);
	}
	void JDx12CommandContext::ClearDepthView(const JDx12GraphicResourceComputeSet& set, const float depthClearValue)
	{
		cmd->ClearDepthStencilView(set.GetCpuDsvHandle(), D3D12_CLEAR_FLAG_DEPTH, depthClearValue, 0, 0, nullptr);
	}
	void JDx12CommandContext::ClearDepthStencilView(const JDx12GraphicResourceComputeSet& set, const float depthClearValue , const uint stencilClearValue)
	{
		cmd->ClearDepthStencilView(set.GetCpuDsvHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depthClearValue, stencilClearValue, 0, nullptr);
	}
	void JDx12CommandContext::ClearRenderTargetView(const JDx12GraphicResourceComputeSet& set, const DirectX::XMVECTORF32 clearColor)
	{
		cmd->ClearRenderTargetView(set.GetCpuRtvHandle(), clearColor, 0, nullptr);
	} 
	void JDx12CommandContext::DrawInstanced(const JUserPtr<JMeshGeometry>& mesh)
	{
		cmd->DrawInstanced(3, 1, 0, 0);
	}
	void JDx12CommandContext::DrawFullScreenTriangle()
	{
		cmd->IASetVertexBuffers(0, 0, nullptr);
		cmd->IASetIndexBuffer(nullptr);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	 
		cmd->DrawInstanced(3, 1, 0, 0);
	} 
	void JDx12CommandContext::DrawIndexedInstanced(const JUserPtr<JMeshGeometry>& mesh, 
		const uint subMeshIndex,
		const uint instanceCount,
		const uint startInstanceLocation)
	{ 
		cmd->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(subMeshIndex), instanceCount, mesh->GetSubmeshStartIndexLocation(subMeshIndex), mesh->GetSubmeshBaseVertexLocation(subMeshIndex), startInstanceLocation);
	}
	void JDx12CommandContext::DrawIndexedInstanced(const JUserPtr<JMeshGeometry>& mesh,
		JDx12RasterizeShaderDataHolderInterface* holder,
		const J_GRAPHIC_SHADER_EXTRA_FUNCTION extraType,
		const uint instanceCount,
		const uint startInstanceLocation)
	{
		cmd->SetPipelineState(holder->GetPso((uint)extraType));
		cmd->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(0), instanceCount, mesh->GetSubmeshStartIndexLocation(0), mesh->GetSubmeshBaseVertexLocation(0), startInstanceLocation);
	}
	void JDx12CommandContext::SetComputeRootSignature(ID3D12RootSignature* root)noexcept
	{
		cmd->SetComputeRootSignature(root);
	}
	void JDx12CommandContext::SetComputeRootDescriptorTable(const uint rootIndex, const J_GRAPHIC_RESOURCE_TYPE rType)
	{
		cmd->SetComputeRootDescriptorTable(rootIndex, gm->GetFirstGpuSrvDescriptorHandle(rType));
	}
	void JDx12CommandContext::SetComputeRootDescriptorTable(const uint rootIndex, const uint index)
	{
		cmd->SetComputeRootDescriptorTable(rootIndex, gm->GetGpuSrvDescriptorHandle(index));
	}
	void JDx12CommandContext::SetComputeRootDescriptorTable(const uint rootIndex, const CD3DX12_GPU_DESCRIPTOR_HANDLE handle)
	{
		cmd->SetComputeRootDescriptorTable(rootIndex, handle);
	}
	void JDx12CommandContext::SetComputeRootShaderResourceView(const uint rootIndex, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint addressOffset)
	{
		frameResource->GetDx12Buffer(type)->SetComputeRootShaderResourceView(cmd, rootIndex, addressOffset);
	}
	void JDx12CommandContext::SetComputeRootShaderResourceView(const uint rootIndex, JDx12GraphicBufferInterface* bufferInterface, const uint addressOffset)
	{
		bufferInterface->SetComputeRootShaderResourceView(cmd, rootIndex, addressOffset);
	}
	void JDx12CommandContext::SetComputeRootShaderResourceView(const uint rootIndex, const D3D12_GPU_VIRTUAL_ADDRESS address)
	{
		cmd->SetComputeRootShaderResourceView(rootIndex, address);
	}
	void JDx12CommandContext::SetComputeRootUnorderedAccessView(const uint rootIndex, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint addressOffset)
	{
		frameResource->GetDx12Buffer(type)->SetComputeRootUnorderedAccessView(cmd, rootIndex, addressOffset);
	}
	void JDx12CommandContext::SetComputeRootUnorderedAccessView(const uint rootIndex, JDx12GraphicBufferInterface* bufferInterface, const uint addressOffset)
	{
		bufferInterface->SetComputeRootUnorderedAccessView(cmd, rootIndex, addressOffset);
	}
	void JDx12CommandContext::SetComputeRootUnorderedAccessView(const uint rootIndex, const D3D12_GPU_VIRTUAL_ADDRESS address)
	{
		cmd->SetComputeRootUnorderedAccessView(rootIndex, address);
	}
	void JDx12CommandContext::SetComputeRootConstantBufferView(const uint rootIndex, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint addressOffset)
	{
		frameResource->GetDx12Buffer(type)->SetComputeCBBufferView(cmd, rootIndex, addressOffset);
	}
	void JDx12CommandContext::SetComputeRootConstantBufferView(const uint rootIndex, JDx12GraphicBufferInterface* bufferInterface, const uint addressOffset)
	{
		bufferInterface->SetComputeCBBufferView(cmd, rootIndex, addressOffset);
	}
	void JDx12CommandContext::SetComputeRootConstantBufferView(const uint rootIndex, const D3D12_GPU_VIRTUAL_ADDRESS address)
	{
		cmd->SetComputeRootConstantBufferView(rootIndex, address);
	}
	void JDx12CommandContext::Dispatch1D(const uint taskCount, const uint groupPerThread)
	{
		cmd->Dispatch(JMathHelper::DivideByMultiple(taskCount, groupPerThread), 1, 1);
	} 
	void JDx12CommandContext::Dispatch2D(const JVector2<uint>& taskCount, const JVector2<uint>& groupPerThread)
	{
		cmd->Dispatch(JMathHelper::DivideByMultiple(taskCount.x, groupPerThread.x), 
			JMathHelper::DivideByMultiple(taskCount.y, groupPerThread.y),
			1);
	}
	void JDx12CommandContext::Dispatch(const JVector3<uint>& taskCount, const JVector3<uint>& groupPerThread)
	{
		cmd->Dispatch(JMathHelper::DivideByMultiple(taskCount.x, groupPerThread.x),
			JMathHelper::DivideByMultiple(taskCount.y, groupPerThread.y),
			JMathHelper::DivideByMultiple(taskCount.z, groupPerThread.z));
	}
	void JDx12CommandContext::Dispatch(const uint groupCountX, const uint groupCountY, const uint groupCountZ)
	{
		cmd->Dispatch(groupCountX, groupCountY, groupCountZ);
	}
	void JDx12CommandContext::Dispatch(const JVector3<uint>& groupCountV)
	{
		cmd->Dispatch(groupCountV.x, groupCountV.y, groupCountV.z);
	}
	void JDx12CommandContext::DispatchIndirect(ID3D12CommandSignature* commandSig,
		const JDx12GraphicResourceComputeSet& argSet,
		const uint argStartOffset,
		const uint maxCommand)
	{
		cmd->ExecuteIndirect(commandSig, maxCommand, argSet.resource, argStartOffset, nullptr, 0);
	}
	void JDx12CommandContext::DispatchIndirect(ID3D12CommandSignature* commandSig,
		const JDx12GraphicResourceComputeSet& argSet,
		const JDx12GraphicResourceComputeSet& counterSet,
		const uint maxCommand,
		const uint argStartOffset,
		const uint counterOffset)
	{
		cmd->ExecuteIndirect(commandSig, maxCommand, argSet.resource, argStartOffset, counterSet.resource, counterOffset);
	}
	void JDx12CommandContext::SetTlasView(const uint rootIndex, const JDx12AcceleratorResourceComputeSet& set)
	{ 
		raytracingCmd->SetComputeRootShaderResourceView(rootIndex, set.holder->GetTlasGpuAddress());
	}
	void JDx12CommandContext::SetPipelineState(ID3D12StateObject* stateObject)
	{
		raytracingCmd->SetPipelineState1(stateObject);
	}
	void JDx12CommandContext::DispatchRays(D3D12_DISPATCH_RAYS_DESC* desc)
	{
		raytracingCmd->DispatchRays(desc);
	}
}