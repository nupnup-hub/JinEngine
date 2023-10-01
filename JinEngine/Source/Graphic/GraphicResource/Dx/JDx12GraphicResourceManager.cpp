#include"JDx12GraphicResourceManager.h"  
#include"JDx12GraphicResourceInfo.h"
#include"../Dx/JDx12GraphicResourceHolder.h"
#include"../JGraphicResourceInterface.h"
#include"../../Device/JGraphicDevice.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../Utility/JLoadTextureFromFile.h"
#include"../../Utility/JD3DUtility.h" 
#include"../../JGraphic.h"

#include"../../../Core/Utility/JCommonUtility.h"  
#include"../../../Core/Exception/JExceptionMacro.h"  
#include"../../../Object/Resource/Mesh/JMeshGeometry.h"   
#include"../../../Window/JWindow.h"
#include"../../../../ThirdParty/DirectX/DDSTextureLoader11.h" 
#include"../../../../ThirdParty/DirectX/TK/Inc/ScreenGrab.h" 
#include<DirectXColors.h>
//Debug
//#include<fstream>
//#include"../../Core/File/JFileIOHelper.h" 
namespace JinEngine
{
	namespace Graphic
	{
		namespace Private
		{
			struct DeviceData
			{
			public:
				JDx12GraphicDevice* dxDevice = nullptr;
				ID3D12Device* device = nullptr;
				ID3D12GraphicsCommandList* commandList = nullptr;
				uint m4xMsaaQuality = 0;
				bool m4xMsaaState = false;
				bool startCommandThisCreation = false;
			public:
				DeviceData(JGraphicDevice* device)
				{
					dxDevice = static_cast<JDx12GraphicDevice*>(device);
					DeviceData::device = dxDevice->GetDevice();
					commandList = dxDevice->GetPublicCmdList();
					m4xMsaaQuality = dxDevice->GetM4xMsaaQuality();
					m4xMsaaState = dxDevice->GetM4xMsaaState();
				}
			};

			static constexpr uint swapChainBufferCount = 2;
			static constexpr uint vertexCapacity = INT_MAX;	//mesh 갯수만큼 할당가능
			static constexpr uint indexCapacity = INT_MAX;		//mesh 갯수만큼 할당가능

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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL:
					return 256;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL:
					return 16;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG:
					return 256;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP:
					return 16 + 16; //(hzb + hd)
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
					return 16;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
					return 16;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D:
					return 1024;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE:
					return 128;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
					return 256;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
					return 128;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY:
					return 16;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE:
					return 128;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::VERTEX:
					return vertexCapacity;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::INDEX:
					return indexCapacity;
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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL:
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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL:
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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG:
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
						return initResourceCapacity * JGraphicResourceManager::GetOcclusionMipMapViewCapacity();
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return initResourceCapacity * JGraphicResourceManager::GetOcclusionMipMapViewCapacity();
					default:
						break;
					}
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY:
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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE:
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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::VERTEX:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return 0;
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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::INDEX:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return 0;
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
				default:
				{
					break;
				}
				}
				return 0;
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateOcclusionQueryResult(ID3D12Device* device, const size_t capa)
			{
				Microsoft::WRL::ComPtr<ID3D12Resource> newResource;

				CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
				auto queryResultDesc = CD3DX12_RESOURCE_DESC::Buffer(8 * capa);

				ThrowIfFailedG(device->CreateCommittedResource(
					&heapProps,
					D3D12_HEAP_FLAG_NONE,
					&queryResultDesc,
					D3D12_RESOURCE_STATE_PREDICATION,
					nullptr,
					IID_PPV_ARGS(&newResource)));
				return std::move(newResource);
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateOcclusionDepthMap(ID3D12Device* device,
				const uint occWidth,
				const uint occHeight)
			{
				Microsoft::WRL::ComPtr<ID3D12Resource> occDsResource;

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

				return std::move(occDsResource);
			}
			static void StartCreation(JGraphicDevice* device, DeviceData& data)
			{
				JDx12GraphicDevice* dxDevice = static_cast<JDx12GraphicDevice*>(device);
				if (dxDevice->CanStartPublicCommand())
				{
					dxDevice->FlushCommandQueue();
					dxDevice->StartPublicCommand();
					data.startCommandThisCreation = true;
				}
			}
			static void EndCreation(JGraphicDevice* device, const DeviceData& data)
			{
				JDx12GraphicDevice* dxDevice = static_cast<JDx12GraphicDevice*>(device);
				if (data.startCommandThisCreation)
				{
					dxDevice->EndPublicCommand();
					dxDevice->FlushCommandQueue();
				}
			}
		}

		void JDx12GraphicResourceManager::ResourceViewInfo::ClearCount()
		{
			count = 0;
		}
		void JDx12GraphicResourceManager::ResourceViewInfo::ClearAllData()
		{
			count = 0;
			capacity = 0;
			offset = 0;
		}
		int JDx12GraphicResourceManager::ResourceViewInfo::GetNextViewIndex()const noexcept
		{
			return offset + count;
		}
		bool JDx12GraphicResourceManager::ResourceViewInfo::HasSpace()const noexcept
		{
			return count < capacity;
		}

		void JDx12GraphicResourceManager::ResourceTypeDesc::ClearCount()
		{
			count = 0;
			for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
				viewInfo[i].ClearCount();
		}
		void JDx12GraphicResourceManager::ResourceTypeDesc::ClearAllData()
		{
			count = 0;
			capacity = 0;
			for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
				viewInfo[i].ClearAllData();
		}
		bool JDx12GraphicResourceManager::ResourceTypeDesc::HasSpace()const noexcept
		{
			return count < capacity;
		}

		/*
		* create resource senario
		1. Create resource and info(Owner Pointer)
		2. caller(impl) is derived interface(has ginfo and access gmanager public method)
		3. impl interface class can derived user access class(restrict version impl derived interface)
		*/
		void JDx12GraphicResourceManager::Initialize(JGraphicDevice* device)
		{
			if (!IsSameDevice(device))
				return;

			ID3D12Device* d3dDevice = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
			rtvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			dsvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			cbvSrvUavDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				typeDesc[i].ClearAllData();

			int totalView[(uint)J_GRAPHIC_BIND_TYPE::COUNT];
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			{
				const J_GRAPHIC_RESOURCE_TYPE rType = (J_GRAPHIC_RESOURCE_TYPE)i;
				typeDesc[i].capacity = Private::InitGraphicResourceCapacity(rType);

				for (uint j = 0; j < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++j)
				{
					if (j == (uint)J_GRAPHIC_BIND_TYPE::UAV)
						continue;

					const J_GRAPHIC_BIND_TYPE bType = (J_GRAPHIC_BIND_TYPE)j;
					typeDesc[i].viewInfo[j].capacity = Private::InitGraphicResourceViewCapacity(rType, bType);
					if (i > 0)
					{
						typeDesc[i].viewInfo[j].offset = totalView[j];
						totalView[j] += typeDesc[i].viewInfo[j].capacity;
					}
					else
					{
						typeDesc[i].viewInfo[j].offset = Private::GraphicResourceViewFixedCount(bType);
						totalView[j] = typeDesc[i].viewInfo[j].offset + typeDesc[i].viewInfo[j].capacity;
					}
				}
			}

			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			{
				const J_GRAPHIC_RESOURCE_TYPE rType = (J_GRAPHIC_RESOURCE_TYPE)i;
				const uint uavIndex = (uint)J_GRAPHIC_BIND_TYPE::UAV;
				typeDesc[i].viewInfo[uavIndex].capacity = Private::InitGraphicResourceViewCapacity(rType, J_GRAPHIC_BIND_TYPE::UAV);
				if (i > 0)
				{
					typeDesc[i].viewInfo[uavIndex].offset = totalView[uavIndex];
					totalView[uavIndex] += typeDesc[i].viewInfo[uavIndex].capacity;
				}
				else
				{
					const uint srvIndex = (uint)J_GRAPHIC_BIND_TYPE::SRV;
					const uint lastResourceIndex = (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT - 1;

					typeDesc[i].viewInfo[uavIndex].offset = Private::GraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::UAV) +
						typeDesc[lastResourceIndex].viewInfo[srvIndex].offset +
						typeDesc[lastResourceIndex].viewInfo[srvIndex].capacity;
					totalView[uavIndex] = typeDesc[i].viewInfo[uavIndex].offset + typeDesc[i].viewInfo[uavIndex].capacity;
				}
			}

			//Debug
			/*
			std::wofstream stream;
			stream.open(L"D:\\JinWooJung\\gDebug.txt", std::ios::app | std::ios::out);

			Core::JEnumInfo* rInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_RESOURCE_TYPE).name());
			Core::JEnumInfo* bInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_BIND_TYPE).name());
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

			BuildRtvDescriptorHeaps(d3dDevice);
			BuildDsvDescriptorHeaps(d3dDevice);
			BuildSrvDescriptorHeaps(d3dDevice);
		}
		void JDx12GraphicResourceManager::Clear()
		{ 
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			{
				resource[i].clear();
				typeDesc[i].ClearAllData();
			}
			rtvHeap.Reset();
			dsvHeap.Reset();
			srvHeap.Reset();
			defaultSceneDsInfo = nullptr;
		}
		J_GRAPHIC_DEVICE_TYPE JDx12GraphicResourceManager::GetDeviceType()const noexcept
		{
			return J_GRAPHIC_DEVICE_TYPE::DX12;
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetCpuRtvDescriptorHandle(int index)const noexcept
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap->GetCPUDescriptorHandleForHeapStart(), index, rtvDescriptorSize);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetGpuRtvDescriptorHandle(int index)const noexcept
		{
			return CD3DX12_GPU_DESCRIPTOR_HANDLE(rtvHeap->GetGPUDescriptorHandleForHeapStart(), index, rtvDescriptorSize);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetCpuDsvDescriptorHandle(int index)const noexcept
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart(), index, dsvDescriptorSize);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetGpuDsvDescriptorHandle(int index)const noexcept
		{
			return CD3DX12_GPU_DESCRIPTOR_HANDLE(dsvHeap->GetGPUDescriptorHandleForHeapStart(), index, dsvDescriptorSize);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetCpuSrvDescriptorHandle(int index)const noexcept
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(srvHeap->GetCPUDescriptorHandleForHeapStart(), index, cbvSrvUavDescriptorSize);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetGpuSrvDescriptorHandle(int index)const noexcept
		{
			return CD3DX12_GPU_DESCRIPTOR_HANDLE(srvHeap->GetGPUDescriptorHandleForHeapStart(), index, cbvSrvUavDescriptorSize);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetCpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept
		{
			switch (bType)
			{
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
				return GetCpuRtvDescriptorHandle(index);
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
				return GetCpuDsvDescriptorHandle(index);
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
				return GetCpuSrvDescriptorHandle(index);
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
				return GetCpuSrvDescriptorHandle(index);
			default:
				return CD3DX12_CPU_DESCRIPTOR_HANDLE();
			}
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetGpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept
		{
			switch (bType)
			{
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
				return GetGpuRtvDescriptorHandle(index);
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
				return GetGpuDsvDescriptorHandle(index);
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
				return GetGpuSrvDescriptorHandle(index);
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
				return GetGpuSrvDescriptorHandle(index);
			default:
				return CD3DX12_GPU_DESCRIPTOR_HANDLE();
			}
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetCpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
			const J_GRAPHIC_BIND_TYPE bType,
			const int rIndex,
			const int bIndex)
		{
			return GetCpuDescriptorHandle(bType, resource[(int)rType][rIndex]->GetHeapIndexStart(bType) + bIndex);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetGpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
			const J_GRAPHIC_BIND_TYPE bType,
			const int rIndex,
			const int bIndex)
		{
			return GetGpuDescriptorHandle(bType, resource[(int)rType][rIndex]->GetHeapIndexStart(bType) + bIndex);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetFirstGpuSrvDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return GetGpuSrvDescriptorHandle(typeDesc[(int)rType].viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].offset);
		}
		ID3D12DescriptorHeap* JDx12GraphicResourceManager::GetDescriptorHeap(const J_GRAPHIC_BIND_TYPE bType)const noexcept
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
		uint JDx12GraphicResourceManager::GetDescriptorSize(const J_GRAPHIC_BIND_TYPE bType)const noexcept
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
		DXGI_FORMAT JDx12GraphicResourceManager::GetBackBufferFormat()const noexcept
		{
			return backBufferFormat;
		}
		DXGI_FORMAT JDx12GraphicResourceManager::GetDepthStencilFormat()const noexcept
		{
			return depthStencilFormat;
		}
		DirectX::XMVECTORF32 JDx12GraphicResourceManager::GetBackBufferClearColor()const noexcept
		{
			return DirectX::XMVECTORF32{{{Constants::backBufferClearColor.x, 
				Constants::backBufferClearColor.y,
				Constants::backBufferClearColor.z, 
				Constants::backBufferClearColor.w}}};
		} 
		D3D12_VERTEX_BUFFER_VIEW JDx12GraphicResourceManager::VertexBufferView(const JUserPtr<JMeshGeometry>& mesh)const noexcept
		{ 
			const int arrayIndex = mesh->GraphicResourceUserInterface().GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::VERTEX, 0);
			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = GetResource(J_GRAPHIC_RESOURCE_TYPE::VERTEX, arrayIndex)->GetGPUVirtualAddress();
			vbv.StrideInBytes = mesh->GetVertexByteSize();
			vbv.SizeInBytes = mesh->GetVertexBufferByteSize();
			return vbv;
		}
		D3D12_INDEX_BUFFER_VIEW JDx12GraphicResourceManager::IndexBufferView(const JUserPtr<JMeshGeometry>& mesh)const noexcept
		{
			const int arrayIndex = mesh->GraphicResourceUserInterface().GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::INDEX, 0);
			D3D12_INDEX_BUFFER_VIEW ibv;
			ibv.BufferLocation = GetResource(J_GRAPHIC_RESOURCE_TYPE::INDEX, arrayIndex)->GetGPUVirtualAddress();
			ibv.Format = mesh->GetIndexByteSize() == sizeof(uint32) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
			ibv.SizeInBytes = mesh->GetIndexBufferByteSize();
			return ibv;
		}
		uint JDx12GraphicResourceManager::GetSwapChainBufferCount()noexcept
		{
			return  Private::swapChainBufferCount;
		}
		uint JDx12GraphicResourceManager::GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return typeDesc[(int)rType].count;
		}
		uint JDx12GraphicResourceManager::GetResourceCapacity(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return typeDesc[(int)rType].capacity;
		}
		uint JDx12GraphicResourceManager::GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return typeDesc[(int)rType].viewInfo[(int)bType].count;
		}
		uint JDx12GraphicResourceManager::GetViewCapacity(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return typeDesc[(int)rType].viewInfo[(int)bType].capacity;
		}
		uint JDx12GraphicResourceManager::GetViewOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return typeDesc[(int)rType].viewInfo[(int)bType].offset;
		}
		uint JDx12GraphicResourceManager::GetTotalViewCount(const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			uint total = 0;
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				total += typeDesc[i].viewInfo[(int)bType].count;
			return total;
		}
		uint JDx12GraphicResourceManager::GetTotalViewCapacity(const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			uint total = 0;
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				total += typeDesc[i].viewInfo[(int)bType].capacity;
			return total;
		}
		ID3D12Resource* JDx12GraphicResourceManager::GetResource(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept
		{
			return resource[(int)rType][index]->resourceHolder->GetResource();
		}
		JGraphicResourceInfo* JDx12GraphicResourceManager::GetInfo(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept
		{
			return resource[(int)rType][index].Get();
		}
		JDx12GraphicResourceInfo* JDx12GraphicResourceManager::GetDxInfo(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept
		{
			return resource[(int)rType][index].Get();
		}
		JDx12GraphicResourceInfo* JDx12GraphicResourceManager::GetDefaultSceneDsInfo()const noexcept
		{
			return static_cast<JDx12GraphicResourceInfo*>(defaultSceneDsInfo.Get());
		}
		uint JDx12GraphicResourceManager::GetHeapIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)
		{
			return typeDesc[(int)rType].viewInfo[(int)bType].count + typeDesc[(int)rType].viewInfo[(int)bType].offset;
		}
		bool JDx12GraphicResourceManager::CanCreateResource(const J_GRAPHIC_RESOURCE_TYPE type)const noexcept
		{
			return true;
		}
		void JDx12GraphicResourceManager::CreateSwapChainBuffer(ID3D12Device* device,
			IDXGIFactory4* dxgiFactory,
			ID3D12CommandQueue* commandQueue,
			Microsoft::WRL::ComPtr<IDXGISwapChain>& swapChain,
			const uint width,
			const uint height,
			bool m4xMsaaState,
			uint m4xMsaaQuality)
		{ 
			JDx12GraphicResourceManager::ResourceTypeDesc& swapChainDesc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN];
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN].clear();
			swapChainDesc.ClearCount();

			bool isNullSwapChain = swapChain == nullptr;
			if (isNullSwapChain)
			{
				swapChain.Reset();
				DXGI_SWAP_CHAIN_DESC sd;
				sd.BufferDesc.Width = width;
				sd.BufferDesc.Height = height;
				sd.BufferDesc.RefreshRate.Numerator = JWindow::GetMaxDisplayFrequency();
				sd.BufferDesc.RefreshRate.Denominator = 1;
				sd.BufferDesc.Format = backBufferFormat;
				sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
				sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
				sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
				sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				sd.BufferCount = Private::swapChainBufferCount;
				sd.OutputWindow = GetWindowHandle();
				sd.Windowed = true;
				sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

				// Note: Swap chain uses queue to perform flush.

				ThrowIfFailedHr(dxgiFactory->CreateSwapChain(
					commandQueue,
					&sd,
					swapChain.GetAddressOf()));
			}
			else
			{
				ThrowIfFailedHr(swapChain->ResizeBuffers(
					Private::swapChainBufferCount,
					width, height,
					backBufferFormat,
					DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
			}

			resource[(int)J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN].resize(Private::swapChainBufferCount);
			for (uint i = 0; i < Private::swapChainBufferCount; ++i)
			{
				Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
				ThrowIfFailedHr(swapChain->GetBuffer(i, IID_PPV_ARGS(&newResource)));
			 
				auto newInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN, std::move(newResource));
				newInfo->SetArrayIndex(i);
				newInfo->SetPrivateName();
				resource[(int)J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN][i] = std::move(newInfo);

				++swapChainDesc.count;
				BindSwapChain(device, i);
			}
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateSceneDepthStencilResource(JGraphicDevice* device,
			const uint viewWidth,
			const uint viewHeight)
		{
			if (!IsSameDevice(device))
				return nullptr;

			Microsoft::WRL::ComPtr<ID3D12Resource> newSceneDsResource;
			JDx12GraphicResourceManager::ResourceTypeDesc& sceneDsDesc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL];

			const bool isValid = !sceneDsDesc.HasSpace() || !sceneDsDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].HasSpace() || !sceneDsDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace();
			if (isValid)
				return nullptr;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			D3D12_RESOURCE_DESC depthStencilDesc;
			depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			depthStencilDesc.Alignment = 0;
			depthStencilDesc.Width = (uint)viewWidth;
			depthStencilDesc.Height = (uint)viewHeight;
			depthStencilDesc.DepthOrArraySize = 1;
			depthStencilDesc.MipLevels = 1;
			depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			depthStencilDesc.SampleDesc.Count = data.m4xMsaaState ? 4 : 1;
			depthStencilDesc.SampleDesc.Quality = data.m4xMsaaState ? (data.m4xMsaaQuality - 1) : 0;
			depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			CD3DX12_CLEAR_VALUE optClear;
			optClear.Format = depthStencilFormat;
			optClear.DepthStencil.Depth = 1.0f;
			optClear.DepthStencil.Stencil = 0;
			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);

			ThrowIfFailedHr(data.device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&depthStencilDesc,
				D3D12_RESOURCE_STATE_DEPTH_READ,
				&optClear,
				IID_PPV_ARGS(&newSceneDsResource)));

			auto newMainDsInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, std::move(newSceneDsResource));
			newMainDsInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL].size());
			newMainDsInfo->SetPrivateName();
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL].push_back(std::move(newMainDsInfo));
			++sceneDsDesc.count;

			const uint resourceIndex = resource[(int)J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL].size() - 1;
			BindMainDepthStencil(data.device, resourceIndex);
			auto& result = resource[(int)J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL][resourceIndex];;

			Private::EndCreation(device, data); 
			return result;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateDebugDepthStencilResource(JGraphicDevice* device,
			const uint viewWidth,
			const uint viewHeight)
		{
			if (!IsSameDevice(device))
				return nullptr;

			Microsoft::WRL::ComPtr<ID3D12Resource> newDebugDsResource;
			JDx12GraphicResourceManager::ResourceTypeDesc& editorDsDesc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL];

			const bool isInValidEditorDs = !editorDsDesc.HasSpace() || !editorDsDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].HasSpace();
			if (isInValidEditorDs)
				return nullptr;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

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
			ThrowIfFailedHr(data.device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&depthStencilDesc,
				D3D12_RESOURCE_STATE_DEPTH_READ,
				&optClear,
				IID_PPV_ARGS(&newDebugDsResource)));

			auto newLayerDepthDebugInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, std::move(newDebugDsResource));
			newLayerDepthDebugInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL].size());
			newLayerDepthDebugInfo->SetPrivateName();
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL].push_back(std::move(newLayerDepthDebugInfo));
			++editorDsDesc.count;

			uint resourceIndex = resource[(int)J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL].size() - 1;
			BindDebugDepthStencil(data.device, resourceIndex);
			auto& result = resource[(int)J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL][resourceIndex];

			Private::EndCreation(device, data);
			return  result;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateLayerDepthDebugResource(JGraphicDevice* device,
			const uint viewWidth,
			const uint viewHeight)
		{
			if (!IsSameDevice(device))
				return nullptr;

			Microsoft::WRL::ComPtr<ID3D12Resource> newLayerDepthDebugResource;
			JDx12GraphicResourceManager::ResourceTypeDesc& layerDepthDebugDesc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG];

			const bool isValid = !layerDepthDebugDesc.HasSpace() || !layerDepthDebugDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace() || !layerDepthDebugDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].HasSpace();
			if (isValid)
				return nullptr;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			D3D12_RESOURCE_DESC debugDesc;
			ZeroMemory(&debugDesc, sizeof(D3D12_RESOURCE_DESC));
			debugDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			debugDesc.Alignment = 0;
			debugDesc.Width = (uint)viewWidth;
			debugDesc.Height = (uint)viewHeight;
			debugDesc.DepthOrArraySize = 1;
			debugDesc.MipLevels = 1;
			debugDesc.SampleDesc.Count = data.m4xMsaaState ? 4 : 1;
			debugDesc.SampleDesc.Quality = data.m4xMsaaState ? (data.m4xMsaaQuality - 1) : 0;
			debugDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			debugDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			debugDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
			ThrowIfFailedG(data.device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&debugDesc,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				nullptr,
				IID_PPV_ARGS(&newLayerDepthDebugResource)));

			auto newLayerDepthDebugInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG, std::move(newLayerDepthDebugResource));
			newLayerDepthDebugInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG].size());
			newLayerDepthDebugInfo->SetPrivateName();
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG].push_back(std::move(newLayerDepthDebugInfo));
			++layerDepthDebugDesc.count;

			const uint resourceIndex = resource[(int)J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG].size() - 1;
			BindLayerDetphMapDebug(data.device, resourceIndex);
			auto& result = resource[(int)J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG][resourceIndex];

			Private::EndCreation(device, data);
			return  result;
		}
		void JDx12GraphicResourceManager::CreateHZBOcclusionResource(JGraphicDevice* device,
			const uint occWidth,
			const uint occHeight,
			_Out_ JUserPtr<JGraphicResourceInfo>& outOccDsInfo,
			_Out_ JUserPtr<JGraphicResourceInfo>& outOccMipMapInfo)
		{
			if (!IsSameDevice(device))
				return;

			Microsoft::WRL::ComPtr<ID3D12Resource> occDsResource;
			Microsoft::WRL::ComPtr<ID3D12Resource> occMipMapResource;
			JDx12GraphicResourceManager::ResourceTypeDesc& occDsDesc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP];
			JDx12GraphicResourceManager::ResourceTypeDesc& occMipMapDesc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP];

			const bool isInValidOccDs = !occDsDesc.HasSpace() || !occDsDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].HasSpace() || !occDsDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace();
			const bool isInValidOccMipMap = !occMipMapDesc.HasSpace() || !occMipMapDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace() || !occMipMapDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].HasSpace();
			if (isInValidOccDs || isInValidOccMipMap)
				return;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			occDsResource = std::move(Private::CreateOcclusionDepthMap(data.device, occWidth, occHeight));
			auto occDsInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, std::move(occDsResource));
			occDsInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP].size());
			occDsInfo->SetPrivateName();
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP].push_back(std::move(occDsInfo));
			++occDsDesc.count;

			uint resourceIndex = resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP].size() - 1;
			BindOcclusionDepthMap(data.device, resourceIndex);
			outOccDsInfo = resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP][resourceIndex];

			D3D12_RESOURCE_DESC mipMapDesc;
			ZeroMemory(&mipMapDesc, sizeof(D3D12_RESOURCE_DESC));
			mipMapDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			mipMapDesc.Alignment = 0;
			mipMapDesc.Width = occWidth;
			mipMapDesc.Height = occHeight;
			mipMapDesc.DepthOrArraySize = 1;
			mipMapDesc.MipLevels = 0;	// 0 = 최대 miplevel 자동계산
			mipMapDesc.SampleDesc.Count = 1;
			mipMapDesc.SampleDesc.Quality = 0;
			mipMapDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			mipMapDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			mipMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
			ThrowIfFailedG(data.device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&mipMapDesc,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				nullptr,
				IID_PPV_ARGS(&occMipMapResource)));

			auto occMipMapInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP, std::move(occMipMapResource));
			occMipMapInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP].size());
			occMipMapInfo->SetPrivateName();
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP].push_back(std::move(occMipMapInfo));
			++occMipMapDesc.count;

			resourceIndex = resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP].size() - 1;
			BindHZBOcclusionDepthMipMap(data.device, resourceIndex);
			outOccMipMapInfo = resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP][resourceIndex];
			Private::EndCreation(device, data);
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateOcclusionResourceDebug(JGraphicDevice* device,
			const uint occWidth,
			const uint occHeight,
			const bool isHzb)
		{
			if (!IsSameDevice(device))
				return nullptr;

			Microsoft::WRL::ComPtr<ID3D12Resource> occDebugResource;
			JDx12GraphicResourceManager::ResourceTypeDesc& occDebugDesc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG];

			const bool isValid = !occDebugDesc.HasSpace() || !occDebugDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace() || !occDebugDesc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].HasSpace();
			if (isValid)
				return nullptr;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
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

			ThrowIfFailedG(data.device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&debugDesc,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				nullptr,
				IID_PPV_ARGS(&occDebugResource)));

			auto occDebugInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG, std::move(occDebugResource));
			occDebugInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG].size());
			occDebugInfo->SetPrivateName();
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG].push_back(std::move(occDebugInfo));
			++occDebugDesc.count;

			uint resourceIndex = resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG].size() - 1;
			if (isHzb)
				BindHZBOcclusionDebug(data.device, resourceIndex);
			else
				BindHdOcclusionDebug(data.device, resourceIndex);
			auto& result = resource[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG][resourceIndex];

			Private::EndCreation(device, data);
			return  result;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::Create2DTexture(JGraphicDevice* device,
			const std::wstring& path,
			const std::wstring& oriFormat)
		{ 
			if (!IsSameDevice(device))
				return nullptr;

			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D];
			if (!desc.HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace())
				return JUserPtr<JGraphicResourceInfo>{};

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource = nullptr;
			Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer = nullptr;

			uint heapIndex = GetHeapIndex(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, J_GRAPHIC_BIND_TYPE::SRV);
			bool res = false;
			JUserPtr<JGraphicResourceInfo> resourceUser = nullptr;
			if (oriFormat == L".dds")
			{
				res = DirectX::CreateDDSTextureFromFile12(data.device,
					data.commandList,
					path.c_str(),
					newResource,
					uploadBuffer) == S_OK;
			}
			else
			{
				res = LoadTextureFromFile(JCUtil::WstrToU8Str(path).c_str(),
					data.device,
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

				auto newInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, std::move(newResource));
				newInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].size());
				newInfo->SetPrivateName(JCUtil::GetFileName(path));
				resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].push_back(std::move(newInfo));
				++desc.count;

				const uint resourceIndex = resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].size() - 1;
				Bind2DTexture(data.device, resourceIndex);
				resourceUser = resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D][resourceIndex];

			}
			Private::EndCreation(device, data);
			if (res && !data.startCommandThisCreation)
			{
				//함수 호출전부터 publicCommand가 작동중일시
				//uploadBuffer에 삭제를 위해 여기서 command를 제출하고 다시 작동시킨다.
				data.dxDevice->EndPublicCommand();
				data.dxDevice->FlushCommandQueue();
				data.dxDevice->StartPublicCommand();
			} 
			uploadBuffer.Reset(); 
			return resourceUser;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateCubeMap(JGraphicDevice* device,
			const std::wstring& path,
			const std::wstring& oriFormat)
		{
			if (!IsSameDevice(device))
				return nullptr;

			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE];
			if (!desc.HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace())
				return JUserPtr<JGraphicResourceInfo>{};

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
			uint heapIndex = GetHeapIndex(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, J_GRAPHIC_BIND_TYPE::SRV);
			bool res = false;
			JUserPtr<JGraphicResourceInfo> resourceUser = nullptr;
			if (oriFormat == L".dds")
			{
				res = DirectX::CreateDDSTextureFromFile12(data.device,
					data.commandList,
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

				auto newInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, std::move(newResource));
				newInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].size());
				newInfo->SetPrivateName(JCUtil::GetFileName(path));
				resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].push_back(std::move(newInfo));
				++desc.count;

				const uint resourceIndex = resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].size() - 1;
				BindCubeMap(data.device, resourceIndex);
				resourceUser = resource[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE][resourceIndex];
			}
			Private::EndCreation(device, data);
			if (res && !data.startCommandThisCreation)
			{
				//함수 호출전부터 publicCommand가 작동중일시
				//uploadBuffer에 삭제를 위해 여기서 command를 제출하고 다시 작동시킨다.
				data.dxDevice->EndPublicCommand();
				data.dxDevice->FlushCommandQueue();
				data.dxDevice->StartPublicCommand();
			}
			uploadBuffer.Reset();
			return resourceUser;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateRenderTargetTexture(JGraphicDevice* device,
			const uint width,
			const uint height)
		{
			if (!IsSameDevice(device))
				return nullptr;

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON];
			if (!desc.HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::RTV].HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace())
				return JUserPtr<JGraphicResourceInfo>{};

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

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
			ThrowIfFailedG(data.device->CreateCommittedResource(
				&heapProperty,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				&optClear,
				IID_PPV_ARGS(&newResource)));

			auto newInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, std::move(newResource));
			newInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON].size());
			newInfo->SetPrivateName();
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON].push_back(std::move(newInfo));
			++desc.count;

			const uint resourceIndex = resource[(int)J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON].size() - 1;
			BindRenderTarget(data.device, resourceIndex);
			auto& result = resource[(int)J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON][resourceIndex];

			Private::EndCreation(device, data);
			return result;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateShadowMapTexture(JGraphicDevice* device,
			const uint width,
			const uint height)
		{
			if (!IsSameDevice(device))
				return nullptr;

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP];
			if (!desc.HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace())
				return JUserPtr<JGraphicResourceInfo>{};

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			D3D12_RESOURCE_DESC resourceDesc;
			ZeroMemory(&resourceDesc, sizeof(D3D12_RESOURCE_DESC));
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resourceDesc.Alignment = 0;
			resourceDesc.Width = width;
			resourceDesc.Height = height;
			resourceDesc.DepthOrArraySize = 1;
			resourceDesc.MipLevels = 1;
			resourceDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			//resourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_CLEAR_VALUE optClear;
			optClear.Format = DXGI_FORMAT_D32_FLOAT;
			optClear.DepthStencil.Depth = 1.0f;
			optClear.DepthStencil.Stencil = 0;

			ThrowIfFailedHr(data.device->CreateCommittedResource(
				&heapProperty,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COMMON,
				&optClear,
				IID_PPV_ARGS(&newResource)));

			auto newInfo = CreateResourceInfo(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, std::move(newResource));
			newInfo->SetArrayIndex(resource[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].size());
			newInfo->SetPrivateName();
			resource[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].push_back(std::move(newInfo));
			++desc.count;

			const uint resourceIndex = resource[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].size() - 1;
			BindShadowMap(data.device, resourceIndex);
			auto& result = resource[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP][resourceIndex];

			Private::EndCreation(device, data);
			return result;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateShadowMapArrayTexture(JGraphicDevice* device,
			const uint width,
			const uint height,
			const uint count)
		{
			if (!IsSameDevice(device))
				return nullptr;

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			J_GRAPHIC_RESOURCE_TYPE type = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)type];
			if (!desc.HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace())
				return JUserPtr<JGraphicResourceInfo>{};

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			D3D12_RESOURCE_DESC resourceDesc;
			ZeroMemory(&resourceDesc, sizeof(D3D12_RESOURCE_DESC));
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resourceDesc.Alignment = 0;
			resourceDesc.Width = width;
			resourceDesc.Height = height;
			resourceDesc.DepthOrArraySize = count;
			resourceDesc.MipLevels = 1;
			//resourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			resourceDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_CLEAR_VALUE optClear;
			optClear.Format = DXGI_FORMAT_D32_FLOAT;
			optClear.DepthStencil.Depth = 1.0f;
			optClear.DepthStencil.Stencil = 0;

			ThrowIfFailedHr(data.device->CreateCommittedResource(
				&heapProperty,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COMMON,
				&optClear,
				IID_PPV_ARGS(&newResource)));
			 
			auto newInfo = CreateResourceInfo(type, std::move(newResource));
			newInfo->SetArrayIndex(resource[(int)type].size());
			newInfo->SetPrivateName();
			resource[(int)type].push_back(std::move(newInfo));
			++desc.count;

			const uint resourceIndex = resource[(int)type].size() - 1;
			BindShadowMapArray(data.device, resourceIndex);
			auto& result = resource[(int)type][resourceIndex];

			Private::EndCreation(device, data);
			return result;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateShadowMapCubeTexture(JGraphicDevice* device,
			const uint width,
			const uint height)
		{
			if (!IsSameDevice(device))
				return nullptr;

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			J_GRAPHIC_RESOURCE_TYPE type = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)type];
			if (!desc.HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].HasSpace() || !desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].HasSpace())
				return JUserPtr<JGraphicResourceInfo>{};

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			D3D12_RESOURCE_DESC resourceDesc;
			ZeroMemory(&resourceDesc, sizeof(D3D12_RESOURCE_DESC));
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resourceDesc.Alignment = 0;
			resourceDesc.Width = width;
			resourceDesc.Height = height;
			resourceDesc.DepthOrArraySize = 6;
			resourceDesc.MipLevels = 1;
			resourceDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			//resourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_CLEAR_VALUE optClear;
			optClear.Format = DXGI_FORMAT_D32_FLOAT;
			//optClear.Format = depthStencilFormat;
			optClear.DepthStencil.Depth = 1.0f;
			optClear.DepthStencil.Stencil = 0;

			ThrowIfFailedHr(data.device->CreateCommittedResource(
				&heapProperty,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COMMON,
				&optClear,
				IID_PPV_ARGS(&newResource)));

			auto newInfo = CreateResourceInfo(type, std::move(newResource));
			newInfo->SetArrayIndex(resource[(int)type].size());
			newInfo->SetPrivateName();
			resource[(int)type].push_back(std::move(newInfo));
			++desc.count;

			const uint resourceIndex = resource[(int)type].size() - 1;
			BindShadowMapCube(data.device, resourceIndex);
			auto& result = resource[(int)type][resourceIndex];

			Private::EndCreation(device, data);
			return result;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateVertexBuffer(JGraphicDevice* device, const std::vector<Core::JStaticMeshVertex>& vertex)
		{
			return CreateBuffer(J_GRAPHIC_RESOURCE_TYPE::VERTEX, device, vertex.data(), sizeof(Core::JStaticMeshVertex) * vertex.size());
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateVertexBuffer(JGraphicDevice* device, const std::vector<Core::JSkinnedMeshVertex>& vertex)
		{
			return CreateBuffer(J_GRAPHIC_RESOURCE_TYPE::VERTEX, device, vertex.data(), sizeof(Core::JSkinnedMeshVertex) * vertex.size());
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateIndexBuffer(JGraphicDevice* device, const std::vector<uint32>& index)
		{
			return CreateBuffer(J_GRAPHIC_RESOURCE_TYPE::INDEX, device, index.data(), sizeof(uint32) * index.size());
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateIndexBuffer(JGraphicDevice* device, const std::vector<uint16>& index)
		{
			return CreateBuffer(J_GRAPHIC_RESOURCE_TYPE::INDEX, device, index.data(), sizeof(uint16) * index.size());
		}
		bool JDx12GraphicResourceManager::DestroyGraphicTextureResource(JGraphicDevice* device, JGraphicResourceInfo* info)
		{
			if (info == nullptr || !IsSameDevice(device))
				return false;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			JDx12GraphicResourceInfo* dxInfo = static_cast<JDx12GraphicResourceInfo*>(info);
			const uint rIndex = dxInfo->GetArrayIndex();
			const J_GRAPHIC_RESOURCE_TYPE rType = dxInfo->GetGraphicResourceType();
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];
			BindViewPtr ptr = GetResourceBindViewPtr(rType);
  
			//ptr can nullptr ex) occlusion query(resource view binding을 사용하지 않는 타입)
			if (ptr != nullptr)
			{
				//type per total view count controll
				for (uint i = 0; i < (int)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
				{
					//바인딩 되어있지 않을시 count < 0
					//heapIndexStart는 바인딩되지 않을 경우 기본값인 -1을 가지고있는 상태이다.
					//graphic resource 타입마다 binding 유형이 정해지기 때문에 유효하다

					desc.viewInfo[i].count = dxInfo->GetHeapIndexStart((J_GRAPHIC_BIND_TYPE)i) - desc.viewInfo[i].offset;
					if (desc.viewInfo[i].count < 0)
						desc.viewInfo[i].count = 0;
				}
				//resource[(int)rType][rIndex].Clear();

				//type per resource view controll
				for (uint i = rIndex + 1; i < desc.count; ++i)
				{
					//init heap st, bind value
					JDx12GraphicResourceInfo* reBindHandle = resource[(int)rType][i].Get();
					for (uint j = 0; j < (int)J_GRAPHIC_BIND_TYPE::COUNT; ++j)
					{
						reBindHandle->SetHeapIndexStart((J_GRAPHIC_BIND_TYPE)j, -1);
						reBindHandle->SetViewCount((J_GRAPHIC_BIND_TYPE)j, 0);
					}

					//reset array, heap st, bind value
					const int newIndex = i - 1;
					reBindHandle->SetArrayIndex(newIndex);
					(this->*ptr)(data.device, i);
				}
			} 
			else
			{
				for (uint i = rIndex + 1; i < desc.count; ++i)
				{
					//reset array
					const int newIndex = i - 1;
					resource[(int)rType][i]->SetArrayIndex(newIndex);
				}
			}

			--desc.count;
			//resource[(int)rType].begin() + rIndex 
			//JOwnerPtr<JGraphicResourceInfo> target = std::move(resource[(int)rType][rIndex]);
			resource[(int)rType].erase(resource[(int)rType].begin() + rIndex);
			//target.Clear();
			Private::EndCreation(device, data);

			return true;
		}
		JOwnerPtr<JDx12GraphicResourceInfo> JDx12GraphicResourceManager::CreateResourceInfo(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, Microsoft::WRL::ComPtr<ID3D12Resource>&& resource)
		{
			JDx12GraphicResourceInfo::GetHandlePtr getHandleLam = [](JDx12GraphicResourceManager* gm, const J_GRAPHIC_BIND_TYPE btype, const uint index)
			{
				return gm->GetGpuDescriptorHandle(btype, index);
			};
			
			auto dx12Holder = std::make_unique<JDx12GraphicResourceHolder>(std::move(resource));
			return JOwnerPtr<JDx12GraphicResourceInfo>(new JDx12GraphicResourceInfo(graphicResourceType, this, std::move(dx12Holder), getHandleLam));
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateBuffer(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, JGraphicDevice* device, const void* bufferData, size_t bufferByteSize)
		{
			if (!IsSameDevice(device))
				return nullptr;

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
			J_GRAPHIC_RESOURCE_TYPE type = graphicResourceType;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)type];
			if (!desc.HasSpace())
				return JUserPtr<JGraphicResourceInfo>{};

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			newResource = JD3DUtility::CreateDefaultBuffer(data.device, data.commandList, bufferData, bufferByteSize, uploadBuffer);

			auto newInfo = CreateResourceInfo(type, std::move(newResource));
			newInfo->SetArrayIndex(resource[(int)type].size());
			newInfo->SetPrivateName();
			resource[(int)type].push_back(std::move(newInfo));
			++desc.count;

			const uint resourceIndex = resource[(int)type].size() - 1;
			auto& result = resource[(int)type][resourceIndex];

			Private::EndCreation(device, data);
			uploadBuffer.Reset();
			return result;
		}
		JDx12GraphicResourceManager::BindViewPtr JDx12GraphicResourceManager::GetResourceBindViewPtr(const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			switch (rType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN:
				return &JDx12GraphicResourceManager::BindSwapChain;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL:
				return &JDx12GraphicResourceManager::BindMainDepthStencil;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL:
				return &JDx12GraphicResourceManager::BindDebugDepthStencil;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG:
				return &JDx12GraphicResourceManager::BindLayerDetphMapDebug;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP:
				return &JDx12GraphicResourceManager::BindOcclusionDepthMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
				return &JDx12GraphicResourceManager::BindHZBOcclusionDepthMipMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
				return &JDx12GraphicResourceManager::BindHZBOcclusionDebug;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D:
				return &JDx12GraphicResourceManager::Bind2DTexture;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE:
				return &JDx12GraphicResourceManager::BindCubeMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
				return &JDx12GraphicResourceManager::BindRenderTarget;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
				return &JDx12GraphicResourceManager::BindShadowMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY:
				return &JDx12GraphicResourceManager::BindShadowMapArray;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE:
				return &JDx12GraphicResourceManager::BindShadowMapCube;
			default:
				return nullptr;
			}
		}
		void JDx12GraphicResourceManager::SetViewCount(JDx12GraphicResourceInfo* handlePtr, const J_GRAPHIC_BIND_TYPE bType, const int nextViewIndex)
		{
			if (handlePtr->GetViewCount(bType) == 0)
			{
				handlePtr->SetHeapIndexStart(bType, nextViewIndex);
				handlePtr->SetViewCount(bType, 1);
			}
			else
			{
				const int viewCount = handlePtr->GetViewCount(bType) + 1;
				handlePtr->SetViewCount(bType, viewCount);
			}
		}
		void JDx12GraphicResourceManager::BindSwapChain(ID3D12Device* device, const uint resourceIndex)
		{
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN];

			ID3D12Resource* resourcePtr = GetResource(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN, resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN)][resourceIndex].Get();
			const int nextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::RTV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::RTV, nextViewIndex);
			device->CreateRenderTargetView(resourcePtr, nullptr, GetCpuRtvDescriptorHandle(nextViewIndex));
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::RTV].count;
		}
		void JDx12GraphicResourceManager::BindMainDepthStencil(ID3D12Device* device, const uint resourceIndex)
		{
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL];

			ID3D12Resource* resourcePtr = GetResource(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL)][resourceIndex].Get();
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
		void JDx12GraphicResourceManager::BindDebugDepthStencil(ID3D12Device* device, const uint resourceIndex)
		{
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL];

			ID3D12Resource* resourcePtr = GetResource(J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL)][resourceIndex].Get();
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
		void JDx12GraphicResourceManager::BindLayerDetphMapDebug(ID3D12Device* device, const uint resourceIndex)
		{
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG];

			ID3D12Resource* resourcePtr = GetResource(J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG, resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG)][resourceIndex].Get();
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
		void JDx12GraphicResourceManager::BindOcclusionDepthMap(ID3D12Device* device, const uint resourceIndex)
		{
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP];

			ID3D12Resource* resourcePtr = GetResource(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP)][resourceIndex].Get();
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
		void JDx12GraphicResourceManager::BindHZBOcclusionDepthMipMap(ID3D12Device* device, const uint resourceIndex)
		{
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP];

			ID3D12Resource* resourcePtr = GetResource(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP, resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP)][resourceIndex].Get();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();
			const int uavNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].GetNextViewIndex();

			uint nowWidth = resourcePtr->GetDesc().Width;
			uint nowHeight = resourcePtr->GetDesc().Height;

			uint occlusionCount = 0;
			uint occlusionMipmapViewCapacity = GetOcclusionMipMapViewCapacity();
			uint minOcclusionSize = GetOcclusionMinSize();

			while (occlusionCount != occlusionMipmapViewCapacity &&
				(nowWidth >= minOcclusionSize && nowHeight >= minOcclusionSize))
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
		void JDx12GraphicResourceManager::BindHZBOcclusionDebug(ID3D12Device* device, const uint resourceIndex)
		{
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG];

			ID3D12Resource* resourcePtr = GetResource(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG, resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG)][resourceIndex].Get();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();
			const int uavNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].GetNextViewIndex();

			uint nowWidth = resourcePtr->GetDesc().Width;
			uint nowHeight = resourcePtr->GetDesc().Height;

			uint occlusionCount = 0;
			uint occlusionMipmapViewCapacity = GetOcclusionMipMapViewCapacity();
			uint minOcclusionSize = GetOcclusionMinSize();

			while (occlusionCount != occlusionMipmapViewCapacity &&
				(nowWidth >= minOcclusionSize && nowHeight >= minOcclusionSize))
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
		void JDx12GraphicResourceManager::BindHdOcclusionDebug(ID3D12Device* device, const uint resourceIndex)
		{
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG];

			ID3D12Resource* resourcePtr = GetResource(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG, resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG)][resourceIndex].Get();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();
			const int uavNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].GetNextViewIndex();

			uint nowWidth = resourcePtr->GetDesc().Width;
			uint nowHeight = resourcePtr->GetDesc().Height;

			D3D12_SHADER_RESOURCE_VIEW_DESC debugSrvDesc = {};
			debugSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			debugSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			debugSrvDesc.Texture2D.MostDetailedMip = 0;
			debugSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			debugSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			debugSrvDesc.Texture2D.MipLevels = resourcePtr->GetDesc().MipLevels;

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);
			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::UAV, uavNextViewIndex);

			device->CreateShaderResourceView(resourcePtr, &debugSrvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));
			device->CreateUnorderedAccessView(resourcePtr, nullptr, &uavDesc, GetCpuSrvDescriptorHandle(uavNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].count;
		}
		void JDx12GraphicResourceManager::Bind2DTexture(ID3D12Device* device, const uint resourceIndex)
		{
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D];

			ID3D12Resource* resourcePtr = GetResource(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D)][resourceIndex].Get();
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
		void JDx12GraphicResourceManager::BindCubeMap(ID3D12Device* device, const uint resourceIndex)
		{
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE];

			ID3D12Resource* resourcePtr = GetResource(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)][resourceIndex].Get();
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
		void JDx12GraphicResourceManager::BindRenderTarget(ID3D12Device* device, const uint resourceIndex)
		{
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON];

			ID3D12Resource* resourcePtr = GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON)][resourceIndex].Get();

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
		void JDx12GraphicResourceManager::BindShadowMap(ID3D12Device* device, const uint resourceIndex)
		{
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP];

			ID3D12Resource* resourcePtr = GetResource(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP)][resourceIndex].Get();

			const int dsvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].GetNextViewIndex();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::DSV, dsvNextViewIndex);
			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);

			// Create DSV to resource so we can render to the shadow map.
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			//dsvDesc.Format = depthStencilFormat;
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.Texture2D.MipSlice = 0; 

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			//srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = resourcePtr->GetDesc().MipLevels;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Texture2D.PlaneSlice = 0;

			device->CreateDepthStencilView(resourcePtr, &dsvDesc, GetCpuDsvDescriptorHandle(dsvNextViewIndex));
			device->CreateShaderResourceView(resourcePtr, &srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].count;
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;
		}
		void JDx12GraphicResourceManager::BindShadowMapArray(ID3D12Device* device, const uint resourceIndex)
		{
			J_GRAPHIC_RESOURCE_TYPE type = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)type];

			ID3D12Resource* resourcePtr = GetResource(type, resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(type)][resourceIndex].Get();

			const int dsvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].GetNextViewIndex();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::DSV, dsvNextViewIndex);
			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);

			// Create DSV to resource so we can render to the shadow map.
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			//dsvDesc.Format = depthStencilFormat;
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.Texture2DArray.ArraySize = resourcePtr->GetDesc().DepthOrArraySize;
			dsvDesc.Texture2DArray.FirstArraySlice = 0;
			dsvDesc.Texture2DArray.MipSlice = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			//srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.ArraySize = resourcePtr->GetDesc().DepthOrArraySize;
			srvDesc.Texture2DArray.FirstArraySlice = 0;
			srvDesc.Texture2DArray.MostDetailedMip = 0;
			srvDesc.Texture2DArray.MipLevels = resourcePtr->GetDesc().MipLevels;
			srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f; 

			device->CreateDepthStencilView(resourcePtr, &dsvDesc, GetCpuDsvDescriptorHandle(dsvNextViewIndex));
			device->CreateShaderResourceView(resourcePtr, &srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].count;
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;
		}
		void JDx12GraphicResourceManager::BindShadowMapCube(ID3D12Device* device, const uint resourceIndex)
		{
			J_GRAPHIC_RESOURCE_TYPE type = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)type];

			ID3D12Resource* resourcePtr = GetResource(type, resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(type)][resourceIndex].Get();

			const int dsvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].GetNextViewIndex();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::DSV, dsvNextViewIndex);
			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);

			// Create DSV to resource so we can render to the shadow map.
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			//dsvDesc.Format = depthStencilFormat;
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.Texture2DArray.ArraySize = 6;
			dsvDesc.Texture2DArray.FirstArraySlice = 0;
			dsvDesc.Texture2DArray.MipSlice = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			//srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = resourcePtr->GetDesc().MipLevels;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f; 

			device->CreateDepthStencilView(resourcePtr, &dsvDesc, GetCpuDsvDescriptorHandle(dsvNextViewIndex));
			device->CreateShaderResourceView(resourcePtr, &srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].count;
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;
		}
		void JDx12GraphicResourceManager::BuildRtvDescriptorHeaps(ID3D12Device* device)
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
			rtvHeapDesc.NumDescriptors = GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::RTV) +
				Private::GraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::RTV);
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			rtvHeapDesc.NodeMask = 0;
			ThrowIfFailedHr(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.GetAddressOf())));
			rtvHeap->SetName(L"RtvHeap");
		}
		void JDx12GraphicResourceManager::BuildDsvDescriptorHeaps(ID3D12Device* device)
		{
			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
			dsvHeapDesc.NumDescriptors = GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::DSV) +
				Private::GraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::DSV);
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			dsvHeapDesc.NodeMask = 0;
			ThrowIfFailedHr(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsvHeap.GetAddressOf())));
			dsvHeap->SetName(L"DsvHeap");
		}
		void JDx12GraphicResourceManager::BuildSrvDescriptorHeaps(ID3D12Device* device)
		{
			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
			srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			srvHeapDesc.NumDescriptors = GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::SRV) +
				GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::UAV) +
				Private::GraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::SRV) +
				Private::GraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::UAV);
			srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			ThrowIfFailedHr(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(srvHeap.GetAddressOf())));
			srvHeap->SetName(L"SrvHeap");
		}
		void JDx12GraphicResourceManager::ResizeWindow(const JGraphicBaseDataSet& base, JGraphicDevice* device)
		{
			if (!IsSameDevice(device))
				return;

			if (defaultSceneDsInfo != nullptr)
				DestroyGraphicTextureResource(device, defaultSceneDsInfo.Release());
			defaultSceneDsInfo = CreateSceneDepthStencilResource(device, base.info.width, base.info.height);
			defaultSceneDsInfo->SetPrivateName(L"EndFrameDs");
		}
		void JDx12GraphicResourceManager::StoreTexture(JGraphicDevice* device, const J_GRAPHIC_RESOURCE_TYPE rType, const int index, const std::wstring& path)
		{
			if (!IsSameDevice(device))
				return;

			auto resource = GetResource(rType, index);
			if (resource == nullptr)
				return;

			if (resource->GetDesc().Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
			{
				device->FlushCommandQueue();
				device->StartPublicCommand();

				JDx12GraphicDevice* dxDevice = static_cast<JDx12GraphicDevice*>(device);
				DirectX::SaveDDSTextureToFile(dxDevice->GetCommandQueue(),
					resource,
					path.c_str(),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					D3D12_RESOURCE_STATE_GENERIC_READ);

				device->EndPublicCommand();
				device->FlushCommandQueue();
			}
		}
		void JDx12GraphicResourceManager::RegisterTypeData()
		{
			using JAllocationDesc = JinEngine::Core::JAllocationDesc;
			using NotifyReAllocPtr = JAllocationDesc::NotifyReAllocF::Ptr;
			using NotifyReAllocF = JAllocationDesc::NotifyReAllocF::Functor;
			using ReceiverPtr = JAllocationDesc::ReceiverPtr;
			using ReAllocatedPtr = JAllocationDesc::ReAllocatedPtr;
			using MemIndex = JAllocationDesc::MemIndex;

			NotifyReAllocPtr notifyPtr = [](ReceiverPtr receiver, ReAllocatedPtr movedPtr, MemIndex index)
			{
				JDx12GraphicResourceInfo* movedInfo = static_cast<JDx12GraphicResourceInfo*>(movedPtr);
				JDx12GraphicResourceManager* manager = movedInfo->manager;

				//Release를 먼저하지않으면 Reset시 유효한 pointer를 소유하므로 pointer 파괴를 시도하며
				//현재 alloc class에서 메모리를 재배치하는 과정에서 에러를 일으킬수 있으므로
				//Release() 한다음 Reset()을 호출해야한다.
				manager->resource[(int)movedInfo->GetGraphicResourceType()][movedInfo->GetArrayIndex()].Release();
				manager->resource[(int)movedInfo->GetGraphicResourceType()][movedInfo->GetArrayIndex()].Reset(movedInfo);
			};
			auto reAllocF = std::make_unique<JAllocationDesc::NotifyReAllocF::Functor>(notifyPtr);
			std::unique_ptr<JAllocationDesc> desc = std::make_unique<JAllocationDesc>();

			desc->notifyReAllocB = UniqueBind(std::move(reAllocF), static_cast<ReceiverPtr>(nullptr), JinEngine::Core::empty, JinEngine::Core::empty);
			JDx12GraphicResourceInfo::StaticTypeInfo().SetAllocationOption(std::move(desc));
		}
		JDx12GraphicResourceManager::JDx12GraphicResourceManager() {}
		JDx12GraphicResourceManager::~JDx12GraphicResourceManager() {}
	}
}

/*
//Debug
 /*
 #include<fstream>
#include"../../Core/File/JFileIOHelper.h"

		std::wofstream stream;
		stream.open(L"D:\\JinWooJung\\gDebug.txt", std::ios::app | std::ios::out);

		Core::JEnumInfo* rInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_RESOURCE_TYPE).name());
		Core::JEnumInfo* bInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_BIND_TYPE).name());
		auto rNameVec = rInfo->GetEnumNameVec();
		auto bNameVec = bInfo->GetEnumNameVec();
		JFileIOHelper::InputSpace(stream, 1);
 */