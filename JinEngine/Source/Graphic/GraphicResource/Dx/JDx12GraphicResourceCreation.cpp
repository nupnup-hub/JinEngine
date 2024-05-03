#include"JDx12GraphicResourceCreation.h" 
#include"JDx12GraphicResourceConstants.h"
#include"JDx12GraphicResourceInfo.h"
#include"JDx12GraphicResourceManager.h" 
#include"JDx12GraphicResourceFormat.h"
#include"../../Utility/Dx/JDx12Utility.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../../Core/Math/JMathHelper.h"
#include"../../../Window/JWindow.h"

namespace JinEngine::Graphic
{
	using CreaetD3dResource = Core::JSFunctorType<JDx12GraphicResourceHolderDesc, const JDeviceData&, const JGraphicResourceCreationDesc&>;
	using BindViewPtr = void(*)(const JBindDesc&);
	using BindOptionViewPtr = void(*)(const JBindDetailDesc&);
	using namespace JinEngine::Graphic::Constants;

	static size_t GetFixedBufferElementSize(const J_GRAPHIC_RESOURCE_TYPE type)
	{
		switch (type)
		{
		case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
			return Constants::lightClusterNodeSize;
		case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET:
			return Constants::lightClusterOffsetSize;
		case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_INITIAL_SAMPLE:
			return Constants::restirSampleSize;
		case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR:
			return Constants::restirReserviorSize;
		case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE:
		case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_FXAA_COLOR_QUEUE:
		case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::BYTE_BUFFER_COMMON:
		case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::STRUCTURE_BUFFER_COMMON:
			return sizeof(uint);
		default:
			return 0;
		}
	}
	static size_t GetFixedBufferElementSize(const J_GRAPHIC_RESOURCE_OPTION_TYPE type)
	{
		switch (type)
		{
		case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER:
			return sizeof(uint);
		default:
			return 0;
		}
	}
	static JVector3<uint> GetFixedDimention(const J_GRAPHIC_RESOURCE_TYPE type)
	{
		switch (type)
		{
		case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE:
			return JVector3<uint>(Constants::exposureBufferCount, 1, 1);
		default:
			return JVector3<uint>::Zero();
		}
	} 
	namespace Resource
	{
		static JDx12GraphicResourceHolderDesc CreateRenderTarget(const JDeviceData& data,
			const JGraphicResourceCreationDesc& creationDesc,
			DXGI_FORMAT format,
			DirectX::XMVECTORF32 clareColor,
			const uint arraySize,
			const bool applyMsaa,
			const bool canUav,
			const bool hasClearValue)
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
			if (canUav)
				desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

			//DXGI_FORMAT_R8G8B8A8_UNORM
			CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_CLEAR_VALUE optClear(format, clareColor);
			ThrowIfFailedG(data.device->CreateCommittedResource(
				&heapProperty,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				hasClearValue ? &optClear : nullptr,
				IID_PPV_ARGS(&newResource)));
			return JDx12GraphicResourceHolderDesc(std::move(newResource), D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		static JDx12GraphicResourceHolderDesc CreateShadowMap(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc, DXGI_FORMAT format, const uint arraySize)
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
			return JDx12GraphicResourceHolderDesc(std::move(newResource), D3D12_RESOURCE_STATE_COMMON);
		}
		static JDx12GraphicResourceHolderDesc CreateBuffer(const JDeviceData& data,
			const JGraphicResourceCreationDesc& creationDesc,
			const D3D12_RESOURCE_FLAGS flag,
			const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			size_t dataSize = 0;
			size_t width = 0;
			if (creationDesc.useEngineDefine)
			{
				dataSize = GetFixedBufferElementSize(rType);
				width = GetFixedDimention(rType).x;
			}
			else
			{
				const size_t fixedSize = GetFixedBufferElementSize(rType);
				const bool useFixedSize = fixedSize != 0 && (creationDesc.formatHint == nullptr || creationDesc.formatHint->elementSize == 0);
				dataSize = useFixedSize ? fixedSize : creationDesc.formatHint->elementSize;
				width = creationDesc.width;
			} 
			const size_t bufferSize = width * dataSize;
			if (bufferSize == 0)
				return JDx12GraphicResourceHolderDesc();

			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			CD3DX12_RESOURCE_DESC resourceDesc(D3D12_RESOURCE_DIMENSION_BUFFER, 0, bufferSize, 1, 1, 1,
				DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flag);

			CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			ThrowIfFailedHr(data.device->CreateCommittedResource(
				&heapProperty,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(&newResource)));
			return JDx12GraphicResourceHolderDesc(std::move(newResource), D3D12_RESOURCE_STATE_COMMON, dataSize);
		}
		static JDx12GraphicResourceHolderDesc CreateSsaoTexture(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc, DXGI_FORMAT format)
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
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			desc.Format = format;

			ThrowIfFailedG(data.device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&newResource)));
			return JDx12GraphicResourceHolderDesc(std::move(newResource), D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		static JDx12GraphicResourceHolderDesc CreateCommmon2DTexture(const JDeviceData& data,
			const JGraphicResourceCreationDesc& creationDesc,
			const DXGI_FORMAT format,
			const DirectX::XMVECTORF32 clareColor,
			const bool canUav,
			const bool hasClearValue,
			const bool generateMipMap)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			D3D12_RESOURCE_DESC desc;
			ZeroMemory(&desc, sizeof(D3D12_RESOURCE_DESC));
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			desc.Alignment = 0;
			desc.Width = creationDesc.width;
			desc.Height = creationDesc.height;
			desc.DepthOrArraySize = 1;
			desc.MipLevels = generateMipMap ? 0 : 1;
			desc.Format = format;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			desc.Flags = D3D12_RESOURCE_FLAG_NONE;
			if (canUav)
				desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

			CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_CLEAR_VALUE optClear(format, clareColor);
			ThrowIfFailedG(data.device->CreateCommittedResource(
				&heapProperty,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_COMMON,
				hasClearValue ? &optClear : nullptr,
				IID_PPV_ARGS(&newResource))); 
			return JDx12GraphicResourceHolderDesc(std::move(newResource), D3D12_RESOURCE_STATE_COMMON);
		}

		static JDx12GraphicResourceHolderDesc CreateSceneDepthStencilResource(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
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
			return JDx12GraphicResourceHolderDesc(std::move(newSceneDsResource), D3D12_RESOURCE_STATE_DEPTH_READ);
		}
		static JDx12GraphicResourceHolderDesc CreateDebugDepthStencilResource(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
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
			return JDx12GraphicResourceHolderDesc(std::move(newDebugDsResource), D3D12_RESOURCE_STATE_DEPTH_READ);
		}
		static JDx12GraphicResourceHolderDesc CreateDebugMapResource(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
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

			return JDx12GraphicResourceHolderDesc(std::move(newLayerDepthDebugResource), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
		static JDx12GraphicResourceHolderDesc CreateHZBOcclusionResource(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
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
			return JDx12GraphicResourceHolderDesc(std::move(occDsResource), D3D12_RESOURCE_STATE_DEPTH_READ);
		}
		static JDx12GraphicResourceHolderDesc CreateHZBOcclusionMipmapResource(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
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
			return JDx12GraphicResourceHolderDesc(std::move(occMipmapResource), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
		static JDx12GraphicResourceHolderDesc CreateOcclusionResourceDebug(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
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
			debugDesc.MipLevels = creationDesc.bindDesc.allowMipmapBind ? 0 : 1;
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
			return JDx12GraphicResourceHolderDesc(std::move(occDebugResource), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
		static JDx12GraphicResourceHolderDesc CreateTexture2D(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateCommmon2DTexture(data, creationDesc, DXGI_FORMAT_R8G8B8A8_UNORM, Constants::GetBackBufferClearColor(), false, false, creationDesc.textureDesc->UseMipmap());
		}
		static JDx12GraphicResourceHolderDesc CreateTextureCommon(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
			if (creationDesc.formatHint != nullptr)
				format = JDx12Format::ConvertApi(creationDesc.formatHint->format);

			return CreateCommmon2DTexture(data, creationDesc, format, Constants::GetBackBufferClearColor(), creationDesc.bindDesc.HasRequestAdditionalBind(J_GRAPHIC_BIND_TYPE::UAV), false, creationDesc.textureDesc->UseMipmap());
		}
		static JDx12GraphicResourceHolderDesc CreateRenderTargetTexture(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateRenderTarget(data,
				creationDesc,
				GetRenderTargetFormat(data.option.rendering.renderTargetFormat),
				GetBackBufferClearColor(),
				1,
				true,
				creationDesc.bindDesc.HasRequestAdditionalBind(J_GRAPHIC_BIND_TYPE::UAV),
				true);
		}
		static JDx12GraphicResourceHolderDesc CreateRenderTargetTextureForLightCull(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateRenderTarget(data, creationDesc, DXGI_FORMAT_R8G8_UNORM, GetWhiteClearColor(), creationDesc.arraySize, false, false, true);
		}
		static JDx12GraphicResourceHolderDesc CreateShadowMapTexture(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateShadowMap(data, creationDesc, DXGI_FORMAT_R32_TYPELESS, 1);
		}
		static JDx12GraphicResourceHolderDesc CreateShadowMapTextureArray(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateShadowMap(data, creationDesc, DXGI_FORMAT_R32_TYPELESS, creationDesc.arraySize);
		}
		static JDx12GraphicResourceHolderDesc CreateShadowMapTextureCube(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateShadowMap(data, creationDesc, DXGI_FORMAT_R32_TYPELESS, Constants::cubeMapPlaneCount);
		}
		static JDx12GraphicResourceHolderDesc CreateLightLinkedList(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateBuffer(data, creationDesc, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST);
		}
		static JDx12GraphicResourceHolderDesc CreateLightOffsetBuffer(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateBuffer(data, creationDesc, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET);
		}
		static JDx12GraphicResourceHolderDesc CreateSsaoTexture(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateSsaoTexture(data, creationDesc, DXGI_FORMAT_R16_UNORM);
		}
		static JDx12GraphicResourceHolderDesc CreateSsaoIntermediateTexture(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateSsaoTexture(data, creationDesc, DXGI_FORMAT_R16G16_UNORM);
		}
		static JDx12GraphicResourceHolderDesc CreateSsaoInterleaveTexture(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateSsaoTexture(data, creationDesc, DXGI_FORMAT_R16_UNORM);
		}
		static JDx12GraphicResourceHolderDesc CreateSsaoDepthTexture(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateSsaoTexture(data, creationDesc, DXGI_FORMAT_R32_FLOAT);
		}
		static JDx12GraphicResourceHolderDesc CreateSsaoDepthInterleaveTexture(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateSsaoTexture(data, creationDesc, DXGI_FORMAT_R32_FLOAT);
		}
		static JDx12GraphicResourceHolderDesc CreateImageProcessingTexture(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			bool useMipmap = creationDesc.textureDesc != nullptr ? creationDesc.textureDesc->UseMipmap() : false;
			return CreateCommmon2DTexture(data, creationDesc, DXGI_FORMAT_R11G11B10_FLOAT, Constants::GetBackBufferClearColor(), true, false, useMipmap);
		}
		static JDx12GraphicResourceHolderDesc CreateExposure(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateBuffer(data, creationDesc, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE);
		}
		static JDx12GraphicResourceHolderDesc CreateLuma(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			DXGI_FORMAT format = DXGI_FORMAT_R8_UNORM;
			if (creationDesc.formatHint != nullptr && !creationDesc.formatHint->isNormalized)
				format = DXGI_FORMAT_R8_UINT;
			bool useMipmap = creationDesc.textureDesc != nullptr ? creationDesc.textureDesc->UseMipmap() : false;
			return CreateCommmon2DTexture(data, creationDesc, format, Constants::GetBackBufferClearColor(), true, false, useMipmap);
		}
		static JDx12GraphicResourceHolderDesc CreateFxaaColor(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateBuffer(data, creationDesc, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_FXAA_COLOR_QUEUE);
		}
		static JDx12GraphicResourceHolderDesc CreateRestirSample(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateBuffer(data, creationDesc, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, J_GRAPHIC_RESOURCE_TYPE::RESTIR_INITIAL_SAMPLE);
		}
		static JDx12GraphicResourceHolderDesc CreateRestirReservior(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateBuffer(data, creationDesc, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR);
		}
		static JDx12GraphicResourceHolderDesc CreateByteBuffer(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateBuffer(data, creationDesc, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, J_GRAPHIC_RESOURCE_TYPE::BYTE_BUFFER_COMMON);
		}
		static JDx12GraphicResourceHolderDesc CreateStructureBuffer(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateBuffer(data, creationDesc, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, J_GRAPHIC_RESOURCE_TYPE::STRUCTURE_BUFFER_COMMON);
		}
		static JDx12GraphicResourceHolderDesc CreateVertexBuffer(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateBuffer(data, creationDesc, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, J_GRAPHIC_RESOURCE_TYPE::VERTEX);
		}
		static JDx12GraphicResourceHolderDesc CreateIndexBuffer(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc)
		{
			return CreateBuffer(data, creationDesc, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, J_GRAPHIC_RESOURCE_TYPE::INDEX);
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
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D:
				return &CreateTexture2D;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON:
				return &CreateTextureCommon;
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
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::IMAGE_PROCESSING:
				return &CreateImageProcessingTexture;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE:
				return &CreateExposure;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_LUMA:
				return &CreateLuma;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_FXAA_COLOR_QUEUE:
				return &CreateFxaaColor;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_INITIAL_SAMPLE:
				return &CreateRestirSample;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR:
				return &CreateRestirReservior;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::BYTE_BUFFER_COMMON:
				return &CreateByteBuffer;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::STRUCTURE_BUFFER_COMMON:
				return &CreateStructureBuffer;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::VERTEX:
				return &CreateVertexBuffer;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::INDEX:
				return &CreateIndexBuffer;
			default:
				return nullptr;
			}
		}
	}
	namespace Option
	{
		static D3D12_RESOURCE_STATES GraphicOptionInitState(const J_GRAPHIC_RESOURCE_OPTION_TYPE type)
		{
			switch (type)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP: 
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY:
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
			uint gBufferLayerStart = (uint)J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP;
			switch (type)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP: 
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY: 
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP:
			{
				desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
				heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				heapFlags = D3D12_HEAP_FLAG_NONE;

				desc.Format = Constants::GetGBufferFormat((uint)type - gBufferLayerStart);
				clearFormat = desc.Format;
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY:
			{
				desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
				heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				heapFlags = D3D12_HEAP_FLAG_NONE;

				desc.Format = DXGI_FORMAT_R32_UINT;
				clearFormat = desc.Format;
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR:
			{
				desc.Flags = Core::AddSQValueEnum(desc.Flags, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER:
			{
				desc = CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_BUFFER, 0, GetFixedBufferElementSize(type), 1, 1, 1,
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
				return Constants::GetBackBufferClearColor();		//same as render taget common clear 
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP: 
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR:
			default:
				return Constants::GetBlackClearColor();
			}
		}
		static bool UseClearColor(const J_GRAPHIC_RESOURCE_OPTION_TYPE type)
		{
			switch (type)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER:
				return false;
			default:
				return true;
			}
		}
	}
	namespace Bind
	{
		static bool CanBindMipmap(const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			switch (rType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
				return true;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
				return true;
			default:
				return false;
			}
		}
		static BindViewPtr GetResourceBindViewPtr(const J_GRAPHIC_RESOURCE_TYPE rType);
		static BindOptionViewPtr GetResourceBindOptionViewPtr(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType);

		static DXGI_FORMAT GetOptionFormat(const JBindDesc& desc, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
		{
			ID3D12Resource* resourcePtr = desc.gm->GetOptionResource(desc.info->GetGraphicResourceType(), opType, desc.info->GetArrayIndex());
			return resourcePtr != nullptr ? resourcePtr->GetDesc().Format : DXGI_FORMAT_UNKNOWN;
		}
		static void CommonBind(const JBindDetailDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.resourceBindDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.resourceBindDesc.info->GetArrayIndex();

			auto gm = bDesc.resourceBindDesc.gm;
			auto getNextViewIndexPtr = bDesc.resourceBindDesc.getNextViewIndex;
			auto addViewIndexPtr = bDesc.resourceBindDesc.addViewIndex;

			ID3D12Resource* resourcePtr = gm->GetResource(rType, resourceIndex);
			JDx12GraphicResourceInfo* handlePtr = bDesc.resourceBindDesc.info;

			for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
			{
				const J_GRAPHIC_BIND_TYPE bType = (J_GRAPHIC_BIND_TYPE)i;
				if (!bDesc.allowBindResource[i])
					continue;

				const int nextViewIndex = getNextViewIndexPtr(gm, rType, bType);
				gm->SetViewCount(handlePtr, bType, nextViewIndex);

				if (bType == J_GRAPHIC_BIND_TYPE::DSV)
					bDesc.resourceBindDesc.device->CreateDepthStencilView(resourcePtr, &bDesc.dsvDesc, gm->GetCpuDsvDescriptorHandle(nextViewIndex));
				else if (bType == J_GRAPHIC_BIND_TYPE::RTV)
					bDesc.resourceBindDesc.device->CreateRenderTargetView(resourcePtr, &bDesc.rtvDesc, gm->GetCpuRtvDescriptorHandle(nextViewIndex));
				else if (bType == J_GRAPHIC_BIND_TYPE::SRV)
					bDesc.resourceBindDesc.device->CreateShaderResourceView(resourcePtr, &bDesc.srvDesc, gm->GetCpuSrvDescriptorHandle(nextViewIndex));
				else if (bType == J_GRAPHIC_BIND_TYPE::UAV)
					bDesc.resourceBindDesc.device->CreateUnorderedAccessView(resourcePtr, bDesc.counterResource, &bDesc.uavDesc, gm->GetCpuSrvDescriptorHandle(nextViewIndex));

				addViewIndexPtr(gm, rType, bType);
			}
		}
		static void BindWithMipmap(JBindDetailDesc& bDesc, const uint maxCount)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.resourceBindDesc.info->GetGraphicResourceType();
			if (!CanBindMipmap(rType))
			{
				CommonBind(bDesc);
				return;
			}
			auto gm = bDesc.resourceBindDesc.gm;
			auto resourcePtr = gm->GetResource(rType, bDesc.resourceBindDesc.info->GetArrayIndex());

			auto desc = resourcePtr->GetDesc();
			JVector2<uint> currSize(desc.Width, desc.Height);
			uint mipCount = 0;
			while (mipCount < desc.MipLevels && mipCount < maxCount)
			{
				bDesc.srvDesc.Texture2D.MostDetailedMip = mipCount;
				bDesc.srvDesc.Texture2D.MipLevels = 1;
				bDesc.uavDesc.Texture2D.MipSlice = mipCount;
				bDesc.rtvDesc.Texture2D.MipSlice = mipCount;
				bDesc.dsvDesc.Texture2D.MipSlice = mipCount;
				CommonBind(bDesc);
				++mipCount;
			}
		}
		static void CommonOptionBind(const JBindDetailDesc& opDesc, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = opDesc.resourceBindDesc.info->GetGraphicResourceType();
			const uint resourceIndex = opDesc.resourceBindDesc.info->GetArrayIndex();

			auto gm = opDesc.resourceBindDesc.gm;
			auto getNextViewIndexPtr = opDesc.resourceBindDesc.getNextViewIndex;
			auto addViewIndexPtr = opDesc.resourceBindDesc.addViewIndex;

			JDx12GraphicResourceInfo* handlePtr = opDesc.resourceBindDesc.info;
			ID3D12Resource* resourcePtr = gm->GetResource(rType, resourceIndex);
			ID3D12Resource* optResourcePtr = gm->GetOptionResource(rType, opType, resourceIndex);

			for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
			{
				const J_GRAPHIC_BIND_TYPE bType = (J_GRAPHIC_BIND_TYPE)i;
				if (!opDesc.allowBindResource[i])
					continue;

				const int nextViewIndex = getNextViewIndexPtr(gm, rType, bType);
				gm->SetOptionViewCount(handlePtr, bType, opType, nextViewIndex);

				if (bType == J_GRAPHIC_BIND_TYPE::DSV)
					opDesc.resourceBindDesc.device->CreateDepthStencilView(optResourcePtr, &opDesc.dsvDesc, gm->GetCpuDsvDescriptorHandle(nextViewIndex));
				else if (bType == J_GRAPHIC_BIND_TYPE::RTV)
					opDesc.resourceBindDesc.device->CreateRenderTargetView(optResourcePtr, &opDesc.rtvDesc, gm->GetCpuRtvDescriptorHandle(nextViewIndex));
				else if (bType == J_GRAPHIC_BIND_TYPE::SRV)
					opDesc.resourceBindDesc.device->CreateShaderResourceView(optResourcePtr, &opDesc.srvDesc, gm->GetCpuSrvDescriptorHandle(nextViewIndex));
				else if (bType == J_GRAPHIC_BIND_TYPE::UAV)
					opDesc.resourceBindDesc.device->CreateUnorderedAccessView(optResourcePtr, nullptr, &opDesc.uavDesc, gm->GetCpuSrvDescriptorHandle(nextViewIndex));
 
				addViewIndexPtr(gm, rType, bType);
			}
		}

		static void BindAlbedoMap(const JBindDetailDesc& opDesc)
		{
			CommonOptionBind(opDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP);
		} 
		static void BindLightingPropertyMap(const JBindDetailDesc& opDesc)
		{
			CommonOptionBind(opDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY);
		}
		static void BindNormalMap(const JBindDetailDesc& opDesc)
		{
			CommonOptionBind(opDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
		} 
		static void BindVelocityMap(const JBindDetailDesc& opDesc)
		{
			CommonOptionBind(opDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY);
		}
		static void BindBlur(const JBindDetailDesc& opDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_OPTION_TYPE opType = J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR;
			const J_GRAPHIC_RESOURCE_TYPE rType = opDesc.resourceBindDesc.info->GetGraphicResourceType();
			const uint arrayIndex = opDesc.resourceBindDesc.info->GetArrayIndex();

			auto gm = opDesc.resourceBindDesc.gm;
			auto getNextViewIndexPtr = opDesc.resourceBindDesc.getNextViewIndex;
			auto addViewIndexPtr = opDesc.resourceBindDesc.addViewIndex;

			ID3D12Resource* resourcePtr = gm->GetResource(rType, arrayIndex);
			ID3D12Resource* optResourcePtr = gm->GetOptionResource(rType, opType, arrayIndex);

			if (opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::UAV])
			{
				const int uavNextViewIndex = getNextViewIndexPtr(gm, rType, J_GRAPHIC_BIND_TYPE::UAV);
				gm->SetViewCount(opDesc.resourceBindDesc.info, J_GRAPHIC_BIND_TYPE::UAV, uavNextViewIndex);
				opDesc.resourceBindDesc.device->CreateUnorderedAccessView(resourcePtr, nullptr, &opDesc.uavDesc, gm->GetCpuSrvDescriptorHandle(uavNextViewIndex));
				addViewIndexPtr(gm, rType, J_GRAPHIC_BIND_TYPE::UAV);
			}

			const int srvNextViewIndex = getNextViewIndexPtr(gm, rType, J_GRAPHIC_BIND_TYPE::SRV);
			const int uavNextViewIndex = getNextViewIndexPtr(gm, rType, J_GRAPHIC_BIND_TYPE::UAV);

			gm->SetOptionViewCount(opDesc.resourceBindDesc.info, J_GRAPHIC_BIND_TYPE::SRV, opType, srvNextViewIndex);
			gm->SetOptionViewCount(opDesc.resourceBindDesc.info, J_GRAPHIC_BIND_TYPE::UAV, opType, uavNextViewIndex);

			opDesc.resourceBindDesc.device->CreateShaderResourceView(optResourcePtr, &opDesc.srvDesc, gm->GetCpuSrvDescriptorHandle(srvNextViewIndex));
			opDesc.resourceBindDesc.device->CreateUnorderedAccessView(optResourcePtr, nullptr, &opDesc.uavDesc, gm->GetCpuSrvDescriptorHandle(uavNextViewIndex));

			addViewIndexPtr(gm, rType, J_GRAPHIC_BIND_TYPE::SRV);
			addViewIndexPtr(gm, rType, J_GRAPHIC_BIND_TYPE::UAV);
		}
		static void BindSwapChain(const JBindDesc& bDesc)
		{
			static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN;
			auto gm = bDesc.gm;

			auto getNextViewIndexPtr = bDesc.getNextViewIndex;
			auto addViewIndexPtr = bDesc.addViewIndex;

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, bDesc.info->GetArrayIndex());
			const int nextViewIndex = getNextViewIndexPtr(gm, rType, J_GRAPHIC_BIND_TYPE::RTV);

			bDesc.gm->SetViewCount(bDesc.info, J_GRAPHIC_BIND_TYPE::RTV, nextViewIndex);
			bDesc.device->CreateRenderTargetView(resourcePtr, nullptr, bDesc.gm->GetCpuRtvDescriptorHandle(nextViewIndex));
			addViewIndexPtr(gm, rType, J_GRAPHIC_BIND_TYPE::RTV);
		}
		static void BindMainDepthStencil(const JBindDesc& bDesc)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC depthSrvDesc = {};
			depthSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			depthSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			depthSrvDesc.Texture2D.MostDetailedMip = 0;
			depthSrvDesc.Texture2D.MipLevels = 1;
			depthSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			depthSrvDesc.Texture2D.PlaneSlice = 0;
			depthSrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

			JBindDetailDesc detail(bDesc);
			detail.InitDsv(false);
			detail.SetSrv(depthSrvDesc);
			CommonBind(detail);

			detail.ClearAllowTrigger();
			depthSrvDesc.Texture2D.PlaneSlice = 1;
			depthSrvDesc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
			detail.SetSrv(depthSrvDesc);
			CommonBind(detail); 
		}
		static void BindDebugDepthStencil(const JBindDesc& bDesc)
		{
			JBindDetailDesc detail(bDesc);
			detail.InitDsv(false);
			CommonBind(detail);
		}
		static void BindDebugMap(const JBindDesc& bDesc)
		{
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

			JBindDetailDesc detail(bDesc);
			detail.SetSrv(debugSrvDesc);
			detail.SetUav(debugUavDesc);
			CommonBind(detail);
		}
		static void BindOcclusionDepthMap(const JBindDesc& bDesc)
		{
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

			JBindDetailDesc detail(bDesc);
			detail.SetDsv(dsvDesc);
			detail.SetSrv(depthSrvDesc);
			CommonBind(detail);
		}
		static void BindHZBOcclusionDepthMipmap(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			
			JBindDetailDesc detail(bDesc);
			D3D12_SHADER_RESOURCE_VIEW_DESC mipMapSrvDesc = {};
			mipMapSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			mipMapSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			mipMapSrvDesc.Texture2D.MostDetailedMip = 0;
			mipMapSrvDesc.Texture2D.MipLevels = 1;
			mipMapSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			mipMapSrvDesc.Texture2D.PlaneSlice = 0;
			mipMapSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;
			 
			detail.SetSrv(mipMapSrvDesc);
			detail.SetUav(uavDesc);
			BindWithMipmap(detail, Constants::occlusionMipmapViewCapacity - 3);
		}
		static void BindOcclusionDebug(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
			JBindDetailDesc detail(bDesc);
			detail.InitSrv(resourcePtr->GetDesc(), false);
			detail.InitUav(resourcePtr->GetDesc(), false);
			 
			if (bDesc.cDesc.allowMipmapBind)
			{
				detail.srvDesc.Texture2D.MipLevels = 1;
				BindWithMipmap(detail, Constants::occlusionMipmapViewCapacity - 3);
			}
			else
				CommonBind(detail);
		}
		static void BindTexture2D(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
			JBindDetailDesc detail(bDesc);
			detail.InitSrv(resourcePtr->GetDesc(), false);
			CommonBind(detail);
		}
		static void BindCubeMap(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = resourcePtr->GetDesc().MipLevels;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			srvDesc.Format = resourcePtr->GetDesc().Format;

			JBindDetailDesc detail(bDesc);
			detail.SetSrv(srvDesc);
			CommonBind(detail);
		}
		static void BindTextureCommon(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
			auto resourceDesc = resourcePtr->GetDesc();
 
			JBindDetailDesc detail(bDesc);
			detail.InitSrv(resourcePtr->GetDesc(), false);
			if (bDesc.cDesc.HasRequestAdditionalBind(J_GRAPHIC_BIND_TYPE::UAV))
				detail.InitUav(resourcePtr->GetDesc(), false);
			CommonBind(detail);
		}
		static void BindRenderTarget(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
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

			JBindDetailDesc detail(bDesc);
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

			auto gbufferBindLam = [&](const JBindDesc& bDesc, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
			{
				const DXGI_FORMAT format = GetOptionFormat(bDesc, opType);
				JBindDetailDesc opDesc(bDesc);
				opDesc.rtvDesc = rtvDesc;
				opDesc.rtvDesc.Format = format;
				opDesc.srvDesc = srvDesc;
				opDesc.srvDesc.Format = format;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::RTV] = true;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::SRV] = true;
				 
				GetResourceBindOptionViewPtr(opType)(opDesc);
			};
			if (bDesc.info->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP))
				gbufferBindLam(bDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP); 
			if (bDesc.info->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY))
				gbufferBindLam(bDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY);
			if (bDesc.info->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP))
				gbufferBindLam(bDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP); 
			if (bDesc.info->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY))
			{
				const DXGI_FORMAT format = GetOptionFormat(bDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY);

				JBindDetailDesc opDesc(bDesc);
				opDesc.InitSrv(resourceDesc, false);
				opDesc.InitUav(resourceDesc, false);
				opDesc.srvDesc.Format = format;
				opDesc.uavDesc.Format = format;

				BindVelocityMap(opDesc);
			}
			if (bDesc.info->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR))
			{
				const DXGI_FORMAT format = GetOptionFormat(bDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR);
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;
 
				JBindDetailDesc opDesc(bDesc);
				opDesc.SetSrv(srvDesc);
				opDesc.SetUav(uavDesc);
				BindBlur(opDesc);
			}
		}
		static void BindRenderTargetForLigthCull(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
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

			JBindDetailDesc detail(bDesc);
			detail.SetRtv(rtvDesc);
			detail.SetSrv(srvDesc);
			CommonBind(detail);
		}
		static void BindShadowMap(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
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

			JBindDetailDesc detail(bDesc);
			detail.SetDsv(dsvDesc);
			detail.SetSrv(srvDesc);
			CommonBind(detail);

			if (bDesc.info->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR))
			{
				const DXGI_FORMAT format = GetOptionFormat(bDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR);
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;

				JBindDetailDesc opDesc(bDesc);
				opDesc.srvDesc = srvDesc;
				opDesc.uavDesc = uavDesc;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::SRV] = true;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
				BindBlur(opDesc);
			}
		}
		static void BindShadowMapArray(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
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

			JBindDetailDesc detail(bDesc);
			detail.SetDsv(dsvDesc);
			detail.SetSrv(srvDesc);
			CommonBind(detail);

			if (bDesc.info->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR))
			{
				const DXGI_FORMAT format = GetOptionFormat(bDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR);
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;

				JBindDetailDesc opDesc(bDesc);
				opDesc.srvDesc = srvDesc;
				opDesc.uavDesc = uavDesc;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::SRV] = true;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
				BindBlur(opDesc);
			}
		}
		static void BindShadowMapCube(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
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

			JBindDetailDesc detail(bDesc);
			detail.SetDsv(dsvDesc);
			detail.SetSrv(srvDesc);
			CommonBind(detail);

			if (bDesc.info->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR))
			{
				const DXGI_FORMAT format = GetOptionFormat(bDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR);
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = 0;

				JBindDetailDesc opDesc(bDesc);
				opDesc.srvDesc = srvDesc;
				opDesc.uavDesc = uavDesc;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::SRV] = true;
				opDesc.allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
				BindBlur(opDesc);
			}
		}
		static void BindLightLinkedList(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
			ID3D12Resource* counterBuffer = bDesc.info->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER) ?
				bDesc.gm->GetOptionResource(rType, J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER, resourceIndex) : nullptr;

			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();
			uint element = resourceDesc.Width / Constants::lightClusterNodeSize;
			if (resourceDesc.Width % Constants::lightClusterNodeSize)
				++element;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = element;
			srvDesc.Buffer.StructureByteStride = Constants::lightClusterNodeSize;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_UNKNOWN; //Needs to be UNKNOWN for structured buffer
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = element;
			uavDesc.Buffer.StructureByteStride = Constants::lightClusterNodeSize;
			uavDesc.Buffer.CounterOffsetInBytes = 0; //First element in UAV counter resourc
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE; //Not a raw view

			JBindDetailDesc detail(bDesc);
			detail.SetSrv(srvDesc);
			detail.SetUav(uavDesc);
			detail.counterResource = counterBuffer;
			CommonBind(detail);
		}
		static void BindLightClusterOffsetBuffer(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();

			uint element = resourceDesc.Width / Constants::lightClusterOffsetSize;
			if (resourceDesc.Width % Constants::lightClusterOffsetSize)
				++element;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_R32_UINT;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = element;
			srvDesc.Buffer.StructureByteStride = 0;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS; //Needs to be DXGI_FORMAT_R32_TYPELESS for RAW
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = element;
			uavDesc.Buffer.StructureByteStride = 0; //Needs to be zero, otherwise interpreted as structured buffer
			uavDesc.Buffer.CounterOffsetInBytes = 0; //Needs to
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

			JBindDetailDesc detail(bDesc);
			detail.SetSrv(srvDesc);
			detail.SetUav(uavDesc);
			CommonBind(detail);
		}
		static void BindSsaoMap(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
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

			//D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			//uavDesc.Format = resourceDesc.Format;
			//uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			//uavDesc.Texture2D.MipSlice = 0;
			//uavDesc.Texture2D.PlaneSlice = 0; 

			JBindDetailDesc detail(bDesc);
			detail.SetRtv(rtvDesc);
			detail.SetSrv(srvDesc);
			//detail.SetUav(uavDesc);
			CommonBind(detail);
		}
		static void BindSsaoIntermediateMap(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
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

			JBindDetailDesc detail(bDesc);
			detail.SetRtv(rtvDesc);
			detail.SetSrv(srvDesc);
			CommonBind(detail);
			//CommonOptionBind(opDesc, J_GRAPHIC_RESOURCE_OPTION_TYPE::SSAO_INTERMEDIATE_MAP);
		}
		static void BindSsaoInterleaveMap(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();
			JBindDetailDesc detail(bDesc);

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
		static void BindSsaoDepthMap(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();
			JBindDetailDesc detail(bDesc);
			detail.InitRtv(resourceDesc, false);
			detail.InitSrv(resourceDesc, false);
			CommonBind(detail);
		}
		static void BindSsaoDepthInterleaveMap(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();
			JBindDetailDesc detail(bDesc);

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
		static void BindImageProcessing(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Format = resourceDesc.Format;
			srvDesc.Texture2D.MipLevels = 1;

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = resourceDesc.Format;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;
			uavDesc.Texture2D.PlaneSlice = 0;

			JBindDetailDesc detail(bDesc);
			detail.SetSrv(srvDesc);
			detail.SetUav(uavDesc);
			CommonBind(detail);
		}
		static void BindLuma(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
			JBindDetailDesc detail(bDesc);
			detail.InitSrv(resourcePtr->GetDesc(), false);
			detail.InitUav(resourcePtr->GetDesc(), false);
			CommonBind(detail);
		}
		static void BindFxaaColor(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();

			const uint elementSize = bDesc.info->GetElementSize();
			uint element = JMathHelper::DivideByMultiple(resourceDesc.Width, elementSize);

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.NumElements = element;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
			uavDesc.Buffer.NumElements = element;
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

			JBindDetailDesc detail(bDesc);
			detail.SetSrv(srvDesc);
			detail.SetUav(uavDesc);
			CommonBind(detail);
		}
		static void BindByteBuffer(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();

			//raw buffer 이므로 32bit단위로 나눠준다.
			const uint elementSize = GetFixedBufferElementSize(J_GRAPHIC_RESOURCE_TYPE::BYTE_BUFFER_COMMON);
			uint element = JMathHelper::DivideByMultiple(resourceDesc.Width, elementSize);

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.NumElements = element;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			uavDesc.Buffer.NumElements = element;
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

			JBindDetailDesc detail(bDesc);
			detail.SetSrv(srvDesc);
			detail.SetUav(uavDesc);
			CommonBind(detail);
		}
		static void BindStructureBuffer(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();

			const uint elementSize = bDesc.info->GetElementSize();
			const uint element = bDesc.info->GetElementCount();

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = element;
			srvDesc.Buffer.StructureByteStride = elementSize;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = element;
			uavDesc.Buffer.StructureByteStride = elementSize;
			uavDesc.Buffer.CounterOffsetInBytes = 0;
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
 
			JBindDetailDesc detail(bDesc);
			detail.SetSrv(srvDesc);
			detail.SetUav(uavDesc);
			CommonBind(detail);
		}
		static void BindReadOnlyStructureBuffer(const JBindDesc& bDesc)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = bDesc.info->GetGraphicResourceType();
			const uint resourceIndex = bDesc.info->GetArrayIndex();

			ID3D12Resource* resourcePtr = bDesc.gm->GetResource(rType, resourceIndex);
			D3D12_RESOURCE_DESC resourceDesc = resourcePtr->GetDesc();

			const uint elementSize = bDesc.info->GetElementSize();
			const uint element = bDesc.info->GetElementCount();

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = element;
			srvDesc.Buffer.StructureByteStride = elementSize;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			 
			JBindDetailDesc detail(bDesc);
			detail.SetSrv(srvDesc); 
			CommonBind(detail);
		}
		static BindViewPtr GetResourceBindViewPtr(const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			switch (rType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN:
				return &BindSwapChain;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL:
				return &BindMainDepthStencil;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL:
				return &BindDebugDepthStencil;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP:
				return &BindDebugMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP:
				return &BindOcclusionDepthMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
				return &BindHZBOcclusionDepthMipmap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
				return &BindOcclusionDebug;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D:
				return &BindTexture2D;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE:
				return &BindCubeMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON:
				return &BindTextureCommon;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
				return &BindRenderTarget;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING:
				return &BindRenderTargetForLigthCull;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
				return &BindShadowMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY:
				return &BindShadowMapArray;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE:
				return &BindShadowMapCube;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
				return &BindLightLinkedList;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET:
				return &BindLightClusterOffsetBuffer;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP:
				return &BindSsaoMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERMEDIATE_MAP:
				return &BindSsaoIntermediateMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERLEAVE_MAP:
				return &BindSsaoInterleaveMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_MAP:
				return &BindSsaoDepthMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_INTERLEAVE_MAP:
				return &BindSsaoDepthInterleaveMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::IMAGE_PROCESSING:
				return &BindImageProcessing;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE:
				return &BindByteBuffer;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_LUMA:
				return &BindLuma;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_FXAA_COLOR_QUEUE:
				return &BindFxaaColor;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_INITIAL_SAMPLE:
				return &BindStructureBuffer;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR:
				return &BindStructureBuffer;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::BYTE_BUFFER_COMMON:
				return &BindByteBuffer;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::STRUCTURE_BUFFER_COMMON:
				return &BindStructureBuffer;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::VERTEX:
				return &BindReadOnlyStructureBuffer;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::INDEX:
				return &BindReadOnlyStructureBuffer;
			default:
				return nullptr;
			}
		}
		static BindOptionViewPtr GetResourceBindOptionViewPtr(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
		{
			switch (opType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP:
				return &BindAlbedoMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY:
				return &BindLightingPropertyMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP:
				return &BindNormalMap; 
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY:
				return &BindVelocityMap;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR:
				return &BindBlur;
			default:
				return nullptr;
			}
		}
	}
	JDeviceData::JDeviceData(JGraphicDevice* device, const JGraphicOption& option)
		:option(option)
	{
		dxDevice = static_cast<JDx12GraphicDevice*>(device);
		JDeviceData::device = dxDevice->GetDevice();
		commandQueue = dxDevice->GetCommandQueue();
		commandList = dxDevice->GetPublicCmdList();
		m4xMsaaQuality = dxDevice->GetM4xMsaaQuality();
		m4xMsaaState = dxDevice->GetM4xMsaaState();
	}

	JBindDesc::JBindDesc(JDx12GraphicResourceManager* gm,
		ID3D12Device* device,
		GetNextViewIndex getNextViewIndex,
		AddViewIndex addViewIndex,
		JDx12GraphicResourceInfo* info,
		const JGraphicResourceCreationDesc& cDesc)
		:gm(gm),
		device(device),
		info(info),
		getNextViewIndex(getNextViewIndex),
		addViewIndex(addViewIndex),
		cDesc(cDesc.bindDesc)
	{
		SetValid(gm != nullptr && device != nullptr && getNextViewIndex != nullptr && addViewIndex != nullptr && info != nullptr);
	}
	JBindDesc::JBindDesc(JDx12GraphicResourceManager* gm,
		ID3D12Device* device,
		GetNextViewIndex getNextViewIndex,
		AddViewIndex addViewIndex,
		JDx12GraphicResourceInfo* info,
		const CommonBinDesc& cDesc)
		:gm(gm),
		device(device),
		info(info),
		getNextViewIndex(getNextViewIndex),
		addViewIndex(addViewIndex),
		cDesc(cDesc)
	{
		SetValid(gm != nullptr && device != nullptr && getNextViewIndex != nullptr && addViewIndex != nullptr && info != nullptr);
		JBindDesc::cDesc.allowMipmapBind = Bind::CanBindMipmap(info->GetGraphicResourceType());
	}

	JBindDetailDesc::JBindDetailDesc(const JBindDesc& resourceBindDesc)
		: resourceBindDesc(resourceBindDesc)
	{
		ClearAllowTrigger();
	}
	void JBindDetailDesc::InitDsv(const bool isArray)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = Constants::GetDepthStencilFormat();
		dsvDesc.Texture2D.MipSlice = 0;
		SetDsv(dsvDesc);
	}
	void JBindDetailDesc::InitRtv(const D3D12_RESOURCE_DESC& resourceDesc, const bool isArray)
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
	void JBindDetailDesc::InitSrv(const D3D12_RESOURCE_DESC& resourceDesc, const bool isArray)
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
	void JBindDetailDesc::InitUav(const D3D12_RESOURCE_DESC& resourceDesc, const bool isArray, ID3D12Resource* counterResource)
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
	void JBindDetailDesc::SetDsv(const D3D12_DEPTH_STENCIL_VIEW_DESC& desc)
	{
		dsvDesc = desc;
		allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::DSV] = true;
	}
	void JBindDetailDesc::SetRtv(const D3D12_RENDER_TARGET_VIEW_DESC& desc)
	{
		rtvDesc = desc;
		allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::RTV] = true;
	}
	void JBindDetailDesc::SetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
	{
		srvDesc = desc;
		allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::SRV] = true;
	}
	void JBindDetailDesc::SetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
	{
		uavDesc = desc;
		allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
	}
	void JBindDetailDesc::ClearAllowTrigger()
	{
		memset(allowBindResource, 0, (uint)J_GRAPHIC_BIND_TYPE::COUNT * sizeof(bool));
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> JDx12GraphicResourceCreation::CreateOcclusionQueryResult(ID3D12Device* device, const size_t capa)
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
	void JDx12GraphicResourceCreation::CreateSwapChainBuffer(HWND windowHandle, JDx12SwapChainCreationData& creationData, const JGraphicOption& option)
	{
		bool isNullSwapChain = (*creationData.swapChain) == nullptr;
		if (isNullSwapChain)
		{ 
			DXGI_SWAP_CHAIN_DESC1 sd = {};
			sd.Width = creationData.width;
			sd.Height = creationData.height; 
			sd.Format = Constants::GetBackBufferFormat(option.postProcess.useHdr);			 
			sd.SampleDesc.Count = creationData.m4xMsaaState ? 4 : 1;
			sd.SampleDesc.Quality = creationData.m4xMsaaState ? (creationData.m4xMsaaQuality - 1) : 0;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = Constants::swapChainBufferCount;
			sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = { 0 };
			fsSwapChainDesc.RefreshRate.Numerator = JWindow::GetMaxDisplayFrequency();
			fsSwapChainDesc.RefreshRate.Denominator = 1;
			fsSwapChainDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			fsSwapChainDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

			fsSwapChainDesc.Windowed = TRUE;

			// Note: Swap chain uses queue to perform flush.
			 
			Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
			ThrowIfFailedG(creationData.dxgiFactory->CreateSwapChainForHwnd(creationData.commandQueue,
				windowHandle,
				&sd,
				&fsSwapChainDesc,
				nullptr,
				&swapChain));
			ThrowIfFailedG(swapChain.As(creationData.swapChain));
		}
		else
		{ 
			ThrowIfFailedG((*creationData.swapChain)->ResizeBuffers(
				Constants::swapChainBufferCount,
				creationData.width, creationData.height,
				Constants::GetBackBufferFormat(option.postProcess.useHdr),
				DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
		}
	}
	void JDx12GraphicResourceCreation::CreateUploadBuffer(ID3D12Device* device,
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
	JDx12GraphicResourceHolderDesc JDx12GraphicResourceCreation::CreateBufferAndUploadCpuBuffer(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
	{
		if (creationDesc.uploadBufferDesc == nullptr)
			return JDx12GraphicResourceHolderDesc();

		const void* uploadData = creationDesc.uploadBufferDesc->data;
		const size_t bufferSize = creationDesc.uploadBufferDesc->bufferSize;
		Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;
		CD3DX12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC defaultBufDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
		// Create the actual default buffer resource.
		ThrowIfFailedHr(data.device->CreateCommittedResource(
			&defaultHeap,
			D3D12_HEAP_FLAG_NONE,
			&defaultBufDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

		// In order to copy CPU memory data into our default buffer, we need to create
		// an intermediate upload heap. 
		CreateUploadBuffer(data.device, defaultBuffer.Get(), uploadBuffer.GetAddressOf(), 1);
		/*
		CD3DX12_HEAP_PROPERTIES uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		ThrowIfFailedHr(device->CreateCommittedResource(
			&uploadHeap,
			D3D12_HEAP_FLAG_NONE,
			&defaultBufDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf())));

		*/
		// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
		// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
		// the intermediate upload heap data will be copied to mBuffer.
		UploadData(data.commandList, defaultBuffer.Get(), uploadBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ, uploadData, bufferSize, 1);
		// Note: uploadBuffer has to be kept alive after the above function calls because
		// the command list has not been executed yet that performs the actual copy.
		// The caller can Release the uploadBuffer after it knows the copy has been executed.

		uint elementSize = 0;
		if (creationDesc.formatHint == nullptr)
			elementSize = bufferSize / creationDesc.width;
		else
			elementSize = creationDesc.formatHint->elementSize;
		return JDx12GraphicResourceHolderDesc(std::move(defaultBuffer), D3D12_RESOURCE_STATE_COMMON, elementSize);
	}
	JDx12GraphicResourceHolderDesc JDx12GraphicResourceCreation::Create(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE type)
	{
		return (Resource::GetCreateD3dResourcePtr(type))(data, creationDesc);
	}
	JDx12GraphicResourceHolderDesc JDx12GraphicResourceCreation::Create(const JDeviceData& data, ID3D12Resource* dxInfoResource, const J_GRAPHIC_RESOURCE_OPTION_TYPE type)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> newResource = nullptr;
		D3D12_HEAP_PROPERTIES heapProperties;
		D3D12_HEAP_FLAGS heapFlag;

		auto resourceDesc = dxInfoResource->GetDesc();
		auto clearFormat = resourceDesc.Format;

		dxInfoResource->GetHeapProperties(&heapProperties, &heapFlag);
		Option::GraphicOptionProperty(type, resourceDesc, heapProperties, heapFlag, clearFormat);

		D3D12_CLEAR_VALUE* optClearPtr = nullptr;
		CD3DX12_CLEAR_VALUE optClear(clearFormat, Option::GraphicOptionClearColor(type));
		if (Option::UseClearColor(type))
			optClearPtr = &optClear;

		D3D12_RESOURCE_STATES initState = Option::GraphicOptionInitState(type);
		ThrowIfFailedHr(data.device->CreateCommittedResource(
			&heapProperties,
			heapFlag,
			&resourceDesc,
			initState,
			optClearPtr,
			IID_PPV_ARGS(&newResource)));

		const uint fixedElementSize = GetFixedBufferElementSize(type);
		if (resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && fixedElementSize > 0)
			return JDx12GraphicResourceHolderDesc(std::move(newResource), initState, fixedElementSize);
		else
			return JDx12GraphicResourceHolderDesc(std::move(newResource), initState);
	}
	bool JDx12GraphicResourceCreation::Load(const JDeviceData& data,
		const JGraphicResourceCreationDesc& creationDesc,
		const J_GRAPHIC_RESOURCE_TYPE type,
		Microsoft::WRL::ComPtr<ID3D12Resource>& newResource,
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer,
		std::unique_ptr<DirectX::ResourceUploadBatch>& uploadBatch, 
		const bool allowWIC )
	{
		HRESULT res = (HRESULT)-1;
		D3D12_RESOURCE_FLAGS flag = D3D12_RESOURCE_FLAG_NONE;
		if (type == J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON && creationDesc.bindDesc.HasRequestAdditionalBind(J_GRAPHIC_BIND_TYPE::UAV))
			flag |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

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
		else if(allowWIC)
		{
			uploadBatch = std::make_unique<DirectX::ResourceUploadBatch>(data.device);
			uploadBatch->Begin();
			//DirectX::WIC_LOADER_FLAGS wFlag = DirectX::WIC_LOADER_FORCE_RGBA32;
			DirectX::WIC_LOADER_FLAGS wFlag = DirectX::WIC_LOADER_DEFAULT;
			if (creationDesc.textureDesc->UseMipmap())
				wFlag |= DirectX::WIC_LOADER_MIP_AUTOGEN;
			res = DirectX::CreateWICTextureFromFileEx(data.device, *uploadBatch, creationDesc.textureDesc->path.c_str(), creationDesc.textureDesc->maxSize, flag, wFlag, newResource.GetAddressOf());
			uploadBatch->End(data.commandQueue);
		}
		return res == S_OK;
	} 
	bool JDx12GraphicResourceCreation::Bind(const JBindDesc& bDesc)
	{
		if (!bDesc.IsValid())
			return false;

		auto ptr = Bind::GetResourceBindViewPtr(bDesc.info->GetGraphicResourceType());
		if (ptr == nullptr)
			return false;

		(ptr)(bDesc);
		return true;
	}
	bool JDx12GraphicResourceCreation::Bind(const JBindDetailDesc& opDesc, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
	{
		if (!opDesc.resourceBindDesc.IsValid())
			return false;

		auto ptr = Bind::GetResourceBindOptionViewPtr(opType);
		if (ptr == nullptr)
			return false;

		(ptr)(opDesc);
		return true;
	}
	bool JDx12GraphicResourceCreation::CanBind(const J_GRAPHIC_RESOURCE_TYPE type)
	{
		return Bind::GetResourceBindViewPtr(type) != nullptr;
	}
	void JDx12GraphicResourceCreation::UploadData(ID3D12GraphicsCommandList* cmdList,
		ID3D12Resource* resource,
		ID3D12Resource* uploadBuffer,
		const D3D12_RESOURCE_STATES beforeState,
		const D3D12_RESOURCE_STATES afterState,
		const void* data,
		size_t byteSize,
		const uint subResourceCount)
	{
		D3D12_SUBRESOURCE_DATA clearData = {};
		clearData.pData = data;
		clearData.RowPitch = byteSize;
		clearData.SlicePitch = clearData.RowPitch;

		JDx12Utility::ResourceTransition(cmdList, resource, beforeState, D3D12_RESOURCE_STATE_COPY_DEST);
		UpdateSubresources<1>(cmdList, resource, uploadBuffer, 0, 0, subResourceCount, &clearData);
		JDx12Utility::ResourceTransition(cmdList, resource, D3D12_RESOURCE_STATE_COPY_DEST, afterState);
	}
}