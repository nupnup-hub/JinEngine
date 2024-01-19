#pragma once 
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
		REGISTER_ENUM_CLASS(J_GRAPHIC_RESOURCE_TYPE, int, SWAP_CHAN = 0,
			SCENE_LAYER_DEPTH_STENCIL, 
			DEBUG_LAYER_DEPTH_STENCIL,
			DEBUG_MAP,
			OCCLUSION_DEPTH_MAP,
			OCCLUSION_DEPTH_MIP_MAP,
			OCCLUSION_DEPTH_MAP_DEBUG, 
			TEXTURE_2D,
			TEXTURE_CUBE,
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
			VERTEX,
			INDEX) 

		//Option에 Bind View는 EnumClass에 선언된 순서로 정렬되며
		//구현부에서 순서에 종속성을 가질수도있다.(ex gbuffer)
		REGISTER_ENUM_CLASS(J_GRAPHIC_RESOURCE_OPTION_TYPE, int, ALBEDO_MAP, 
			NORMAL_MAP, 
			TANGENT_MAP, 
			LIGHTING_PROPERTY,
			BLUR,  
			COUNTER_BUFFER)

		REGISTER_ENUM_CLASS(J_GRAPHIC_MIP_MAP_TYPE, int, NONE,
			GRAPHIC_API_DEFAULT,
			GAUSSIAN,
			BOX,
			KAISER)

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
				CREATE_VF4
			};
		public:
			std::wstring path;
			std::wstring oriFormat;
			uint maxSize = 0; 
			bool allowUav = false;
		public:
			//custom data 
			std::vector<JVector4F> v4Data;
		public:
			JMipmapGenerationDesc mipMapDesc;
			CREATION_TYPE creationType;
		public:
			JTextureCreationDesc(const std::wstring& path, const std::wstring& oriFormat, const uint maxSize)
				:path(path), oriFormat(oriFormat), maxSize(maxSize), creationType(CREATION_TYPE::LOAD)
			{}
			JTextureCreationDesc(const std::wstring& path, const std::wstring& oriFormat, const uint maxSize, const JMipmapGenerationDesc& mipMapDesc)
				:path(path), oriFormat(oriFormat), maxSize(maxSize), mipMapDesc(mipMapDesc), creationType(CREATION_TYPE::LOAD)
			{}
			JTextureCreationDesc(const std::vector<JVector4F>& data)
				:v4Data(data), creationType(CREATION_TYPE::CREATE_VF4)
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
		struct JBufferCreationDesc
		{
		public:
			void* data = nullptr;
			size_t bufferSize = 0; 
		public:
			JBufferCreationDesc(void* data, const size_t bufferSize)
				:data(data), bufferSize(bufferSize)
			{}
		};
		struct JGraphicResourceBindDesc
		{
		public:
			bool useEngineDefine = true;
			bool requestAdditionalBind[(uint)J_GRAPHIC_BIND_TYPE::COUNT];
		public:
			JGraphicResourceBindDesc() = default;
			JGraphicResourceBindDesc(const JGraphicResourceBindDesc& rhs)
				:useEngineDefine(rhs.useEngineDefine)
			{
				for (int i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
					requestAdditionalBind[i] = rhs.requestAdditionalBind[i];
			}
			JGraphicResourceBindDesc& operator=(const JGraphicResourceBindDesc& rhs)
			{
				useEngineDefine = rhs.useEngineDefine;
				for (int i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
					requestAdditionalBind[i] = rhs.requestAdditionalBind[i];
			}
		public:
			bool HasRequestAdditionalBind(const J_GRAPHIC_BIND_TYPE bType)const noexcept
			{
				return !useEngineDefine && requestAdditionalBind[(uint)bType];
			}
		};
		/*
		struct JGraphicCustomFormatHint
		{ 
		public: 
			uint alternativeFormatNumber = 0;		//0 is engine defined and 1 ... n is defiended by implement class
			bool useEngineDefined = true;	
		};
		*/
		struct JGraphicResourceCreationDesc
		{ 
		public:
			uint width = 1;
			uint height = 1;
			uint arraySize = 1;  
		public:
			JGraphicResourceBindDesc bindDesc;
			//JGraphicCustomFormatHint formatHint;					//can use if !IsFixedFormat()
		public:
			std::unique_ptr<JTextureCreationDesc> textureDesc;		//texture2d, texture cube
			std::unique_ptr<JOcclusionDebugCreationDesc> occDebugDesc;	//occ resource
			std::unique_ptr<JBufferCreationDesc> bufferDesc;
		public:
			JGraphicResourceCreationDesc(uint width = 1, uint height = 1, uint arraySize = 1)
				:width(width), height(height), arraySize(arraySize)
			{}
			JGraphicResourceCreationDesc(const JVector2F& size)
				:width(size.x), height(size.y)
			{} 
			JGraphicResourceCreationDesc(std::unique_ptr<JTextureCreationDesc>&& desc)
				:textureDesc(std::move(desc))
			{}
			JGraphicResourceCreationDesc(std::unique_ptr<JBufferCreationDesc>&& desc)
				:bufferDesc(std::move(desc))
			{  
				width = bufferDesc->bufferSize;
			}
		public:
			bool IsValid()const noexcept
			{  
				return width > 0 && height > 0 && arraySize > 0;
			}
		};
		class JGraphicResourceType
		{
		public:
			static bool IsFixedBind(const J_GRAPHIC_RESOURCE_TYPE type)
			{
				switch (type)
				{
					//has optional uav for IBL...
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
					return false;
				default:
					return true;
				}
			}
			static bool IsFixedFormat(const J_GRAPHIC_RESOURCE_TYPE type)
			{
				switch (type)
				{			 
				default:
					return true;
				}
			}
			static bool CanUseOption(const J_GRAPHIC_RESOURCE_OPTION_TYPE optionType, const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				switch (optionType)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::TANGENT_MAP:
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY:
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