#pragma once
#include "JDxGraphicResourceHolderInterface.h" 
#include<wrl/client.h>   
#include<d3dx12.h>
#include<memory>

namespace JinEngine
{
	namespace Graphic
	{
		struct JDx12GraphicResourceHolderDesc
		{
		public:
			Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
			D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
			uint elementSize = 0;
		public:
			JDx12GraphicResourceHolderDesc() = default;
			JDx12GraphicResourceHolderDesc(Microsoft::WRL::ComPtr<ID3D12Resource>&& newResource, const D3D12_RESOURCE_STATES state);
			JDx12GraphicResourceHolderDesc(Microsoft::WRL::ComPtr<ID3D12Resource>&& newResource, const D3D12_RESOURCE_STATES state, const uint elementSize);
		public:
			bool IsValid()const noexcept;
		};

		class JDx12GraphicResourceHolder final : public JDxGraphicResourceHolderInterface
		{
		private:
			Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
			D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
			D3D12_RESOURCE_STATES defaultState = D3D12_RESOURCE_STATE_COMMON;
		private:
			uint elementSize = 0;
		public:
			JDx12GraphicResourceHolder() = default; 
			JDx12GraphicResourceHolder(JDx12GraphicResourceHolderDesc&& desc);
			JDx12GraphicResourceHolder(const JDx12GraphicResourceHolder& rhs) = delete;
			JDx12GraphicResourceHolder& operator=(const JDx12GraphicResourceHolder & rhs) = delete;
			JDx12GraphicResourceHolder(JDx12GraphicResourceHolder&& rhs) = default;
			JDx12GraphicResourceHolder& operator=(JDx12GraphicResourceHolder&& rhs) = default;
			~JDx12GraphicResourceHolder();
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			uint GetWidth()const noexcept final;
			uint GetHeight()const noexcept final;
			uint GetArrayCount()const noexcept final;
			uint GetElementCount()const noexcept final;
			uint GetElementSize()const noexcept final;
			JVector2<uint> GetResourceSize()const noexcept final;
			J_GRAPHIC_RESOURCE_FORMAT GetEngineFormat()const noexcept final;
			ID3D12Resource* GetResource() const noexcept;
			D3D12_RESOURCE_STATES GetState()const noexcept;
			D3D12_RESOURCE_STATES GetDefaultState()const noexcept;
		public:
			void SetPrivateName(const std::wstring& name)noexcept final;
			void SetState(const D3D12_RESOURCE_STATES newState)noexcept;
		public: 
			bool IsBuffer()const noexcept;
			bool HasValidResource()const noexcept final;
		public:
			void Swap(Microsoft::WRL::ComPtr<ID3D12Resource>&& newResource, const D3D12_RESOURCE_STATES newState)noexcept;
			BYTE* Map()const noexcept;
			void UnMap()const noexcept;
		public:
			void Clear()final;
		private:
			void ClearResource();
		};
	}
}