#pragma once
#include "JDxGraphicResourceHolderInterface.h" 
#include<wrl/client.h>   
#include<d3dx12.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12GraphicResourceHolder final : public JDxGraphicResourceHolderInterface
		{
		private:
			Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;  
		public:
			JDx12GraphicResourceHolder() = default;
			JDx12GraphicResourceHolder(Microsoft::WRL::ComPtr<ID3D12Resource>&& resource);
			JDx12GraphicResourceHolder(const JDx12GraphicResourceHolder& rhs) = delete;
			JDx12GraphicResourceHolder& operator=(const JDx12GraphicResourceHolder & rhs) = delete;
			JDx12GraphicResourceHolder(JDx12GraphicResourceHolder&& rhs) = default;
			JDx12GraphicResourceHolder& operator=(JDx12GraphicResourceHolder&& rhs) = default;
			~JDx12GraphicResourceHolder();
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			uint GetWidth()const noexcept final;
			uint GetHeight()const noexcept final;
			ID3D12Resource* GetResource() const noexcept;
		public:
			void SetPrivateName(const std::wstring& name)noexcept final;
		public:
			bool HasValidResource()const noexcept final;
		public:
			void Swap(Microsoft::WRL::ComPtr<ID3D12Resource>&& newResource)noexcept;
			BYTE* Map()const noexcept;
			void UnMap()const noexcept;
		public:
			void Clear() final;
		};
	}
}