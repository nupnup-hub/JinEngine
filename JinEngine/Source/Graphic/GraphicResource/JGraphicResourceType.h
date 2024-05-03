#pragma once 
#include"JGraphicResourceConstants.h" 
#include"../JGraphicEnum.h"
#include"../Image/JImageProcessingEnum.h"
#include"../../Core/Reflection/JReflection.h"

namespace JinEngine
{
	namespace Graphic
	{
		REGISTER_ENUM_CLASS(J_GRAPHIC_BIND_TYPE, int, RTV = 0,
			DSV,
			SRV,
			UAV)

		//중간에 타입추가할시
		//엔진 텍스쳐 메타데이터 초기화 필요 
		//메타데이터 유효성 검사필요
		//COMMON 접미사는 옵션으로 resource desc를 설정할수있을시에 추가
		//그이외에는 Fixed option을 지향 
		REGISTER_ENUM_CLASS(J_GRAPHIC_RESOURCE_TYPE, int, SWAP_CHAN = 0,
			SCENE_LAYER_DEPTH_STENCIL, 
			DEBUG_LAYER_DEPTH_STENCIL,
			DEBUG_MAP,
			OCCLUSION_DEPTH_MAP,
			OCCLUSION_DEPTH_MIP_MAP,
			OCCLUSION_DEPTH_MAP_DEBUG, 
			TEXTURE_2D,								
			TEXTURE_CUBE,
			TEXTURE_COMMON,
			RENDER_RESULT_COMMON, 
			RENDER_RESULT_LIGHT_CULLING,
			SHADOW_MAP,
			SHADOW_MAP_ARRAY,
			SHADOW_MAP_CUBE,
			LIGHT_LINKED_LIST,
			LIGHT_OFFSET,  
			SSAO_MAP,
			SSAO_INTERMEDIATE_MAP,
			SSAO_INTERLEAVE_MAP,
			SSAO_DEPTH_MAP,
			SSAO_DEPTH_INTERLEAVE_MAP, 
			IMAGE_PROCESSING,
			POST_PROCESS_EXPOSURE,
			POST_PROCESS_LUMA,
			POST_PROCESS_FXAA_COLOR_QUEUE,
			RESTIR_INITIAL_SAMPLE,
			RESTIR_RESERVOIR,				 
			BYTE_BUFFER_COMMON,					
			STRUCTURE_BUFFER_COMMON,
			VERTEX,
			INDEX)

		//Option에 Bind View는 EnumClass에 선언된 순서로 정렬되며
		//구현부에서 순서에 종속성을 가질수도있다.(ex gbuffer)
		REGISTER_ENUM_CLASS(J_GRAPHIC_RESOURCE_OPTION_TYPE, int, ALBEDO_MAP,   
			LIGHTING_PROPERTY,
			NORMAL_MAP,  
			VELOCITY,
			BLUR,  
			COUNTER_BUFFER)

		REGISTER_ENUM_CLASS(J_GRAPHIC_MIP_MAP_TYPE, int, NONE,
			GRAPHIC_API_DEFAULT,
			GAUSSIAN,
			BOX,
			KAISER)

		enum class J_GRAPHIC_RESOURCE_FORMAT
		{
			API_SPECIALIZED = 0, 
			R32G32B32A32_FLOAT,
			R32G32B32A32_UINT,
			R32G32B32A32_SINT,
			R32G32B32_FLOAT ,
			R32G32B32_UINT ,
			R32G32B32_SINT,
			R16G16B16A16_FLOAT,
			R16G16B16A16_UNORM ,
			R16G16B16A16_UINT,
			R16G16B16A16_SNORM,
			R16G16B16A16_SINT,
			R32G32_FLOAT,
			R32G32_UINT,
			R32G32_SINT,  
			R10G10B10A2_UNORM,
			R10G10B10A2_UINT,
			R11G11B10_FLOAT, 
			R8G8B8A8_UNORM,
			R8G8B8A8_UNORM_SRGB,
			R8G8B8A8_UINT,
			R8G8B8A8_SNORM,
			R8G8B8A8_SINT, 
			R16G16_FLOAT,
			R16G16_UNORM,
			R16G16_UINT,
			R16G16_SNORM,
			R16G16_SINT, 
			D32_FLOAT,
			R32_FLOAT,
			R32_UINT,
			R32_SINT,
			D24_S8,  
			R8G8_UNORM,
			R8G8_UINT,
			R8G8_SNORM,
			R8G8_SINT, 
			R16_FLOAT,
			D16_UNORM,
			R16_UNORM,
			R16_UINT,
			R16_SNORM,
			R16_SINT,
			R8_UNORM,
			R8_UINT,
			R8_SNORM,
			R8_SINT
		};

		struct JMipmapGenerationDesc
		{
		public:
			J_GRAPHIC_MIP_MAP_TYPE type = J_GRAPHIC_MIP_MAP_TYPE::GRAPHIC_API_DEFAULT;
			J_KERNEL_SIZE kernelSize = J_KERNEL_SIZE::_3x3;
			float sharpnessFactor = 1;
			//mipmap 갯수 조절 기능 추가필요.
		public:
			bool operator==(const JMipmapGenerationDesc& rhs)const noexcept
			{
				return type == rhs.type && kernelSize == rhs.kernelSize && sharpnessFactor == rhs.sharpnessFactor;
			}
		};
		struct JTextureCreationDesc
		{
		public:
			enum class CREATION_TYPE
			{
				LOAD,
				CREATE,
			};
		public:
			std::wstring path;
			std::wstring oriFormat;
			uint maxSize = 0;
		public:
			JMipmapGenerationDesc mipMapDesc;
			CREATION_TYPE creationType;
		public:
			JTextureCreationDesc()
				:creationType(CREATION_TYPE::CREATE)
			{}
			JTextureCreationDesc(const std::wstring& path, const std::wstring& oriFormat, const uint maxSize)
				:path(path), oriFormat(oriFormat), maxSize(maxSize), creationType(CREATION_TYPE::LOAD)
			{}
			JTextureCreationDesc(const std::wstring& path, const std::wstring& oriFormat, const uint maxSize, const JMipmapGenerationDesc& mipMapDesc)
				:path(path), oriFormat(oriFormat), maxSize(maxSize), mipMapDesc(mipMapDesc), creationType(CREATION_TYPE::LOAD)
			{}
		public:
			bool UseMipmap()const noexcept
			{
				return mipMapDesc.type != J_GRAPHIC_MIP_MAP_TYPE::NONE;
			}
		};
		struct JOcclusionDebugCreationDesc
		{
		public:
			bool isHzb = false;
		};
		//for upload buffer
		struct JUploadBufferCreationDesc
		{
		public:
			const void* data = nullptr;
			size_t bufferSize = 0;
			//size_t bufferElementSize = 0;
		public:
			bool useEngineDefine = false;
		public:
			JUploadBufferCreationDesc()
			{}
			JUploadBufferCreationDesc(const void* data, size_t bufferSize)
				:data(data), bufferSize(bufferSize)
			{}
		};
		struct JGraphicResourceBindDesc
		{
		public:
			bool useEngineDefinedBindType = true;										//api 구현부에 binding 책임 전가.
			bool requestAdditionalBind[(uint)J_GRAPHIC_BIND_TYPE::COUNT];
		public:
			bool allowMipmapBind = false;
		public:
			JGraphicResourceBindDesc()
			{
				memset(requestAdditionalBind, false, (size_t)J_GRAPHIC_BIND_TYPE::COUNT * sizeof(bool));
			}
			JGraphicResourceBindDesc(const JGraphicResourceBindDesc& rhs)
				:useEngineDefinedBindType(rhs.useEngineDefinedBindType),
				allowMipmapBind(rhs.allowMipmapBind)
			{
				for (int i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
					requestAdditionalBind[i] = rhs.requestAdditionalBind[i];
			}
			JGraphicResourceBindDesc& operator=(const JGraphicResourceBindDesc& rhs)
			{
				useEngineDefinedBindType = rhs.useEngineDefinedBindType;
				allowMipmapBind = rhs.allowMipmapBind;
				for (int i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
					requestAdditionalBind[i] = rhs.requestAdditionalBind[i];
			}
		public:
			void SetRequestAdditionalBind(const J_GRAPHIC_BIND_TYPE bType) noexcept
			{
				requestAdditionalBind[(uint)bType] = true;
				useEngineDefinedBindType = false;
			}
		public:
			bool HasRequestAdditionalBind(const J_GRAPHIC_BIND_TYPE bType)const noexcept
			{
				return !useEngineDefinedBindType && requestAdditionalBind[(uint)bType];
			}
		};
		struct JGraphicFormatHint
		{  
		public:
			J_GRAPHIC_RESOURCE_FORMAT format;
		public:  
			size_t elementSize = 0;  
		public:
			bool isUnsigned = false;
			bool isNormalized = false;   
		};
		struct JGraphicResourceCreationDesc
		{ 
		public:
			uint width = 1;
			uint height = 1;
			uint arraySize = 1;   
		public:
			bool useEngineDefine = false;										//resource dimension에 대한 책임을 api구현부에 전가 구현부에서 허용가능한 type만 유효한 값으로 취급					 
		public:
			JGraphicResourceBindDesc bindDesc;								
		public:
			std::unique_ptr<JTextureCreationDesc> textureDesc;					//texture2d, texture cube 
			std::unique_ptr<JUploadBufferCreationDesc> uploadBufferDesc;
		public:
			std::unique_ptr<JGraphicFormatHint> formatHint;						//can use if !IsFixedFormat()
		public:
			JGraphicResourceCreationDesc(uint width = 1, uint height = 1, uint arraySize = 1)
				:width(width), height(height), arraySize(arraySize)
			{}
			JGraphicResourceCreationDesc(const JVector2<uint>& size)
				:width(size.x), height(size.y)
			{}
			JGraphicResourceCreationDesc(const JVector2F& size)
				:width((uint)size.x), height((uint)size.y)
			{} 
			JGraphicResourceCreationDesc(std::unique_ptr<JTextureCreationDesc>&& desc)
				:textureDesc(std::move(desc))
			{} 
		public:
			bool IsValid()const noexcept
			{  
				return width > 0 && height > 0 && arraySize > 0;
			}
		};
		class JGraphicResourceTypeAttribute
		{
		public:
			static bool CanUseOption(const J_GRAPHIC_RESOURCE_OPTION_TYPE optionType, const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				switch (optionType)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP: 
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP: 
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY:
				{
					switch (rType)
					{
					case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
						return true;
					default:
						return false;
					}
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR:
				{
					switch (rType)
					{
					case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
						return true;
					case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
						return true;
					case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY:
						return true;
					case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE:
						return true;
					default:
						return false;
					}
				} 
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER:
				{
					switch (rType)
					{
					case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
						return true;
					default:
						return false;
					}
				};
				default:
					return false;
				}
			}	 
			/**
			* @brief create graphic resource and swap exist JGraphicResourceInfo graphic resource
			* 주로 user ptr을 소유하는 객체가 아닌 다른 요소에 의해서 graphic resource만 재할당이 필요한경우 사용된다
			* ex) graphic option에 cluster option이 변경될경우 light cluster관련 resource 전부 재할당.
			*/
			static bool CanReAlloc(const J_GRAPHIC_RESOURCE_TYPE type)
			{
				switch (type)
				{   
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET:
					return true;
				default:
					return false;
				}
			}
		};
	}
}