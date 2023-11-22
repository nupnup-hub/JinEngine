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
			LAYER_DEPTH_MAP_DEBUG,
			OCCLUSION_DEPTH_MAP,
			OCCLUSION_DEPTH_MIP_MAP,
			OCCLUSION_DEPTH_MAP_DEBUG, 
			TEXTURE_2D,
			TEXTURE_CUBE,
			RENDER_RESULT_COMMON, 
			SHADOW_MAP,
			SHADOW_MAP_ARRAY,
			SHADOW_MAP_CUBE,
			VERTEX,
			INDEX)

		//Option에 Bind View는 EnumClass에 선언된 순서로 정렬된다
		REGISTER_ENUM_CLASS(J_GRAPHIC_RESOURCE_OPTION_TYPE, int, POST_PROCESSING, NORMAL_MAP, AMBIENT_OCCLISION_MAP)		
		REGISTER_ENUM_CLASS(J_GRAPHIC_MIP_MAP_TYPE, int, NONE,
			GRAPHIC_API_DEFAULT,
			GAUSSIAN,
			BOX,
			KAISER)

		struct JMipmapGenerateDesc
		{ 
		public:
			J_GRAPHIC_MIP_MAP_TYPE type = J_GRAPHIC_MIP_MAP_TYPE::GRAPHIC_API_DEFAULT;
			J_KENEL_SIZE kernelSize = J_KENEL_SIZE::_3x3;
			float sharpnessFactor = 1;
		public:
			bool operator==(const JMipmapGenerateDesc& rhs)const noexcept
			{
				return type == rhs.type && kernelSize == rhs.kernelSize && sharpnessFactor == rhs.sharpnessFactor;
			}
		};
		struct JTextureCreateDesc
		{
		public:
			std::wstring path;
			std::wstring oriFormat;
			uint maxSize = 0;
			bool useMipmap = true;		//texture 생성전에 JMipmapGenerateDesc상태에 따라 값이 결정됨.
			bool allowUav = false;
		public:
			JMipmapGenerateDesc mipMapDesc;
		public:
			JTextureCreateDesc(const std::wstring& path, const std::wstring& oriFormat, const uint maxSize)
				:path(path), oriFormat(oriFormat), maxSize(maxSize)
			{}
			JTextureCreateDesc(const std::wstring& path, const std::wstring& oriFormat, const uint maxSize, const JMipmapGenerateDesc& mipMapDesc)
				:path(path), oriFormat(oriFormat), maxSize(maxSize), mipMapDesc(mipMapDesc)
			{}
		};
		class JGraphicResourceType
		{
		public:
			static bool CanUseOption(const J_GRAPHIC_RESOURCE_OPTION_TYPE optionType, const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				switch (optionType)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::POST_PROCESSING:
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
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP:
				{
					switch (rType)
					{
					case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
						return true;
					default:
						return false;
					}
				}
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::AMBIENT_OCCLISION_MAP:
				{
					switch (rType)
					{
					case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
						return true;
					default:
						return false;
					}
				}
				default:
					return false;
				}
			}	 
		};
	}
}