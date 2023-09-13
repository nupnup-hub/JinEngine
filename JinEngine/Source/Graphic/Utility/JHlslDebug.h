#pragma once
#include"../Buffer/JBufferType.h"
#include"../Buffer/Dx/JDx12GraphicBuffer.h"
#include<memory>

namespace JinEngine
{
	namespace Graphic
	{
		template<typename T>
		class JHlslDebug
		{
		private:
			const std::wstring name;
		private:
			std::unique_ptr<JDx12GraphicBuffer<T>> uaResult;
			std::unique_ptr<JDx12GraphicBuffer<T>> rbResult;
			int rootParamIndex;
		private:
			T* mappedData;
		public:
			JHlslDebug(const std::wstring name, const int rootParamIndex)
				:name(name), rootParamIndex(rootParamIndex)
			{}
			~JHlslDebug()
			{
				Clear();
			}
		public:
			void Build(JGraphicDevice* device, int capacity)
			{ 
				uaResult = std::make_unique<JDx12GraphicBuffer<T>>((name + L"_HlslDebug").c_str(), J_GRAPHIC_BUFFER_TYPE::UNORDERED_ACCEESS);
				rbResult = std::make_unique<JDx12GraphicBuffer<T>>((name + L"_HlslDebug").c_str(), J_GRAPHIC_BUFFER_TYPE::READ_BACK);
				
				uaResult->Build(device, capacity);
				rbResult->Build(device, capacity);
			}
			void Clear()
			{ 
				if (mappedData != nullptr)
					UnMap();
				uaResult.reset();
				rbResult.reset();
			}
		public:
			void SettingGraphic(ID3D12GraphicsCommandList* commandList)
			{
				JD3DUtility::ResourceTransition(commandList, uaResult->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				commandList->SetGraphicsRootUnorderedAccessView(rootParamIndex, uaResult->GetResource()->GetGPUVirtualAddress());
			}
			void SettingCompute(ID3D12GraphicsCommandList* commandList)
			{
				JD3DUtility::ResourceTransition(commandList, uaResult->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				commandList->SetComputeRootUnorderedAccessView(rootParamIndex, uaResult->GetResource()->GetGPUVirtualAddress());
			}
			void End(ID3D12GraphicsCommandList* commandList)
			{
				CopyDebugResult(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON); 
			}
		public:
			void CopyDebugResult(ID3D12GraphicsCommandList* commandList, const D3D12_RESOURCE_STATES beforeState, const D3D12_RESOURCE_STATES afterState)
			{
				JD3DUtility::ResourceTransition(commandList, uaResult->GetResource(), beforeState, D3D12_RESOURCE_STATE_COPY_SOURCE);
				commandList->CopyResource(rbResult->GetResource(), uaResult->GetResource());
				JD3DUtility::ResourceTransition(commandList, uaResult->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, afterState);
			}
		public:
			T* Map(_Out_ int& count)
			{
				if(mappedData == nullptr)
					ThrowIfFailedHr(rbResult->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappedData)));
				count = rbResult->GetElementCount();
				return mappedData;
			}
			void UnMap()
			{
				rbResult->GetResource()->Unmap(0, nullptr);
				mappedData = nullptr;
			}
		};
	}
}