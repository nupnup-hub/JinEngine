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
	class JCamera;
	namespace Graphic
	{ 
		class JGraphicResourceInfo;
		class JGraphicSingleResourceUserInterface;
		class JGraphicMultiResourceUserInterface;
		class JCullingInterface;

		class JGraphicSingleResourceInterface
		{
		private:
			friend class JGraphicSingleResourceUserInterface;
		private:
			JUserPtr<Graphic::JGraphicResourceInfo> info = nullptr;
		protected:
			bool Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat);
			bool CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat);
			bool CreateRenderTargetTexture(uint textureWidth = 0, uint textureHeight = 0);
			bool CreateShadowMapTexture(uint textureWidth = 0, uint textureHeight = 0);
			bool DestroyTexture();
		public:
			bool HasTxtHandle()const noexcept; 
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
		
		class JGraphicMultiResourceInterface
		{
		private:
			friend class JGraphicMultiResourceUserInterface;
			friend class JCullingInterface;
		private:
			JUserPtr<Graphic::JGraphicResourceInfo> info[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
		protected:
			bool CreateSceneDepthStencil();
			bool CreateSceneDepthStencilDebug();
			bool CreateDebugDepthStencil();
			bool CreateOcclusionDepth();
			bool CreateOcclusionDepthDebug();
			bool Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat);
			bool CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat);
			bool CreateRenderTargetTexture(uint textureWidth = 0, uint textureHeight = 0);
			bool CreateShadowMapTexture(uint textureWidth = 0, uint textureHeight = 0);
			bool DestroyTexture(const J_GRAPHIC_RESOURCE_TYPE rType);
			void DestroyAllTexture();
		public:
			bool HasTxtHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept; 
		public: 
			uint GetResourceWidth(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			uint GetResourceHeight(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			uint GetResourceArrayIndex(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			uint GetHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept;
			uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_BIND_TYPE bType, const uint bIndex) const noexcept;
		public:
			bool IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
		};

		class JGraphicSingleResourceUserInterface final
		{
		private:
			JUserPtr<Graphic::JGraphicResourceInfo> info = nullptr;
		public:
			JGraphicSingleResourceUserInterface(JUserPtr<Graphic::JGraphicResourceInfo> info);
			JGraphicSingleResourceUserInterface(JGraphicSingleResourceInterface* gInterface);
			~JGraphicSingleResourceUserInterface() = default;
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

		class JGraphicMultiResourceUserInterface final
		{
		private:
			JUserPtr<Graphic::JGraphicResourceInfo> info[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
		public: 
			JGraphicMultiResourceUserInterface(JGraphicMultiResourceInterface* gInterface);
			~JGraphicMultiResourceUserInterface() = default;
		public: 
			uint GetResourceWidth(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			uint GetResourceHeight(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			uint GetResourceArrayIndex(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			uint GetHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept;
			uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_BIND_TYPE bType, const uint bIndex) const noexcept;
		public:
			bool IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
		};
	}
}
