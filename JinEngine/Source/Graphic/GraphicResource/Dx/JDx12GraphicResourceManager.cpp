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
 
#include"../../../Develop/Debug/JDevelopDebug.h"
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

			using CreaetD3dResource = Core::JSFunctorType<Microsoft::WRL::ComPtr<ID3D12Resource>, const DeviceData&, const JGraphicResourceCreationDesc&>;

			static constexpr uint swapChainBufferCount = 2;
			static constexpr uint vertexCapacity = INT_MAX;	//mesh 갯수만큼 할당가능
			static constexpr uint indexCapacity = INT_MAX;		//mesh 갯수만큼 할당가능 
			static constexpr size_t lightClusterNodeSize = sizeof(uint64);
			static constexpr size_t lightClusterOffsetSize = sizeof(uint32);

			static DXGI_FORMAT GetBackBufferFormat()noexcept
			{
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			}
			static DXGI_FORMAT GetDepthStencilFormat()noexcept
			{
				return DXGI_FORMAT_D24_UNORM_S8_UINT;
			}
			static DirectX::XMVECTORF32 GetBackBufferClearColor()noexcept
			{
				const JVector4F v4 = Constants::BackBufferClearColor();
				return DirectX::XMVECTORF32{ {{v4.x, v4.y, v4.z, v4.w}} };
			}
			static DirectX::XMVECTORF32 GetBlackColor()noexcept
			{
				return DirectX::XMVECTORF32{ {{0, 0, 0, 0}} };
			} 
			static DirectX::XMVECTORF32 GetWhiteColor()noexcept
			{
				return DirectX::XMVECTORF32{ {{1.0f, 1.0f, 1.0f, 1.0f}} };
			} 

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
					return 512;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL:
					return 16;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP:
					return 512;
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
					return 512;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING:
					return 512 * 3;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
					return 256;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY:
					return 16;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE:
					return 128;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
					return 16;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET:
					return 16;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP:
					return 16;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERMEDIATE_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERLEAVE_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_INTERLEAVE_MAP:
					return 8;
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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP:
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
						return initResourceCapacity;
					default:
						break;
					}
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING:
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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET:
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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERMEDIATE_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_MAP:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return InitGraphicResourceCapacity(rType);
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
						return InitGraphicResourceCapacity(rType);
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return 0;
					default:
						return 0;
					}
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERLEAVE_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_INTERLEAVE_MAP: 
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return InitGraphicResourceCapacity(rType) * InitGraphicResourceCapacity(rType);
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
						return InitGraphicResourceCapacity(rType) * InitGraphicResourceCapacity(rType);
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return 0;
					default:
						return 0;
					}
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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::TANGENT_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY:
				{
					switch (bType)
					{
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
						return InitGraphicResourceCapacity(rType);
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
						return 0;
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
						return InitGraphicResourceCapacity(rType);
					case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
						return 0;
					default:
						return 0;
					}
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR:
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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER:
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

			static uint IsBuffer(const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				switch (rType)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::VERTEX:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::INDEX:
					return true;
				default:
					return false;
				}
			}
			static uint IsBuffer(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
			{
				switch (opType)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER:
					return true;
				default:
					return false;
				}
			}
 
			Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTarget(const DeviceData& data, 
				const JGraphicResourceCreationDesc& creationDesc,
				DXGI_FORMAT format,
				DirectX::XMVECTORF32 clareColor,
				const uint arraySize,
				const bool applyMsaa,
				const bool canUav)
			{
				Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
				D3D12_RESOURCE_DESC desc;
				ZeroMemory(&desc, sizeof(D3D12_RESOURCE_DESC));
				desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				desc.Alignment = 0;
				desc.Width = creationDesc.width;
				desc.Height = creationDesc.height;
				desc.DepthOrArraySize = arraySize;
				desc.MipLevels = 1;
				desc.Format = format;
				desc.SampleDesc.Count = (applyMsaa && data.m4xMsaaState) ? 4 : 1;
				desc.SampleDesc.Quality = (applyMsaa && data.m4xMsaaState) ? (data.m4xMsaaQuality - 1) : 0;
				desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
				if (canUav && creationDesc.bindDesc.HasRequestAdditionalBind(J_GRAPHIC_BIND_TYPE::UAV))
					desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

				//DXGI_FORMAT_R8G8B8A8_UNORM
				CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				CD3DX12_CLEAR_VALUE optClear(format, clareColor);
				ThrowIfFailedG(data.device->CreateCommittedResource(
					&heapProperty,
					D3D12_HEAP_FLAG_NONE,
					&desc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					&optClear,
					IID_PPV_ARGS(&newResource)));
				return newResource;
			}
			Microsoft::WRL::ComPtr<ID3D12Resource> CreateShadowMap(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc, DXGI_FORMAT format, const uint arraySize)
			{
				//Constants::cubeMapPlaneCount;
				Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
				D3D12_RESOURCE_DESC desc;
				ZeroMemory(&desc, sizeof(D3D12_RESOURCE_DESC));
				desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				desc.Alignment = 0;
				desc.Width = creationDesc.width;
				desc.Height = creationDesc.height;
				desc.DepthOrArraySize = arraySize;
				desc.MipLevels = 1;
				desc.Format = DXGI_FORMAT_R32_TYPELESS;
				//resourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
				desc.SampleDesc.Count = 1;
				desc.SampleDesc.Quality = 0;
				desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

				CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				CD3DX12_CLEAR_VALUE optClear;
				optClear.Format = DXGI_FORMAT_D32_FLOAT;
				//optClear.Format = depthStencilFormat;
				optClear.DepthStencil.Depth = 1.0f;
				optClear.DepthStencil.Stencil = 0;

				ThrowIfFailedHr(data.device->CreateCommittedResource(
					&heapProperty,
					D3D12_HEAP_FLAG_NONE,
					&desc,
					D3D12_RESOURCE_STATE_COMMON,
					&optClear,
					IID_PPV_ARGS(&newResource)));
				return newResource;
			}
			Microsoft::WRL::ComPtr<ID3D12Resource> CreateLightList(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc, const size_t bufferSize)
			{
				Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
				CD3DX12_RESOURCE_DESC resourceDesc(D3D12_RESOURCE_DIMENSION_BUFFER, 0, bufferSize, 1, 1, 1,
					DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

				CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				ThrowIfFailedHr(data.device->CreateCommittedResource(
					&heapProperty,
					D3D12_HEAP_FLAG_NONE,
					&resourceDesc,
					D3D12_RESOURCE_STATE_COMMON,
					nullptr,
					IID_PPV_ARGS(&newResource)));
				return newResource;
			}
			Microsoft::WRL::ComPtr<ID3D12Resource> CreateSsaoTexture(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc, DXGI_FORMAT format)
			{
				Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
				CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
				D3D12_RESOURCE_DESC desc;
				ZeroMemory(&desc, sizeof(D3D12_RESOURCE_DESC));
				desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				desc.Alignment = 0;
				desc.Width = creationDesc.width;
				desc.Height = creationDesc.height;
				desc.DepthOrArraySize = creationDesc.arraySize;
				desc.MipLevels = 1;
				desc.SampleDesc.Count = 1;
				desc.SampleDesc.Quality = 0;
				desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
				desc.Format = format;

				ThrowIfFailedG(data.device->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&desc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&newResource)));
				return newResource;
			}

			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateSceneDepthStencilResource(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				Microsoft::WRL::ComPtr<ID3D12Resource> newSceneDsResource;
				D3D12_RESOURCE_DESC depthStencilDesc;
				depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				depthStencilDesc.Alignment = 0;
				depthStencilDesc.Width = (uint)creationDesc.width;
				depthStencilDesc.Height = (uint)creationDesc.height;
				depthStencilDesc.DepthOrArraySize = 1;
				depthStencilDesc.MipLevels = 1;
				depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
				depthStencilDesc.SampleDesc.Count = data.m4xMsaaState ? 4 : 1;
				depthStencilDesc.SampleDesc.Quality = data.m4xMsaaState ? (data.m4xMsaaQuality - 1) : 0;
				depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

				CD3DX12_CLEAR_VALUE optClear;
				optClear.Format = GetDepthStencilFormat();
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
				return newSceneDsResource;
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDebugDepthStencilResource(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				Microsoft::WRL::ComPtr<ID3D12Resource> newDebugDsResource;
				D3D12_RESOURCE_DESC depthStencilDesc;
				depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				depthStencilDesc.Alignment = 0;
				depthStencilDesc.Width = creationDesc.width;
				depthStencilDesc.Height = creationDesc.height;
				depthStencilDesc.DepthOrArraySize = 1;
				depthStencilDesc.MipLevels = 1;
				depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
				depthStencilDesc.SampleDesc.Count = 1;
				depthStencilDesc.SampleDesc.Quality = 0;
				depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

				CD3DX12_CLEAR_VALUE optClear;
				optClear.Format = GetDepthStencilFormat();
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
				return newDebugDsResource;
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDebugMapResource(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				Microsoft::WRL::ComPtr<ID3D12Resource> newLayerDepthDebugResource;
				D3D12_RESOURCE_DESC debugDesc;
				ZeroMemory(&debugDesc, sizeof(D3D12_RESOURCE_DESC));
				debugDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				debugDesc.Alignment = 0;
				debugDesc.Width = creationDesc.width;
				debugDesc.Height = creationDesc.height;
				debugDesc.DepthOrArraySize = 1;
				debugDesc.MipLevels = 1;
				debugDesc.SampleDesc.Count = 1;
				debugDesc.SampleDesc.Quality = 0;
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
				return newLayerDepthDebugResource;
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateHZBOcclusionResource(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				Microsoft::WRL::ComPtr<ID3D12Resource> occDsResource;

				D3D12_RESOURCE_DESC depthStencilDesc;
				ZeroMemory(&depthStencilDesc, sizeof(D3D12_RESOURCE_DESC));
				depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				depthStencilDesc.Alignment = 0;
				depthStencilDesc.Width = creationDesc.width;
				depthStencilDesc.Height = creationDesc.height;
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
				ThrowIfFailedG(data.device->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&depthStencilDesc,
					D3D12_RESOURCE_STATE_DEPTH_READ,
					&optClear,
					IID_PPV_ARGS(&occDsResource)));

				return occDsResource;
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateHZBOcclusionMipmapResource(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				D3D12_RESOURCE_DESC mipMapDesc;
				ZeroMemory(&mipMapDesc, sizeof(D3D12_RESOURCE_DESC));
				mipMapDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				mipMapDesc.Alignment = 0;
				mipMapDesc.Width = creationDesc.width;
				mipMapDesc.Height = creationDesc.height;
				mipMapDesc.DepthOrArraySize = 1;
				mipMapDesc.MipLevels = 0;	// 0 = 최대 miplevel 자동계산
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
				return occMipmapResource;
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateOcclusionResourceDebug(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				Microsoft::WRL::ComPtr<ID3D12Resource> occDebugResource;
				CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
				D3D12_RESOURCE_DESC debugDesc;
				ZeroMemory(&debugDesc, sizeof(D3D12_RESOURCE_DESC));
				debugDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				debugDesc.Alignment = 0;
				debugDesc.Width = creationDesc.width;
				debugDesc.Height = creationDesc.height;
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
				return occDebugResource;
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTargetTexture(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				return CreateRenderTarget(data, creationDesc, GetBackBufferFormat(), GetBackBufferClearColor(), 1, true, true);			 
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTargetTextureForLightCull(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				return CreateRenderTarget(data, creationDesc, DXGI_FORMAT_R8G8_UNORM, GetWhiteColor(), creationDesc.arraySize, false, false);
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateShadowMapTexture(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				return CreateShadowMap(data, creationDesc, DXGI_FORMAT_R32_TYPELESS, 1);			 
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateShadowMapTextureArray(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				return CreateShadowMap(data, creationDesc, DXGI_FORMAT_R32_TYPELESS, creationDesc.arraySize);
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateShadowMapTextureCube(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				return CreateShadowMap(data, creationDesc, DXGI_FORMAT_R32_TYPELESS, Constants::cubeMapPlaneCount);
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateLightLinkedList(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
{
				return CreateLightList(data, creationDesc, (size_t)creationDesc.width * Private::lightClusterNodeSize);
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateLightOffsetBuffer(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				return CreateLightList(data, creationDesc, (size_t)creationDesc.width * Private::lightClusterOffsetSize);
			}					
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateSsaoTexture(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				return CreateSsaoTexture(data, creationDesc, DXGI_FORMAT_R16_UNORM);
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateSsaoIntermediateTexture(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				return CreateSsaoTexture(data, creationDesc, DXGI_FORMAT_R16G16_UNORM);
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateSsaoInterleaveTexture(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{ 
				return CreateSsaoTexture(data, creationDesc, DXGI_FORMAT_R16_UNORM);
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateSsaoDepthTexture(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				return CreateSsaoTexture(data, creationDesc, DXGI_FORMAT_R32_FLOAT);
			}
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateSsaoDepthInterleaveTexture(const DeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
			{
				return CreateSsaoTexture(data, creationDesc, DXGI_FORMAT_R32_FLOAT);
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
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* device,
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
			static CreaetD3dResource::Ptr GetCreateD3dResourcePtr(const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				switch (rType)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL:
					return &CreateSceneDepthStencilResource;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL:
					return &CreateDebugDepthStencilResource;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP:
					return &CreateDebugMapResource;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP:
					return &CreateHZBOcclusionResource;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
					return &CreateHZBOcclusionMipmapResource;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
					return &CreateOcclusionResourceDebug;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
					return &CreateRenderTargetTexture;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING:
					return &CreateRenderTargetTextureForLightCull;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
					return &CreateShadowMapTexture;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY:
					return &CreateShadowMapTextureArray;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE:
					return &CreateShadowMapTextureCube;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
					return &CreateLightLinkedList;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET:
					return &CreateLightOffsetBuffer;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP:
					return &CreateSsaoTexture;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERMEDIATE_MAP:
					return &CreateSsaoIntermediateTexture;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERLEAVE_MAP:
					return &CreateSsaoInterleaveTexture;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_MAP:
					return &CreateSsaoDepthTexture;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_INTERLEAVE_MAP:
					return &CreateSsaoDepthInterleaveTexture;
				default:
					return nullptr;
				}
			}

			static D3D12_RESOURCE_STATES GraphicOptionInitState(const J_GRAPHIC_RESOURCE_OPTION_TYPE type)
			{
				switch (type)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::TANGENT_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY:
					return D3D12_RESOURCE_STATE_GENERIC_READ;
					/*
					case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR:
						break; 
					*/
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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP:
				{
					desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
					desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
					heapFlags = D3D12_HEAP_FLAG_NONE;
					clearFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::TANGENT_MAP:
				{
					desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
					desc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
					heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
					heapFlags = D3D12_HEAP_FLAG_NONE;
					clearFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY:
				{
					desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
					desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
					heapFlags = D3D12_HEAP_FLAG_NONE;
					clearFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR:
				{
					desc.Flags = Core::AddSQValueEnum(desc.Flags, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER:
				{
					desc = CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_BUFFER, 0, sizeof(uint), 1, 1, 1,
						DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
					heapFlags = D3D12_HEAP_FLAG_NONE;
					heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
					break;
				}
				default:
					break;
				}
			}
			static DirectX::XMVECTORF32 GraphicOptionClearColor(const J_GRAPHIC_RESOURCE_OPTION_TYPE type)
			{
				switch (type)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP:
					return GetBackBufferClearColor();		//same as render taget common clear
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::TANGENT_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR: 
				default:
					return GetBlackColor();
				}
			}
			static bool GraphicOptionNeedRebind(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)noexcept
			{
				switch (rType)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
				{
					switch (opType)
					{
					case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER:
						return false;
					default:
						break;
					}
				}
				}
				return true;
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
					return false;
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

		JDx12GraphicResourceManager::BindDesc::BindDesc(ID3D12Device* device, const uint resourceIndex, const JGraphicResourceCreationDesc& cDesc, const bool useMipmap)
			:device(device), resourceIndex(resourceIndex), cDesc(cDesc.bindDesc), useMipmap(useMipmap)
		{}
		JDx12GraphicResourceManager::BindDesc::BindDesc(ID3D12Device* device, const uint resourceIndex, const CommonBinDesc& cDesc, const bool useMipmap)
			:device(device), resourceIndex(resourceIndex), cDesc(cDesc), useMipmap(useMipmap)
		{}

		JDx12GraphicResourceManager::BindDetailDesc::BindDetailDesc(const BindDesc& resourceBindDesc)
			: resourceBindDesc(resourceBindDesc)
		{
			ClearAllowTrigger();
		}
		void JDx12GraphicResourceManager::BindDetailDesc::InitDsv(const bool isArray)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Format = Private::GetDepthStencilFormat();
			dsvDesc.Texture2D.MipSlice = 0;
			SetDsv(dsvDesc);
		}
		void JDx12GraphicResourceManager::BindDetailDesc::InitRtv(const D3D12_RESOURCE_DESC& resourceDesc, const bool isArray)
		{
			if (isArray)
			{
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				rtvDesc.Format = resourceDesc.Format;
				rtvDesc.Texture2DArray.MipSlice = 0;
				rtvDesc.Texture2DArray.PlaneSlice = 0;
				rtvDesc.Texture2DArray.FirstArraySlice = 0;
				rtvDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
			}
			else
			{
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				rtvDesc.Format = resourceDesc.Format;
				rtvDesc.Texture2D.MipSlice = 0;
				rtvDesc.Texture2D.PlaneSlice = 0;
			} 
			SetRtv(rtvDesc);
		}
		void JDx12GraphicResourceManager::BindDetailDesc::InitSrv(const D3D12_RESOURCE_DESC& resourceDesc, const bool isArray)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			if (isArray)
			{
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; 
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Format = resourceDesc.Format;
				srvDesc.Texture2DArray.MostDetailedMip = 0;
				srvDesc.Texture2DArray.MipLevels = resourceDesc.MipLevels;
				srvDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
				srvDesc.Texture2DArray.FirstArraySlice = 0;
				srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f; 
			}
			else
			{
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Format = resourceDesc.Format;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				srvDesc.Texture2D.PlaneSlice = 0;
			}		
			SetSrv(srvDesc);
		}
		void JDx12GraphicResourceManager::BindDetailDesc::InitUav(const D3D12_RESOURCE_DESC& resourceDesc, const bool isArray, ID3D12Resource* counterResource)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			if (isArray)
			{
				uavDesc.Format = resourceDesc.Format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY; 
				uavDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
				uavDesc.Texture2DArray.FirstArraySlice = 0; 
				uavDesc.Texture2DArray.MipSlice = 0;
			}
			else
			{ 
				uavDesc.Format = resourceDesc.Format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;
			}
			SetUav(uavDesc);
		}
		void JDx12GraphicResourceManager::BindDetailDesc::SetDsv(const D3D12_DEPTH_STENCIL_VIEW_DESC& desc)
		{
			dsvDesc = desc;
			allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::DSV] = true;
		}
		void JDx12GraphicResourceManager::BindDetailDesc::SetRtv(const D3D12_RENDER_TARGET_VIEW_DESC& desc)
		{
			rtvDesc = desc;
			allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::RTV] = true;
		}
		void JDx12GraphicResourceManager::BindDetailDesc::SetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
		{
			srvDesc = desc;
			allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::SRV] = true;
		}
		void JDx12GraphicResourceManager::BindDetailDesc::SetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
		{
			uavDesc = desc;
			allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
		}
		void JDx12GraphicResourceManager::BindDetailDesc::ClearAllowTrigger()
		{
			memset(allowBindResource, 0, (uint)J_GRAPHIC_BIND_TYPE::COUNT * sizeof(bool));
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
		DXGI_FORMAT JDx12GraphicResourceManager::GetBackBufferFormat()noexcept
		{
			return Private::GetBackBufferFormat();
		}
		DXGI_FORMAT JDx12GraphicResourceManager::GetDepthStencilFormat() noexcept
		{
			return Private::GetDepthStencilFormat();
		}
		DirectX::XMVECTORF32 JDx12GraphicResourceManager::GetBlackColor() noexcept
		{
			return Private::GetBlackColor();
		}
		DirectX::XMVECTORF32 JDx12GraphicResourceManager::GetBackBufferClearColor() noexcept
		{
			return Private::GetBackBufferClearColor();
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
		bool JDx12GraphicResourceManager::CanCreateResource(const J_GRAPHIC_RESOURCE_TYPE rType, JGraphicDevice* device)noexcept
		{
			if (!IsSameDevice(device))
				return false;

			return CanCreateResource(rType);
		}
		bool JDx12GraphicResourceManager::CanCreateOptionResource(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			auto& desc = typeDesc[(uint)rType];
			bool hasSpace = true;
			if (Private::IsBuffer(opType))
				hasSpace = true;
			else
			{
				for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
					hasSpace &= Private::InitGraphicResourceOptionViewCapacity(opType, rType, (J_GRAPHIC_BIND_TYPE)i) > 0 ? desc.viewInfo[i].HasSpace() : true;
			}
			return desc.HasSpace() & JGraphicResourceType::CanUseOption(opType, rType) & hasSpace;
		}
		void JDx12GraphicResourceManager::CreateUploadBuffer(ID3D12Device* device,
			ID3D12Resource* resource,
			ID3D12Resource** uploadBuffer,
			const uint subResourceCount)
		{
			const UINT64 uploadBufferSize = GetRequiredIntermediateSize(resource, 0, subResourceCount);
			CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

			ThrowIfFailedHr(device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(uploadBuffer)));
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
				sd.BufferDesc.Format = Private::GetBackBufferFormat();
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
					Private::GetBackBufferFormat(),
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
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateResource(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			if (device == nullptr || !creationDesc.IsValid())
				return nullptr;
			 
			switch (rType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN:
				return nullptr;			//invalid call 
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
				return CreateOcclusionResourceDebug(device, creationDesc);
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D:
				return CreateTexture2D(device, creationDesc);
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE:
				return CreateCubeMap(device, creationDesc);
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
				return CreateLightLinkedList(device, creationDesc);
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::VERTEX:
				return CreateBuffer(device, creationDesc, rType);
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::INDEX:
				return CreateBuffer(device, creationDesc, rType);
			default:
				return CommonCreationProcess(device, creationDesc, rType);
			}
		}
		bool JDx12GraphicResourceManager::CreateOption(JGraphicDevice* device, JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
		{
			if (info == nullptr || !IsSameDevice(info.Get()))
				return false;

			const J_GRAPHIC_RESOURCE_TYPE rType = info->GetGraphicResourceType();
			if (!CanCreateOptionResource(opType, rType))
				return false;

			JDx12GraphicResourceInfo* dxInfo = static_cast<JDx12GraphicResourceInfo*>(info.Get());
			if (dxInfo->HasOption(opType))
				return false;

			Private::DeviceData data(device);
			device->StartPublicCommandSet(data.startCommandThisCreation);

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource = nullptr;
			D3D12_HEAP_PROPERTIES heapProperties;
			D3D12_HEAP_FLAGS heapFlag;

			auto dxResource = dxInfo->resourceHolder->GetResource();
			auto resourceDesc = dxResource->GetDesc();
			auto clearFormat = resourceDesc.Format;
			dxResource->GetHeapProperties(&heapProperties, &heapFlag);
			Private::GraphicOptionProperty(opType, resourceDesc, heapProperties, heapFlag, clearFormat);

			D3D12_CLEAR_VALUE* optClearPtr = nullptr;
			CD3DX12_CLEAR_VALUE optClear(clearFormat, Private::GraphicOptionClearColor(opType));
			if (!Private::IsBuffer(opType))
				optClearPtr = &optClear;

			ThrowIfFailedHr(data.device->CreateCommittedResource(
				&heapProperties,
				heapFlag,
				&resourceDesc,
				Private::GraphicOptionInitState(opType),
				optClearPtr,
				IID_PPV_ARGS(&newResource)));

			CreateOption(opType, dxInfo, std::move(newResource));
			if (Private::GraphicOptionNeedRebind(rType, opType))
			{
				ClearDescViewCountUntil(dxInfo);
				ReBind(data.device, rType, dxInfo->GetArrayIndex());
			}
			device->EndPublicCommandSet(data.startCommandThisCreation);
			return true;
		}
		bool JDx12GraphicResourceManager::DestroyGraphicTextureResource(JGraphicDevice* device, JGraphicResourceInfo* info)
		{
			if (info == nullptr || !IsSameDevice(device))
				return false;

			Private::DeviceData data(device);
			device->StartPublicCommandSet(data.startCommandThisCreation);

			JDx12GraphicResourceInfo* dxInfo = static_cast<JDx12GraphicResourceInfo*>(info);
			const uint rIndex = dxInfo->GetArrayIndex();
			const J_GRAPHIC_RESOURCE_TYPE rType = dxInfo->GetGraphicResourceType();
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];
			BindViewPtr ptr = GetResourceBindViewPtr(rType);

			//ptr can nullptr ex) occlusion query(resource view binding을 사용하지 않는 타입)
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
			device->EndPublicCommandSet(data.startCommandThisCreation);

			return true;
		}
		bool JDx12GraphicResourceManager::DestroyGraphicOption(JGraphicDevice* device, JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE optype)
		{
			if (info == nullptr || !IsSameDevice(device))
				return false;

			if (!info->HasOption(optype))
				return false;

			Private::DeviceData data(device);
			device->StartPublicCommandSet(data.startCommandThisCreation);
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
			device->EndPublicCommandSet(data.startCommandThisCreation);
			return true;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CommonCreationProcess(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType, const ExtraOption& extraOption)
		{
			if (!CanCreateResource(rType, device))
			{
				MessageBox(0, L"Fail Create R", std::to_wstring((uint)rType).c_str(), 0);
				return nullptr;
			}

			JUserPtr<JGraphicResourceInfo> userPtr = nullptr;

			Private::DeviceData data(device);
			device->StartPublicCommandSet(data.startCommandThisCreation);

			auto createD3dResourcePtr = Private::GetCreateD3dResourcePtr(rType);
			auto bindPtr = GetResourceBindViewPtr(rType);
			if (createD3dResourcePtr != nullptr)
			{
				CreateResourceInfo(rType, createD3dResourcePtr(data, creationDesc));
				const uint resourceIndex = resource[(int)rType].size() - 1;
				if (bindPtr != nullptr && !extraOption.bindResourceManually)
					(this->*bindPtr)(BindDesc(data.device, resourceIndex, creationDesc));
				auto& result = resource[(int)rType][resourceIndex];
				userPtr = result;
			}
			device->EndPublicCommandSet(data.startCommandThisCreation);
			return userPtr;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateOcclusionResourceDebug(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG;
			if (!CanCreateResource(rType, device))
				return nullptr;

			Private::DeviceData data(device);
			device->StartPublicCommandSet(data.startCommandThisCreation);

			CreateResourceInfo(rType, Private::CreateOcclusionResourceDebug(device, creationDesc));

			uint resourceIndex = resource[(int)rType].size() - 1;
			if (creationDesc.occDebugDesc->isHzb)
				BindHZBOcclusionDebug(BindDesc(data.device, resourceIndex, creationDesc));
			else
				BindHdOcclusionDebug(BindDesc(data.device, resourceIndex, creationDesc));
			auto& result = resource[(int)rType][resourceIndex];
			device->EndPublicCommandSet(data.startCommandThisCreation);
			return  result;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateTexture2D(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D;
			if (!CanCreateResource(rType, device))
				return nullptr;

			Private::DeviceData data(device);
			device->StartPublicCommandSet(data.startCommandThisCreation);

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource = nullptr;
			Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer = nullptr;
			std::unique_ptr<DirectX::ResourceUploadBatch> uploadBatch = nullptr;
			JUserPtr<JGraphicResourceInfo> resourceUser = nullptr;
			HRESULT res = (HRESULT)-1L;

			if (creationDesc.textureDesc->creationType == JTextureCreationDesc::CREATION_TYPE::LOAD)
			{
				uint heapIndex = GetHeapIndex(rType, J_GRAPHIC_BIND_TYPE::SRV);
				const bool isFirst = JCUtil::GetFileFormat(creationDesc.textureDesc->path) != Core::JFileConstant::GetFileFormatW();
				D3D12_RESOURCE_FLAGS flag = creationDesc.textureDesc->allowUav ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

				if (creationDesc.textureDesc->oriFormat == L".dds")
				{
					res = DirectX::CreateDDSTextureFromFile12(data.device,
						data.commandList,
						creationDesc.textureDesc->path.c_str(),
						newResource,
						uploadBuffer,
						creationDesc.textureDesc->maxSize,
						nullptr,
						flag,
						creationDesc.textureDesc->UseMipmap());
				}
				else
				{
					uploadBatch = std::make_unique<DirectX::ResourceUploadBatch>(data.device);
					uploadBatch->Begin();
					DirectX::WIC_LOADER_FLAGS wFlag = DirectX::WIC_LOADER_FORCE_RGBA32;
					if (creationDesc.textureDesc->UseMipmap())
						wFlag |= DirectX::WIC_LOADER_MIP_AUTOGEN;
					res = DirectX::CreateWICTextureFromFileEx(data.device, *uploadBatch, creationDesc.textureDesc->path.c_str(), creationDesc.textureDesc->maxSize, flag, wFlag, newResource.GetAddressOf());
					uploadBatch->End(data.commandQueue);
				}
			}
			else
			{
				D3D12_RESOURCE_DESC texDesc;
				ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
				texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				texDesc.Alignment = 0;
				texDesc.Width = creationDesc.width;
				texDesc.Height = creationDesc.height;
				texDesc.DepthOrArraySize = 1;
				texDesc.MipLevels = 1;		//mipmap 갯수 조절 기능 추가필요.
				texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;
				texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

				CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
				ThrowIfFailedHr(data.device->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&texDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&newResource)));

				CreateUploadBuffer(data.device, newResource.Get(), uploadBuffer.GetAddressOf());
				JD3DUtility::UploadData(data.commandList,
					newResource.Get(),
					uploadBuffer.Get(),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					creationDesc.textureDesc->v4Data.data(),
					texDesc.Width,
					texDesc.Height,
					(uint)sizeof(JVector4F),
					1);
				res = S_OK;
			}
			if (SUCCEEDED(res))
			{
				CreateResourceInfo(rType, std::move(newResource));
				const uint resourceIndex = resource[(int)rType].size() - 1;
				Bind2DTexture(BindDesc(data.device, resourceIndex, creationDesc, creationDesc.textureDesc->UseMipmap()));
				resourceUser = resource[(int)rType][resourceIndex];
			}
			else
				MessageBox(0, L"FAIL", 0, 0);
			device->EndPublicCommandSet(data.startCommandThisCreation);
			if (SUCCEEDED(res) && !data.startCommandThisCreation)
			{
				//함수 호출전부터 publicCommand가 작동중일시
				//uploadBuffer에 삭제를 위해 여기서 command를 제출하고 다시 작동시킨다.
				data.dxDevice->ReStartPublicCommandSet();
				//data.dxDevice->EndPublicCommand();
				//data.dxDevice->FlushCommandQueue();
				//data.dxDevice->StartPublicCommand();
			}
			uploadBuffer.Reset();
			uploadBatch = nullptr;
			return resourceUser;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateCubeMap(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE;
			if (!CanCreateResource(rType, device))
				return nullptr;

			Private::DeviceData data(device);
			device->StartPublicCommandSet(data.startCommandThisCreation);

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;

			uint heapIndex = GetHeapIndex(rType, J_GRAPHIC_BIND_TYPE::SRV);
			D3D12_RESOURCE_FLAGS flag = creationDesc.textureDesc->allowUav ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
			JUserPtr<JGraphicResourceInfo> resourceUser = nullptr;
			bool res = false;
			if (creationDesc.textureDesc->oriFormat == L".dds")
			{
				res = DirectX::CreateDDSTextureFromFile12(data.device,
					data.commandList,
					creationDesc.textureDesc->path.c_str(),
					newResource,
					uploadBuffer,
					creationDesc.textureDesc->maxSize,
					nullptr,
					flag,
					creationDesc.textureDesc->UseMipmap()) == S_OK;
			}
			else
				assert(L"InValid cube map format");
			if (res)
			{
				std::wstring folder;
				std::wstring name;
				std::wstring format;
				JCUtil::DecomposeFilePath(creationDesc.textureDesc->path, folder, name, format);

				CreateResourceInfo(rType, std::move(newResource));

				const uint resourceIndex = resource[(int)rType].size() - 1;
				BindCubeMap(BindDesc(data.device, resourceIndex, creationDesc, creationDesc.textureDesc->UseMipmap()));
				resourceUser = resource[(int)rType][resourceIndex];
			}
			device->EndPublicCommandSet(data.startCommandThisCreation);
			if (res && !data.startCommandThisCreation)
			{
				//함수 호출전부터 publicCommand가 작동중일시
				//uploadBuffer에 삭제를 위해 여기서 command를 제출하고 다시 작동시킨다.
				data.dxDevice->ReStartPublicCommandSet();
				//data.dxDevice->EndPublicCommand();
				//data.dxDevice->FlushCommandQueue();
				//data.dxDevice->StartPublicCommand();
			}
			uploadBuffer.Reset();
			return resourceUser;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateLightLinkedList(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc)
		{
			ExtraOption extraOption;
			extraOption.bindResourceManually = true;

			auto user = CommonCreationProcess(device, creationDesc, J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST, extraOption);
			if (user != nullptr)
			{
				if (CreateOption(device, user, J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER))
				{
					Private::DeviceData data(device);
					device->StartPublicCommandSet(data.startCommandThisCreation);
					BindLightLinkedList(BindDesc(data.device, user->GetArrayIndex(), creationDesc));
					device->EndPublicCommandSet(data.startCommandThisCreation);
				}
				else
					DestroyGraphicTextureResource(device, user.Release());
			}
			return user;
		}
		JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateBuffer(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			if (!CanCreateResource(rType, device) || creationDesc.bufferDesc == nullptr)
				return nullptr;

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;

			Private::DeviceData data(device);
			device->StartPublicCommandSet(data.startCommandThisCreation);

			newResource = Private::CreateDefaultBuffer(data.device, data.commandList, creationDesc.bufferDesc->data, creationDesc.bufferDesc->bufferSize, uploadBuffer);
			CreateResourceInfo(rType, std::move(newResource));

			const uint resourceIndex = resource[(int)rType].size() - 1;
			auto& result = resource[(int)rType][resourceIndex];

			device->EndPublicCommandSet(data.startCommandThisCreation);
			if (result != nullptr && !data.startCommandThisCreation)
			{
				//함수 호출전부터 publicCommand가 작동중일시
				//uploadBuffer에 삭제를 위해 여기서 command를 제출하고 다시 작동시킨다.
				data.dxDevice->ReStartPublicCommandSet();
			}
			uploadBuffer.Reset();
			return result;
		}
		void JDx12GraphicResourceManager::CreateResourceInfo(const J_GRAPHIC_RESOURCE_TYPE rType, Microsoft::WRL::ComPtr<ID3D12Resource>&& d3dResource)
		{
			JDx12GraphicResourceInfo::GetHandlePtr getHandleLam = [](JDx12GraphicResourceManager* gm, const J_GRAPHIC_BIND_TYPE btype, const uint index)
			{
				return gm->GetGpuDescriptorHandle(btype, index);
			};

			auto dx12Holder = std::make_unique<JDx12GraphicResourceHolder>(std::move(d3dResource));
			auto newInfo = JOwnerPtr<JDx12GraphicResourceInfo>(new JDx12GraphicResourceInfo(rType, this, std::move(dx12Holder), getHandleLam));

			newInfo->SetArrayIndex(resource[(int)rType].size());
			newInfo->SetPrivateName();
			resource[(int)rType].push_back(std::move(newInfo));
			++typeDesc[(int)rType].count;
		}
		bool JDx12GraphicResourceManager::CreateOption(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, JDx12GraphicResourceInfo* dxInfo, Microsoft::WRL::ComPtr<ID3D12Resource>&& resource)
		{
			if (dxInfo == nullptr || resource == nullptr)
				return false;

			dxInfo->SetOption(opType, std::make_unique<JDx12GraphicResourceHolder>(std::move(resource)));
			dxInfo->SetPrivateOptionName(opType, Core::GetWName(opType));
			return true;
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
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP:
				return &JDx12GraphicResourceManager::BindDebugMap;
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
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING:
				return &JDx12GraphicResourceManager::BindRenderTargetForLigthCull;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
				return &JDx12GraphicResourceManager::BindShadowMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY:
				return &JDx12GraphicResourceManager::BindShadowMapArray;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE:
				return &JDx12GraphicResourceManager::BindShadowMapCube;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
				return &JDx12GraphicResourceManager::BindLightLinkedList;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET:
				return &JDx12GraphicResourceManager::BindLightClusterOffsetBuffer;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP:
				return &JDx12GraphicResourceManager::BindSsaoMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERMEDIATE_MAP:
				return &JDx12GraphicResourceManager::BindSsaoIntermediateMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERLEAVE_MAP:
				return &JDx12GraphicResourceManager::BindSsaoInterleaveMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_MAP:
				return &JDx12GraphicResourceManager::BindSsaoDepthMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_INTERLEAVE_MAP:
				return &JDx12GraphicResourceManager::BindSsaoDepthInterleaveMap;
			default:
				return nullptr;
			}
		}
		JDx12GraphicResourceManager::BindOptionViewPtr JDx12GraphicResourceManager::GetResourceBindOptionViewPtr(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
		{
			switch (opType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP:
				return &JDx12GraphicResourceManager::BindAlbedoMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP:
				return &JDx12GraphicResourceManager::BindNormalMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::TANGENT_MAP:
				return &JDx12GraphicResourceManager::BindTangentMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY:
				return &JDx12GraphicResourceManager::BindLightingPropertyMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR:
				return &JDx12GraphicResourceManager::BindBlur; 
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
				//바인딩 되어있지 않을시 count < 0
				//heapIndexStart는 바인딩되지 않을 경우 기본값인 -1을 가지고있는 상태이다.
				//graphic resource 타입마다 binding 유형이 정해지기 때문에 유효하다
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

			const bool fixedBind = JGraphicResourceType::IsFixedBind(rType);
			//type per resource view controll
			for (uint i = index; i < desc.count; ++i)
			{
				//init heap st, bind value
				BindDesc desc(device, i);
				JDx12GraphicResourceInfo* reBindHandle = resource[(int)rType][i].Get();
				for (uint j = 0; j < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++j)
				{
					const J_GRAPHIC_BIND_TYPE bType = (J_GRAPHIC_BIND_TYPE)j;
					if (!fixedBind && reBindHandle->HasView(bType))
						desc.cDesc.requestAdditionalBind[j] = true;
					else
						desc.cDesc.requestAdditionalBind[j] = false;

					reBindHandle->SetHeapIndexStart(bType, invalidIndex);
					reBindHandle->SetViewCount(bType, 0);
					for (uint k = 0; k < (uint)J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNT; ++k)
					{
						const J_GRAPHIC_RESOURCE_OPTION_TYPE opType = (J_GRAPHIC_RESOURCE_OPTION_TYPE)k;
						reBindHandle->SetHeapOptionIndexStart(bType, opType, invalidIndex);
						reBindHandle->SetOptionViewCount(bType, opType, 0);
					}
				}

				//reset array, heap st, bind value
				if (additionalArrayIndex != 0)
				{
					const int newIndex = i + additionalArrayIndex;
					reBindHandle->SetArrayIndex(newIndex);
				} 
				if (!fixedBind)
					desc.cDesc.useEngineDefine = false;
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

			D3D12_SHADER_RESOURCE_VIEW_DESC depthSrvDesc = {};
			depthSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			depthSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			depthSrvDesc.Texture2D.MostDetailedMip = 0;
			depthSrvDesc.Texture2D.MipLevels = 1;
			depthSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			depthSrvDesc.Texture2D.PlaneSlice = 0;
			depthSrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

			BindDetailDesc detail(bDesc); 
			detail.rType = rType;
			detail.InitDsv(false);
			detail.SetSrv(depthSrvDesc);
			CommonBind(detail);

			detail.ClearAllowTrigger();
			depthSrvDesc.Texture2D.PlaneSlice = 1;
			depthSrvDesc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
			detail.SetSrv(depthSrvDesc);
			CommonBind(detail);
		}
		void JDx12GraphicResourceManager::BindDebugDepthStencil(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL;
			BindDetailDesc detail(bDesc);
			detail.rType = rType;
			detail.InitDsv(false);
			CommonBind(detail);
		}
		void JDx12GraphicResourceManager::BindDebugMap(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];

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

			BindDetailDesc detail(bDesc);
			detail.rType = rType; 
			detail.SetSrv(debugSrvDesc);
			detail.SetUav(debugUavDesc);
			CommonBind(detail);
		}
		void JDx12GraphicResourceManager::BindOcclusionDepthMap(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];
  
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
 
			BindDetailDesc detail(bDesc);
			detail.rType = rType;
			detail.SetDsv(dsvDesc);
			detail.SetSrv(depthSrvDesc);
			CommonBind(detail);
		}
		void JDx12GraphicResourceManager::BindHZBOcclusionDepthMipmap(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP;
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];

			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			uint nowWidth = resourcePtr->GetDesc().Width;
			uint nowHeight = resourcePtr->GetDesc().Height;

			uint occlusionCount = 0;
			uint occlusionMipmapViewCapacity = GetOcclusionMipmapViewCapacity();
			uint minOcclusionSize = GetOcclusionMinSize();

			BindDetailDesc detail(bDesc);
			detail.rType = rType;

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

				detail.SetSrv(mipMapSrvDesc);
				detail.SetUav(uavDesc);
				CommonBind(detail);
 
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
			uint nowWidth = resourcePtr->GetDesc().Width;
			uint nowHeight = resourcePtr->GetDesc().Height;

			uint occlusionCount = 0;
			uint occlusionMipmapViewCapacity = GetOcclusionMipmapViewCapacity();
			uint minOcclusionSize = GetOcclusionMinSize();

			BindDetailDesc detail(bDesc);
			detail.rType = rType;
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

				detail.SetSrv(debugSrvDesc);
				detail.SetUav(uavDesc);
				CommonBind(detail);

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
			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);

			BindDetailDesc detail(bDesc);
			detail.rType = rType;
			detail.InitSrv(resourcePtr->GetDesc(), false);
			detail.InitUav(resourcePtr->GetDesc(), false);
			CommonBind(detail);
		}
		void JDx12GraphicResourceManager::Bind2DTexture(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D;
			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);

			BindDetailDesc detail(bDesc);
			detail.rType = rType;
			detail.InitSrv(resourcePtr->GetDesc(), false);
			CommonBind(detail);
		}
		void JDx12GraphicResourceManager::BindCubeMap(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE;
			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = resourcePtr->GetDesc().MipLevels;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			srvDesc.Format = resourcePtr->GetDesc().Format;

			BindDetailDesc detail(bDesc);
			detail.rType = rType;
			detail.SetSrv(srvDesc);
			CommonBind(detail);
		}
		void JDx12GraphicResourceManager::BindRenderTarget(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON;
			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(rType)][bDesc.resourceIndex].Get();

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

			BindDetailDesc detail(bDesc);
			detail.rType = rType;
			detail.SetRtv(rtvDesc);
			detail.SetSrv(srvDesc); 
			if (bDesc.cDesc.HasRequestAdditionalBind(J_GRAPHIC_BIND_TYPE::UAV))
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = resourceDesc.Format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;
				uavDesc.Texture2D.PlaneSlice = 0;
				detail.SetUav(uavDesc);
			}
			CommonBind(detail);

			if (handlePtr->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP))
			{
				BindDetailDesc opDesc(bDesc);
				opDesc.rType = rType;
				opDesc.rtvDesc = rtvDesc;
				opDesc.rtvDesc.Format = resourceDesc.Format;// DXGI_FORMAT_R8G8B8A8_UNORM;
				opDesc.srvDesc = srvDesc;
				opDesc.srvDesc.Format = resourceDesc.Format;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::RTV] = true;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::SRV] = true;
				BindAlbedoMap(opDesc);
			}
			if (handlePtr->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP))
			{
				BindDetailDesc opDesc(bDesc);
				opDesc.rType = rType; 
				opDesc.rtvDesc = rtvDesc;
				opDesc.rtvDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
				opDesc.srvDesc = srvDesc;
				opDesc.srvDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::RTV] = true;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::SRV] = true;
				BindNormalMap(opDesc);
			}
			if (handlePtr->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::TANGENT_MAP))
			{
				BindDetailDesc opDesc(bDesc);
				opDesc.rType = rType; 
				opDesc.rtvDesc = rtvDesc;
				opDesc.rtvDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
				opDesc.srvDesc = srvDesc;
				opDesc.srvDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::RTV] = true;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::SRV] = true;
				BindTangentMap(opDesc);
			}
			if (handlePtr->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY))
			{
				BindDetailDesc opDesc(bDesc);
				opDesc.rType = rType; 
				opDesc.rtvDesc = rtvDesc;
				opDesc.rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				opDesc.srvDesc = srvDesc;
				opDesc.srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::RTV] = true;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::SRV] = true;
				BindLightingPropertyMap(opDesc);
			}
			if (handlePtr->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR))
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = resourceDesc.Format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;

				BindDetailDesc opDesc(bDesc);
				opDesc.rType = rType; 
				opDesc.srvDesc = srvDesc;
				opDesc.uavDesc = uavDesc;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::SRV] = true;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;

				BindBlur(opDesc);
			} 
		}
		void JDx12GraphicResourceManager::BindRenderTargetForLigthCull(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING;
			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Format = resourceDesc.Format;
			rtvDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
			rtvDesc.Texture2DArray.FirstArraySlice = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Format = resourceDesc.Format;
			srvDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
			srvDesc.Texture2DArray.MipLevels = 1;
			srvDesc.Texture2DArray.FirstArraySlice = 0;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			
			BindDetailDesc detail(bDesc);
			detail.rType = rType;
			detail.SetRtv(rtvDesc);
			detail.SetSrv(srvDesc);
			CommonBind(detail);
		}
		void JDx12GraphicResourceManager::BindShadowMap(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP;
			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(rType)][bDesc.resourceIndex].Get();

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

			BindDetailDesc detail(bDesc);
			detail.rType = rType;
			detail.SetDsv(dsvDesc);
			detail.SetSrv(srvDesc);
			CommonBind(detail);

			if (handlePtr->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR))
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = resourceDesc.Format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;

				BindDetailDesc opDesc(bDesc);
				opDesc.rType = rType;
				opDesc.srvDesc = srvDesc;
				opDesc.uavDesc = uavDesc;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::SRV] = true;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
				BindBlur(opDesc);
			}
		}
		void JDx12GraphicResourceManager::BindShadowMapArray(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE type = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY; 
			ID3D12Resource* resourcePtr = GetResource(type, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(type)][bDesc.resourceIndex].Get();
			 
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

			BindDetailDesc detail(bDesc);
			detail.rType = type;
			detail.SetDsv(dsvDesc);
			detail.SetSrv(srvDesc);
			CommonBind(detail);

			if (handlePtr->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR))
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = resourceDesc.Format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;

				BindDetailDesc opDesc(bDesc);
				opDesc.rType = type; 
				opDesc.srvDesc = srvDesc;
				opDesc.uavDesc = uavDesc;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::SRV] = true;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
				BindBlur(opDesc);
			}
		}
		void JDx12GraphicResourceManager::BindShadowMapCube(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE type = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE;
			ID3D12Resource* resourcePtr = GetResource(type, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(type)][bDesc.resourceIndex].Get();

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

			BindDetailDesc detail(bDesc);
			detail.rType = type;
			detail.SetDsv(dsvDesc);
			detail.SetSrv(srvDesc);
			CommonBind(detail);

			if (handlePtr->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR))
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = resourceDesc.Format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;

				BindDetailDesc opDesc(bDesc);
				opDesc.rType = type; 
				opDesc.srvDesc = srvDesc;
				opDesc.uavDesc = uavDesc;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::SRV] = true;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
				BindBlur(opDesc);
			}
		}
		void JDx12GraphicResourceManager::BindLightLinkedList(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST;
			JDx12GraphicResourceInfo* handlePtr = resource[int(rType)][bDesc.resourceIndex].Get();
			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			ID3D12Resource* counterBuffer = handlePtr->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER) ?
				handlePtr->optionHolderSet->holder[(uint)J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER]->GetResource() : nullptr;
 
			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();
			uint element = resourceDesc.Width / Private::lightClusterNodeSize;
			if (resourceDesc.Width % Private::lightClusterNodeSize)
				++element;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = element;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			srvDesc.Buffer.StructureByteStride = Private::lightClusterNodeSize;

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_UNKNOWN; //Needs to be UNKNOWN for structured buffer
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = element;
			uavDesc.Buffer.StructureByteStride = Private::lightClusterNodeSize;
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE; //Not a raw view
			uavDesc.Buffer.CounterOffsetInBytes = 0; //First element in UAV counter resourc

			BindDetailDesc detail(bDesc);
			detail.rType = rType;
			detail.SetSrv(srvDesc);
			detail.SetUav(uavDesc);
			detail.counterResource = counterBuffer;
			CommonBind(detail);
		}
		void JDx12GraphicResourceManager::BindLightClusterOffsetBuffer(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET;
			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(rType)][bDesc.resourceIndex].Get();
			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();
		
			uint element = resourceDesc.Width / Private::lightClusterOffsetSize;
			if (resourceDesc.Width % Private::lightClusterOffsetSize)
				++element;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_R32_UINT;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = element;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			srvDesc.Buffer.StructureByteStride = 0;

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS; //Needs to be DXGI_FORMAT_R32_TYPELESS for RAW
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = element;
			uavDesc.Buffer.StructureByteStride = 0; //Needs to be zero, otherwise interpreted as structured buffer
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
			uavDesc.Buffer.CounterOffsetInBytes = 0; //Needs to

			BindDetailDesc detail(bDesc);
			detail.rType = rType;
			detail.SetSrv(srvDesc);
			detail.SetUav(uavDesc); 
			CommonBind(detail);
		}
		void JDx12GraphicResourceManager::BindSsaoMap(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP;
			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			
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

			BindDetailDesc detail(bDesc);
			detail.rType = rType;
			detail.SetRtv(rtvDesc);
			detail.SetSrv(srvDesc);
			CommonBind(detail);
		}
		void JDx12GraphicResourceManager::BindSsaoIntermediateMap(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERMEDIATE_MAP;
			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
		
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

			BindDetailDesc detail(bDesc);
			detail.rType = rType;
			detail.SetRtv(rtvDesc);
			detail.SetSrv(srvDesc); 
			CommonBind(detail);
			//CommonOptionBind(opDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::SSAO_INTERMEDIATE_MAP);
		}
		void JDx12GraphicResourceManager::BindSsaoInterleaveMap(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERLEAVE_MAP;
			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc(); 
			BindDetailDesc detail(bDesc);
			detail.rType = rType;
 
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = resourceDesc.Format;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.MipSlice = 0;
			rtvDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
			rtvDesc.Texture2DArray.FirstArraySlice = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = resourceDesc.Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2DArray.MostDetailedMip = 0;
			srvDesc.Texture2DArray.MipLevels = 1;
			srvDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
			srvDesc.Texture2DArray.FirstArraySlice = 0;

			detail.SetRtv(rtvDesc);
			detail.SetSrv(srvDesc);
			CommonBind(detail);

			rtvDesc.Texture2DArray.ArraySize = 1;
			srvDesc.Texture2DArray.ArraySize = 1;
			for (uint i = 0; i < resourceDesc.DepthOrArraySize; ++i)
			{
				rtvDesc.Texture2DArray.FirstArraySlice = i;
				srvDesc.Texture2DArray.FirstArraySlice = i;
				detail.SetRtv(rtvDesc);
				detail.SetSrv(srvDesc);
				CommonBind(detail);
			} 
		}
		void JDx12GraphicResourceManager::BindSsaoDepthMap(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_MAP;
			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();
			BindDetailDesc detail(bDesc);
			detail.rType = rType;
			detail.InitRtv(resourceDesc, false);
			detail.InitSrv(resourceDesc, false);
			CommonBind(detail);
		}
		void JDx12GraphicResourceManager::BindSsaoDepthInterleaveMap(const BindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_INTERLEAVE_MAP;
			ID3D12Resource* resourcePtr = GetResource(rType, bDesc.resourceIndex);
			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();
			BindDetailDesc detail(bDesc);
			detail.rType = rType;

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = resourceDesc.Format;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.MipSlice = 0;
			rtvDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
			rtvDesc.Texture2DArray.FirstArraySlice = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = resourceDesc.Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2DArray.MostDetailedMip = 0;
			srvDesc.Texture2DArray.MipLevels = 1;
			srvDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
			srvDesc.Texture2DArray.FirstArraySlice = 0;

			detail.SetRtv(rtvDesc);
			detail.SetSrv(srvDesc);
			CommonBind(detail);

			rtvDesc.Texture2DArray.ArraySize = 1;
			srvDesc.Texture2DArray.ArraySize = 1;
			for (uint i = 0; i < resourceDesc.DepthOrArraySize; ++i)
			{
				rtvDesc.Texture2DArray.FirstArraySlice = i;
				srvDesc.Texture2DArray.FirstArraySlice = i;
				detail.SetRtv(rtvDesc);
				detail.SetSrv(srvDesc);
				CommonBind(detail);
			}
		}
		void JDx12GraphicResourceManager::BindAlbedoMap(const BindDetailDesc& opDesc)
		{
			CommonOptionBind(opDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP);
		}
		void JDx12GraphicResourceManager::BindNormalMap(const BindDetailDesc& opDesc)
		{
			CommonOptionBind(opDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
		}
		void JDx12GraphicResourceManager::BindTangentMap(const BindDetailDesc& opDesc)
		{
			CommonOptionBind(opDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::TANGENT_MAP);
		}
		void JDx12GraphicResourceManager::BindLightingPropertyMap(const BindDetailDesc& opDesc)
		{
			CommonOptionBind(opDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY);
		}	
		void JDx12GraphicResourceManager::BindBlur(const BindDetailDesc& opDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_OPTION_TYPE opType = J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR;
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
		void JDx12GraphicResourceManager::CommonBind(const BindDetailDesc& bDesc)
		{
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)bDesc.rType];
			ID3D12Resource* resourcePtr = GetResource(bDesc.rType, bDesc.resourceBindDesc.resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = resource[int(bDesc.rType)][bDesc.resourceBindDesc.resourceIndex].Get();

			for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
			{
				const J_GRAPHIC_BIND_TYPE bType = (J_GRAPHIC_BIND_TYPE)i;
				if (!bDesc.allowBindResource[i])
					continue;

				const int nextViewIndex = desc.viewInfo[(int)bType].GetNextViewIndex();
				SetViewCount(handlePtr, bType, nextViewIndex);

				if (bType == J_GRAPHIC_BIND_TYPE::DSV)
					bDesc.resourceBindDesc.device->CreateDepthStencilView(resourcePtr, &bDesc.dsvDesc, GetCpuDsvDescriptorHandle(nextViewIndex));
				else if (bType == J_GRAPHIC_BIND_TYPE::RTV)
					bDesc.resourceBindDesc.device->CreateRenderTargetView(resourcePtr, &bDesc.rtvDesc, GetCpuRtvDescriptorHandle(nextViewIndex));
				else if (bType == J_GRAPHIC_BIND_TYPE::SRV)
					bDesc.resourceBindDesc.device->CreateShaderResourceView(resourcePtr, &bDesc.srvDesc, GetCpuSrvDescriptorHandle(nextViewIndex));
				else if (bType == J_GRAPHIC_BIND_TYPE::UAV)
					bDesc.resourceBindDesc.device->CreateUnorderedAccessView(resourcePtr, bDesc.counterResource, &bDesc.uavDesc, GetCpuSrvDescriptorHandle(nextViewIndex));

				++desc.viewInfo[i].count; 
			}
		}
		void JDx12GraphicResourceManager::CommonOptionBind(const BindDetailDesc& opDesc, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
		{ 
			JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)opDesc.rType];
			JDx12GraphicResourceInfo* handlePtr = resource[int(opDesc.rType)][opDesc.resourceBindDesc.resourceIndex].Get();
			ID3D12Resource* resourcePtr = GetResource(opDesc.rType, opDesc.resourceBindDesc.resourceIndex);
			ID3D12Resource* optResourcePtr = handlePtr->optionHolderSet->holder[(uint)opType]->GetResource();

			for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
			{
				const J_GRAPHIC_BIND_TYPE bType = (J_GRAPHIC_BIND_TYPE)i;
				if (!opDesc.allowBindResource[i])
					continue;
 
				const int nextViewIndex = desc.viewInfo[(int)bType].GetNextViewIndex();
				SetOptionViewCount(handlePtr, bType, opType, nextViewIndex);
  
				if (bType == J_GRAPHIC_BIND_TYPE::DSV)
					opDesc.resourceBindDesc.device->CreateDepthStencilView(optResourcePtr, &opDesc.dsvDesc, GetCpuDsvDescriptorHandle(nextViewIndex));
				else if (bType == J_GRAPHIC_BIND_TYPE::RTV)
					opDesc.resourceBindDesc.device->CreateRenderTargetView(optResourcePtr, &opDesc.rtvDesc, GetCpuRtvDescriptorHandle(nextViewIndex));
				else if (bType == J_GRAPHIC_BIND_TYPE::SRV)
					opDesc.resourceBindDesc.device->CreateShaderResourceView(optResourcePtr, &opDesc.srvDesc, GetCpuSrvDescriptorHandle(nextViewIndex));
				else if (bType == J_GRAPHIC_BIND_TYPE::UAV)
					opDesc.resourceBindDesc.device->CreateUnorderedAccessView(optResourcePtr, nullptr, &opDesc.uavDesc, GetCpuSrvDescriptorHandle(nextViewIndex));

				++desc.viewInfo[i].count;
			}
		}
		bool JDx12GraphicResourceManager::ReAllocTypePerAllResource(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			if (device == nullptr || !IsSameDevice(device) || !JGraphicResourceType::CanReAlloc(rType))
				return false;

			Private::DeviceData data(device);
			device->StartPublicCommandSet(data.startCommandThisCreation);

			auto createD3dResourcePtr = Private::GetCreateD3dResourcePtr(rType);
			auto& vec = resource[(uint)rType];
			for (const auto& gInfo : vec)
			{
				auto dx12Holder = std::make_unique<JDx12GraphicResourceHolder>(createD3dResourcePtr(data, creationDesc));
				gInfo->resourceHolder = std::move(dx12Holder);
				gInfo->SetPrivateName();
			}

			ReBind(data.device, rType, 0);
			device->EndPublicCommandSet(data.startCommandThisCreation);
			return true;
		}
		bool JDx12GraphicResourceManager::CopyResource(JGraphicDevice* device, const JUserPtr<JGraphicResourceInfo>& from, const JUserPtr<JGraphicResourceInfo>& to)
		{
			if (device == nullptr || !IsSameDevice(device) || from == nullptr || to == nullptr)
				return false;

			Private::DeviceData data(device);
			device->StartPublicCommandSet(data.startCommandThisCreation);
			auto fromR = GetResource(from->GetGraphicResourceType(), from->GetArrayIndex());
			auto toR = GetResource(to->GetGraphicResourceType(), to->GetArrayIndex());

			JD3DUtility::ResourceTransition(data.commandList, fromR, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE);
			JD3DUtility::ResourceTransition(data.commandList, toR, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
			data.commandList->CopyResource(toR, fromR);
			JD3DUtility::ResourceTransition(data.commandList, toR, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			JD3DUtility::ResourceTransition(data.commandList, fromR, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			device->EndPublicCommandSet(data.startCommandThisCreation);
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
				Private::InitGraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::RTV) +
				Private::InitGraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::RTV) +
				Private::MPBCount(J_GRAPHIC_BIND_TYPE::RTV);
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
				Private::InitGraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::DSV) +
				Private::InitGraphicResourceViewFixedCount(J_GRAPHIC_BIND_TYPE::DSV) +
				Private::MPBCount(J_GRAPHIC_BIND_TYPE::DSV);
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

			JGraphicResourceCreationDesc desc(base.info.width, base.info.height); 
			defaultSceneDsInfo = CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL);
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