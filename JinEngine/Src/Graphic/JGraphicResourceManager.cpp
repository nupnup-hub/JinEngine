#include"JGraphicResourceManager.h"  
#include"JGraphicTextureType.h"
#include"JGraphicTextureHandle.h"

#include"../../Lib/LoadTextureFromFile.h"
#include"../../Lib/DirectX/DDSTextureLoader.h" 
#include"../Utility/JCommonUtility.h"
#include"../Core/Exception/JExceptionMacro.h"

#include<DirectXColors.h>

namespace JinEngine
{
	namespace Graphic
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE JGraphicResourceManager::GetCpuRtvDescriptorHandle(int index)const noexcept
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap->GetCPUDescriptorHandleForHeapStart(), index, rtvDescriptorSize);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicResourceManager::GetGpuRtvDescriptorHandle(int index)const noexcept
		{
			return CD3DX12_GPU_DESCRIPTOR_HANDLE(rtvHeap->GetGPUDescriptorHandleForHeapStart(), index, rtvDescriptorSize);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE JGraphicResourceManager::GetCpuDsvDescriptorHandle(int index)const noexcept
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart(), index, dsvDescriptorSize);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicResourceManager::GetGpuDsvDescriptorHandle(int index)const noexcept
		{
			return CD3DX12_GPU_DESCRIPTOR_HANDLE(dsvHeap->GetGPUDescriptorHandleForHeapStart(), index, dsvDescriptorSize);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE JGraphicResourceManager::GetCpuSrvDescriptorHandle(int index)const noexcept
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(srvHeap->GetCPUDescriptorHandleForHeapStart(), index, cbvSrvUavDescriptorSize);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicResourceManager::GetGpuSrvDescriptorHandle(int index)const noexcept
		{
			return CD3DX12_GPU_DESCRIPTOR_HANDLE(srvHeap->GetGPUDescriptorHandleForHeapStart(), index, cbvSrvUavDescriptorSize);
		}

		uint JGraphicResourceManager::GetSrvOcclusionDepthStart()const noexcept
		{
			return srvFixedCount;
		}
		uint JGraphicResourceManager::GetSrvOcclusionDebugStart()const noexcept
		{ 
			return GetSrvOcclusionDepthStart() + occlusionCapacity;
		}
		uint JGraphicResourceManager::GetSrvUser2DTextureStart()const noexcept
		{
			return GetSrvOcclusionDebugStart() + occlusionDebugCapacity;
		}
		uint JGraphicResourceManager::GetSrvUserCubeMapStart()const noexcept
		{
			return GetSrvUser2DTextureStart() + user2DTextureCapacity;
		}
		uint JGraphicResourceManager::GetSrvRenderResultStart()const noexcept
		{
			return GetSrvUserCubeMapStart() + userCubeMapCapacity;
		}
		uint JGraphicResourceManager::GetSrvShadowMapStart()const noexcept
		{
			return GetSrvRenderResultStart() + renderResultCapacity;
		}
		uint JGraphicResourceManager::GetUavOcclusionDepthStart()const noexcept
		{
			return GetSrvShadowMapStart() + shadowMapCapacity;
		}
		uint JGraphicResourceManager::GetUavOcclusionDebugStart()const noexcept
		{
			//occlusion = (count - 1)uav + 1dsv
			return GetUavOcclusionDepthStart() + occlusionCapacity - 1;
		}
		uint JGraphicResourceManager::GetRtvRenderResultStart()const noexcept
		{
			return swapChainBufferCount;
		}
		uint JGraphicResourceManager::GetRtvShadowMapStart()const noexcept
		{
			return GetRtvRenderResultStart() + renderResultCapacity;
		}
		uint JGraphicResourceManager::GetDsvOcclusionDepthStart()const noexcept
		{
			return mainBufDsCount;
		}
		uint JGraphicResourceManager::GetDsvShadowMapStart()const noexcept
		{
			return GetDsvOcclusionDepthStart() + occlusionDsCount;	//main buff + occlusion buff
		}
		uint JGraphicResourceManager::GetTotalRtvCount()const noexcept
		{
			return swapChainBufferCount + renderResultCount + shadowMapCount;
		}
		uint JGraphicResourceManager::GetTotalRtvCapacity()const noexcept
		{
			return swapChainBufferCount + renderResultCapacity + shadowMapCapacity;
		}
		uint JGraphicResourceManager::GetTotalDsvCount()const noexcept
		{
			return mainBufDsCount + occlusionDsCount + shadowMapCount;
		}
		uint JGraphicResourceManager::GetTotalDsvCapacity()const noexcept
		{
			return mainBufDsCount + occlusionDsCount + shadowMapCapacity;
		}
		uint JGraphicResourceManager::GetTotalRsCount()const noexcept
		{
			return renderResultCount + shadowMapCount;
		}
		uint JGraphicResourceManager::GetTotalRsCapacity()const noexcept
		{
			return renderResultCapacity + shadowMapCapacity;
		}
		uint JGraphicResourceManager::GetTotalTextureCount()const noexcept
		{
			return srvFixedCount + occlusionCount + occlusionDebugCount +
				user2DTextureCount + userCubeMapCount + renderResultCount + shadowMapCount +
				uavCount;
		}
		uint JGraphicResourceManager::GetTotalTextureCapacity()const noexcept
		{
			return srvFixedCount + occlusionCapacity + occlusionDebugCapacity + 
				user2DTextureCapacity + userCubeMapCapacity + renderResultCapacity + shadowMapCapacity +
				uavCapacity;
		}
		uint JGraphicResourceManager::GetOcclusionQueryHeapCapacity()const noexcept
		{
			return occlusionQuaryCapacity;
		}
		uint JGraphicResourceManager::GetOcclusionDsCapacity()const noexcept
		{
			return occlusionCapacity;
		}
		ID3D12QueryHeap* JGraphicResourceManager::GetOcclusionQueryHeap()const noexcept
		{
			return occlusionQueryHeap.Get();
		}
		ID3D12Resource* JGraphicResourceManager::GetOcclusionResult()const noexcept
		{
			return occlusionQueryResult.Get();
		}
		void JGraphicResourceManager::BuildRtvDescriptorHeaps(ID3D12Device* device)
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
			rtvHeapDesc.NumDescriptors = GetTotalRtvCapacity();
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			rtvHeapDesc.NodeMask = 0;
			ThrowIfFailedHr(device->CreateDescriptorHeap(
				&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.GetAddressOf())));
		}
		void JGraphicResourceManager::BuildDsvDescriptorHeaps(ID3D12Device* device)
		{
			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
			dsvHeapDesc.NumDescriptors = GetTotalDsvCapacity();
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			dsvHeapDesc.NodeMask = 0;
			ThrowIfFailedHr(device->CreateDescriptorHeap(
				&dsvHeapDesc, IID_PPV_ARGS(dsvHeap.GetAddressOf())));
		}
		void JGraphicResourceManager::BuildSrvDescriptorHeaps(ID3D12Device* device)
		{
			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
			srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			srvHeapDesc.NumDescriptors = GetTotalTextureCapacity();
			srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			ThrowIfFailedHr(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(srvHeap.GetAddressOf())));
		}
		void JGraphicResourceManager::BuildOcclusionQueryHeap(ID3D12Device* device)
		{
			D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
			queryHeapDesc.Count = occlusionQuaryCapacity;
			queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
			ThrowIfFailedHr(device->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&occlusionQueryHeap)));
		}
		JGraphicTextureHandle* JGraphicResourceManager::Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap,
			const std::wstring& path,
			const std::wstring& oriFormat,
			ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> newTexture;
			JGraphicTextureHandle* handleCash = nullptr;
			uint heapIndex = GetSrvUser2DTextureStart() + user2DTextureCount;
			size_t width = 0;
			size_t height = 0;
			bool res = false;

			if (oriFormat == L".dds")
			{
				res = DirectX::CreateDDSTextureFromFile12(device,
					commandList,
					path.c_str(),
					newTexture,
					uploadHeap,
					width,
					height) == S_OK;

				if (res)
				{
					D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					srvDesc.Texture2D.MostDetailedMip = 0;
					srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					srvDesc.TextureCube.MostDetailedMip = 0;
					srvDesc.TextureCube.MipLevels = newTexture->GetDesc().MipLevels;
					srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
					srvDesc.Format = newTexture->GetDesc().Format;
					device->CreateShaderResourceView(newTexture.Get(), &srvDesc, GetCpuSrvDescriptorHandle(heapIndex));
				}
			}
			else
			{
				res = LoadTextureFromFile(JCUtil::WstrToU8Str(path).c_str(),
					device,
					GetCpuSrvDescriptorHandle(heapIndex),
					newTexture,
					uploadHeap,
					width,
					height);
			}
			if (res)
			{
				std::unique_ptr<JGraphicTextureHandle> newHandle =
					std::make_unique<JGraphicTextureHandle>(J_GRAPHIC_TEXTURE_TYPE::TEXTURE_2D, (uint)width, (uint)height);
				newHandle->srvHeapIndex = heapIndex;
				SetGraphicBuffIndex(*newHandle, user2DTextureCount);
				handleCash = newHandle.get();

				user2DTextureHandle.push_back(std::move(newHandle));
				user2DTextureResouce.push_back(std::move(newTexture));
				++user2DTextureCount;
			}
			return handleCash;
		}
		JGraphicTextureHandle* JGraphicResourceManager::CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap,
			const std::wstring& path,
			const std::wstring& oriFormat,
			ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> newTexture;
			JGraphicTextureHandle* handleCash = nullptr;
			uint heapIndex = GetSrvUserCubeMapStart() + userCubeMapCount;
			size_t width = 0;
			size_t height = 0;
			bool res = false;

			if (oriFormat == L".dds")
			{
				res = DirectX::CreateDDSTextureFromFile12(device,
					commandList,
					path.c_str(),
					newTexture,
					uploadHeap,
					width,
					height) == S_OK;

				if (res)
				{
					D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					srvDesc.Texture2D.MostDetailedMip = 0;
					srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
					srvDesc.TextureCube.MostDetailedMip = 0;
					srvDesc.TextureCube.MipLevels = newTexture->GetDesc().MipLevels;
					srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
					srvDesc.Format = newTexture->GetDesc().Format;
					device->CreateShaderResourceView(newTexture.Get(), &srvDesc, GetCpuSrvDescriptorHandle(heapIndex));
				}
			}
			else
			{
				res = LoadCubemapFromFile(JCUtil::WstrToU8Str(path).c_str(),
					device,
					GetCpuSrvDescriptorHandle(heapIndex),
					newTexture,
					uploadHeap,
					width,
					height);
			}
			if (res)
			{
				std::unique_ptr<JGraphicTextureHandle> newHandle =
					std::make_unique<JGraphicTextureHandle>(J_GRAPHIC_TEXTURE_TYPE::TEXTURE_CUBE, (uint)width, (uint)height);
				newHandle->srvHeapIndex = heapIndex;
				SetGraphicBuffIndex(*newHandle, userCubeMapCount);
				handleCash = newHandle.get();

				userCubeMapHandle.push_back(std::move(newHandle));
				userCubeMapResouce.push_back(std::move(newTexture));
				++userCubeMapCount;
			}
			return handleCash;
		}
		JGraphicTextureHandle* JGraphicResourceManager::CreateRenderTargetTexture(ID3D12Device* device, const uint width, const uint height)
		{
			CD3DX12_CLEAR_VALUE optClear(DXGI_FORMAT_R16G16B16A16_FLOAT, DirectX::Colors::Snow);
			CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			rtvDesc.Texture2D.MipSlice = 0;
			rtvDesc.Texture2D.PlaneSlice = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			srvDesc.Texture2D.MipLevels = 1;

			D3D12_RESOURCE_DESC resourceDesc;
			ZeroMemory(&resourceDesc, sizeof(D3D12_RESOURCE_DESC));
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resourceDesc.Alignment = 0;
			resourceDesc.Width = width;
			resourceDesc.Height = height;
			resourceDesc.DepthOrArraySize = 1;
			resourceDesc.MipLevels = 1;
			resourceDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap->GetCPUDescriptorHandleForHeapStart());
			rtvHandle.Offset(GetRtvRenderResultStart() + renderResultCount, rtvDescriptorSize);

			CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvHeap->GetCPUDescriptorHandleForHeapStart());
			srvHandle.Offset(GetSrvRenderResultStart() + renderResultCount, cbvSrvUavDescriptorSize);

			Microsoft::WRL::ComPtr<ID3D12Resource> newTexture;
			ThrowIfFailedHr(device->CreateCommittedResource(
				&heapProperty,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				&optClear,
				IID_PPV_ARGS(&newTexture)));

			device->CreateRenderTargetView(newTexture.Get(), &rtvDesc, rtvHandle);
			device->CreateShaderResourceView(newTexture.Get(), &srvDesc, srvHandle);

			std::unique_ptr<JGraphicTextureHandle> newHandle =
				std::make_unique<JGraphicTextureHandle>(J_GRAPHIC_TEXTURE_TYPE::RENDER_RESULT_COMMON, (uint)width, (uint)height);

			newHandle->rtvHeapIndex = GetRtvRenderResultStart() + renderResultCount;
			newHandle->srvHeapIndex = GetSrvRenderResultStart() + renderResultCount;
			SetGraphicBuffIndex(*newHandle, renderResultCount);

			JGraphicTextureHandle* handleCash = newHandle.get();
			renderResultHandle.push_back(std::move(newHandle));
			renderResultResource.push_back(std::move(newTexture));
			++renderResultCount;
			return handleCash;
		}
		JGraphicTextureHandle* JGraphicResourceManager::CreateShadowMapTexture(ID3D12Device* device, const uint width, const uint height)
		{
			CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

			D3D12_RESOURCE_DESC resourceDesc;
			ZeroMemory(&resourceDesc, sizeof(D3D12_RESOURCE_DESC));
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resourceDesc.Alignment = 0;
			resourceDesc.Width = width;
			resourceDesc.Height = height;
			resourceDesc.DepthOrArraySize = 1;
			resourceDesc.MipLevels = 1;
			resourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			CD3DX12_CLEAR_VALUE optClear(DXGI_FORMAT_R16G16B16A16_FLOAT, DirectX::Colors::Snow);
			optClear.Format = depthStencilFormat;
			optClear.DepthStencil.Depth = 1.0f;
			optClear.DepthStencil.Stencil = 0;

			Microsoft::WRL::ComPtr<ID3D12Resource> newTexture;
			ThrowIfFailedHr(device->CreateCommittedResource(
				&heapProperty,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COMMON,
				&optClear,
				IID_PPV_ARGS(&newTexture)));

			// Create DSV to resource so we can render to the shadow map.
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Format = depthStencilFormat;
			dsvDesc.Texture2D.MipSlice = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Texture2D.PlaneSlice = 0;

			CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart());
			dsvHandle.Offset(GetDsvShadowMapStart() + shadowMapCount, dsvDescriptorSize);

			CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvHeap->GetCPUDescriptorHandleForHeapStart());
			srvHandle.Offset(GetSrvShadowMapStart() + shadowMapCount, cbvSrvUavDescriptorSize);

			device->CreateDepthStencilView(newTexture.Get(), &dsvDesc, dsvHandle);
			device->CreateShaderResourceView(newTexture.Get(), &srvDesc, srvHandle);

			std::unique_ptr<JGraphicTextureHandle> newHandle =
				std::make_unique<JGraphicTextureHandle>(J_GRAPHIC_TEXTURE_TYPE::RENDER_RESULT_SHADOW_MAP, (uint)width, (uint)height);

			newHandle->dsvHeapIndex = GetDsvShadowMapStart() + shadowMapCount;
			newHandle->srvHeapIndex = GetSrvShadowMapStart() + shadowMapCount;
			SetGraphicBuffIndex(*newHandle, shadowMapCount);

			JGraphicTextureHandle* handleCash = newHandle.get();
			shadowMapHandle.push_back(std::move(newHandle));
			shadowMapResource.push_back(std::move(newTexture));
			++shadowMapCount;
			return handleCash;
		}
		bool JGraphicResourceManager::DestroyGraphicTextureResource(ID3D12Device* device, JGraphicTextureHandle** handle)
		{
			if (*handle == nullptr)
				return false;

			const uint vIndex = GetGraphicBuffIndex(**handle);
			const J_GRAPHIC_TEXTURE_TYPE gRType = (*handle)->graphicResourceType;
			*handle = nullptr;

			switch (gRType)
			{
			case J_GRAPHIC_TEXTURE_TYPE::TEXTURE_2D:
			{
				user2DTextureResouce[vIndex].Reset();
				user2DTextureResouce.erase(user2DTextureResouce.begin() + vIndex);
				user2DTextureHandle.erase(user2DTextureHandle.begin() + vIndex);
				--user2DTextureCount;
				for (uint i = vIndex; i < user2DTextureCount; ++i)
				{
					--user2DTextureHandle[i]->srvHeapIndex;
					SetGraphicBuffIndex(*user2DTextureHandle[i], GetGraphicBuffIndex(*user2DTextureHandle[i]) - 1);
					ReBind2DTexture(device, GetGraphicBuffIndex(*user2DTextureHandle[i]), user2DTextureHandle[i]->srvHeapIndex);
				}
				return true;
			}
			case J_GRAPHIC_TEXTURE_TYPE::TEXTURE_CUBE:
			{
				userCubeMapResouce[vIndex].Reset();
				userCubeMapResouce.erase(userCubeMapResouce.begin() + vIndex);
				userCubeMapHandle.erase(userCubeMapHandle.begin() + vIndex);
				--userCubeMapCount;
				for (uint i = vIndex; i < userCubeMapCount; ++i)
				{
					--userCubeMapHandle[i]->srvHeapIndex;
					SetGraphicBuffIndex(*userCubeMapHandle[i], GetGraphicBuffIndex(*userCubeMapHandle[i]) - 1);
					ReBindCubeMap(device, GetGraphicBuffIndex(*userCubeMapHandle[i]), userCubeMapHandle[i]->srvHeapIndex);
				}
				return true;
			}
			case J_GRAPHIC_TEXTURE_TYPE::RENDER_RESULT_COMMON:
			{
				renderResultResource[vIndex].Reset();
				renderResultResource.erase(renderResultResource.begin() + vIndex);
				renderResultHandle.erase(renderResultHandle.begin() + vIndex);
				--renderResultCount;
				for (uint i = vIndex; i < renderResultCount; ++i)
				{
					--renderResultHandle[i]->srvHeapIndex;
					--renderResultHandle[i]->rtvHeapIndex;

					SetGraphicBuffIndex(*renderResultHandle[i], GetGraphicBuffIndex(*renderResultHandle[i]) - 1);
					ReBindRenderTarget(device, GetGraphicBuffIndex(*renderResultHandle[i]), renderResultHandle[i]->rtvHeapIndex, renderResultHandle[i]->srvHeapIndex);
				}
				return true;
			}
			case J_GRAPHIC_TEXTURE_TYPE::RENDER_RESULT_SHADOW_MAP:
			{
				shadowMapResource[vIndex].Reset();
				shadowMapResource.erase(shadowMapResource.begin() + vIndex);
				shadowMapHandle.erase(shadowMapHandle.begin() + vIndex);
				--shadowMapCount;
				for (uint i = vIndex; i < shadowMapCount; ++i)
				{
					--shadowMapHandle[i]->srvHeapIndex;
					--shadowMapHandle[i]->dsvHeapIndex;

					SetGraphicBuffIndex(*shadowMapHandle[i], GetGraphicBuffIndex(*shadowMapHandle[i]) - 1);
					ReBindShadowMapTexture(device, GetGraphicBuffIndex(*shadowMapHandle[i]), shadowMapHandle[i]->dsvHeapIndex, shadowMapHandle[i]->srvHeapIndex);
				}
				return true;
			}
			default:
				return false;
			}
		}
		void JGraphicResourceManager::ReBind2DTexture(ID3D12Device* device, const uint resourceIndex, const uint heapIndex)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));
			srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = user2DTextureResouce[resourceIndex]->GetDesc().MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			device->CreateShaderResourceView(user2DTextureResouce[resourceIndex].Get(), &srvDesc, GetCpuSrvDescriptorHandle(heapIndex));
		}
		void JGraphicResourceManager::ReBindCubeMap(ID3D12Device* device, const uint resourceIndex, const uint heapIndex)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = userCubeMapResouce[resourceIndex]->GetDesc().MipLevels;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			srvDesc.Format = userCubeMapResouce[resourceIndex]->GetDesc().Format;

			device->CreateShaderResourceView(userCubeMapResouce[resourceIndex].Get(), &srvDesc, GetCpuSrvDescriptorHandle(heapIndex));
		}
		void JGraphicResourceManager::ReBindRenderTarget(ID3D12Device* device, const uint resourceIndex, const uint rtvHeapIndex, const uint srvHeapIndex)
		{
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			rtvDesc.Texture2D.MipSlice = 0;
			rtvDesc.Texture2D.PlaneSlice = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			srvDesc.Texture2D.MipLevels = 1;

			device->CreateRenderTargetView(renderResultResource[resourceIndex].Get(), &rtvDesc, GetCpuRtvDescriptorHandle(rtvHeapIndex));
			device->CreateShaderResourceView(renderResultResource[resourceIndex].Get(), &srvDesc, GetCpuSrvDescriptorHandle(srvHeapIndex));
		}
		void JGraphicResourceManager::ReBindShadowMapTexture(ID3D12Device* device, const uint resourceIndex, const uint dsvHeapIndex, const uint srvHeapIndex)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Texture2D.PlaneSlice = 0;

			// Create DSV to resource so we can render to the shadow map.
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Format = depthStencilFormat;
			dsvDesc.Texture2D.MipSlice = 0;

			device->CreateDepthStencilView(shadowMapResource[resourceIndex].Get(), &dsvDesc, GetCpuDsvDescriptorHandle(dsvHeapIndex));
			device->CreateShaderResourceView(shadowMapResource[resourceIndex].Get(), &srvDesc, GetCpuSrvDescriptorHandle(srvHeapIndex));
		} 
		void JGraphicResourceManager::CreateOcclusionQueryResource(ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList,
			const uint width,
			const uint height,
			bool m4xMsaaState,
			uint m4xMsaaQuality)
		{
			occlusionQueryResult.Reset();
			CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
			auto queryResultDesc = CD3DX12_RESOURCE_DESC::Buffer(8 * occlusionQuaryCapacity);

			ThrowIfFailedHr(device->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&queryResultDesc,
				D3D12_RESOURCE_STATE_PREDICATION,
				nullptr,
				IID_PPV_ARGS(&occlusionQueryResult)));

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Format = depthStencilFormat;
			dsvDesc.Texture2D.MipSlice = 0;

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC debugSrvDesc = {};
			debugSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			debugSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			debugSrvDesc.Texture2D.MostDetailedMip = 0;
			debugSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			debugSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			debugSrvDesc.Texture2D.MipLevels = 1;

			occlusionDepthMap.clear();
			occlusionDebug.clear();
			occlusionDepthMap.resize(occlusionCapacity);
			occlusionDebug.resize(occlusionCapacity);

			if (occlusionCount > 0)
				uavCount -= (occlusionCount - 1);
			if (occlusionDebugCount > 0)
				uavCount -= occlusionDebugCount;

			occlusionCount = occlusionDebugCount =  0;
			uint nowWidth = width;
			uint nowHeight = height;
			while (occlusionCount != occlusionCapacity)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC depthSrvDesc = {};
				depthSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				depthSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				depthSrvDesc.Texture2D.MostDetailedMip = 0;
				depthSrvDesc.Texture2D.MipLevels = 1;
				depthSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				depthSrvDesc.Texture2D.PlaneSlice = 0;
	
				D3D12_RESOURCE_DESC depthStencilDesc;
				ZeroMemory(&depthStencilDesc, sizeof(D3D12_RESOURCE_DESC));
				depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				depthStencilDesc.Alignment = 0;
				depthStencilDesc.Width = (uint)nowWidth;
				depthStencilDesc.Height = (uint)nowHeight;
				depthStencilDesc.DepthOrArraySize = 1;
				depthStencilDesc.MipLevels = 1;
				depthStencilDesc.SampleDesc.Count = 1;
				depthStencilDesc.SampleDesc.Quality = 0;
				depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				if (occlusionCount == 0)
				{
					depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
					depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
					depthSrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
				}
				else
				{
					depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
					depthStencilDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					depthSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				}

				 
				Microsoft::WRL::ComPtr<ID3D12Resource> newOcclusion;
				CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
				if (occlusionCount == 0)
				{
					CD3DX12_CLEAR_VALUE optClear(DXGI_FORMAT_R16G16B16A16_FLOAT, DirectX::Colors::Snow);
					optClear.Format = depthStencilFormat;
					optClear.DepthStencil.Depth = 1.0f;
					optClear.DepthStencil.Stencil = 0;

					ThrowIfFailedHr(device->CreateCommittedResource(
						&heapProperties,
						D3D12_HEAP_FLAG_NONE,
						&depthStencilDesc,
						D3D12_RESOURCE_STATE_GENERIC_READ,
						&optClear,
						IID_PPV_ARGS(&newOcclusion)));
				}
				else
				{
					ThrowIfFailedHr(device->CreateCommittedResource(
						&heapProperties,
						D3D12_HEAP_FLAG_NONE,
						&depthStencilDesc,
						D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
						nullptr,
						IID_PPV_ARGS(&newOcclusion)));

				}
				occlusionDepthMap[occlusionCount] = std::move(newOcclusion);

				if (occlusionCount == 0)
				{
					CD3DX12_CPU_DESCRIPTOR_HANDLE depthDsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart());
					depthDsvHandle.Offset(GetDsvOcclusionDepthStart(), dsvDescriptorSize);
					device->CreateDepthStencilView(occlusionDepthMap[occlusionCount].Get(), &dsvDesc, depthDsvHandle);
				}
				else
				{
					CD3DX12_CPU_DESCRIPTOR_HANDLE depthUavHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvHeap->GetCPUDescriptorHandleForHeapStart());
					depthUavHandle.Offset(GetUavOcclusionDepthStart() + occlusionCount - 1, cbvSrvUavDescriptorSize);
					device->CreateUnorderedAccessView(occlusionDepthMap[occlusionCount].Get(), nullptr, &uavDesc, depthUavHandle);
				}

				CD3DX12_CPU_DESCRIPTOR_HANDLE depthSrvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvHeap->GetCPUDescriptorHandleForHeapStart());
				depthSrvHandle.Offset(GetSrvOcclusionDepthStart() + occlusionCount, cbvSrvUavDescriptorSize);
				device->CreateShaderResourceView(occlusionDepthMap[occlusionCount].Get(), &depthSrvDesc, depthSrvHandle);

				if (nowWidth > 1)
					nowWidth /= 2;
				if (nowHeight > 1)
					nowHeight /= 2;
				++occlusionCount;
			}

			nowWidth = width;
			nowHeight = height;
			for (uint i = 0; i < occlusionCount; ++i)
			{
				D3D12_RESOURCE_DESC debugDesc;
				ZeroMemory(&debugDesc, sizeof(D3D12_RESOURCE_DESC));
				debugDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				debugDesc.Alignment = 0;
				debugDesc.Width = nowWidth;
				debugDesc.Height = nowHeight;
				debugDesc.DepthOrArraySize = 1;
				debugDesc.MipLevels = 1;
				debugDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				debugDesc.SampleDesc.Count = 1;
				debugDesc.SampleDesc.Quality = 0;
				debugDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				debugDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

				Microsoft::WRL::ComPtr<ID3D12Resource> newDebug;
				CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
				ThrowIfFailedHr(device->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&debugDesc,
					D3D12_RESOURCE_STATE_COMMON,
					nullptr,
					IID_PPV_ARGS(&newDebug)));
				occlusionDebug[i] = std::move(newDebug);

				CD3DX12_CPU_DESCRIPTOR_HANDLE debugSrvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvHeap->GetCPUDescriptorHandleForHeapStart());
				debugSrvHandle.Offset(GetSrvOcclusionDebugStart() + i, cbvSrvUavDescriptorSize);
				device->CreateShaderResourceView(occlusionDebug[i].Get(), &debugSrvDesc, debugSrvHandle);

				CD3DX12_CPU_DESCRIPTOR_HANDLE debugUavHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvHeap->GetCPUDescriptorHandleForHeapStart());
				debugUavHandle.Offset(GetUavOcclusionDebugStart() + i, cbvSrvUavDescriptorSize);
				device->CreateUnorderedAccessView(occlusionDebug[i].Get(), nullptr, &uavDesc, debugUavHandle);

				if (nowWidth > 1)
					nowWidth /= 2;
				if (nowHeight > 1)
					nowHeight /= 2;
				++occlusionDebugCount;
			}
		}
		void JGraphicResourceManager::CreateDepthStencilResource(ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList,
			const uint viewWidth,
			const uint viewHeight,
			bool m4xMsaaState,
			uint m4xMsaaQuality)
		{
			mainDepthStencil.Reset();
			D3D12_RESOURCE_DESC depthStencilDesc;
			depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			depthStencilDesc.Alignment = 0;
			depthStencilDesc.Width = (uint)viewWidth;
			depthStencilDesc.Height = (uint)viewHeight;
			depthStencilDesc.DepthOrArraySize = 1;
			depthStencilDesc.MipLevels = 1;
			depthStencilDesc.Format = depthStencilFormat;
			depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
			depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
			depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			CD3DX12_CLEAR_VALUE optClear(DXGI_FORMAT_R16G16B16A16_FLOAT, DirectX::Colors::Snow);
			optClear.Format = depthStencilFormat;
			optClear.DepthStencil.Depth = 1.0f;
			optClear.DepthStencil.Stencil = 0;
			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);

			ThrowIfFailedHr(device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&depthStencilDesc,
				D3D12_RESOURCE_STATE_COMMON,
				&optClear,
				IID_PPV_ARGS(&mainDepthStencil)));

			device->CreateDepthStencilView(mainDepthStencil.Get(), nullptr, dsvHeap->GetCPUDescriptorHandleForHeapStart());

			CD3DX12_RESOURCE_BARRIER dsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mainDepthStencil.Get(),
				D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			commandList->ResourceBarrier(1, &dsBarrier);
		}
		void JGraphicResourceManager::Clear()
		{
			rtvHeap.Reset();
			dsvHeap.Reset();
			srvHeap.Reset();
			occlusionQueryHeap.Reset();
			for (int i = 0; i < swapChainBufferCount; ++i)
				swapChainBuffer[i].Reset();

			user2DTextureHandle.clear();
			user2DTextureResouce.clear();
			userCubeMapHandle.clear();
			userCubeMapResouce.clear();
			renderResultHandle.clear();
			renderResultResource.clear();
			shadowMapHandle.clear();
			shadowMapResource.clear();
			occlusionDepthMap.clear();
			occlusionDebug.clear();
			occlsionDepthHandle.clear();
			occlsionDebugHandle.clear();
			occlusionQueryResult.Reset();			
			mainDepthStencil.Reset();
			 
			occlusionCount = occlusionDebugCount = 0;
			uavCount = shadowMapCount = renderResultCount = userCubeMapCount = user2DTextureCount = 0;
		}
		JGraphicResourceManager::JGraphicResourceManager() {}
		JGraphicResourceManager::~JGraphicResourceManager() {}
	}
}