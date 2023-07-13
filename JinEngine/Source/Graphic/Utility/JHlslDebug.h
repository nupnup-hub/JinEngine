#pragma once
#include"../Upload/JUploadBuffer.h"
#include<memory>
namespace JinEngine
{
	namespace Graphic
	{
		template<typename T>
		class JHlslDebug
		{
		private:
			std::unique_ptr<JUploadBuffer<T>> debugResult;
			int rootParamIndex;
		private:
			T* mappedData;
		public:
			JHlslDebug(const int rootParamIndex)
				:rootParamIndex(rootParamIndex)
			{}
			~JHlslDebug()
			{
				Clear();
			}
		public:
			void Build(ID3D12Device* device, int capacity)
			{ 
				debugResult = std::make_unique<JUploadBuffer<T>>(L"HlslDebug", J_UPLOAD_BUFFER_TYPE::READ_BACK);
				debugResult->Build(device, capacity);
			}
			void Clear()
			{ 
				if (mappedData != nullptr)
					UnMap();
				debugResult.reset();
			}
		public:
			void SetGraphicShader(ID3D12GraphicsCommandList* commandList)
			{
				commandList->SetGraphicsRootUnorderedAccessView(rootParamIndex, debugResult->Resource()->GetGPUVirtualAddress());
			}
			void SetComputeShader(ID3D12GraphicsCommandList* commandList)
			{
				commandList->SetComputeRootUnorderedAccessView(rootParamIndex, debugResult->Resource()->GetGPUVirtualAddress());
			}
		public:
			T* Map(_Out_ int& count)
			{
				ThrowIfFailedHr(debugResult->Resource()->Map(0, nullptr, reinterpret_cast<void**>(&mappedData)));
				count = debugResult->ElementCount();
				return mappedData;
			}
			void UnMap()
			{
				debugResult->Resource()->Unmap(0, nullptr);
				mappedData = nullptr;
			}
		};
	}
}