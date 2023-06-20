#include"JGraphicResourceInfo.h"
#include"JGraphicResourceInterface.h"
#include"../JGraphic.h"
#include"../JGraphicPrivate.h"
#include"../JGraphicDrawList.h"
#include"../../../ThirdParty/DirectX/TK/Src/d3dx12.h"

namespace JinEngine
{
	namespace Graphic
	{
		bool JGraphicSingleResourceInterface::Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat)
		{
			if (info != nullptr)
				return false;

			info = JGraphicPrivate::ResourceInterface::Create2DTexture(uploadBuffer, path, oriFormat);
			uploadBuffer.Reset();
			return info != nullptr;
		}
		bool JGraphicSingleResourceInterface::CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat)
		{
			if (info != nullptr)
				return false;

			info = JGraphicPrivate::ResourceInterface::CreateCubeMap(uploadBuffer, path, oriFormat);
			uploadBuffer.Reset();
			return info != nullptr;
		}
		bool JGraphicSingleResourceInterface::CreateRenderTargetTexture(uint textureWidth, uint textureHeight)
		{
			if (info != nullptr)
				return false;

			info = JGraphicPrivate::ResourceInterface::CreateRenderTargetTexture(textureWidth, textureHeight);
			return info != nullptr;
		}
		bool JGraphicSingleResourceInterface::CreateShadowMapTexture(uint textureWidth, uint textureHeight)
		{
			if (info != nullptr)
				return false;

			info = JGraphicPrivate::ResourceInterface::CreateShadowMapTexture(textureWidth, textureHeight);
			return info != nullptr;
		}
		bool JGraphicSingleResourceInterface::DestroyTexture()
		{
			return JGraphicResourceInfo::Destroy(info.Release());
		}
		bool JGraphicSingleResourceInterface::HasTxtHandle()const noexcept
		{
			return info != nullptr;
		}
		Graphic::J_GRAPHIC_RESOURCE_TYPE JGraphicSingleResourceInterface::GetGraphicResourceType()const noexcept
		{
			return info->GetGraphicResourceType();
		}
		uint JGraphicSingleResourceInterface::GetResourceWidth()const noexcept
		{
			return info->GetWidth();
		}
		uint JGraphicSingleResourceInterface::GetResourceHeight()const noexcept
		{
			return info->GetHeight();
		}
		uint JGraphicSingleResourceInterface::GetResourceArrayIndex()const noexcept
		{
			return info->GetArrayIndex();
		}
		uint JGraphicSingleResourceInterface::GetHeapIndexStart(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return info->GetHeapIndexStart(bType);
		}
		uint JGraphicSingleResourceInterface::GetViewCount(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return info->GetViewCount(bType);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicSingleResourceInterface::GetGpuHandle(const Graphic::J_GRAPHIC_BIND_TYPE bType, const uint bIndex) const noexcept
		{
			return JGraphicPrivate::ResourceInterface::GetGpuDescriptorHandle(info->GetGraphicResourceType(), bType, info->GetArrayIndex(), bIndex);
		}
		bool JGraphicSingleResourceInterface::IsValidHandle()const noexcept
		{
			return info != nullptr;
		}

		bool JGraphicMultiResourceInterface::CreateSceneDepthStencil()
		{
			if (info[(uint)J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL] != nullptr)
				return false;

			info[(uint)J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL] = JGraphicPrivate::ResourceInterface::CreateSceneDepthStencilResource();
			return info[(uint)J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL] != nullptr;
		}
		bool JGraphicMultiResourceInterface::CreateSceneDepthStencilDebug()
		{
			if (info[(uint)J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL_DEBUG] != nullptr)
				return false;

			info[(uint)J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL_DEBUG] = JGraphicPrivate::ResourceInterface::CreateSceneDepthStencilDebugResource();
			return info[(uint)J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL_DEBUG] != nullptr;
		}
		bool JGraphicMultiResourceInterface::CreateDebugDepthStencil()
		{
			if (info[(uint)J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL] != nullptr)
				return false;

			info[(uint)J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL] = JGraphicPrivate::ResourceInterface::CreateDebugDepthStencilResource();
			return info[(uint)J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL] != nullptr;
		}
		bool JGraphicMultiResourceInterface::CreateOcclusionDepth()
		{
			if (info[(uint)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP] != nullptr)
				return false;
			if (info[(uint)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP] != nullptr)
				return false;

			JGraphicPrivate::ResourceInterface::CreateOcclusionHZBResource(info[(uint)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP],
				info[(uint)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP]);
			return info[(uint)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP] != nullptr && info[(uint)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP] != nullptr;
		}
		bool JGraphicMultiResourceInterface::CreateOcclusionDepthDebug()
		{
			if (info[(uint)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG] != nullptr)
				return false;

			info[(uint)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG] = JGraphicPrivate::ResourceInterface::CreateOcclusionHZBResourceDebug();
			return info[(uint)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG] != nullptr;
		}
		bool JGraphicMultiResourceInterface::Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat)
		{
			if (info[(uint)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D] != nullptr)
				return false;

			info[(uint)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D] = JGraphicPrivate::ResourceInterface::Create2DTexture(uploadBuffer, path, oriFormat);
			uploadBuffer.Reset();
			return info != nullptr;
		}
		bool JGraphicMultiResourceInterface::CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat)
		{
			if (info[(uint)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE] != nullptr)
				return false;

			info[(uint)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE] = JGraphicPrivate::ResourceInterface::CreateCubeMap(uploadBuffer, path, oriFormat);
			uploadBuffer.Reset();
			return info != nullptr;
		}
		bool JGraphicMultiResourceInterface::CreateRenderTargetTexture(uint textureWidth, uint textureHeight)
		{
			if (info[(uint)J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON] != nullptr)
				return false;

			info[(uint)J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON] = JGraphicPrivate::ResourceInterface::CreateRenderTargetTexture(textureWidth, textureHeight);
			return info != nullptr;
		}
		bool JGraphicMultiResourceInterface::CreateShadowMapTexture(uint textureWidth, uint textureHeight)
		{
			if (info[(uint)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP] != nullptr)
				return false;

			info[(uint)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP] = JGraphicPrivate::ResourceInterface::CreateShadowMapTexture(textureWidth, textureHeight);
			return info != nullptr;
		}
		bool JGraphicMultiResourceInterface::DestroyTexture(const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			return JGraphicResourceInfo::Destroy(info[(uint)rType].Release());
		}
		void JGraphicMultiResourceInterface::DestroyAllTexture()
		{
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			{
				if (info[i] != nullptr)
					JGraphicResourceInfo::Destroy(info[i].Release());
			}
		}
		bool JGraphicMultiResourceInterface::HasTxtHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return info[(uint)rType] != nullptr;
		}
		uint JGraphicMultiResourceInterface::GetResourceWidth(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return info[(uint)rType]->GetWidth();
		}
		uint JGraphicMultiResourceInterface::GetResourceHeight(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return info[(uint)rType]->GetHeight(); 
		}
		uint JGraphicMultiResourceInterface::GetResourceArrayIndex(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return info[(uint)rType]->GetArrayIndex(); 
		}
		uint JGraphicMultiResourceInterface::GetHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return info[(uint)rType]->GetHeapIndexStart(bType); 
		}
		uint JGraphicMultiResourceInterface::GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return info[(uint)rType]->GetViewCount(bType); 
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicMultiResourceInterface::GetGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_BIND_TYPE bType, const uint bIndex) const noexcept
		{
			return JGraphicPrivate::ResourceInterface::GetGpuDescriptorHandle(rType, bType, info[(uint)rType]->GetArrayIndex(), bIndex);
		}
		bool JGraphicMultiResourceInterface::IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return info != nullptr;
		}

		JGraphicSingleResourceUserInterface::JGraphicSingleResourceUserInterface(JUserPtr<Graphic::JGraphicResourceInfo> info)
			:info(info)
		{}
		JGraphicSingleResourceUserInterface::JGraphicSingleResourceUserInterface(JGraphicSingleResourceInterface* gInterface)
			: info(gInterface->info)
		{}
		Graphic::J_GRAPHIC_RESOURCE_TYPE JGraphicSingleResourceUserInterface::GetGraphicResourceType()const noexcept
		{
			return info != nullptr ? info->GetGraphicResourceType() : Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D;
		}
		uint JGraphicSingleResourceUserInterface::GetResourceWidth()const noexcept
		{
			return info != nullptr ? info->GetWidth() : 0;
		}
		uint JGraphicSingleResourceUserInterface::GetResourceHeight()const noexcept
		{
			return info != nullptr ? info->GetHeight() : 0;
		}
		uint JGraphicSingleResourceUserInterface::GetResourceArrayIndex()const noexcept
		{
			return info != nullptr ? info->GetArrayIndex() : 0;
		}
		uint JGraphicSingleResourceUserInterface::GetHeapIndexStart(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return info != nullptr ? info->GetHeapIndexStart(bType) : 0;
		}
		uint JGraphicSingleResourceUserInterface::GetViewCount(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return info != nullptr ? info->GetViewCount(bType) : 0;
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicSingleResourceUserInterface::GetGpuHandle(const Graphic::J_GRAPHIC_BIND_TYPE bType, const uint bIndex) const noexcept
		{
			if (info != nullptr)
				return JGraphicPrivate::ResourceInterface::GetGpuDescriptorHandle(info->GetGraphicResourceType(), bType, info->GetArrayIndex(), bIndex);
			else
				return CD3DX12_GPU_DESCRIPTOR_HANDLE();
		}
		bool JGraphicSingleResourceUserInterface::IsValidHandle()const noexcept
		{
			return info != nullptr;
		}
		 
		JGraphicMultiResourceUserInterface::JGraphicMultiResourceUserInterface(JGraphicMultiResourceInterface* gInterface)
		{
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				info[i] = gInterface->info[i];
		}
		uint JGraphicMultiResourceUserInterface::GetResourceWidth(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return info[(uint)rType] != nullptr ? info[(uint)rType]->GetWidth() : 0;
		}
		uint JGraphicMultiResourceUserInterface::GetResourceHeight(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return info[(uint)rType] != nullptr ? info[(uint)rType]->GetHeight() : 0;
		}
		uint JGraphicMultiResourceUserInterface::GetResourceArrayIndex(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return info[(uint)rType] != nullptr ? info[(uint)rType]->GetArrayIndex() : 0;
		}
		uint JGraphicMultiResourceUserInterface::GetHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return info[(uint)rType] != nullptr ? info[(uint)rType]->GetHeapIndexStart(bType) : 0;
		}
		uint JGraphicMultiResourceUserInterface::GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return info[(uint)rType] != nullptr ? info[(uint)rType]->GetViewCount(bType) : 0;
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicMultiResourceUserInterface::GetGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_BIND_TYPE bType, const uint bIndex) const noexcept
		{
			if (info[(uint)rType] != nullptr)
				return JGraphicPrivate::ResourceInterface::GetGpuDescriptorHandle(info[(uint)rType]->GetGraphicResourceType(), bType, info[(uint)rType]->GetArrayIndex(), bIndex);
			else
				return CD3DX12_GPU_DESCRIPTOR_HANDLE();
		}
		bool JGraphicMultiResourceUserInterface::IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return info[(uint)rType] != nullptr;
		}
	}
}