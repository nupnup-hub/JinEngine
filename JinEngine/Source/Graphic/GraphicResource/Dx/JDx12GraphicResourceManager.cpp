#include"JDx12GraphicResourceManager.h"  
#include"JDx12GraphicResourceInfo.h"
#include"JLoadTextureFromFile.h"
#include"../Dx/JDx12GraphicResourceHolder.h"
#include"../JGraphicResourceInterface.h"
#include"../../Device/JGraphicDevice.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../Utility/Dx/JD3DUtility.h" 
#include"../../JGraphic.h"

#include"../../../Core/Utility/JCommonUtility.h"  
#include"../../../Core/Exception/JExceptionMacro.h"  
#include"../../../Core/File/JFileConstant.h"  
#include"../../../Object/Resource/Mesh/JMeshGeometry.h"   
#include"../../../Window/JWindow.h"
#include"../../../../ThirdParty/DirectX/DDSTextureLoader11.h" 
#include"../../../../ThirdParty/DirectX/TK/Inc/ScreenGrab.h" 
#include"../../../../ThirdParty/DirectX/TK/Inc/WICTextureLoader.h" 
#include"../../../../ThirdParty/DirectX/TK/Inc/ResourceUploadBatch.h" 
#include"../../../../ThirdParty/Tif/tiffio.h"
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
				ID3D12CommandQueue* commandQueue = nullptr;
				ID3D12GraphicsCommandList* commandList = nullptr;
				uint m4xMsaaQuality = 0;
				bool m4xMsaaState = false;
				bool startCommandThisCreation = false;
			public:
				DeviceData(JGraphicDevice* device)
				{
					dxDevice = static_cast<JDx12GraphicDevice*>(device);
					DeviceData::device = dxDevice->GetDevice();
					commandQueue = dxDevice->GetCommandQueue();
					commandList = dxDevice->GetPublicCmdList();
					m4xMsaaQuality = dxDevice->GetM4xMsaaQuality();
					m4xMsaaState = dxDevice->GetM4xMsaaState();
				}
			};

			static constexpr uint swapChainBufferCount = 2;
			static constexpr uint vertexCapacity = INT_MAX;	//mesh ������ŭ �Ҵ簡��
			static constexpr uint indexCapacity = INT_MAX;		//mesh ������ŭ �Ҵ簡��
 
			static uint InitGraphicResourceViewFixedCount(const J_GRAPHIC_BIND_TYPE bType)
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
				const uint initResourceCapacity = InitGraphicResourceCapacity(rType);
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
						return initResourceCapacity * JGraphicResourceManager::GetOcclusionMipmapViewCapacity();
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return initResourceCapacity * JGraphicResourceManager::GetOcclusionMipmapViewCapacity();
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
			static uint InitGraphicResourceOptionViewCapacity(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)
			{
				if (!JGraphicResourceType::CanUseOption(opType, rType))
					return 0;

				switch (opType)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::POST_PROCESSING:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::AMBIENT_OCCLISION_MAP:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
						return InitGraphicResourceCapacity(rType);
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return InitGraphicResourceCapacity(rType);
					default:
						return 0;
					}
				}
				default:
					return 0;
				}
			}
			static uint InitGraphicResourceAdditionalViewCapacity(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)
			{
				uint sum = 0;
				for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNT; ++i)
				{
					J_GRAPHIC_RESOURCE_OPTION_TYPE opType = (J_GRAPHIC_RESOURCE_OPTION_TYPE)i;
					if (!JGraphicResourceType::CanUseOption(opType, rType))
						continue;
					
					sum += InitGraphicResourceOptionViewCapacity(opType, rType, bType);
				}
				return sum;
			}
			static uint InitGraphicResourceTotalViewCapaicty(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)
			{
				return InitGraphicResourceViewCapacity(rType, bType) + InitGraphicResourceAdditionalViewCapacity(rType, bType);
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
			Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* device,
				ID3D12GraphicsCommandList* cmdList,
				const void* initData,
				uint64 byteSize,
				Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
			{
				ComPtr<ID3D12Resource> defaultBuffer;

				CD3DX12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				CD3DX12_RESOURCE_DESC defaultBufDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
				// Create the actual default buffer resource.
				ThrowIfFailedHr(device->CreateCommittedResource(
					&defaultHeap,
					D3D12_HEAP_FLAG_NONE,
					&defaultBufDesc,
					D3D12_RESOURCE_STATE_COMMON,
					nullptr,
					IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

				// In order to copy CPU memory data into our default buffer, we need to create
				// an intermediate upload heap. 
				CD3DX12_HEAP_PROPERTIES uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

				ThrowIfFailedHr(device->CreateCommittedResource(
					&uploadHeap,
					D3D12_HEAP_FLAG_NONE,
					&defaultBufDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(uploadBuffer.GetAddressOf())));


				// Describe the data we want to copy into the default buffer.
				D3D12_SUBRESOURCE_DATA subResourceData = {};
				subResourceData.pData = initData;
				subResourceData.RowPitch = byteSize;
				subResourceData.SlicePitch = subResourceData.RowPitch;

				// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
				// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
				// the intermediate upload heap data will be copied to mBuffer.
				CD3DX12_RESOURCE_BARRIER preBarrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
					D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
				CD3DX12_RESOURCE_BARRIER afterBarrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
					D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

				cmdList->ResourceBarrier(1, &preBarrier);
				UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
				cmdList->ResourceBarrier(1, &afterBarrier);

				// Note: uploadBuffer has to be kept alive after the above function calls because
				// the command list has not been executed yet that performs the actual copy.
				// The caller can Release the uploadBuffer after it knows the copy has been executed.


				return defaultBuffer;
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
			  
			static D3D12_RESOURCE_STATES GraphicOptionInitState(const J_GRAPHIC_RESOURCE_TYPE type)
			{
				//for post processing
				switch (type)
				{ 
				/*
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL:
					return D3D12_RESOURCE_STATE_DEPTH_READ;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL:
					return D3D12_RESOURCE_STATE_DEPTH_READ;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG:
					return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP:
					return D3D12_RESOURCE_STATE_DEPTH_READ;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
					return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
					return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
				*/
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
					return D3D12_RESOURCE_STATE_GENERIC_READ; 
				default:
					return D3D12_RESOURCE_STATE_COMMON;
				}
			}
			static void GraphicOptionProperty(const J_GRAPHIC_RESOURCE_OPTION_TYPE type,
				_Inout_ D3D12_RESOURCE_DESC& desc,
				_Inout_ D3D12_HEAP_PROPERTIES& heapProperties,
				_Inout_ D3D12_HEAP_FLAGS& heapFlags,
				_Inout_ DXGI_FORMAT& clearFormat)
			{
				switch (type)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::POST_PROCESSING:
				{
					desc.Flags = Core::AddSQValueEnum(desc.Flags, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP:
				{ 
					desc.Flags = Core::AddSQValueEnum(desc.Flags, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
					desc.Format = DXGI_FORMAT_R16G16_TYPELESS;
					heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
					heapFlags = D3D12_HEAP_FLAG_NONE;
					clearFormat = DXGI_FORMAT_R16G16_FLOAT;
					break;
				} 
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::AMBIENT_OCCLISION_MAP:
				{
					desc.Flags = Core::AddSQValueEnum(desc.Flags, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
					desc.Format = DXGI_FORMAT_R16_TYPELESS;
					heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
					heapFlags = D3D12_HEAP_FLAG_NONE;
					clearFormat = DXGI_FORMAT_R16_FLOAT;
					break;
				}
				default:
					break;
				}
			}
			static bool CanCreateMipmap(const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				switch (rType)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE:
					return true;
				default:
					break;
				}
			}
			static uint MPBCount(const J_GRAPHIC_BIND_TYPE bType)
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return JDx12GraphicResourceManager::MPBCapactiy();
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return JDx12GraphicResourceManager::MPBCapactiy();
				default:
					return 0;
				}
			}
		}

		JDx12GraphicResourceManager::BindDesc::BindDesc(ID3D12Device* device, const uint resourceIndex, const bool useMipmap)
			:device(device), 
			resourceIndex(resourceIndex),
			useMipmap(useMipmap)
		{}
		JDx12GraphicResourceManager::BindOptionDesc::BindOptionDesc(const BindDesc& resourceBindDesc)
			: resourceBindDesc(resourceBindDesc)
		{}

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
					typeDesc[i].viewInfo[j].capacity = Private::InitGraphicResourceTotalViewCapaicty(rType, bType);
					if (i > 0)
					{
						typeDesc[i].viewInfo[j].offset = totalView[j];
						totalView[j] += typeDesc[i].viewInfo[j].capacity;
					}
					else
					{
						typeDesc[i].viewInfo[j].offset = Private::InitGraphicResourceViewFixedCount(bType);
						totalView[j] = typeDesc[i].viewInfo[j].offset + typeDesc[i].viewInfo[j].capacity;
					}
				}
			}

			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			{
				const J_GRAPHIC_RESOURCE_TYPE rType = (J_GRAPHIC_RESOURCE_TYPE)i;
				const uint uavIndex = (uint)J_GRAPHIC_BIND_TYPE::UAV;
				typeDesc[i].viewInfo[uavIndex].capacity = Private::InitGraphicResourceTotalViewCapaicty(rType, J_GRAPHIC_BIND_TYPE::UAV);
				if (i > 0)
				{
					typeDesc[i].viewInfo[uavIndex].offset = totalView[uavIndex];
					totalView[uavIndex] += typeDesc[i].viewInfo[uavIndex].capacity;
				}
				else
				{
					const uint srvIndex = (uint)J_GRAPHIC_BIND_TYPE::SRV;
					const uint lastResourceIndex = (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT - 1;

					typeDesc[i].viewInfo[uavIndex].offset = Private::InitGraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::UAV) +
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
			mpb.Clear();
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
		CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetMPBCpuDescriptorHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{ 
			auto info = mpb.Get(handle);
			return info != nullptr ? GetCpuDescriptorHandle(bType, info->index[(uint)bType]) : CD3DX12_CPU_DESCRIPTOR_HANDLE();
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetMPBGpuDescriptorHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			auto info = mpb.Get(handle);
			return info != nullptr ? GetGpuDescriptorHandle(bType, info->index[(uint)bType]) : CD3DX12_GPU_DESCRIPTOR_HANDLE();
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
		DirectX::XMVECTORF32 JDx12GraphicResourceManager::GetBlackColor()const noexcept
		{
			return DirectX::XMVECTORF32{ {{0, 0, 0, 0}} };
		}
		DirectX::XMVECTORF32 JDx12GraphicResourceManager::GetBackBufferClearColor()const noexcept
		{
			return DirectX::XMVECTORF32{ {{Constants::backBufferClearColor.x,
				Constants::backBufferClearColor.y,
				Constants::backBufferClearColor.z,
				Constants::backBufferClearColor.w}} };
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
		ResourceHandle JDx12GraphicResourceManager::GetResourceGpuHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept
		{
			return (ResourceHandle)GetGpuDescriptorHandle(bType, index).ptr;
		}
		ResourceHandle JDx12GraphicResourceManager::GetMPBResourceCpuHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return (ResourceHandle)GetMPBCpuDescriptorHandle(handle, bType).ptr;
		}
		ResourceHandle JDx12GraphicResourceManager::GetMPBResourceGpuHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return (ResourceHandle)GetMPBGpuDescriptorHandle(handle, bType).ptr;
		}
		ID3D12Resource* JDx12GraphicResourceManager::GetResource(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept
		{
			return resource[(int)rType][index]->resourceHolder->GetResource();
		}
		ID3D12Resource* JDx12GraphicResourceManager::GetOptionResource(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, int index)const noexcept
		{
			return resource[(int)rType][index]->optionHolderSet->holder[(uint)opType]->GetResource();
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
		bool JDx12GraphicResourceManager::CanCreateResource(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			auto& desc = typeDesc[(uint)rType];
			bool hasSpace = true;
			for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
				hasSpace &= Private::InitGraphicResourceViewCapacity(rType, (J_GRAPHIC_BIND_TYPE)i) > 0 ? desc.viewInfo[i].HasSpace() : true;
			return desc.HasSpace() & hasSpace;
		}
		bool JDx12GraphicResourceManager::CanCreateResource(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, JGraphicDevice* device)noexcept
		{
			if (!IsSameDevice(device))
				return false;

			return CanCreateResource(graphicResourceType);
		}
		bool JDx12GraphicResourceManager::CanCreateOptionResource(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			auto& desc = typeDesc[(uint)rType];
			bool hasSpace = true;
			for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
				hasSpace &= Private::InitGraphicResourceOptionViewCapacity(opType, rType, (J_GRAPHIC_BIND_TYPE)i) > 0 ? desc.viewInfo[i].HasSpace() : true;
			return desc.HasSpace() & hasSpace;
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
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN;
			JDx12GraphicResourceManager::ResourceTypeDesc& swapChainDesc = typeDesc[(int)rType];
			resource[(int)rType].clear();
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

			//resource[(int)rType].resize(Private::swapChainBufferCount);
			for (uint i = 0; i < Private::swapChainBufferCount; ++i)
			{
				Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
				ThrowIfFailedHr(swapChain->GetBuffer(i, IID_PPV_ARGS(&newResource)));
				CreateResourceInfo(rType, std::move(newResource));
				BindSwapChain(BindDesc(device, i));
			}
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateSceneDepthStencilResource(JGraphicDevice* device, const uint viewWidth, const uint viewHeight)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL;
			if (!CanCreateResource(rType, device))
				return nullptr;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			Microsoft::WRL::ComPtr<ID3D12Resource> newSceneDsResource; ;
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
			CreateResourceInfo(rType, std::move(newSceneDsResource));

			const uint resourceIndex = resource[(int)rType].size() - 1;
			BindMainDepthStencil(BindDesc(data.device, resourceIndex));
			auto& result = resource[(int)rType][resourceIndex];;

			Private::EndCreation(device, data);
			return result;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateDebugDepthStencilResource(JGraphicDevice* device, const uint viewWidth, const uint viewHeight)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL;
			if (!CanCreateResource(rType, device))
				return nullptr;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			Microsoft::WRL::ComPtr<ID3D12Resource> newDebugDsResource;
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
			CreateResourceInfo(rType, std::move(newDebugDsResource));

			uint resourceIndex = resource[(int)rType].size() - 1;
			BindDebugDepthStencil(BindDesc(data.device, resourceIndex));
			auto& result = resource[(int)rType][resourceIndex];

			Private::EndCreation(device, data);
			return  result;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateLayerDepthDebugResource(JGraphicDevice* device, const uint viewWidth, const uint viewHeight)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG;
			if (!CanCreateResource(rType, device))
				return nullptr;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			Microsoft::WRL::ComPtr<ID3D12Resource> newLayerDepthDebugResource;
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
			CreateResourceInfo(rType, std::move(newLayerDepthDebugResource));

			const uint resourceIndex = resource[(int)rType].size() - 1;
			BindLayerDetphMapDebug(BindDesc(data.device, resourceIndex));
			auto& result = resource[(int)rType][resourceIndex];

			Private::EndCreation(device, data);
			return  result;
		}
		void JDx12GraphicResourceManager::CreateHZBOcclusionResource(JGraphicDevice* device,
			const uint occWidth,
			const uint occHeight,
			_Out_ JUserPtr<JGraphicResourceInfo>& outOccDsInfo,
			_Out_ JUserPtr<JGraphicResourceInfo>& outOccMipmapInfo)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE fRType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP;
			static constexpr J_GRAPHIC_RESOURCE_TYPE sRType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP;
			if (!CanCreateResource(fRType, device) || !CanCreateResource(sRType, device))
				return;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);
			 
			Microsoft::WRL::ComPtr<ID3D12Resource> occDsResource = std::move(Private::CreateOcclusionDepthMap(data.device, occWidth, occHeight));
			CreateResourceInfo(fRType, std::move(occDsResource));

			uint resourceIndex = resource[(int)fRType].size() - 1;
			BindOcclusionDepthMap(BindDesc(data.device, resourceIndex));
			outOccDsInfo = resource[(int)fRType][resourceIndex];

			D3D12_RESOURCE_DESC mipMapDesc;
			ZeroMemory(&mipMapDesc, sizeof(D3D12_RESOURCE_DESC));
			mipMapDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			mipMapDesc.Alignment = 0;
			mipMapDesc.Width = occWidth;
			mipMapDesc.Height = occHeight;
			mipMapDesc.DepthOrArraySize = 1;
			mipMapDesc.MipLevels = 0;	// 0 = �ִ� miplevel �ڵ����
			mipMapDesc.SampleDesc.Count = 1;
			mipMapDesc.SampleDesc.Quality = 0;
			mipMapDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			mipMapDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			mipMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			Microsoft::WRL::ComPtr<ID3D12Resource> occMipmapResource;
			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
			ThrowIfFailedG(data.device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&mipMapDesc,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				nullptr,
				IID_PPV_ARGS(&occMipmapResource)));
			CreateResourceInfo(sRType, std::move(occMipmapResource));

			resourceIndex = resource[(int)sRType].size() - 1;
			BindHZBOcclusionDepthMipmap(BindDesc(data.device, resourceIndex));
			outOccMipmapInfo = resource[(int)sRType][resourceIndex];
			Private::EndCreation(device, data);
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateOcclusionResourceDebug(JGraphicDevice* device,
			const uint occWidth,
			const uint occHeight,
			const bool isHzb)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG;
			if (!CanCreateResource(rType, device))
				return nullptr;
			
			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			Microsoft::WRL::ComPtr<ID3D12Resource> occDebugResource;
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
			CreateResourceInfo(rType, std::move(occDebugResource));

			uint resourceIndex = resource[(int)rType].size() - 1;
			if (isHzb)
				BindHZBOcclusionDebug(BindDesc(data.device, resourceIndex));
			else
				BindHdOcclusionDebug(BindDesc(data.device, resourceIndex));
			auto& result = resource[(int)rType][resourceIndex];

			Private::EndCreation(device, data);
			return  result;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::Create2DTexture(JGraphicDevice* device, const JTextureCreateDesc& createDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D;
			if (!CanCreateResource(rType, device))
				return nullptr;
			 	
			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource = nullptr;
			Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer = nullptr;

			std::unique_ptr<DirectX::ResourceUploadBatch> uploadBatch = nullptr;
			uint heapIndex = GetHeapIndex(rType, J_GRAPHIC_BIND_TYPE::SRV);
			const bool isFirst = JCUtil::GetFileFormat(createDesc.path) != Core::JFileConstant::GetFileFormatW();
			D3D12_RESOURCE_FLAGS flag = createDesc.allowUav ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
			JUserPtr<JGraphicResourceInfo> resourceUser = nullptr;	 
			HRESULT res;

			if (createDesc.oriFormat == L".dds")
			{
				res = DirectX::CreateDDSTextureFromFile12(data.device,
					data.commandList,
					createDesc.path.c_str(),
					newResource,
					uploadBuffer,
					createDesc.maxSize,
					nullptr,
					flag);
			}
			else
			{
				uploadBatch = std::make_unique<DirectX::ResourceUploadBatch>(data.device);
				uploadBatch->Begin(); 
				res = DirectX::CreateWICTextureFromFileEx(data.device, *uploadBatch, createDesc.path.c_str(), createDesc.maxSize, flag, DirectX::WIC_LOADER_FORCE_RGBA32, newResource.GetAddressOf());
				uploadBatch->End(data.commandQueue); 
			}
			if (SUCCEEDED(res))
			{
				std::wstring folder;
				std::wstring name;
				std::wstring format;
				JCUtil::DecomposeFilePath(createDesc.path, folder, name, format);

				CreateResourceInfo(rType, std::move(newResource));

				const uint resourceIndex = resource[(int)rType].size() - 1;
				Bind2DTexture(BindDesc(data.device, resourceIndex, createDesc.useMipmap));
				resourceUser = resource[(int)rType][resourceIndex];
			}
			else
				MessageBox(0, L"FAIL", 0, 0);
			Private::EndCreation(device, data);
			if (SUCCEEDED(res) && !data.startCommandThisCreation)
			{
				//�Լ� ȣ�������� publicCommand�� �۵����Ͻ�
				//uploadBuffer�� ������ ���� ���⼭ command�� �����ϰ� �ٽ� �۵���Ų��.
				data.dxDevice->EndPublicCommand();
				data.dxDevice->FlushCommandQueue();
				data.dxDevice->StartPublicCommand();
			}
			uploadBuffer.Reset();
			uploadBatch = nullptr;
			return resourceUser;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateCubeMap(JGraphicDevice* device, const JTextureCreateDesc& createDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE;
			if (!CanCreateResource(rType, device))
				return nullptr;
			 
			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;

			uint heapIndex = GetHeapIndex(rType, J_GRAPHIC_BIND_TYPE::SRV);
			D3D12_RESOURCE_FLAGS flag = createDesc.allowUav ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
			JUserPtr<JGraphicResourceInfo> resourceUser = nullptr;
			bool res = false;
			if (createDesc.oriFormat == L".dds")
			{
				res = DirectX::CreateDDSTextureFromFile12(data.device,
					data.commandList,
					createDesc.path.c_str(),
					newResource,
					uploadBuffer,
					createDesc.maxSize,
					nullptr,
					flag) == S_OK;
			}
			else
				assert(L"InValid cube map format");
			if (res)
			{
				std::wstring folder;
				std::wstring name;
				std::wstring format;
				JCUtil::DecomposeFilePath(createDesc.path, folder, name, format);

				CreateResourceInfo(rType, std::move(newResource));

				const uint resourceIndex = resource[(int)rType].size() - 1;
				BindCubeMap(BindDesc(data.device, resourceIndex, createDesc.useMipmap));
				resourceUser = resource[(int)rType][resourceIndex];
			}
			Private::EndCreation(device, data);
			if (res && !data.startCommandThisCreation)
			{
				//�Լ� ȣ�������� publicCommand�� �۵����Ͻ�
				//uploadBuffer�� ������ ���� ���⼭ command�� �����ϰ� �ٽ� �۵���Ų��.
				data.dxDevice->EndPublicCommand();
				data.dxDevice->FlushCommandQueue();
				data.dxDevice->StartPublicCommand();
			}
			uploadBuffer.Reset();
			return resourceUser;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateRenderTargetTexture(JGraphicDevice* device, const uint width, const uint height)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON;
			if (!CanCreateResource(rType, device))
				return nullptr;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
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
			CreateResourceInfo(rType, std::move(newResource));

			const uint resourceIndex = resource[(int)rType].size() - 1;
			BindRenderTarget(BindDesc(data.device, resourceIndex));
			auto& result = resource[(int)rType][resourceIndex];

			Private::EndCreation(device, data);
			return result;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateShadowMapTexture(JGraphicDevice* device, const uint width, const uint height)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP;
			if (!CanCreateResource(rType, device))
				return nullptr;
			 
			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
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
			CreateResourceInfo(rType, std::move(newResource));

			const uint resourceIndex = resource[(int)rType].size() - 1;
			BindShadowMap(BindDesc(data.device, resourceIndex));
			auto& result = resource[(int)rType][resourceIndex];

			Private::EndCreation(device, data);
			return result;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateShadowMapTextureArray(JGraphicDevice* device, const uint width, const uint height, const uint count)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY;
			if (!CanCreateResource(rType, device))
				return nullptr;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
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
			CreateResourceInfo(rType, std::move(newResource));

			const uint resourceIndex = resource[(int)rType].size() - 1;
			BindShadowMapArray(BindDesc(data.device, resourceIndex));
			auto& result = resource[(int)rType][resourceIndex];

			Private::EndCreation(device, data);
			return result;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateShadowMapTextureCube(JGraphicDevice* device, const uint width, const uint height)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE;
			if (!CanCreateResource(rType, device))
				return nullptr;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			D3D12_RESOURCE_DESC resourceDesc;
			ZeroMemory(&resourceDesc, sizeof(D3D12_RESOURCE_DESC));
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resourceDesc.Alignment = 0;
			resourceDesc.Width = width;
			resourceDesc.Height = height;
			resourceDesc.DepthOrArraySize = Constants::cubeMapPlaneCount;
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
			CreateResourceInfo(rType, std::move(newResource));

			const uint resourceIndex = resource[(int)rType].size() - 1;
			BindShadowMapCube(BindDesc(data.device, resourceIndex));
			auto& result = resource[(int)rType][resourceIndex];

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
		bool JDx12GraphicResourceManager::CreateOption(JGraphicDevice* device, JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
		{
			if (info == nullptr || !IsSameDevice(info.Get()))
				return false;
			 
			const J_GRAPHIC_RESOURCE_TYPE rType = info->GetGraphicResourceType();
			if(!JGraphicResourceType::CanUseOption(opType, rType) || !CanCreateOptionResource(opType, rType))
				return false;

			JDx12GraphicResourceInfo* dxInfo = static_cast<JDx12GraphicResourceInfo*>(info.Get());
			if (dxInfo->HasOptional(opType))
				return false;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource = nullptr;
			D3D12_HEAP_PROPERTIES heapProperties;
			D3D12_HEAP_FLAGS heapFlag;

			auto dxResource = dxInfo->resourceHolder->GetResource(); 
			auto resourceDesc = dxResource->GetDesc();
			auto clearFormat = resourceDesc.Format;
			dxResource->GetHeapProperties(&heapProperties, &heapFlag);  
			Private::GraphicOptionProperty(opType, resourceDesc, heapProperties, heapFlag, clearFormat);
		  
			CD3DX12_CLEAR_VALUE optClear(clearFormat, GetBlackColor());
			ThrowIfFailedHr(data.device->CreateCommittedResource(
				&heapProperties,
				heapFlag,
				&resourceDesc,
				Private::GraphicOptionInitState(rType),
				&optClear,
				IID_PPV_ARGS(&newResource)));
			 
			CreateOption(opType, dxInfo, std::move(newResource));
			ClearDescViewCountUntil(dxInfo);
			ReBind(data.device, rType, dxInfo->GetArrayIndex());
			Private::EndCreation(device, data);
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

			//ptr can nullptr ex) occlusion query(resource view binding�� ������� �ʴ� Ÿ��)
			if (ptr != nullptr)
			{
				ClearDescViewCountUntil(dxInfo);
				ReBind(data.device, rType, rIndex + 1, -1);
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
		bool JDx12GraphicResourceManager::DestroyGraphicOption(JGraphicDevice* device, JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE optype)
		{
			if (info == nullptr || !IsSameDevice(device))
				return false;

			if (!info->HasOptional(optype))
				return false;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);
			JDx12GraphicResourceInfo* dxInfo = static_cast<JDx12GraphicResourceInfo*>(info.Get());
			const J_GRAPHIC_RESOURCE_TYPE rType = dxInfo->GetGraphicResourceType();

			dxInfo->optionHolderSet->holder[(uint)optype] = nullptr;
			bool isEmpty = true;
			for (const auto& data : dxInfo->optionHolderSet->holder)
				isEmpty &= data == nullptr;
			if (isEmpty)
				dxInfo->optionHolderSet = nullptr;

			ClearDescViewCountUntil(dxInfo);
			ReBind(data.device, rType, dxInfo->GetArrayIndex());
			Private::EndCreation(device, data);
			return true;
		}
		void JDx12GraphicResourceManager::CreateResourceInfo(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, Microsoft::WRL::ComPtr<ID3D12Resource>&& d3dResource)
		{
			JDx12GraphicResourceInfo::GetHandlePtr getHandleLam = [](JDx12GraphicResourceManager* gm, const J_GRAPHIC_BIND_TYPE btype, const uint index)
			{
				return gm->GetGpuDescriptorHandle(btype, index);
			};

			auto dx12Holder = std::make_unique<JDx12GraphicResourceHolder>(std::move(d3dResource));
			auto newInfo = JOwnerPtr<JDx12GraphicResourceInfo>(new JDx12GraphicResourceInfo(graphicResourceType, this, std::move(dx12Holder), getHandleLam));
		 
			newInfo->SetArrayIndex(resource[(int)graphicResourceType].size());
			newInfo->SetPrivateName();
			resource[(int)graphicResourceType].push_back(std::move(newInfo));
			++typeDesc[(int)graphicResourceType].count;	
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateBuffer(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, JGraphicDevice* device, const void* bufferData, size_t bufferByteSize)
		{
			if (!CanCreateResource(graphicResourceType, device))
				return nullptr;

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);

			newResource = Private::CreateDefaultBuffer(data.device, data.commandList, bufferData, bufferByteSize, uploadBuffer);
			CreateResourceInfo(graphicResourceType, std::move(newResource));

			const uint resourceIndex = resource[(int)graphicResourceType].size() - 1;
			auto& result = resource[(int)graphicResourceType][resourceIndex];

			Private::EndCreation(device, data);
			uploadBuffer.Reset();
			return result;
		}
		void JDx12GraphicResourceManager::CreateOption(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, JDx12GraphicResourceInfo* dxInfo,  Microsoft::WRL::ComPtr<ID3D12Resource>&& resource)
		{
			if (dxInfo == nullptr || resource == nullptr)
				return;

			dxInfo->SetOption(opType, std::make_unique<JDx12GraphicResourceHolder>(std::move(resource))); 
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
				return &JDx12GraphicResourceManager::BindHZBOcclusionDepthMipmap;
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
		JDx12GraphicResourceManager::BindOptionViewPtr JDx12GraphicResourceManager::GetResourceBindOptionViewPtr(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
		{
			switch (opType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::POST_PROCESSING:
				return &JDx12GraphicResourceManager::BindPostProcessing;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP:
				return &JDx12GraphicResourceManager::BindNormalMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::AMBIENT_OCCLISION_MAP:
				return &JDx12GraphicResourceManager::BindAmibientOcclusionMap;
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
		void JDx12GraphicResourceManager::SetOptionViewCount(JDx12GraphicResourceInfo* handlePtr, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const int nextViewIndex)
		{
			if (handlePtr->GetOptionViewCount(bType, opType) == 0)
			{
				handlePtr->SetHeapOptionIndexStart(bType, opType, nextViewIndex);
				handlePtr->SetOptionViewCount(bType, opType, 1);
			}
			else
			{
				const int viewCount = handlePtr->GetOptionViewCount(bType, opType) + 1;
				handlePtr->SetOptionViewCount(bType, opType, viewCount);
			}
		}
		void JDx12GraphicResourceManager::ClearDescViewCountUntil(JDx12GraphicResourceInfo* dxInfo)
		{
			if (dxInfo == nullptr)
				return;

			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)dxInfo->GetGraphicResourceType()];
			//type per total view count controll
			for (uint i = 0; i < (int)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
			{
				//���ε� �Ǿ����� ������ count < 0
				//heapIndexStart�� ���ε����� ���� ��� �⺻���� -1�� �������ִ� �����̴�.
				//graphic resource Ÿ�Ը��� binding ������ �������� ������ ��ȿ�ϴ�
				desc.viewInfo[i].count = dxInfo->GetHeapIndexStart((J_GRAPHIC_BIND_TYPE)i) - desc.viewInfo[i].offset;
				if (desc.viewInfo[i].count < 0)
					desc.viewInfo[i].count = 0;
			}
		}
		void JDx12GraphicResourceManager::ReBind(ID3D12Device* device, const J_GRAPHIC_RESOURCE_TYPE rType, const uint index, const int additionalArrayIndex)
		{
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];
			BindViewPtr ptr = GetResourceBindViewPtr(rType);
			if (ptr == nullptr)
				return;

			//type per resource view controll
			for (uint i = index; i < desc.count; ++i)
			{
				//init heap st, bind value
				JDx12GraphicResourceInfo* reBindHandle = resource[(int)rType][i].Get();
				for (uint j = 0; j < (int)J_GRAPHIC_BIND_TYPE::COUNT; ++j)
				{
					reBindHandle->SetHeapIndexStart((J_GRAPHIC_BIND_TYPE)j, -1);
					reBindHandle->SetViewCount((J_GRAPHIC_BIND_TYPE)j, 0);
				}

				//reset array, heap st, bind value
				if (additionalArrayIndex != 0)
				{
					const int newIndex = i + additionalArrayIndex;
					reBindHandle->SetArrayIndex(newIndex);
				}
				BindDesc desc(device, i);
				(this->*ptr)(desc);
			}
		}
		void JDx12GraphicResourceManager::BindSwapChain(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];

			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(rType)][bDesc.resourceIndex].Get();
			const int nextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::RTV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::RTV, nextViewIndex);
			bDesc.device->CreateRenderTargetView(resourcePtr, nullptr, GetCpuRtvDescriptorHandle(nextViewIndex));
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::RTV].count;
		}
		void JDx12GraphicResourceManager::BindMainDepthStencil(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];

			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(rType)][bDesc.resourceIndex].Get();
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

			bDesc.device->CreateDepthStencilView(resourcePtr, &dsvDesc, GetCpuDsvDescriptorHandle(dsvNextViewIndex));

			D3D12_SHADER_RESOURCE_VIEW_DESC depthSrvDesc = {};
			depthSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			depthSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			depthSrvDesc.Texture2D.MostDetailedMip = 0;
			depthSrvDesc.Texture2D.MipLevels = 1;
			depthSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			depthSrvDesc.Texture2D.PlaneSlice = 0;
			depthSrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

			//Depth 
			bDesc.device->CreateShaderResourceView(resourcePtr, &depthSrvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));

			////Stencil 
			depthSrvDesc.Texture2D.PlaneSlice = 1;
			depthSrvDesc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
			bDesc.device->CreateShaderResourceView(resourcePtr, &depthSrvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex + 1));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].count;
			desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count += 2;
		}
		void JDx12GraphicResourceManager::BindDebugDepthStencil(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];

			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[(int)rType][bDesc.resourceIndex].Get();
			const int dsvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::DSV, dsvNextViewIndex);

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Format = depthStencilFormat;
			dsvDesc.Texture2D.MipSlice = 0;

			bDesc.device->CreateDepthStencilView(resourcePtr, &dsvDesc, GetCpuDsvDescriptorHandle(dsvNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].count;
		}
		void JDx12GraphicResourceManager::BindLayerDetphMapDebug(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];

			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(rType)][bDesc.resourceIndex].Get();
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

			bDesc.device->CreateShaderResourceView(resourcePtr, &debugSrvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));
			bDesc.device->CreateUnorderedAccessView(resourcePtr, nullptr, &debugUavDesc, GetCpuSrvDescriptorHandle(uavNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].count;
		}
		void JDx12GraphicResourceManager::BindOcclusionDepthMap(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];

			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(rType)][bDesc.resourceIndex].Get();
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

			bDesc.device->CreateDepthStencilView(resourcePtr, &dsvDesc, GetCpuDsvDescriptorHandle(dsvNextViewIndex));
			bDesc.device->CreateShaderResourceView(resourcePtr, &depthSrvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].count;
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;
		}
		void JDx12GraphicResourceManager::BindHZBOcclusionDepthMipmap(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];

			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(rType)][bDesc.resourceIndex].Get();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();
			const int uavNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].GetNextViewIndex();

			uint nowWidth = resourcePtr->GetDesc().Width;
			uint nowHeight = resourcePtr->GetDesc().Height;

			uint occlusionCount = 0;
			uint occlusionMipmapViewCapacity = GetOcclusionMipmapViewCapacity();
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

				bDesc.device->CreateShaderResourceView(resourcePtr, &mipMapSrvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex + occlusionCount));
				bDesc.device->CreateUnorderedAccessView(resourcePtr, nullptr, &uavDesc, GetCpuSrvDescriptorHandle(uavNextViewIndex + occlusionCount));

				if (nowWidth > 1)
					nowWidth /= 2;
				if (nowHeight > 1)
					nowHeight /= 2;
				++occlusionCount;
			}
			desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count += occlusionCount;
			desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].count += occlusionCount;
		}
		void JDx12GraphicResourceManager::BindHZBOcclusionDebug(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];

			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(rType)][bDesc.resourceIndex].Get();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();
			const int uavNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].GetNextViewIndex();

			uint nowWidth = resourcePtr->GetDesc().Width;
			uint nowHeight = resourcePtr->GetDesc().Height;

			uint occlusionCount = 0;
			uint occlusionMipmapViewCapacity = GetOcclusionMipmapViewCapacity();
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

				bDesc.device->CreateShaderResourceView(resourcePtr, &debugSrvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex + occlusionCount));
				bDesc.device->CreateUnorderedAccessView(resourcePtr, nullptr, &uavDesc, GetCpuSrvDescriptorHandle(uavNextViewIndex + occlusionCount));

				if (nowWidth > 1)
					nowWidth /= 2;
				if (nowHeight > 1)
					nowHeight /= 2;
				++occlusionCount;
			}
			desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count += occlusionCount;
			desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].count += occlusionCount;
		}
		void JDx12GraphicResourceManager::BindHdOcclusionDebug(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];

			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(rType)][bDesc.resourceIndex].Get();
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

			bDesc.device->CreateShaderResourceView(resourcePtr, &debugSrvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));
			bDesc.device->CreateUnorderedAccessView(resourcePtr, nullptr, &uavDesc, GetCpuSrvDescriptorHandle(uavNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].count;
		}
		void JDx12GraphicResourceManager::Bind2DTexture(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];

			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(rType)][bDesc.resourceIndex].Get();

			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));
			srvDesc.Format = resourceDesc.Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = bDesc.useMipmap ? resourceDesc.MipLevels : 1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			 
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();
			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);

			bDesc.device->CreateShaderResourceView(resourcePtr, &srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;
		}
		void JDx12GraphicResourceManager::BindCubeMap(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];

			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(rType)][bDesc.resourceIndex].Get();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = bDesc.useMipmap ? resourcePtr->GetDesc().MipLevels : 1;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			srvDesc.Format = resourcePtr->GetDesc().Format;

			bDesc.device->CreateShaderResourceView(resourcePtr, &srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;
		}
		void JDx12GraphicResourceManager::BindRenderTarget(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];

			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(rType)][bDesc.resourceIndex].Get();

			const int rtvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::RTV].GetNextViewIndex();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::RTV, rtvNextViewIndex);
			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);

			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Format = resourceDesc.Format;
			rtvDesc.Texture2D.MipSlice = 0;
			rtvDesc.Texture2D.PlaneSlice = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Format = resourceDesc.Format;
			srvDesc.Texture2D.MipLevels = 1;

			bDesc.device->CreateRenderTargetView(resourcePtr, &rtvDesc, GetCpuRtvDescriptorHandle(rtvNextViewIndex));
			bDesc.device->CreateShaderResourceView(resourcePtr, &srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::RTV].count;
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;

			if (handlePtr->HasOptional(J_GRAPHIC_RESOURCE_OPTION_TYPE::POST_PROCESSING))
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = resourceDesc.Format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;

				BindOptionDesc opDesc(bDesc);
				opDesc.rType = rType;
				opDesc.srvDesc = srvDesc;
				opDesc.uavDesc = uavDesc;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;

				BindPostProcessing(opDesc);
			}
			if (handlePtr->HasOptional(J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP))
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;
				 
				BindOptionDesc opDesc(bDesc);
				opDesc.rType = rType;
				opDesc.srvDesc = srvDesc;
				opDesc.srvDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
				opDesc.uavDesc = uavDesc;
				BindNormalMap(opDesc);
			}
			if (handlePtr->HasOptional(J_GRAPHIC_RESOURCE_OPTION_TYPE::AMBIENT_OCCLISION_MAP))
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = DXGI_FORMAT_R16_FLOAT;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;
				 
				BindOptionDesc opDesc(bDesc);
				opDesc.rType = rType;
				opDesc.srvDesc = srvDesc;
				opDesc.srvDesc.Format = DXGI_FORMAT_R16_FLOAT;
				opDesc.uavDesc = uavDesc;
				BindAmibientOcclusionMap(opDesc);
			}
		} 
		void JDx12GraphicResourceManager::BindShadowMap(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];

			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(rType)][bDesc.resourceIndex].Get();

			const int dsvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].GetNextViewIndex();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::DSV, dsvNextViewIndex);
			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);

			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc(); 
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
			srvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Texture2D.PlaneSlice = 0;

			bDesc.device->CreateDepthStencilView(resourcePtr, &dsvDesc, GetCpuDsvDescriptorHandle(dsvNextViewIndex));
			bDesc.device->CreateShaderResourceView(resourcePtr, &srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].count;
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;

			if (handlePtr->HasOptional(J_GRAPHIC_RESOURCE_OPTION_TYPE::POST_PROCESSING))
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = resourceDesc.Format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;

				BindOptionDesc opDesc(bDesc);
				opDesc.rType = rType;
				opDesc.srvDesc = srvDesc;
				opDesc.uavDesc = uavDesc;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;

				BindPostProcessing(opDesc);
			}
		}
		void JDx12GraphicResourceManager::BindShadowMapArray(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE type = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)type];

			ID3D12Resource* resourcePtr = GetResource(type, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(type)][bDesc.resourceIndex].Get();

			const int dsvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].GetNextViewIndex();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::DSV, dsvNextViewIndex);
			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);

			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			//dsvDesc.Format = depthStencilFormat;
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
			dsvDesc.Texture2DArray.FirstArraySlice = 0;
			dsvDesc.Texture2DArray.MipSlice = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			//srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
			srvDesc.Texture2DArray.FirstArraySlice = 0;
			srvDesc.Texture2DArray.MostDetailedMip = 0;
			srvDesc.Texture2DArray.MipLevels = resourceDesc.MipLevels;
			srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;

			bDesc.device->CreateDepthStencilView(resourcePtr, &dsvDesc, GetCpuDsvDescriptorHandle(dsvNextViewIndex));
			bDesc.device->CreateShaderResourceView(resourcePtr, &srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].count;
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;

			if (handlePtr->HasOptional(J_GRAPHIC_RESOURCE_OPTION_TYPE::POST_PROCESSING))
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = resourceDesc.Format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;

				BindOptionDesc opDesc(bDesc);
				opDesc.rType = type;
				opDesc.srvDesc = srvDesc;
				opDesc.uavDesc = uavDesc;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;

				BindPostProcessing(opDesc);
			}
		}
		void JDx12GraphicResourceManager::BindShadowMapCube(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE type = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)type];

			ID3D12Resource* resourcePtr = GetResource(type, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(type)][bDesc.resourceIndex].Get();

			const int dsvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].GetNextViewIndex();
			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();

			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::DSV, dsvNextViewIndex);
			SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, srvNextViewIndex);

			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc(); 
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			//dsvDesc.Format = depthStencilFormat;
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.Texture2DArray.ArraySize = Constants::cubeMapPlaneCount;
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
			srvDesc.TextureCube.MipLevels = resourceDesc.MipLevels;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			 
			bDesc.device->CreateDepthStencilView(resourcePtr, &dsvDesc, GetCpuDsvDescriptorHandle(dsvNextViewIndex));
			bDesc.device->CreateShaderResourceView(resourcePtr, &srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));

			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::DSV].count;
			++desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].count;

			if (handlePtr->HasOptional(J_GRAPHIC_RESOURCE_OPTION_TYPE::POST_PROCESSING))
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = resourceDesc.Format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;

				BindOptionDesc opDesc(bDesc);
				opDesc.rType = type;
				opDesc.srvDesc = srvDesc;
				opDesc.uavDesc = uavDesc;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;

				BindPostProcessing(opDesc);
			}
		}
		void JDx12GraphicResourceManager::BindPostProcessing(const BindOptionDesc& opDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_OPTION_TYPE opType = J_GRAPHIC_RESOURCE_OPTION_TYPE::POST_PROCESSING;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)opDesc.rType];
			JDx12GraphicResourceInfo* handlePtr = resource[int(opDesc.rType)][opDesc.resourceBindDesc.resourceIndex].Get();
			ID3D12Resource* resourcePtr = GetResource(opDesc.rType, opDesc.resourceBindDesc.resourceIndex);
			ID3D12Resource* optResourcePtr = handlePtr->optionHolderSet->holder[(uint)opType]->GetResource(); 

			if (opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::UAV])
			{
				const int uavNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].GetNextViewIndex();
				SetViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::UAV, uavNextViewIndex);
				opDesc.resourceBindDesc.device->CreateUnorderedAccessView(resourcePtr, nullptr, &opDesc.uavDesc, GetCpuSrvDescriptorHandle(uavNextViewIndex));
				++desc.viewInfo[(uint)J_GRAPHIC_BIND_TYPE::UAV].count;
			}

			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();
			const int uavNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].GetNextViewIndex();
	 
			SetOptionViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, opType, srvNextViewIndex);
			SetOptionViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::UAV, opType, uavNextViewIndex);
			  
			opDesc.resourceBindDesc.device->CreateShaderResourceView(optResourcePtr, &opDesc.srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));
			opDesc.resourceBindDesc.device->CreateUnorderedAccessView(optResourcePtr, nullptr, &opDesc.uavDesc, GetCpuSrvDescriptorHandle(uavNextViewIndex));

			++desc.viewInfo[(uint)J_GRAPHIC_BIND_TYPE::SRV].count;
			++desc.viewInfo[(uint)J_GRAPHIC_BIND_TYPE::UAV].count;
		}
		void JDx12GraphicResourceManager::BindNormalMap(const BindOptionDesc& opDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_OPTION_TYPE opType = J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)opDesc.rType]; 
			JDx12GraphicResourceInfo* handlePtr = resource[int(opDesc.rType)][opDesc.resourceBindDesc.resourceIndex].Get();
			ID3D12Resource* resourcePtr = GetResource(opDesc.rType, opDesc.resourceBindDesc.resourceIndex);
			ID3D12Resource* optResourcePtr = handlePtr->optionHolderSet->holder[(uint)opType]->GetResource();

			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();
			const int uavNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].GetNextViewIndex();

			SetOptionViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, opType, srvNextViewIndex);
			SetOptionViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::UAV, opType, uavNextViewIndex);

			opDesc.resourceBindDesc.device->CreateShaderResourceView(optResourcePtr, &opDesc.srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));
			opDesc.resourceBindDesc.device->CreateUnorderedAccessView(optResourcePtr, nullptr, &opDesc.uavDesc, GetCpuSrvDescriptorHandle(uavNextViewIndex));

			++desc.viewInfo[(uint)J_GRAPHIC_BIND_TYPE::SRV].count;
			++desc.viewInfo[(uint)J_GRAPHIC_BIND_TYPE::UAV].count;
		}
		void JDx12GraphicResourceManager::BindAmibientOcclusionMap(const BindOptionDesc& opDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_OPTION_TYPE opType = J_GRAPHIC_RESOURCE_OPTION_TYPE::AMBIENT_OCCLISION_MAP;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)opDesc.rType]; 
			JDx12GraphicResourceInfo* handlePtr = resource[int(opDesc.rType)][opDesc.resourceBindDesc.resourceIndex].Get();
			ID3D12Resource* resourcePtr = GetResource(opDesc.rType, opDesc.resourceBindDesc.resourceIndex);
			ID3D12Resource* optResourcePtr = handlePtr->optionHolderSet->holder[(uint)opType]->GetResource();

			const int srvNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].GetNextViewIndex();
			const int uavNextViewIndex = desc.viewInfo[(int)J_GRAPHIC_BIND_TYPE::UAV].GetNextViewIndex();

			SetOptionViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::SRV, opType, srvNextViewIndex);
			SetOptionViewCount(handlePtr, J_GRAPHIC_BIND_TYPE::UAV, opType, uavNextViewIndex);

			opDesc.resourceBindDesc.device->CreateShaderResourceView(optResourcePtr, &opDesc.srvDesc, GetCpuSrvDescriptorHandle(srvNextViewIndex));
			opDesc.resourceBindDesc.device->CreateUnorderedAccessView(optResourcePtr, nullptr, &opDesc.uavDesc, GetCpuSrvDescriptorHandle(uavNextViewIndex));

			++desc.viewInfo[(uint)J_GRAPHIC_BIND_TYPE::SRV].count;
			++desc.viewInfo[(uint)J_GRAPHIC_BIND_TYPE::UAV].count;
		}
		bool JDx12GraphicResourceManager::CopyResource(JGraphicDevice* device, const JUserPtr<JGraphicResourceInfo>& from, const JUserPtr<JGraphicResourceInfo>& to)
		{
			if (device == nullptr || !IsSameDevice(device) || from == nullptr || to == nullptr)
				return false;

			Private::DeviceData data(device);
			Private::StartCreation(device, data);
			auto fromR = GetResource(from->GetGraphicResourceType(), from->GetArrayIndex());
			auto toR = GetResource(to->GetGraphicResourceType(), to->GetArrayIndex());

			JD3DUtility::ResourceTransition(data.commandList, fromR, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE);
			JD3DUtility::ResourceTransition(data.commandList, toR, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
			data.commandList->CopyResource(toR, fromR);
			JD3DUtility::ResourceTransition(data.commandList, toR, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			JD3DUtility::ResourceTransition(data.commandList, fromR, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			 
			Private::EndCreation(device, data);
			return true;
		}
		bool JDx12GraphicResourceManager::SettingMipmapBind(JGraphicDevice* device, const JUserPtr<JGraphicResourceInfo>& info, const bool isReadOnly, _Out_ std::vector<Core::JDataHandle>& handle)
		{ 
			handle.clear();
			if (device == nullptr || !IsSameDevice(device) || info == nullptr || !Private::CanCreateMipmap(info->GetGraphicResourceType()) || !mpb.CanAdd())
				return false;
			 
			auto resource = static_cast<JDx12GraphicResourceInfo*>(info.Get())->resourceHolder->GetResource();
			auto desc = resource->GetDesc(); 
			if (mpb.GetEmptyCount() < desc.MipLevels || desc.MipLevels < 2)
				return false;

			Private::DeviceData data(device);
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));
			srvDesc.Format = desc.Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; 
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = desc.Format;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D; 

			if (info->GetGraphicResourceType() == J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			}

			const uint mipLevelCount = desc.MipLevels;
			for (uint i = 0; i < mipLevelCount; ++i)
			{ 
				srvDesc.Texture2D.MostDetailedMip = i;
				srvDesc.Texture2D.MipLevels = 1;
				srvDesc.Texture2D.ResourceMinLODClamp = i;
				uavDesc.Texture2D.MipSlice = i; 

				auto mpbInfo = JPtrUtil::MakeOwnerPtr<MPBInfo>();
				mpbInfo->info = info;
				mpbInfo->index[(uint)J_GRAPHIC_BIND_TYPE::SRV] = GetMPBOffset(J_GRAPHIC_BIND_TYPE::SRV) + mpb.GetValidIndex();
				data.device->CreateShaderResourceView(resource, &srvDesc, GetCpuSrvDescriptorHandle(mpbInfo->index[(uint)J_GRAPHIC_BIND_TYPE::SRV]));

				if (!isReadOnly)
				{
					mpbInfo->index[(uint)J_GRAPHIC_BIND_TYPE::UAV] = GetMPBOffset(J_GRAPHIC_BIND_TYPE::UAV) + mpb.GetValidIndex();
					data.device->CreateUnorderedAccessView(resource, nullptr, &uavDesc, GetCpuSrvDescriptorHandle(mpbInfo->index[(uint)J_GRAPHIC_BIND_TYPE::UAV]));
				}
				handle.push_back(mpb.Add(std::move(mpbInfo)));
			}  
			return true;
		}
		void JDx12GraphicResourceManager::DestroyMPB(JGraphicDevice* device, Core::JDataHandle& handle)
		{
			if (device == nullptr || !IsSameDevice(device))
				return;
			 
			mpb.Remove(handle); 
		}
		uint JDx12GraphicResourceManager::GetMPBOffset(const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			switch (bType)
			{
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
			{
				return GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::SRV) + GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::UAV) +
					Private::InitGraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::SRV) + Private::InitGraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::UAV);
			}
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
			{
				return GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::SRV) + GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::UAV) +
					Private::InitGraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::SRV) + Private::InitGraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::UAV) +
					Private::MPBCount(J_GRAPHIC_BIND_TYPE::SRV);
			}
			default:
				return 0;
			}
		}
		void JDx12GraphicResourceManager::BuildRtvDescriptorHeaps(ID3D12Device* device)
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
			rtvHeapDesc.NumDescriptors = GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::RTV) +
				Private::InitGraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::RTV);
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
				Private::InitGraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::DSV);
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
				Private::InitGraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::SRV) +
				Private::InitGraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::UAV) + 
				Private::MPBCount(J_GRAPHIC_BIND_TYPE::SRV) + 
				Private::MPBCount(J_GRAPHIC_BIND_TYPE::UAV);
			 
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

				//Release�� �������������� Reset�� ��ȿ�� pointer�� �����ϹǷ� pointer �ı��� �õ��ϸ�
				//���� alloc class���� �޸𸮸� ���ġ�ϴ� �������� ������ ����ų�� �����Ƿ�
				//Release() �Ѵ��� Reset()�� ȣ���ؾ��Ѵ�.
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