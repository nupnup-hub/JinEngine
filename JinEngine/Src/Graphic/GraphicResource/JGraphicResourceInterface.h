#pragma once 
#include"../../Core/JDataType.h"
#include"JGraphicResourceType.h"
#include<string>
#include<wrl/client.h> 

struct ID3D12Resource;
struct CD3DX12_CPU_DESCRIPTOR_HANDLE;
struct CD3DX12_GPU_DESCRIPTOR_HANDLE;

namespace JinEngine
{
	class JScene;
	class JComponent;
	namespace Graphic
	{ 
		class JGraphicResourceInfo;
		class JGraphicResourceUserInterface;
		class JGraphicResourceInterface
		{
		private:
			friend class JGraphicResourceUserInterface;
		private:
			Core::JUserPtr<Graphic::JGraphicResourceInfo> info = nullptr;
		protected:
			bool Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat);
			bool CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat);
			bool CreateRenderTargetTexture(uint textureWidth = 0, uint textureHeight = 0);
			bool CreateShadowMapTexture(uint textureWidth = 0, uint textureHeight = 0);
			bool DestroyTexture();
			bool HasTxtHandle()const noexcept;
			void AddDrawRequest(JScene* scene, JComponent* jComp);
			void PopDrawRequest(JScene* scene, JComponent* jComp);
		public:
			Graphic::J_GRAPHIC_RESOURCE_TYPE GetGraphicResourceType()const noexcept;
			uint GetResourceWidth()const noexcept;
			uint GetResourceHeight()const noexcept;
			uint GetResourceArrayIndex()const noexcept;
			uint GetHeapIndexStart(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept;
			uint GetViewCount(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept;  
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(const Graphic::J_GRAPHIC_BIND_TYPE bType, const uint bIndex) const noexcept;
		public:
			bool IsValidHandle()const noexcept;
		};
		 
		class JGraphicResourceUserInterface final
		{
		private:
			Core::JUserPtr<Graphic::JGraphicResourceInfo> info = nullptr;
		public:
			JGraphicResourceUserInterface(Core::JUserPtr<Graphic::JGraphicResourceInfo> info);
			JGraphicResourceUserInterface(JGraphicResourceInterface* gInterface);
			~JGraphicResourceUserInterface() = default;
		public:
			Graphic::J_GRAPHIC_RESOURCE_TYPE GetGraphicResourceType()const noexcept;
			uint GetResourceWidth()const noexcept;
			uint GetResourceHeight()const noexcept;
			uint GetResourceArrayIndex()const noexcept;
			uint GetHeapIndexStart(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept;
			uint GetViewCount(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(const Graphic::J_GRAPHIC_BIND_TYPE bType, const uint bIndex) const noexcept;
		public:
			bool IsValidHandle()const noexcept;
		};

	}
}
