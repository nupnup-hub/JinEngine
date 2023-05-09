#include"JGraphicResourceManager.h"   
#include"JGraphicResourceInfo.h"
#include"../JGraphic.h"

#include"../../Utility/JLoadTextureFromFile.h"
#include"../../Utility/JCommonUtility.h"
#include"../../Core/Exception/JExceptionMacro.h"  
#include"../../../Lib/DirectX/DDSTextureLoader11.h" 
#include<DirectXColors.h>
//Debug
//#include<fstream>
//#include"../../Core/File/JFileIOHelper.h"

namespace JinEngine
{
	namespace Graphic
	{
		namespace Constants
		{
			static constexpr uint swapChainBufferCount = 2;
			static constexpr uint occlusionQueryHeapCapacity = 250;
			static constexpr uint occlusionMipmapViewCapacity = 10;
			static constexpr uint minOcclusionSize = 8;

			static uint GraphicResourceViewFixedCount(const J_GRAPHIC_BIND_TYPE bType)
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return 1;	//Imgui reserved
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					return 0;
				}
			}
			static uint InitGraphicResourceCapacity(const J_GRAPHIC_RESOURCE_TYPE type)
			{
				switch (type)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN:
					return swapChainBufferCount;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL:
					return 1;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG:
					return 1;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL:
					return 1;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP:
					return 1;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
					return 1;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_DEBUG_MAP:
					return 1;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D:
					return 1000;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE:
					return 250;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
					return 250;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
					return 250;
				default:
					return 0;
				}
			}
			static uint InitGraphicResourceViewCapacity(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)
			{
				const int initResourceCapacity = InitGraphicResourceCapacity(rType);
				switch (rType)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return initResourceCapacity;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return 0;
					default:
						break;
					}
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
						return initResourceCapacity;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
						return initResourceCapacity + 1;	//detph & stencil
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return 0;
					default:
						break;
					}
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
						return initResourceCapacity;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return initResourceCapacity;
					default:
						break;
					}
					break;
				}

				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
						return initResourceCapacity;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return 0;
					default:
						break;
					}
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
						return initResourceCapacity;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
						return initResourceCapacity;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return 0;
					default:
						break;
					}
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
						return 10;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return 10;
					default:
						break;
					}
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_DEBUG_MAP:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
						return 10;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return 10;
					default:
						break;
					}
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
						return initResourceCapacity;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return 0;
					default:
						break;
					}
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
						return initResourceCapacity;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return 0;
					default:
						break;
					}
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return initResourceCapacity;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
						return initResourceCapacity;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return 0;
					default:
						break;
					}
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
						return initResourceCapacity;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
						return initResourceCapacity;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return 0;
					default:
						break;
					}
					break;
				}
				default:
				{
					break;
				}
				}
			}
			static std::wstring DefaultName(const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				switch (rType)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN:
					return L"Swap Chain";
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL:
					return L"Main Depth Stencil";
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG:
					return L"Main Depth Stencil Debug";
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL:
					return L"Editor Depth Stencil";
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP:
					return L"Occlusion Depth Map";
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
					return L"Occlusion Depth Mip Map";
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_DEBUG_MAP:
					return L"Occlusion Depth Debug Map";
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D:
					return L"Texture2D";
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE:
					return L"TextureCube";
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
					return L"RenderResult";
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
					return L"ShadowMap";
				default:
					return L"InValid Microsoft::WRL::ComPtr<ID3D12Resource>";
				}
			}
		}
		template<typename T, std::enable_if_t<std::is_base_of_v< ID3D12Pageable, T>, int> = 0>
		static void SetPrivateData(T* resource, const std::wstring& name)
		{
			resource->SetPrivateData(WKPDID_D3DDebugObjectNameW, name.size(), name.c_str());
		}
		template<typename T, std::enable_if_t<std::is_base_of_v< ID3D12Pageable, T>, int> = 0>
		static void SetPrivateData(T* resource, const std::wstring& name, int index)
		{
			std::wstring fName = name + L"_" + std::to_wstring(index);
			SetPrivateData(resource, fName);
		}

		void JGraphicResourceManager::ResourceViewInfo::ClearCount()
		{
			count = 0;
		}
		void JGraphicResourceManager::ResourceViewInfo::ClearAllData()
		{
			count = 0;
			capacity = 0;
			offset = 0;
		}
		int JGraphicResourceManager::ResourceViewInfo::GetNextViewIndex()const noexcept
		{
			return offset + count;
		}
		bool JGraphicResourceManager::ResourceViewInfo::HasSpace()const noexcept
		{
			return count < capacity;
		}
		void JGraphicResourceManager::ResourceTypeDesc::ClearCount()
		{
			count = 0;
			for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
				viewInfo[i].ClearCount();
		}
		void JGraphicResourceManager::ResourceTypeDesc::ClearAllData()
		{
			count = 0;
			capacity = 0;
			for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
				viewInfo[i].ClearAllData();
		}
		bool JGraphicResourceManager::ResourceTypeDesc::HasSpace()const noexcept
		{
			return count < capacity;
		}
		void JGraphicResourceManager::Initialize(ID3D12Device* device)
		{
			rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			cbvSrvUavDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				typeDesc[i].ClearAllData();

			int totalView[(uint)J_GRAPHIC_BIND_TYPE::COUNT];
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			{
				const J_GRAPHIC_RESOURCE_TYPE rType = (J_GRAPHIC_RESOURCE_TYPE)i;
				typeDesc[i].capacity = Constants::InitGraphicResourceCapacity(rType);

				for (uint j = 0; j < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++j)
				{
					if (j == (uint)J_GRAPHIC_BIND_TYPE::UAV)
						continue;

					const J_GRAPHIC_BIND_TYPE bType = (J_GRAPHIC_BIND_TYPE)j;
					typeDesc[i].viewInfo[j].capacity = Constants::InitGraphicResourceViewCapacity(rType, bType);
					if (i > 0)
					{
						typeDesc[i].viewInfo[j].offset = totalView[j];
						totalView[j] += typeDesc[i].viewInfo[j].capacity;
					}
					else
					{
						typeDesc[i].viewInfo[j].offset = Constants::GraphicResourceViewFixedCount(bType);
						totalView[j] = typeDesc[i].viewInfo[j].offset + typeDesc[i].viewInfo[j].capacity;
					}
				}
			}

			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			{
				const J_GRAPHIC_RESOURCE_TYPE rType = (J_GRAPHIC_RESOURCE_TYPE)i;
				const uint uavIndex = (uint)J_GRAPHIC_BIND_TYPE::UAV;
				typeDesc[i].viewInfo[uavIndex].capacity = Constants::InitGraphicResourceViewCapacity(rType, J_GRAPHIC_BIND_TYPE::UAV);
				if (i > 0)
				{
					typeDesc[i].viewInfo[uavIndex].offset = totalView[uavIndex];
					totalView[uavIndex] += typeDesc[i].viewInfo[uavIndex].capacity;
				}
				else
				{
					const uint srvIndex = (uint)J_GRAPHIC_BIND_TYPE::SRV;
					const uint lastResourceIndex = (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT - 1;

					typeDesc[i].viewInfo[uavIndex].offset = Constants::GraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::UAV) +
						typeDesc[lastResourceIndex].viewInfo[srvIndex].offset +
						typeDesc[lastResourceIndex].viewInfo[srvIndex].capacity;
					totalView[uavIndex] = typeDesc[i].viewInfo[uavIndex].offset + typeDesc[i].viewInfo[uavIndex].capacity;
				}
			}

			//Debug
			/*
			std::wofstream stream;
			stream.open(L"D:\\JinWooJung\\gDebug.txt", std::ios::app | std::ios::out);

			Core::JEnumInfo* rInfo = Core::JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_RESOURCE_TYPE).name());
			Core::JEnumInfo* bInfo = Core::JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_BIND_TYPE).name());
			auto rNameVec = rInfo->GetEnumNameVec();
			auto bNameVec = bInfo->GetEnumNameVec();

			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			{
				JFileIOHelper::StoreJString(stream, L"RType:", JCUtil::StrToWstr(rNameVec[i]));
				for (uint j = 0; j < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++j)
				{
					JFileIOHelper::StoreJString(stream, L"BType:", JCUtil::StrToWstr(bNameVec[j]));
					JFileIOHelper::StoreAtomicData(stream, L"Count:", typeDesc[i].viewInfo[j].count);
					JFileIOHelper::StoreAtomicData(stream, L"Capacity:", typeDesc[i].viewInfo[j].capacity);
					JFileIOHelper::StoreAtomicData(stream, L"Offset:", typeDesc[i].viewInfo[j].offset);
				}
				JFileIOHelper::InputSpace(stream, 1);
			}
			JFileIOHelper::InputSpace(stream, 2);
			stream.close();
			*/

			BuildRtvDescriptorHeaps(device);
			BuildDsvDescriptorHeaps(device);
			BuildSrvDescriptorHeaps(device);
			BuildOccQueryHeaps(device);
		}
		void JGraphicResourceManager::Clear()
		{
			rtvHeap.Reset();
			dsvHeap.Reset();
			srvHeap.Reset();
			occlusionQueryHeap.Reset();
			occlusionQueryResult.Reset();
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			{
				resource[i].clear();
				typeDesc[i].ClearAllData();
			}
		}
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
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicResourceManager::GetFirstGpuSrvDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return GetGpuSrvDescriptorHandle(typeDesc[(int)rType].viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].offset);
		}
		ID3D12DescriptorHeap* JGraphicResourceManager::GetDescriptorHeap(const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			switch (bType)
			{
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
				return rtvHeap.Get();
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
				return dsvHeap.Get();
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
				return srvHeap.Get();
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
				return srvHeap.Get();
			default:
				return nullptr;
			}
		}
		uint JGraphicResourceManager::GetDescriptorSize(const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			switch (bType)
			{
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
				return rtvDescriptorSize;
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
				return dsvDescriptorSize;
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
				return cbvSrvUavDescriptorSize;
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
				return cbvSrvUavDescriptorSize;
			default:
				return 0;
			}
		}
		DXGI_FORMAT JGraphicResourceManager::GetBackBufferFormat()const noexcept
		{
			return backBufferFormat;
		}
		DXGI_FORMAT JGraphicResourceManager::GetDepthStencilFormat()const noexcept
		{
			return depthStencilFormat;
		}
		DirectX::XMVECTORF32 JGraphicResourceManager::GetBackBufferClearColor()const noexcept
		{
			return DirectX::Colors::DimGray;
		}
		uint JGraphicResourceManager::GetSwapChainBufferCount()noexcept
		{
			return  Constants::swapChainBufferCount;
		}
		uint JGraphicResourceManager::GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return typeDesc[(int)rType].count;
		}
		uint JGraphicResourceManager::GetResourceCapacity(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return typeDesc[(int)rType].capacity;
		}
		uint JGraphicResourceManager::GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return typeDesc[(int)rType].viewInfo[(int)bType].count;
		}
		uint JGraphicResourceManager::GetViewOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return typeDesc[(int)rType].viewInfo[(int)bType].offset;
		}
		uint JGraphicResourceManager::GetTotalViewCount(const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			uint total = 0;
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				total += typeDesc[i].viewInfo[(int)bType].count;
			return total;
		}
		uint JGraphicResourceManager::GetTotalViewCapacity(const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			uint total = 0;
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				total += typeDesc[i].viewInfo[(int)bType].capacity;
			return total;
		}
		ID3D12Resource* JGraphicResourceManager::GetResource(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept
		{
			return resource[(int)rType][index]->resource.Get();
		}
		JGraphicResourceInfo* JGraphicResourceManager::GetInfo(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept
		{
			return resource[(int)rType][index].Get();
		}
		uint JGraphicResourceManager::GetOcclusionMipMapViewCapacity()noexcept
		{
			return Constants::occlusionMipmapViewCapacity;
		}
		uint JGraphicResourceManager::GetOcclusionMinSize()noexcept
		{
			return Constants::minOcclusionSize;
		}
		uint JGraphicResourceManager::GetOcclusionQueryCapacity()const noexcept
		{
			return Constants::occlusionQueryHeapCapacity;
		}
		ID3D12QueryHeap* JGraphicResourceManager::GetOcclusionQueryHeap()const noexcept
		{
			return occlusionQueryHeap.Get();
		}
		ID3D12Resource* JGraphicResourceManager::GetOcclusionQueryResult()const noexcept
		{
			return occlusionQueryResult.Get();
		}
		uint JGraphicResourceManager::GetHeapIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)
		{
			return typeDesc[(int)rType].viewInfo[(int)bType].count + typeDesc[(int)rType].viewInfo[(int)bType].offset;
		}
		void JGraphicResourceManager::CreateSwapChainBuffer(ID3D12Device* device, IDXGISwapChain* swapChain, const uint width, const uint height)
		{
			JGraphicResourceManager::ResourceTypeDesc& swapChainDesc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN];
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN].clear();
			swapChainDesc.ClearCount();

			ThrowIfFailedHr(swapChain->ResizeBuffers(
				Constants::swapChainBufferCount,
				width, height,
				backBufferFormat,
				DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

			resource[(int)J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN].resize(Constants::swapChainBufferCount);
			for (uint i = 0; i < Constants::swapChainBufferCount; ++i)
			{
				Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
				ThrowIfFailedHr(swapChain->GetBuffer(i, IID_PPV_ARGS(&newResource)));
				SetPrivateData(newResource.Get(), Constants::DefaultName(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN), i);

				auto newInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN, std::move(newResource));
				newInfo->SetArrayIndex(i);
				resource[(int)J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN][i] = std::move(newInfo);

				++swapChainDesc.count;
				BindSwapChain(device, i);
			}
		}
		void JGraphicResourceManager::CreateMainDepthStencilResource(ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList,
			const uint viewWidth,
			const uint viewHeight,
			bool m4xMsaaState,
			uint m4xMsaaQuality)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> newMainDsResource;
			Microsoft::WRL::ComPtr<ID3D12Resource> newMainDsDebugResource;

			JGraphicResourceManager::ResourceTypeDesc& mainDsDesc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL];
			JGraphicResourceManager::ResourceTypeDesc& mainDsDebugDesc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG];

			resource[(int)J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL].clear();
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG].clear();
			mainDsDesc.ClearCount();
			mainDsDebugDesc.ClearCount();

			const bool isInValidMainDs = !mainDsDesc.HasSpace() || !mainDsDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].HasSpace() || !mainDsDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace();
			const bool isInValidMDs = !mainDsDebugDesc.HasSpace() || !mainDsDebugDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace() || !mainDsDebugDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].HasSpace();
			if (isInValidMainDs || isInValidMDs)
				return;

			D3D12_RESOURCE_DESC depthStencilDesc;
			depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			depthStencilDesc.Alignment = 0;
			depthStencilDesc.Width = (uint)viewWidth;
			depthStencilDesc.Height = (uint)viewHeight;
			depthStencilDesc.DepthOrArraySize = 1;
			depthStencilDesc.MipLevels = 1;
			depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
			depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
			depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			CD3DX12_CLEAR_VALUE optClear;
			optClear.Format = depthStencilFormat;
			optClear.DepthStencil.Depth = 1.0f;
			optClear.DepthStencil.Stencil = 0;
			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);

			ThrowIfFailedHr(device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&depthStencilDesc,
				D3D12_RESOURCE_STATE_DEPTH_READ,
				&optClear,
				IID_PPV_ARGS(&newMainDsResource)));
			SetPrivateData(newMainDsResource.Get(), Constants::DefaultName(J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL), mainDsDesc.count);

			auto newMainDsInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL, std::move(newMainDsResource));
			newMainDsInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL].size());
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL].push_back(std::move(newMainDsInfo));
			++mainDsDesc.count;

			BindMainDepthStencil(device, resource[(int)J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL].size() - 1);

			D3D12_RESOURCE_DESC debugDesc;
			ZeroMemory(&debugDesc, sizeof(D3D12_RESOURCE_DESC));
			debugDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			debugDesc.Alignment = 0;
			debugDesc.Width = (uint)viewWidth;
			debugDesc.Height = (uint)viewHeight;
			debugDesc.DepthOrArraySize = 1;
			debugDesc.MipLevels = 1;
			debugDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
			debugDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
			debugDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			debugDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			debugDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			ThrowIfFailedG(device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&debugDesc,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				nullptr,
				IID_PPV_ARGS(&newMainDsDebugResource)));
			SetPrivateData(newMainDsDebugResource.Get(), Constants::DefaultName(J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG), mainDsDebugDesc.count);

			auto newMainDsDebugInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG, std::move(newMainDsDebugResource));
			newMainDsDebugInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG].size());
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG].push_back(std::move(newMainDsDebugInfo));
			++mainDsDebugDesc.count;

			BindMainDepthStencilDebug(device, resource[(int)J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG].size() - 1);
		}
		void JGraphicResourceManager::CreateDebugDepthStencilResource(ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList,
			const uint viewWidth,
			const uint viewHeight)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> newDebugDsResource;
			JGraphicResourceManager::ResourceTypeDesc& editorDsDesc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL];

			resource[(int)J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL].clear();
			editorDsDesc.ClearCount();

			const bool isInValidEditorDs = !editorDsDesc.HasSpace() || !editorDsDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].HasSpace();
			if (isInValidEditorDs)
				return;

			D3D12_RESOURCE_DESC depthStencilDesc;
			depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			depthStencilDesc.Alignment = 0;
			depthStencilDesc.Width = (uint)viewWidth;
			depthStencilDesc.Height = (uint)viewHeight;
			depthStencilDesc.DepthOrArraySize = 1;
			depthStencilDesc.MipLevels = 1;
			depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			depthStencilDesc.SampleDesc.Count = 1;
			depthStencilDesc.SampleDesc.Quality = 0;
			depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			CD3DX12_CLEAR_VALUE optClear;
			optClear.Format = depthStencilFormat;
			optClear.DepthStencil.Depth = 1.0f;
			optClear.DepthStencil.Stencil = 0;
			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
			ThrowIfFailedHr(device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&depthStencilDesc,
				D3D12_RESOURCE_STATE_DEPTH_READ,
				&optClear,
				IID_PPV_ARGS(&newDebugDsResource)));
			SetPrivateData(newDebugDsResource.Get(), Constants::DefaultName(J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL), editorDsDesc.count);

			auto newMainDsDebugInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL, std::move(newDebugDsResource));
			newMainDsDebugInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL].size());
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL].push_back(std::move(newMainDsDebugInfo));
			++editorDsDesc.count;

			BindDebugDepthStencil(device, resource[(int)J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL].size() - 1);
		}
		void JGraphicResourceManager::CreateOcclusionQueryResource(ID3D12Device* device)
		{
			CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
			auto queryResultDesc = CD3DX12_RESOURCE_DESC::Buffer(8 * Constants::occlusionQueryHeapCapacity);

			ThrowIfFailedG(device->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&queryResultDesc,
				D3D12_RESOURCE_STATE_PREDICATION,
				nullptr,
				IID_PPV_ARGS(&occlusionQueryResult)));
			SetPrivateData(occlusionQueryResult.Get(), L"QueryResult");
		}
		void JGraphicResourceManager::CreateOcclusionHZBResource(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const uint occWidth, const uint occHeight)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> occDsResource;
			Microsoft::WRL::ComPtr<ID3D12Resource> occMipMapResource;
			Microsoft::WRL::ComPtr<ID3D12Resource> occDebugResource;

			JGraphicResourceManager::ResourceTypeDesc& occDsDesc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP];
			JGraphicResourceManager::ResourceTypeDesc& occMipMapDesc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP];
			JGraphicResourceManager::ResourceTypeDesc& occDebugDesc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_DEBUG_MAP];

			const bool isInValidOccDs = !occDsDesc.HasSpace() || !occDsDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].HasSpace() || !occDsDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace();
			const bool isInValidOccMipMap = !occMipMapDesc.HasSpace() || !occMipMapDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace() || !occMipMapDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].HasSpace();
			const bool isInValidOccDebug = !occDebugDesc.HasSpace() || !occDebugDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace() || !occDebugDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].HasSpace();
			if (isInValidOccDs || isInValidOccMipMap || isInValidOccDebug)
				return;

			D3D12_RESOURCE_DESC depthStencilDesc;
			ZeroMemory(&depthStencilDesc, sizeof(D3D12_RESOURCE_DESC));
			depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			depthStencilDesc.Alignment = 0;
			depthStencilDesc.Width = occWidth;
			depthStencilDesc.Height = occHeight;
			depthStencilDesc.DepthOrArraySize = 1;
			depthStencilDesc.MipLevels = 1;
			depthStencilDesc.SampleDesc.Count = 1;
			depthStencilDesc.SampleDesc.Quality = 0;
			depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			depthStencilDesc.Format = DXGI_FORMAT_R32_TYPELESS;

			float optColor[4] = { 1.0f, 0, 0, 0 };
			CD3DX12_CLEAR_VALUE optClear(DXGI_FORMAT_D32_FLOAT, optColor);
			//optClear.Format = depthStencilFormat;
			optClear.DepthStencil.Depth = 1.0f;
			optClear.DepthStencil.Stencil = 0;

			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
			ThrowIfFailedG(device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&depthStencilDesc,
				D3D12_RESOURCE_STATE_DEPTH_READ,
				&optClear,
				IID_PPV_ARGS(&occDsResource)));
			SetPrivateData(occDsResource.Get(), Constants::DefaultName(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP), occDsDesc.count);

			auto occDsInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, std::move(occDsResource));
			occDsInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP].size());
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP].push_back(std::move(occDsInfo));
			++occDsDesc.count;

			BindOcclusionHZBDepthMap(device, resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP].size() - 1);

			D3D12_RESOURCE_DESC mipMapDesc;
			ZeroMemory(&mipMapDesc, sizeof(D3D12_RESOURCE_DESC));
			mipMapDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			mipMapDesc.Alignment = 0;
			mipMapDesc.Width = occWidth;
			mipMapDesc.Height = occHeight;
			mipMapDesc.DepthOrArraySize = 1;
			mipMapDesc.MipLevels = 0;
			mipMapDesc.SampleDesc.Count = 1;
			mipMapDesc.SampleDesc.Quality = 0;
			mipMapDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			mipMapDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			mipMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			ThrowIfFailedG(device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&mipMapDesc,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				nullptr,
				IID_PPV_ARGS(&occMipMapResource)));
			SetPrivateData(occMipMapResource.Get(), Constants::DefaultName(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP), occMipMapDesc.count);

			auto occMipMapInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP, std::move(occMipMapResource));
			occMipMapInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP].size());
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP].push_back(std::move(occMipMapInfo));
			++occMipMapDesc.count;

			BindOcclusionHZBDepthMipMap(device, resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP].size() - 1);

			D3D12_RESOURCE_DESC debugDesc;
			ZeroMemory(&debugDesc, sizeof(D3D12_RESOURCE_DESC));
			debugDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			debugDesc.Alignment = 0;
			debugDesc.Width = occWidth;
			debugDesc.Height = occHeight;
			debugDesc.DepthOrArraySize = 1;
			debugDesc.MipLevels = 0;
			debugDesc.SampleDesc.Count = 1;
			debugDesc.SampleDesc.Quality = 0;
			debugDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			debugDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			debugDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			ThrowIfFailedG(device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&debugDesc,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				nullptr,
				IID_PPV_ARGS(&occDebugResource)));
			SetPrivateData(occDebugResource.Get(), Constants::DefaultName(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_DEBUG_MAP), occDebugDesc.count);

			auto occDebugInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_DEBUG_MAP, std::move(occDebugResource));
			occDebugInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_DEBUG_MAP].size());
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_DEBUG_MAP].push_back(std::move(occDebugInfo));
			++occDebugDesc.count;

			BindOcclusionHZBDebug(device, resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_DEBUG_MAP].size() - 1);
		}
		JUserPtr<JGraphicResourceInfo> JGraphicResourceManager::Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer,
			const std::wstring& path,
			const std::wstring& oriFormat,
			ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList)
		{
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D];
			if (!desc.HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace())
				return JUserPtr<JGraphicResourceInfo>{};

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			uint heapIndex = GetHeapIndex(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, J_GRAPHIC_BIND_TYPE::SRV);
			bool res = false;
			if (oriFormat == L".dds")
			{
				res = DirectX::CreateDDSTextureFromFile12(device,
					commandList,
					path.c_str(),
					newResource,
					uploadBuffer) == S_OK;
			}
			else
			{
				res = LoadTextureFromFile(JCUtil::WstrToU8Str(path).c_str(),
					device,
					GetCpuSrvDescriptorHandle(heapIndex),
					newResource,
					uploadBuffer);
			}
			if (res)
			{
				std::wstring folder;
				std::wstring name;
				std::wstring format;
				JCUtil::DecomposeFilePath(path, folder, name, format);
				SetPrivateData(newResource.Get(), name + format, desc.count);

				auto newInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, std::move(newResource));
				newInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].size());
				resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].push_back(std::move(newInfo));
				++desc.count;

				const uint resourceIndex = resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].size() - 1;
				Bind2DTexture(device, resourceIndex);
				return resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D][resourceIndex];
			}
			else
				return JUserPtr<JGraphicResourceInfo>{};
		}
		JUserPtr<JGraphicResourceInfo> JGraphicResourceManager::CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer,
			const std::wstring& path,
			const std::wstring& oriFormat,
			ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList)
		{
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE];
			if (!desc.HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace())
				return JUserPtr<JGraphicResourceInfo>{};

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			uint heapIndex = GetHeapIndex(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, J_GRAPHIC_BIND_TYPE::SRV);
			bool res = false;

			if (oriFormat == L".dds")
			{
				res = DirectX::CreateDDSTextureFromFile12(device,
					commandList,
					path.c_str(),
					newResource,
					uploadBuffer) == S_OK;
			}
			else
				assert(L"InValid cube map format");
			if (res)
			{
				std::wstring folder;
				std::wstring name;
				std::wstring format;
				JCUtil::DecomposeFilePath(path, folder, name, format);
				SetPrivateData(newResource.Get(), name + format, desc.count);

				auto newInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, std::move(newResource));
				newInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].size());
				resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].push_back(std::move(newInfo));
				++desc.count;

				const uint resourceIndex = resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].size() - 1;
				BindCubeMap(device, resourceIndex);
				return resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE][resourceIndex];
			}
			else
				return JUserPtr<JGraphicResourceInfo>{};
		}
		JUserPtr<JGraphicResourceInfo> JGraphicResourceManager::CreateRenderTargetTexture(ID3D12Device* device, const uint width, const uint height)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON];
			if (!desc.HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::RTV].HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace())
				return JUserPtr<JGraphicResourceInfo>{};

			D3D12_RESOURCE_DESC resourceDesc;
			ZeroMemory(&resourceDesc, sizeof(D3D12_RESOURCE_DESC));
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resourceDesc.Alignment = 0;
			resourceDesc.Width = width;
			resourceDesc.Height = height;
			resourceDesc.DepthOrArraySize = 1;
			resourceDesc.MipLevels = 1;
			resourceDesc.Format = backBufferFormat;
			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

			//DXGI_FORMAT_R8G8B8A8_UNORM
			CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_CLEAR_VALUE optClear(backBufferFormat, GetBackBufferClearColor());
			ThrowIfFailedG(device->CreateCommittedResource(
				&heapProperty,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				&optClear,
				IID_PPV_ARGS(&newResource)));
			SetPrivateData(newResource.Get(), Constants::DefaultName(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON), desc.count);

			auto newInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, std::move(newResource));
			newInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON].size());
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON].push_back(std::move(newInfo));
			++desc.count;

			const uint resourceIndex = resource[(int)J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON].size() - 1;
			BindRenderTarget(device, resourceIndex);
			return resource[(int)J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON][resourceIndex];
		}
		JUserPtr<JGraphicResourceInfo> JGraphicResourceManager::CreateShadowMapTexture(ID3D12Device* device, const uint width, const uint height)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP];
			if (!desc.HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace())
				return JUserPtr<JGraphicResourceInfo>{};

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

			CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_CLEAR_VALUE optClear;
			optClear.Format = depthStencilFormat;
			optClear.DepthStencil.Depth = 1.0f;
			optClear.DepthStencil.Stencil = 0;

			ThrowIfFailedHr(device->CreateCommittedResource(
				&heapProperty,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COMMON,
				&optClear,
				IID_PPV_ARGS(&newResource)));
			SetPrivateData(newResource.Get(), Constants::DefaultName(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP), desc.count);

			auto newInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, std::move(newResource));
			newInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].size());
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].push_back(std::move(newInfo));
			++desc.count;

			const uint resourceIndex = resource[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].size() - 1;
			BindShadowMap(device, resourceIndex);
			return resource[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP][resourceIndex];
		}
		bool JGraphicResourceManager::DestroyGraphicTextureResource(ID3D12Device* device, JGraphicResourceInfo* info)
		{
			if (info == nullptr)
				return false;

			const J_GRAPHIC_RESOURCE_TYPE rType = info->GetGraphicResourceType();
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];
			BindViewPtr ptr = GetResourceBindViewPtr(rType);

			for (uint i = 0; i < (int)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
			{
				//바인딩 되어있지 않을시 count < 0
				//heapIndexStart는 바인딩되지 않을 경우 기본값인 -1을 가지고있는 상태이다.
				//graphic resource 타입마다 binding 유형이 정해지기 때문에 유효하다

				desc.viewInfo[i].count = info->GetHeapIndexStart((J_GRAPHIC_BIND_TYPE)i) - desc.viewInfo[i].offset;
				if (desc.viewInfo[i].count < 0)
					desc.viewInfo[i].count = 0;
			}
			const uint rIndex = info->GetArrayIndex();
			//resource[(int)rType][rIndex].Clear();
			for (uint i = rIndex + 1; i < desc.count; ++i)
			{
				JGraphicResourceInfo* reBindHandle = resource[(int)rType][i].Get();
				for (uint j = 0; j < (int)J_GRAPHIC_BIND_TYPE::COUNT; ++j)
				{
					reBindHandle->SetHeapIndexStart((J_GRAPHIC_BIND_TYPE)j, -1);
					reBindHandle->SetViewCount((J_GRAPHIC_BIND_TYPE)j, 0);
				}
				reBindHandle->SetArrayIndex(i - 1);
				(this->*ptr)(device, i);
			}
			resource[(int)rType].erase(resource[(int)rType].begin() + rIndex);
			--desc.count;
			return true;
		}
		JOwnerPtr<JGraphicResourceInfo> JGraphicResourceManager::CreateResourceInfo(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, Microsoft::WRL::ComPtr<ID3D12Resource>&& resource)
		{
			return JOwnerPtr<JGraphicResourceInfo>(new JGraphicResourceInfo(this, graphicResourceType, std::move(resource)));
		}
		JGraphicResourceManager::BindViewPtr JGraphicResourceManager::GetResourceBindViewPtr(const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			switch (rType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN:
				return &JGraphicResourceManager::BindSwapChain;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL:
				return &JGraphicResourceManager::BindMainDepthStencil;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG:
				return &JGraphicResourceManager::BindMainDepthStencilDebug;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL:
				return &JGraphicResourceManager::BindDebugDepthStencil;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP:
				return &JGraphicResourceManager::BindOcclusionHZBDepthMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
				return &JGraphicResourceManager::BindOcclusionHZBDepthMipMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_DEBUG_MAP:
				return &JGraphicResourceManager::BindOcclusionHZBDebug;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D:
				return &JGraphicResourceManager::Bind2DTexture;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE:
				return &JGraphicResourceManager::BindCubeMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
				return &JGraphicResourceManager::BindRenderTarget;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
				return &JGraphicResourceManager::BindShadowMap;
			default:
				return nullptr;
			}
		}
		void JGraphicResourceManager::SetViewCount(JGraphicResourceInfo* handlePtr, const J_GRAPHIC_BIND_TYPE bType, const int nextViewIndex)
		{
			if (handlePtr->GetViewCount(bType) == 0)
			{
				handlePtr->SetHeapIndexStart(bType, nextViewIndex);
				handlePtr->SetViewCount(bType, 1);
			}
			else
				handlePtr->SetViewCount(bType, handlePtr->GetViewCount(bType) + 1);
		}
		void JGraphicResourceManager::BindSwapChain(ID3D12Device* device, const uint resourceIndex)
		{
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN];

			ID3D12Resource* resourcePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN)][resourceIndex]->resource.Get();
			JGraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN)][resourceIndex].Get();
			const int nextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::RTV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::RTV, nextViewIndex);
			device->CreateRenderTargetView(resourcePtr, nullptr, GetCpuRtvDescriptorHandle(nextViewIndex));
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::RTV].count;
		}
		void JGraphicResourceManager::BindMainDepthStencil(ID3D12Device* device, const uint resourceIndex)
		{
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL];

			ID3D12Resource* resourcePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL)][resourceIndex]->resource.Get();
			JGraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL)][resourceIndex].Get();
			const int dsvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].GetNextViewIndex();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::DSV, dsvNextViewIndex);
			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);
			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Format = depthStencilFormat;
			dsvDesc.Texture2D.MipSlice = 0;

			device->CreateDepthStencilView(resourcePtr, &dsvDesc, GetCpuDsvDescriptorHandle(dsvNextViewIndex));

			D3D12_SHADER_RESOURCE_VIEW_DESC depthSrvDesc = {};
			depthSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			depthSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			depthSrvDesc.Texture2D.MostDetailedMip = 0;
			depthSrvDesc.Texture2D.MipLevels = 1;
			depthSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			depthSrvDesc.Texture2D.PlaneSlice = 0;
			depthSrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

			//Depth 
			device->CreateShaderResourceView(resourcePtr, &depthSrvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));

			////Stencil 
			depthSrvDesc.Texture2D.PlaneSlice = 1;
			depthSrvDesc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
			device->CreateShaderResourceView(resourcePtr, &depthSrvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex + 1));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].count;
			desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count += 2;
		}
		void JGraphicResourceManager::BindMainDepthStencilDebug(ID3D12Device* device, const uint resourceIndex)
		{
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG];

			ID3D12Resource* resourcePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG)][resourceIndex]->resource.Get();
			JGraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG)][resourceIndex].Get();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();
			const int uavNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);
			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::UAV, uavNextViewIndex);

			D3D12_SHADER_RESOURCE_VIEW_DESC debugSrvDesc = {};
			debugSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			debugSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			debugSrvDesc.Texture2D.MostDetailedMip = 0;
			debugSrvDesc.Texture2D.MipLevels = 1;
			debugSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			debugSrvDesc.Texture2D.PlaneSlice = 0;
			debugSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			D3D12_UNORDERED_ACCESS_VIEW_DESC debugUavDesc = {};
			debugUavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			debugUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			debugUavDesc.Texture2D.MipSlice = 0;

			device->CreateShaderResourceView(resourcePtr, &debugSrvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));
			device->CreateUnorderedAccessView(resourcePtr, nullptr, &debugUavDesc, GetCpuSrvDescriptorHandle(uavNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].count;
		}
		void JGraphicResourceManager::BindDebugDepthStencil(ID3D12Device* device, const uint resourceIndex)
		{
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL];

			ID3D12Resource* resourcePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL)][resourceIndex]->resource.Get();
			JGraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL)][resourceIndex].Get();
			const int dsvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::DSV, dsvNextViewIndex);

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Format = depthStencilFormat;
			dsvDesc.Texture2D.MipSlice = 0;

			device->CreateDepthStencilView(resourcePtr, &dsvDesc, GetCpuDsvDescriptorHandle(dsvNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].count;
		}
		void JGraphicResourceManager::BindOcclusionHZBDepthMap(ID3D12Device* device, const uint resourceIndex)
		{
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP];

			ID3D12Resource* resourcePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP)][resourceIndex]->resource.Get();
			JGraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP)][resourceIndex].Get();
			const int dsvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].GetNextViewIndex();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::DSV, dsvNextViewIndex);
			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.Texture2D.MipSlice = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC depthSrvDesc = {};
			depthSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			depthSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			depthSrvDesc.Texture2D.MostDetailedMip = 0;
			depthSrvDesc.Texture2D.MipLevels = 1;
			depthSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			depthSrvDesc.Texture2D.PlaneSlice = 0;
			depthSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;

			device->CreateDepthStencilView(resourcePtr, &dsvDesc, GetCpuDsvDescriptorHandle(dsvNextViewIndex));
			device->CreateShaderResourceView(resourcePtr, &depthSrvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].count;
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;
		}
		void JGraphicResourceManager::BindOcclusionHZBDepthMipMap(ID3D12Device* device, const uint resourceIndex)
		{
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP];

			ID3D12Resource* resourcePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP)][resourceIndex]->resource.Get();
			JGraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP)][resourceIndex].Get();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();
			const int uavNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].GetNextViewIndex();

			uint nowWidth = resourcePtr->GetDesc().Width;
			uint nowHeight = resourcePtr->GetDesc().Height;

			uint occlusionCount = 0;
			while (occlusionCount != Constants::occlusionMipmapViewCapacity &&
				(nowWidth >= Constants::minOcclusionSize && nowHeight >= Constants::minOcclusionSize))
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC mipMapSrvDesc = {};
				mipMapSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				mipMapSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				mipMapSrvDesc.Texture2D.MostDetailedMip = occlusionCount;
				mipMapSrvDesc.Texture2D.MipLevels = 1;
				mipMapSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				mipMapSrvDesc.Texture2D.PlaneSlice = 0;
				mipMapSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = occlusionCount;

				SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);
				SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::UAV, uavNextViewIndex);

				device->CreateShaderResourceView(resourcePtr, &mipMapSrvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex + occlusionCount));
				device->CreateUnorderedAccessView(resourcePtr, nullptr, &uavDesc, GetCpuSrvDescriptorHandle(uavNextViewIndex + occlusionCount));

				if (nowWidth > 1)
					nowWidth /= 2;
				if (nowHeight > 1)
					nowHeight /= 2;
				++occlusionCount;
			}
			desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count += occlusionCount;
			desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].count += occlusionCount;
		}
		void JGraphicResourceManager::BindOcclusionHZBDebug(ID3D12Device* device, const uint resourceIndex)
		{
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_DEBUG_MAP];

			ID3D12Resource* resourcePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_DEBUG_MAP)][resourceIndex]->resource.Get();
			JGraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_DEBUG_MAP)][resourceIndex].Get();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();
			const int uavNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].GetNextViewIndex();

			uint nowWidth = resourcePtr->GetDesc().Width;
			uint nowHeight = resourcePtr->GetDesc().Height;

			uint occlusionCount = 0;
			while (occlusionCount != Constants::occlusionMipmapViewCapacity &&
				(nowWidth >= Constants::minOcclusionSize && nowHeight >= Constants::minOcclusionSize))
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC debugSrvDesc = {};
				debugSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				debugSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				debugSrvDesc.Texture2D.MostDetailedMip = occlusionCount;
				debugSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				debugSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				debugSrvDesc.Texture2D.MipLevels = 1;

				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = occlusionCount;

				SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);
				SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::UAV, uavNextViewIndex);

				device->CreateShaderResourceView(resourcePtr, &debugSrvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex + occlusionCount));
				device->CreateUnorderedAccessView(resourcePtr, nullptr, &uavDesc, GetCpuSrvDescriptorHandle(uavNextViewIndex + occlusionCount));

				if (nowWidth > 1)
					nowWidth /= 2;
				if (nowHeight > 1)
					nowHeight /= 2;
				++occlusionCount;
			}
			desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count += occlusionCount;
			desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].count += occlusionCount;
		}
		void JGraphicResourceManager::Bind2DTexture(ID3D12Device* device, const uint resourceIndex)
		{
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D];

			ID3D12Resource* resourcePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D)][resourceIndex]->resource.Get();
			JGraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D)][resourceIndex].Get();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));
			srvDesc.Format = resourcePtr->GetDesc().Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = resourcePtr->GetDesc().MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			device->CreateShaderResourceView(resourcePtr, &srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;
		}
		void JGraphicResourceManager::BindCubeMap(ID3D12Device* device, const uint resourceIndex)
		{
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE];

			ID3D12Resource* resourcePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)][resourceIndex]->resource.Get();
			JGraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)][resourceIndex].Get();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = resourcePtr->GetDesc().MipLevels;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			srvDesc.Format = resourcePtr->GetDesc().Format;

			device->CreateShaderResourceView(resourcePtr, &srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;
		}
		void JGraphicResourceManager::BindRenderTarget(ID3D12Device* device, const uint resourceIndex)
		{
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON];

			ID3D12Resource* resourcePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON)][resourceIndex]->resource.Get();
			JGraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON)][resourceIndex].Get();

			const int rtvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::RTV].GetNextViewIndex();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::RTV, rtvNextViewIndex);
			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Format = resourcePtr->GetDesc().Format;
			rtvDesc.Texture2D.MipSlice = 0;
			rtvDesc.Texture2D.PlaneSlice = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Format = resourcePtr->GetDesc().Format;
			srvDesc.Texture2D.MipLevels = 1;

			device->CreateRenderTargetView(resourcePtr, &rtvDesc, GetCpuRtvDescriptorHandle(rtvNextViewIndex));
			device->CreateShaderResourceView(resourcePtr, &srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::RTV].count;
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;
		}
		void JGraphicResourceManager::BindShadowMap(ID3D12Device* device, const uint resourceIndex)
		{
			JGraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP];

			ID3D12Resource* resourcePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP)][resourceIndex]->resource.Get();
			JGraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP)][resourceIndex].Get();

			const int dsvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].GetNextViewIndex();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::DSV, dsvNextViewIndex);
			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);

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

			device->CreateDepthStencilView(resourcePtr, &dsvDesc, GetCpuDsvDescriptorHandle(dsvNextViewIndex));
			device->CreateShaderResourceView(resourcePtr, &srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].count;
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;
		}
		void JGraphicResourceManager::BuildRtvDescriptorHeaps(ID3D12Device* device)
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
			rtvHeapDesc.NumDescriptors = GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::RTV) +
				Constants::GraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::RTV);
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			rtvHeapDesc.NodeMask = 0;
			ThrowIfFailedHr(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.GetAddressOf())));
			SetPrivateData(rtvHeap.Get(), L"RtvHeap");
		}
		void JGraphicResourceManager::BuildDsvDescriptorHeaps(ID3D12Device* device)
		{
			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
			dsvHeapDesc.NumDescriptors = GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::DSV) +
				Constants::GraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::DSV);
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			dsvHeapDesc.NodeMask = 0;
			ThrowIfFailedHr(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsvHeap.GetAddressOf())));
			SetPrivateData(dsvHeap.Get(), L"DsvHeap");
		}
		void JGraphicResourceManager::BuildSrvDescriptorHeaps(ID3D12Device* device)
		{
			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
			srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			srvHeapDesc.NumDescriptors = GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::SRV) +
				GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::UAV) +
				Constants::GraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::SRV) +
				Constants::GraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::UAV);
			srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			ThrowIfFailedHr(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(srvHeap.GetAddressOf())));
			SetPrivateData(srvHeap.Get(), L"Srv Heap");
		}
		void JGraphicResourceManager::BuildOccQueryHeaps(ID3D12Device* device)
		{
			occlusionQueryHeap.Reset();
			D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
			queryHeapDesc.Count = Constants::occlusionQueryHeapCapacity;
			queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
			ThrowIfFailedHr(device->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&occlusionQueryHeap)));
			SetPrivateData(occlusionQueryHeap.Get(), L"Occlusion Query Heap");
		}
		void JGraphicResourceManager::RegisterTypeData()
		{
			using JAllocationDesc = JinEngine::Core::JAllocationDesc;
			using NotifyReAllocPtr = JAllocationDesc::NotifyReAllocF::Ptr;
			using NotifyReAllocF = JAllocationDesc::NotifyReAllocF::Functor;
			using ReceiverPtr = JAllocationDesc::ReceiverPtr;
			using ReAllocatedPtr = JAllocationDesc::ReAllocatedPtr;
			using MemIndex = JAllocationDesc::MemIndex;

			NotifyReAllocPtr notifyPtr = [](ReceiverPtr receiver, ReAllocatedPtr movedPtr, MemIndex index)
			{
				JGraphicResourceInfo* movedInfo = static_cast<JGraphicResourceInfo*>(movedPtr);
				JGraphicResourceManager* manager = movedInfo->manager;

				manager->resource[(int)movedInfo->GetGraphicResourceType()][movedInfo->GetArrayIndex()].Release();
				manager->resource[(int)movedInfo->GetGraphicResourceType()][movedInfo->GetArrayIndex()].Reset(movedInfo);
			};
			auto reAllocF = std::make_unique<JAllocationDesc::NotifyReAllocF::Functor>(notifyPtr);
			std::unique_ptr<JAllocationDesc> desc = std::make_unique<JAllocationDesc>();

			desc->notifyReAllocB = UniqueBind(std::move(reAllocF), static_cast<ReceiverPtr>(nullptr), JinEngine::Core::empty, JinEngine::Core::empty);
			JGraphicResourceInfo::StaticTypeInfo().SetAllocationOption(std::move(desc));
		}
		JGraphicResourceManager::JGraphicResourceManager() {}
		JGraphicResourceManager::~JGraphicResourceManager() {}
	}
}

/*
//Debug
 /*
 #include<fstream>
#include"../../Core/File/JFileIOHelper.h"

		std::wofstream stream;
		stream.open(L"D:\\JinWooJung\\gDebug.txt", std::ios::app | std::ios::out);

		Core::JEnumInfo* rInfo = Core::JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_RESOURCE_TYPE).name());
		Core::JEnumInfo* bInfo = Core::JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_BIND_TYPE).name());
		auto rNameVec = rInfo->GetEnumNameVec();
		auto bNameVec = bInfo->GetEnumNameVec();
		JFileIOHelper::InputSpace(stream, 1);
 */