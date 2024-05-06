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
#include"JDxGraphicBufferInterface.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceHolder.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceCreation.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../Utility/Dx/JDx12Utility.h"   
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include<d3d12.h>
#include<wrl/client.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12GraphicBufferInterface : public JDxGraphicBufferInterface
		{
		public:
			virtual JDx12GraphicResourceHolder* GetHolder()noexcept = 0;
			virtual ID3D12Resource* GetResource()const noexcept = 0;
		public:
			virtual void SetGraphicCBBufferView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset) = 0;
			virtual void SetComputeCBBufferView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset) = 0;
			virtual void SetGraphicsRootShaderResourceView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset = 0) = 0;
			virtual void SetComputeRootShaderResourceView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset = 0) = 0;
			virtual void SetGraphicsRootUnorderedAccessView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset = 0) = 0;
			virtual void SetComputeRootUnorderedAccessView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset = 0) = 0;
		};
		 
		class JDx12GraphicBuffer : public JDx12GraphicBufferInterface
		{
		private:
			bool canMapped = false;
			bool useFixedInitState = false;
		private:
			std::wstring name;
		private:
			JDx12GraphicResourceHolder holder;
			BYTE* mappedData = nullptr;
			uint elementcount = 0;
			uint elementByteSize = 0;
			J_GRAPHIC_BUFFER_TYPE type;
			D3D12_RESOURCE_STATES fixedInitState = (D3D12_RESOURCE_STATES)invalidIndex;
		private:
			uint originalElementSize = 0;
		public:
			JDx12GraphicBuffer(const std::wstring& name = L"", const J_GRAPHIC_BUFFER_TYPE type = J_GRAPHIC_BUFFER_TYPE::COMMON, const size_t elementByteSize = 1);
			JDx12GraphicBuffer(const JDx12GraphicBuffer& rhs) = delete;
			JDx12GraphicBuffer& operator=(const JDx12GraphicBuffer& rhs) = delete;
			JDx12GraphicBuffer(JDx12GraphicBuffer&& rhs) = default;
			JDx12GraphicBuffer& operator=(JDx12GraphicBuffer&& rhs) = default;
			~JDx12GraphicBuffer();
		public:
			void Build(JGraphicDevice* device, const uint newElementcount) final;
			void Build(ID3D12Device* device, const uint newElementcount);
			void Clear()noexcept final;
			void StuffValue(ID3D12GraphicsCommandList* cmdList,
				ID3D12Resource* uploadBuffer,
				const D3D12_RESOURCE_STATES beforeState,
				const D3D12_RESOURCE_STATES afterState,
				const void* data);
			template<typename T>
			void SutffClearValue(ID3D12GraphicsCommandList* cmdList,
				ID3D12Resource* uploadBuffer,
				const D3D12_RESOURCE_STATES beforeState,
				const D3D12_RESOURCE_STATES afterState,
				const T clearValue)
			{ 
				if (sizeof(T) != elementByteSize || type != J_GRAPHIC_BUFFER_TYPE::READ_BACK || type != J_GRAPHIC_BUFFER_TYPE::COMMON)
					return;

				JDx12GraphicResourceCreation::UploadClearData(cmdList, GetResource(), uploadBuffer, beforeState, afterState, clearValue, GetElementCount(), GetElementByteSize());
			}
		private:
			void BuildReadBack(ID3D12Device* device, D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON);
			void BuildUav(ID3D12Device* device, D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON);
			void BuildOccPredict(ID3D12Device* device);
			void BuildUpload(ID3D12Device* device);
			void BuildCommon(ID3D12Device* device, D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON);
			void BuildHolder(Microsoft::WRL::ComPtr<ID3D12Resource>&& buffer, const D3D12_RESOURCE_STATES initState);
		public:
			void CopyData(const uint elementIndex, const void* data)final;
			void CopyData(const uint elementIndex, const uint count, const void* data, const uint dataElementSize) final;
			template<typename T>
			void CopyData(const uint elementIndex, const uint count, const T& data)
			{
				CopyData(elementIndex, count, &data, sizeof(T));
			}
			template<typename T>
			void CopyData(const uint elementIndex, const uint count, const T* data)
			{
				CopyData(elementIndex, count, data, sizeof(T));
			}
			template<typename T>
			void CopyData(const uint elementIndex, const uint count, const std::vector<T>& dataVec)
			{
				CopyData(elementIndex, count, dataVec.data(), sizeof(T));
			}
		public:
			std::wstring GetName()const noexcept;
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			J_GRAPHIC_BUFFER_TYPE GetBufferType()const noexcept final;
			uint GetElementCount()const noexcept final;
			uint GetElementByteSize()const noexcept final;
			BYTE* GetCpuBytePointer()const noexcept final;
			void* GetCpuPointer()const noexcept;
			JDx12GraphicResourceHolder* GetHolder()noexcept final;
			ID3D12Resource* GetResource()const noexcept final;
			D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress(const uint addressOffset = 0)noexcept; 
			BYTE GetData(const uint index)const noexcept;
		public: 
			/**
			* @param invalid index = use type per init state
			*/
			void SetFixedInitState(const D3D12_RESOURCE_STATES state);
			void SetGraphicCBBufferView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset)final;
			void SetComputeCBBufferView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset)final;
			void SetGraphicsRootShaderResourceView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset)final;
			void SetComputeRootShaderResourceView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset) final;
			void SetGraphicsRootUnorderedAccessView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset)final;
			void SetComputeRootUnorderedAccessView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset)final;
		public:
			bool CanMappedCpuPointer()const noexcept;
		private:
			void ClearOwnResource();
		private:
			static uint CalcConstantBufferByteSize(const uint byteSize)noexcept;
		};

		template<typename T>
		class JDx12GraphicBufferT final : public JDx12GraphicBuffer
		{ 
		public: 
			JDx12GraphicBufferT(const std::wstring& name = L"", const J_GRAPHIC_BUFFER_TYPE type = J_GRAPHIC_BUFFER_TYPE::COMMON)
				:JDx12GraphicBuffer(name, type, sizeof(T))
			{}
			JDx12GraphicBufferT(const JDx12GraphicBufferT& rhs) = delete;
			JDx12GraphicBufferT& operator=(const JDx12GraphicBufferT& rhs) = delete;
			JDx12GraphicBufferT(JDx12GraphicBufferT&& rhs) = default;
			JDx12GraphicBufferT& operator=(JDx12GraphicBufferT&& rhs) = default;
		public: 
			void StuffValue(ID3D12GraphicsCommandList* cmdList,
				ID3D12Resource* uploadBuffer,
				const D3D12_RESOURCE_STATES beforeState,
				const D3D12_RESOURCE_STATES afterState,
				const T* data)
			{
				JDx12GraphicBuffer::StuffValue(cmdList, uploadBuffer, beforeState, afterState, data); 
			}
			void SutffClearValue(ID3D12GraphicsCommandList* cmdList,
				ID3D12Resource* uploadBuffer,
				const D3D12_RESOURCE_STATES beforeState,
				const D3D12_RESOURCE_STATES afterState,
				const T clearValue)
			{
				JDx12GraphicBuffer::SutffClearValue(cmdList, uploadBuffer, beforeState, afterState, clearValue); 
			}		 
		public: 
			void CopyData(const int elementIndex, const T& data)
			{
				JDx12GraphicBuffer::CopyData(elementIndex, &data); 
			}
			void CopyData(const uint elementIndex, const T* data)
			{
				JDx12GraphicBuffer::CopyData(elementIndex, data);
			}
			void CopyData(const uint elementIndex, const uint count, const T& data)
			{
				JDx12GraphicBuffer::CopyData(elementIndex, count, &data, sizeof(T));
			}
			void CopyData(const uint elementIndex, const uint count, const T* data)
			{
				JDx12GraphicBuffer::CopyData(elementIndex, count, data, sizeof(T));
			}
		public: 
			T* GetCpuPointer()const noexcept
			{
				return static_cast<T*>(static_cast<void*>(GetCpuBytePointer()));
			} 
			T GetData(const uint index)const noexcept
			{ 
				return (T)(GetCpuBytePointer()[index * elementByteSize]);
			} 
		};
	}
}