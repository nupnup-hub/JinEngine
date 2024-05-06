/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"../JGraphicResourceConstants.h"
#include"../../JGraphicConstants.h"
#include"../../JGraphicEnum.h"
#include"../../../Core/JCoreEssential.h"
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h" 
#include<DirectXColors.h> 

namespace JinEngine
{
	namespace Graphic
	{
		namespace Constants
		{ 
			static constexpr uint vertexBufferCapacity = 1024 * 4;			 
			static constexpr uint indexBufferCapacity = 1024 * 4;
			static constexpr uint bloomSampleCount = 5;			//high quality

			static constexpr uint gBufferAlbedoLayer = 0;
			static constexpr uint gBufferLightPropertyLayer = gBufferAlbedoLayer + 1;
			static constexpr uint gBufferNormalAndTangentLayer = gBufferLightPropertyLayer + 1;
			//static constexpr uint gBufferVelocityLayer = 3;
			static constexpr uint gBufferLayerCount = gBufferNormalAndTangentLayer + 1;

			static constexpr size_t lightClusterNodeSize = sizeof(uint64);
			static constexpr size_t lightClusterOffsetSize = sizeof(uint32);

			static constexpr size_t restirSampleSize = 44;	//44(rand 1); 68(non pack)
			static constexpr size_t restirReserviorSize = restirSampleSize + 8; // 
			static constexpr uint resitrReserviorBufferCount = 2;

			static constexpr uint histogramBufferCount = 256;
			static constexpr uint exposureBufferCount = 8;
			static constexpr float exposureDefaultValue = 2.0f;
			static constexpr float exposureDefaultMinLog = -12.0f;
			static constexpr float exposureDefaultMaxLog = 4.0f;

			static constexpr uint restirDenoiseMipmapCount = 4;

			static DXGI_FORMAT GetBackBufferFormat(const bool isHdr)noexcept
			{
				return isHdr ? DXGI_FORMAT_R10G10B10A2_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
				//return DXGI_FORMAT_R8G8B8A8_UNORM;
			}
			static DXGI_FORMAT GetRenderTargetFormat(const J_GRAPHIC_FORMAT format)noexcept
			{ 
				switch (format)
				{ 
				case JinEngine::Graphic::J_GRAPHIC_FORMAT::R10G10B10A2_UNORM:
					return DXGI_FORMAT_R10G10B10A2_UNORM;
				case JinEngine::Graphic::J_GRAPHIC_FORMAT::R16G16B16A16_UNORM:
					return DXGI_FORMAT_R16G16B16A16_UNORM;
				case JinEngine::Graphic::J_GRAPHIC_FORMAT::R16G16B16A16_FLOAT:
					return DXGI_FORMAT_R16G16B16A16_FLOAT;
				case JinEngine::Graphic::J_GRAPHIC_FORMAT::R32G32B32A32_FLOAT:
					return DXGI_FORMAT_R32G32B32A32_FLOAT;
				default:
					return DXGI_FORMAT_R8G8B8A8_UNORM;
				}  
			}
			static DXGI_FORMAT GetDepthStencilFormat() noexcept
			{
				return DXGI_FORMAT_D24_UNORM_S8_UINT;
			}
			static DirectX::XMVECTORF32 GetBlackClearColor() noexcept
			{
				return DirectX::XMVECTORF32{ {{0, 0, 0, 0}} };
			}
			static DirectX::XMVECTORF32 GetBackBufferClearColor() noexcept
			{
				const JVector4F v4 = BackBufferClearColor();
				return DirectX::XMVECTORF32{ {{v4.x, v4.y, v4.z, v4.w}} };
			}
			static DirectX::XMVECTORF32 GetWhiteClearColor()noexcept
			{
				return DirectX::XMVECTORF32{ {{1.0f, 1.0f, 1.0f, 1.0f}} };
			} 
			static uint CalBloomWidth(const uint rtWidth)noexcept
			{
				return rtWidth > 2560 ? 1280 : 640;
			}
			static uint CalBloomHeight(const uint rtHeight)noexcept
			{
				return  rtHeight > 1440 ? 768 : 384;
			} 
			static DXGI_FORMAT GetGBufferFormat(const uint layerNumber)
			{ 
				if (layerNumber == gBufferAlbedoLayer)
					return DXGI_FORMAT_R8G8B8A8_UNORM; 
				else if (layerNumber == gBufferLightPropertyLayer)
					return DXGI_FORMAT_R8G8B8A8_UNORM;
				else if (layerNumber == gBufferNormalAndTangentLayer)
					return DXGI_FORMAT_R10G10B10A2_UNORM;
				//else if (layerNumber == gBufferVelocityLayer)
				//	return DXGI_FORMAT_R32_UINT;
					//return DXGI_FORMAT_R32_UINT;
				//if (layerNumber == gBufferVelocityLayer)
				//	return DXGI_FORMAT_R16G16_UNORM;
				else
					return DXGI_FORMAT_UNKNOWN;
			}
		}
	}
}