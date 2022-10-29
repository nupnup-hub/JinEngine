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
		uint JGraphicResourceManager::GetSrvUser2DTextureStart()const noexcept
		{
			return srvFixedCount;
		}
		uint JGraphicResourceManager::GetSrvUserCubeTextureStart()const noexcept
		{
			return GetSrvUser2DTextureStart() + user2DTextureCapacity;
		}
		uint JGraphicResourceManager::GetSrvRenderResultStart()const noexcept
		{
			return GetSrvUserCubeTextureStart() + userCubeTextureCapacity;
		}
		uint JGraphicResourceManager::GetSrvShadowMapStart()const noexcept
		{
			return GetSrvRenderResultStart() + renderResultCapacity;
		}
		uint JGraphicResourceManager::GetRtvRenderResultStart()const noexcept
		{
			return swapChainBuffercount;
		}
		uint JGraphicResourceManager::GetRtvShadowMapStart()const noexcept
		{
			return GetRtvRenderResultStart() + renderResultCapacity;
		}
		uint JGraphicResourceManager::GetDsvShadowMapStart()const noexcept
		{
			return 1;
		}
		uint JGraphicResourceManager::GetTotalRtvCount()const noexcept
		{
			return swapChainBuffercount + renderResultCount + shadowMapCount;
		}
		uint JGraphicResourceManager::GetTotalRtvCapacity()const noexcept
		{
			return swapChainBuffercount + renderResultCapacity + shadowMapCapacity;
		}
		uint JGraphicResourceManager::GetTotalDsvCount()const noexcept
		{
			return 1 + shadowMapCount;
		}
		uint JGraphicResourceManager::GetTotalDsvCapacity()const noexcept
		{
			return 1 + shadowMapCapacity;
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
			return user2DTextureCount + userCubeTextureCount + renderResultCount + shadowMapCount;
		}
		uint JGraphicResourceManager::GetTotalTextureCapacity()const noexcept
		{
			return user2DTextureCapacity + userCubeTextureCapacity + renderResultCapacity + shadowMapCapacity;
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
		void JGraphicResourceManager::BuildDepthStencilResource(ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList,
			const uint viewWidth,
			const uint viewHeight,
			bool m4xMsaaState,
			uint m4xMsaaQuality)
		{
			depthStencil.Reset();
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
				IID_PPV_ARGS(&depthStencil)));

			device->CreateDepthStencilView(depthStencil.Get(), nullptr, dsvHeap->GetCPUDescriptorHandleForHeapStart());

			CD3DX12_RESOURCE_BARRIER dsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(depthStencil.Get(),
				D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			commandList->ResourceBarrier(1, &dsBarrier);
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
		JGraphicTextureHandle* JGraphicResourceManager::CreateCubeTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap,
			const std::wstring& path,
			const std::wstring& oriFormat,
			ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> newTexture;
			JGraphicTextureHandle* handleCash = nullptr;
			uint heapIndex = GetSrvUserCubeTextureStart() + userCubeTextureCount;
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
				SetGraphicBuffIndex(*newHandle, userCubeTextureCount);
				handleCash = newHandle.get();

				userCubeTextureHandle.push_back(std::move(newHandle));
				userCubeTextureResouce.push_back(std::move(newTexture));
				++userCubeTextureCount;
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

			D3D12_SHADER_RESOURCE_VIEW_DESC rsSrvDesc = {};
			rsSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			rsSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			rsSrvDesc.Texture2D.MostDetailedMip = 0;
			rsSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			rsSrvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			rsSrvDesc.Texture2D.MipLevels = 1;

			D3D12_RESOURCE_DESC rsResourceDesc;
			ZeroMemory(&rsResourceDesc, sizeof(D3D12_RESOURCE_DESC));
			rsResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			rsResourceDesc.Alignment = 0;
			rsResourceDesc.Width = width;
			rsResourceDesc.Height = height;
			rsResourceDesc.DepthOrArraySize = 1;
			rsResourceDesc.MipLevels = 1;
			rsResourceDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			rsResourceDesc.SampleDesc.Count = 1;
			rsResourceDesc.SampleDesc.Quality = 0;
			rsResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			rsResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		 
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap->GetCPUDescriptorHandleForHeapStart());
			rtvHandle.Offset(GetRtvRenderResultStart() + renderResultCount, rtvDescriptorSize);

			CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvHeap->GetCPUDescriptorHandleForHeapStart());
			srvHandle.Offset(GetSrvRenderResultStart() + renderResultCount, cbvSrvUavDescriptorSize);

			Microsoft::WRL::ComPtr<ID3D12Resource> newTexture;
			ThrowIfFailedHr(device->CreateCommittedResource(
				&heapProperty,
				D3D12_HEAP_FLAG_NONE,
				&rsResourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				&optClear,
				IID_PPV_ARGS(&newTexture)));

			device->CreateRenderTargetView(newTexture.Get(), &rtvDesc, rtvHandle);
			device->CreateShaderResourceView(newTexture.Get(), &rsSrvDesc, srvHandle);

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

			D3D12_RESOURCE_DESC shadowMapRsResourceDesc;
			ZeroMemory(&shadowMapRsResourceDesc, sizeof(D3D12_RESOURCE_DESC));
			shadowMapRsResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			shadowMapRsResourceDesc.Alignment = 0;
			shadowMapRsResourceDesc.Width = width;
			shadowMapRsResourceDesc.Height = height;
			shadowMapRsResourceDesc.DepthOrArraySize = 1;
			shadowMapRsResourceDesc.MipLevels = 1;
			shadowMapRsResourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			shadowMapRsResourceDesc.SampleDesc.Count = 1;
			shadowMapRsResourceDesc.SampleDesc.Quality = 0;
			shadowMapRsResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			shadowMapRsResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			D3D12_SHADER_RESOURCE_VIEW_DESC shadowMapRsSrvDesc = {};
			shadowMapRsSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			shadowMapRsSrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			shadowMapRsSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			shadowMapRsSrvDesc.Texture2D.MostDetailedMip = 0;
			shadowMapRsSrvDesc.Texture2D.MipLevels = 1;
			shadowMapRsSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			shadowMapRsSrvDesc.Texture2D.PlaneSlice = 0;

			// Create DSV to resource so we can render to the shadow map.
			D3D12_DEPTH_STENCIL_VIEW_DESC shadowMapRsDsvDesc;
			shadowMapRsDsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			shadowMapRsDsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			shadowMapRsDsvDesc.Format = depthStencilFormat;
			shadowMapRsDsvDesc.Texture2D.MipSlice = 0;

			CD3DX12_CLEAR_VALUE optClear(DXGI_FORMAT_R16G16B16A16_FLOAT, DirectX::Colors::Snow);
			optClear.Format = depthStencilFormat;
			optClear.DepthStencil.Depth = 1.0f;
			optClear.DepthStencil.Stencil = 0;

			CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart());
			dsvHandle.Offset(GetDsvShadowMapStart() + shadowMapCount, dsvDescriptorSize);

			CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvHeap->GetCPUDescriptorHandleForHeapStart());
			srvHandle.Offset(GetSrvShadowMapStart() + shadowMapCount, cbvSrvUavDescriptorSize);

			Microsoft::WRL::ComPtr<ID3D12Resource> newTexture;
			ThrowIfFailedHr(device->CreateCommittedResource(
				&heapProperty,
				D3D12_HEAP_FLAG_NONE,
				&shadowMapRsResourceDesc,
				D3D12_RESOURCE_STATE_COMMON,
				&optClear,
				IID_PPV_ARGS(&newTexture)));

			device->CreateDepthStencilView(newTexture.Get(), &shadowMapRsDsvDesc, dsvHandle);
			device->CreateShaderResourceView(newTexture.Get(), &shadowMapRsSrvDesc, srvHandle);

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
				userCubeTextureResouce[vIndex].Reset();
				userCubeTextureResouce.erase(userCubeTextureResouce.begin() + vIndex);
				userCubeTextureHandle.erase(userCubeTextureHandle.begin() + vIndex);
				--userCubeTextureCount;
				for (uint i = vIndex; i < userCubeTextureCount; ++i)
				{
					--userCubeTextureHandle[i]->srvHeapIndex;
					SetGraphicBuffIndex(*userCubeTextureHandle[i], GetGraphicBuffIndex(*userCubeTextureHandle[i]) - 1);
					ReBindCubeTexture(device, GetGraphicBuffIndex(*userCubeTextureHandle[i]), userCubeTextureHandle[i]->srvHeapIndex);
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
		void JGraphicResourceManager::ReBindCubeTexture(ID3D12Device* device, const uint resourceIndex, const uint heapIndex)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = userCubeTextureResouce[resourceIndex]->GetDesc().MipLevels;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			srvDesc.Format = userCubeTextureResouce[resourceIndex]->GetDesc().Format;

			device->CreateShaderResourceView(userCubeTextureResouce[resourceIndex].Get(), &srvDesc, GetCpuSrvDescriptorHandle(heapIndex));
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
			D3D12_SHADER_RESOURCE_VIEW_DESC shadowMapRsSrvDesc = {};
			shadowMapRsSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			shadowMapRsSrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			shadowMapRsSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			shadowMapRsSrvDesc.Texture2D.MostDetailedMip = 0;
			shadowMapRsSrvDesc.Texture2D.MipLevels = 1;
			shadowMapRsSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			shadowMapRsSrvDesc.Texture2D.PlaneSlice = 0;

			// Create DSV to resource so we can render to the shadow map.
			D3D12_DEPTH_STENCIL_VIEW_DESC shadowMapRsDsvDesc;
			shadowMapRsDsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			shadowMapRsDsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			shadowMapRsDsvDesc.Format = depthStencilFormat;
			shadowMapRsDsvDesc.Texture2D.MipSlice = 0;

			device->CreateDepthStencilView(shadowMapResource[resourceIndex].Get(), &shadowMapRsDsvDesc, GetCpuDsvDescriptorHandle(dsvHeapIndex));
			device->CreateShaderResourceView(shadowMapResource[resourceIndex].Get(), &shadowMapRsSrvDesc, GetCpuSrvDescriptorHandle(srvHeapIndex));
		}
		void JGraphicResourceManager::Clear()
		{
			rtvHeap.Reset();
			dsvHeap.Reset();
			srvHeap.Reset();
			for (int i = 0; i < swapChainBuffercount; ++i)
				swapChainBuffer[i].Reset();

			user2DTextureHandle.clear();
			user2DTextureResouce.clear();
			userCubeTextureHandle.clear();
			userCubeTextureResouce.clear();
			renderResultHandle.clear();
			renderResultResource.clear();
			shadowMapHandle.clear();
			shadowMapResource.clear();
			depthStencil.Reset();
		}
		JGraphicResourceManager::JGraphicResourceManager() {}
		JGraphicResourceManager::~JGraphicResourceManager() {}
	}
}