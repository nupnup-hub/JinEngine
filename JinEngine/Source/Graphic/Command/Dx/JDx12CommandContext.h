#pragma once
#include"../JCommandContext.h"
#include"../../FrameResource/JFrameResourceEnum.h"
#include"../../GraphicResource/JGraphicResourceType.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../Culling/JCullingType.h"
#include"../../Culling/Dx/JDx12CullingManager.h"
#include"../../Accelerator/Dx/JDx12GpuAcceleratorManager.h"
#include"../../Accelerator/JGpuAcceleratorInterface.h"
#include"../../Shader/JShaderType.h"
#include"../../JGraphicConstants.h" 
#include"../../../Core/Utility/JTypeTraitUtility.h"
#include"../../../Core/Math/JMathHelper.h"
#include<d3dx12.h>

namespace JinEngine
{
	class JMeshGeometry;
	class JRenderItem;
	namespace Graphic
	{  
		class JDx12GraphicDevice; 
		class JDx12GraphicResourceInfo;
		class JDx12GraphicResourceHolder;
		class JDx12RasterizeShaderDataHolderInterface;
		class JDx12FrameResource;
		class JDx12GraphicBufferInterface; 
		class JHlslDebugBase;

		struct JGraphicInfo;
		struct JGraphicOption;
		 
		class JDx12CommandContext final: public JCommandContext
		{  
		private:
			static constexpr uint maxBarrierBufferCount = 16; 
		private:
			ID3D12GraphicsCommandList* cmd = nullptr;
			JDx12GraphicDevice* device = nullptr;
			JDx12GraphicResourceManager* gm = nullptr;
			JDx12CullingManager* cm = nullptr;
			JDx12GpuAcceleratorManager* am = nullptr;
			JDx12FrameResource* frameResource = nullptr;
		private:
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> raytracingCmd;
		private:
			const JGraphicInfo* info = nullptr;
			const JGraphicOption* option = nullptr;
		private:
			D3D12_RESOURCE_BARRIER resourceBarrierBuffer[maxBarrierBufferCount];
			uint barrierBufferCount = 0;
		public:
			JDx12CommandContext(const std::string& name, const bool isAlwausActivated = false);
			JDx12CommandContext(const JDx12CommandContext& rhs) = delete;
			JDx12CommandContext(JDx12CommandContext&& rhs) = default;
			JDx12CommandContext& operator=(const JDx12CommandContext& rhs) = delete;
			JDx12CommandContext& operator=(JDx12CommandContext&& rhs) = default;
			~JDx12CommandContext();
		public:
			bool Begin()override;
			void End()override;
			void Initialize(ID3D12GraphicsCommandList* cmd,
				JDx12GraphicDevice* device,
				JDx12GraphicResourceManager* gm,
				JDx12CullingManager* cm,
				JDx12GpuAcceleratorManager* am,
				JDx12FrameResource* frameResource,
				const JGraphicInfo* info,
				const JGraphicOption* option,
				const bool settingRaycastDevice = false);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			uint GetDescriptorSize(const J_GRAPHIC_BIND_TYPE bType)const noexcept; 
			ID3D12QueryHeap* GetQueryHeap(const uint index)const noexcept; 
		public:
			void SetPipelineState(JDx12RasterizeShaderDataHolderInterface* holder, const uint index = 0);
		public: 
			JDx12GraphicResourceComputeSet ComputeSet(JGraphicResourceInfo* info);
			JDx12GraphicResourceComputeSet ComputeSet(const JUserPtr<JGraphicResourceInfo>& info);
			JDx12GraphicResourceComputeSet ComputeSet(const JGraphicResourceUserInterface& gInterface, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType);
			JDx12GraphicResourceComputeSet ComputeSet(const JGraphicResourceUserInterface& gInterface, const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex);
			JDx12GraphicResourceComputeSet ComputeSet(JGraphicResourceInfo* info, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType);
			JDx12GraphicResourceComputeSet ComputeSet(const JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType);
		public:
			JDx12CullingResourceComputeSet ComputeSet(const JUserPtr<JCullingInfo>& info);
			JDx12CullingResourceComputeSet ComputeSet(const JCullingUserInterface& cInterface, const J_CULLING_TYPE cType, const J_CULLING_TARGET cTarget);
		public:
			JDx12AcceleratorResourceComputeSet ComputeSet(const JUserPtr<JGpuAcceleratorInfo>& info);
			JDx12AcceleratorResourceComputeSet ComputeSet(const JGpuAcceleratorUserInterface& user);
		public:
			//Common 
			void Transition(JDx12GraphicResourceHolder* holder, const D3D12_RESOURCE_STATES newState, const bool flushImmediate = false);
			void Transition(JDx12GraphicResourceComputeSetBufferBase* base, const D3D12_RESOURCE_STATES newState, const bool flushImmediate = false);
			template<size_t ...Is>
			void Transition(JDx12GraphicResourceHolder* holder, const D3D12_RESOURCE_STATES newState, const bool flushImmediate, std::index_sequence<Is...>)
			{
				((Transition(holder[Is], newState, false)), ...);

				if (flushImmediate || barrierBufferCount == 16)
					FlushResourceBarriers();
			}
			template<size_t ...Is>
			void Transition(JDx12GraphicResourceComputeSet* computeSet, const D3D12_RESOURCE_STATES newState, const bool flushImmediate, std::index_sequence<Is...>)
			{
				((Transition(computeSet[Is].holder, newState, false)), ...);

				if (flushImmediate || barrierBufferCount == 16)
					FlushResourceBarriers();
			}
			void InsertUAVBarrier(JDx12GraphicResourceHolder* holder, const bool flushImmediate = false);
			void InsertAliasBarrier(JDx12GraphicResourceHolder* before, JDx12GraphicResourceHolder* after, const bool flushImmediate = false);
			void FlushResourceBarriers(); 
		public:
			void CopyResource(JDx12GraphicResourceHolder* from, JDx12GraphicResourceHolder* to);
			void CopyResource(JHlslDebugBase* base);
			template<size_t ...Is>
			void CopyResource(JDx12GraphicResourceComputeSet* from, JDx12GraphicResourceComputeSet* to, std::index_sequence<Is...>)
			{ 
				((Transition(from[Is].holder, D3D12_RESOURCE_STATE_COPY_SOURCE, false)), ...);
				((Transition(to[Is].holder, D3D12_RESOURCE_STATE_COPY_DEST, false)), ...);
				FlushResourceBarriers();
				 
				((cmd->CopyResource(to[Is].holder->GetResource(), from[Is].holder->GetResource())), ...);
			}
			template<size_t ...Is>
			void CopyResource(JDx12GraphicResourceComputeSet** from, JDx12GraphicResourceComputeSet** to, std::index_sequence<Is...>)
			{
				((Transition((*from[Is]).holder, D3D12_RESOURCE_STATE_COPY_SOURCE, false)), ...);
				((Transition((*to[Is]).holder, D3D12_RESOURCE_STATE_COPY_DEST, false)), ...);
				FlushResourceBarriers();

				((cmd->CopyResource((*to[Is]).holder->GetResource(), (*from[Is]).holder->GetResource())), ...); 
			}
		public:
			void ResolveQueryData(ID3D12QueryHeap* heap, JDx12GraphicResourceHolder* holder, const D3D12_QUERY_TYPE type, const uint offset, const uint count, const uint elementSize);
			void ResolveOcclusionQueryData(const JDx12CullingResourceComputeSet& set, const uint offset, const uint count);
			void BeginQuery(ID3D12QueryHeap* heap, const D3D12_QUERY_TYPE type, const uint index);
			void BeginOcclusionQuery(const JDx12CullingResourceComputeSet& set, const uint index);
			void EndQuery(ID3D12QueryHeap* heap, const D3D12_QUERY_TYPE type, const uint index);
			void EndOcclusionQuery(const JDx12CullingResourceComputeSet& set, const uint index);
		public:
			//Graphic
			void SetGraphicsRootSignature(ID3D12RootSignature* root)noexcept;
			//Bind all type resource by srv
			void SetGraphicsRootDescriptorTable(const uint rootIndex, const J_GRAPHIC_RESOURCE_TYPE rType);
			void SetGraphicsRootDescriptorTable(const uint rootIndex, const uint index);
			void SetGraphicsRootDescriptorTable(const uint rootIndex, const CD3DX12_GPU_DESCRIPTOR_HANDLE handle);
			void SetGraphicsRootShaderResourceView(const uint rootIndex, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint addressOffset = 0);
			void SetGraphicsRootShaderResourceView(const uint rootIndex, JDx12GraphicBufferInterface* bufferInterface, const uint addressOffset = 0);
			void SetGraphicsRootShaderResourceView(const uint rootIndex, const D3D12_GPU_VIRTUAL_ADDRESS address);
			void SetGraphicsRootUnorderedAccessView(const uint rootIndex, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint addressOffset = 0);
			void SetGraphicsRootConstantBufferView(const uint rootIndex, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint addressOffset);
			void SetGraphicsRootConstantBufferView(const uint rootIndex, JDx12GraphicBufferInterface* bufferInterface, const uint addressOffset);
			void SetGraphicsRootConstantBufferView(const uint rootIndex, const D3D12_GPU_VIRTUAL_ADDRESS address);
			template<typename T>
			void SetGraphicsRoot32BitConstants(const uint rootIndex, const uint offset, const T& data)
			{
				if constexpr (Core::JVectorDetermine<T>::value)
					cmd->SetGraphicsRoot32BitConstants(rootIndex, data.GetDigitCount(), &data, offset);
				else
					cmd->SetGraphicsRoot32BitConstants(rootIndex, 1, &data, offset);
			}
		public:
			void SetDepthStencilView(const JDx12GraphicResourceComputeSet& dsSet);
			void SetRenderTargetView(const JDx12GraphicResourceComputeSet& rtSet, const uint rtCount = 1, const bool isContinuous =  false);
			void SetRenderTargetView(const JDx12GraphicResourceComputeSet& rtSet, const JDx12GraphicResourceComputeSet& dsSet, const uint rtCount = 1, const bool isContinuous = false);
			void SetViewportAndRect(const JVector2F& rtSize);
			void SetViewport(const D3D12_VIEWPORT& viewPort);
			void SetScissorRect(const D3D12_RECT& rect);
			void SetMeshGeometryData(const JUserPtr<JRenderItem>& rItem, const uint slotCount = 0);
			void SetMeshGeometryData(const JUserPtr<JMeshGeometry>& mesh, const uint slotCount = 0);
			void SetStencilRef(const uint value);
		public:
			void ClearDepthView(const JDx12GraphicResourceComputeSet& set, const float depthClearValue = 1.0f);
			void ClearDepthStencilView(const JDx12GraphicResourceComputeSet& set, const float depthClearValue = 1.0f, const uint stencilClearValue = Constants::stencilclearValue);
			void ClearRenderTargetView(const JDx12GraphicResourceComputeSet& set, const DirectX::XMVECTORF32 clearColor);
		public: 
			void DrawInstanced(const JUserPtr<JMeshGeometry>& mesh);
			/**
			* @brief bind triangle geometry data and draw
			*/
			void DrawFullScreenTriangle();  
			void DrawIndexedInstanced(const JUserPtr<JMeshGeometry>& mesh, 
				const uint subMeshIndex = 0,
				const uint instanceCount = 1, 
				const uint startInstanceLocation = 0);
			void DrawIndexedInstanced(const JUserPtr<JMeshGeometry>& mesh, 
				JDx12RasterizeShaderDataHolderInterface* holder,
				const J_GRAPHIC_SHADER_EXTRA_FUNCTION extraType,
				const uint instanceCount = 1,
				const uint startInstanceLocation = 0);
		public:
			//Compute
			void SetComputeRootSignature(ID3D12RootSignature* root)noexcept;
			//Bind all type resource by srv
			void SetComputeRootDescriptorTable(const uint rootIndex, const J_GRAPHIC_RESOURCE_TYPE rType); 
			void SetComputeRootDescriptorTable(const uint rootIndex, const uint index);
			void SetComputeRootDescriptorTable(const uint rootIndex, const CD3DX12_GPU_DESCRIPTOR_HANDLE handle);
			void SetComputeRootShaderResourceView(const uint rootIndex, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint addressOffset = 0);
			void SetComputeRootShaderResourceView(const uint rootIndex, JDx12GraphicBufferInterface* bufferInterface, const uint addressOffset = 0);
			void SetComputeRootShaderResourceView(const uint rootIndex, const D3D12_GPU_VIRTUAL_ADDRESS address);
			void SetComputeRootUnorderedAccessView(const uint rootIndex, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint addressOffset = 0);
			void SetComputeRootUnorderedAccessView(const uint rootIndex, JDx12GraphicBufferInterface* bufferInterface, const uint addressOffset = 0);
			void SetComputeRootUnorderedAccessView(const uint rootIndex, const D3D12_GPU_VIRTUAL_ADDRESS address);
			void SetComputeRootConstantBufferView(const uint rootIndex, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint addressOffset);
			void SetComputeRootConstantBufferView(const uint rootIndex, JDx12GraphicBufferInterface* bufferInterface, const uint addressOffset);
			void SetComputeRootConstantBufferView(const uint rootIndex, const D3D12_GPU_VIRTUAL_ADDRESS address);
			template<typename T>
			void SetComputeRoot32BitConstants(const uint rootIndex, const uint offset, const T& data)
			{
				if constexpr (Core::JVectorDetermine<T>::value)
					cmd->SetComputeRoot32BitConstants(rootIndex, data.GetDigitCount(), &data, offset);
				else if constexpr (Core::IsNumber_V<T>)
					cmd->SetComputeRoot32BitConstants(rootIndex, 1, &data, offset);
				else
					cmd->SetComputeRoot32BitConstants(rootIndex, JMathHelper::DivideByMultiple(sizeof(T), 4), &data, offset);
			}
			template<typename T>
			void SetComputeRoot32BitConstants(const uint rootIndex, const uint offset, const T& data, const size_t count)
			{
				cmd->SetComputeRoot32BitConstants(rootIndex, count, &data, offset);
			}
		public:
			void Dispatch1D(const uint taskCount, const uint groupPerThread);
			void Dispatch2D(const JVector2<uint>& taskCount, const JVector2<uint>& groupPerThread);
			void Dispatch(const JVector3<uint>& taskCount, const JVector3<uint>& groupPerThread);
			void Dispatch(const uint groupCountX, const uint groupCountY, const uint groupCountZ);
			void Dispatch(const JVector3<uint>& groupCountV);
			void DispatchIndirect(ID3D12CommandSignature* commandSig, 
				const JDx12GraphicResourceComputeSet& argSet, 
				const uint argStartOffset = 0,
				const uint maxCommand = 1);
			void DispatchIndirect(ID3D12CommandSignature* commandSig,
				const JDx12GraphicResourceComputeSet& argSet,
				const JDx12GraphicResourceComputeSet& counterSet,
				const uint argStartOffset = 0,
				const uint counterOffset = 0,
				const uint maxCommand = 1);
		public:
			//Raycast  
			void SetTlasView(const uint rootIndex, const JDx12AcceleratorResourceComputeSet& set);
			void SetPipelineState(ID3D12StateObject* stateObject);
			void DispatchRays(D3D12_DISPATCH_RAYS_DESC* desc);
		public: 
			template<typename ParamComputeSet, typename FuncComputeSet, typename ...Param>
			void ApplyOne(ParamComputeSet computeSet,
				void(JDx12CommandContext::* funcPtr)(FuncComputeSet, Param...),
				Param... param)
			{
				if constexpr (std::is_same_v<std::remove_cv<ParamComputeSet>, std::remove_cv<FuncComputeSet>>)
					(this->*funcPtr)(computeSet, param...);
				else
				{
					if constexpr (std::is_pointer_v<ParamComputeSet>)
						(this->*funcPtr)(*computeSet, param...);
					else if constexpr (std::is_pointer_v<FuncComputeSet>)
						(this->*funcPtr)(&computeSet, param...);
				}		 
			}
			template<typename ParamComputeSetPointer, typename FuncComputeSet, size_t ...Is, typename ...Param>
			void Apply(ParamComputeSetPointer computeSet,
				void(JDx12CommandContext::* funcPtr)(FuncComputeSet, Param...),
				std::index_sequence<Is...>,
				Param... param)
			{
				if constexpr(std::is_pointer_v<ParamComputeSetPointer>)
					((ApplyOne(&computeSet[Is], funcPtr, param...)), ...); 
			}
		};
	}
}